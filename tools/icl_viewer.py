#!/usr/bin/env python3
"""
ICL Viewer - Browse Windows Icon Library (.icl) files on macOS/Linux.

Self-bootstrapping: creates a venv and installs dependencies on first run.

Usage:
    python3 icl_viewer.py [optional_path_to.icl]
"""

import sys
import os
import subprocess
import struct
import io
from pathlib import Path

# ---------------------------------------------------------------------------
# Self-bootstrap: ensure we're running inside a venv with deps installed
# ---------------------------------------------------------------------------

VENV_DIR = Path(__file__).resolve().parent / ".icl_viewer_venv"
REQUIRED_PACKAGES = ["pefile", "Pillow", "PyQt6"]


def bootstrap():
    """Create venv and install deps if needed, then re-exec inside the venv."""
    venv_python = VENV_DIR / "bin" / "python3"

    if not venv_python.exists():
        print("First run — setting up virtual environment...")
        subprocess.check_call([sys.executable, "-m", "venv", str(VENV_DIR)])
        print("Installing dependencies (pefile, Pillow, PyQt6)...")
        subprocess.check_call([
            str(venv_python), "-m", "pip", "install", "--quiet", *REQUIRED_PACKAGES
        ])
        print("Setup complete!\n")

    # Re-exec under the venv python
    os.execv(str(venv_python), [str(venv_python), __file__] + sys.argv[1:])


# If we're not in the venv yet, bootstrap and re-exec
if not sys.prefix.startswith(str(VENV_DIR)):
    bootstrap()

# ---------------------------------------------------------------------------
# Now we're running inside the venv — safe to import everything
# ---------------------------------------------------------------------------

import pefile
from PIL import Image
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QGridLayout, QScrollArea, QLabel, QPushButton, QFileDialog,
    QMessageBox, QSplitter, QFrame, QSizePolicy,
)
from PyQt6.QtGui import QPixmap, QImage, QIcon, QPalette, QColor, QFont
from PyQt6.QtCore import Qt, QSize, QByteArray, QBuffer


# ---------------------------------------------------------------------------
# ICO / icon-resource parsing
# ---------------------------------------------------------------------------

def parse_grp_icon_dir(data: bytes):
    if len(data) < 6:
        return []
    _reserved, _type, count = struct.unpack_from("<HHH", data, 0)
    entries = []
    offset = 6
    for _ in range(count):
        if offset + 14 > len(data):
            break
        (width, height, color_count, _reserved,
         planes, bit_count, bytes_in_res, icon_id) = struct.unpack_from(
            "<BBBBHHIH", data, offset
        )
        entries.append({
            "width": width or 256,
            "height": height or 256,
            "color_count": color_count,
            "planes": planes,
            "bit_count": bit_count,
            "bytes_in_res": bytes_in_res,
            "icon_id": icon_id,
        })
        offset += 14
    return entries


def icon_resource_to_image(data: bytes):
    """Convert RT_ICON resource blob to PIL Image (handles PNG and BMP DIB)."""
    if data[:8] == b"\x89PNG\r\n\x1a\n":
        return Image.open(io.BytesIO(data)).convert("RGBA")

    if len(data) < 40:
        return None

    header_size = struct.unpack_from("<I", data, 0)[0]
    bmp_width = struct.unpack_from("<i", data, 4)[0]
    bmp_height = struct.unpack_from("<i", data, 8)[0]
    bit_count = struct.unpack_from("<H", data, 14)[0]

    actual_height = abs(bmp_height) // 2
    patched_dib = data[:8] + struct.pack("<i", actual_height) + data[12:]

    colors_used = struct.unpack_from("<I", data, 32)[0]
    if colors_used == 0 and bit_count <= 8:
        colors_used = 1 << bit_count
    palette_size = colors_used * 4
    pixel_offset = 14 + header_size + palette_size

    bmp_header = struct.pack("<2sIHHI", b"BM", 14 + len(patched_dib), 0, 0, pixel_offset)
    bmp_data = bmp_header + patched_dib

    try:
        img = Image.open(io.BytesIO(bmp_data)).convert("RGBA")

        xor_row_size = ((bmp_width * bit_count + 31) // 32) * 4
        xor_size = xor_row_size * actual_height
        mask_row_size = ((bmp_width + 31) // 32) * 4
        mask_offset = header_size + palette_size + xor_size
        mask_size = mask_row_size * actual_height

        if bit_count < 32 and mask_offset + mask_size <= len(data):
            mask_data = data[mask_offset: mask_offset + mask_size]
            mask_img = Image.new("L", (bmp_width, actual_height), 255)
            px = mask_img.load()
            for y in range(actual_height):
                row_start = (actual_height - 1 - y) * mask_row_size
                row = mask_data[row_start: row_start + mask_row_size]
                for x in range(bmp_width):
                    byte_idx = x // 8
                    bit_idx = 7 - (x % 8)
                    if byte_idx < len(row) and (row[byte_idx] >> bit_idx) & 1:
                        px[x, y] = 0
            img.putalpha(mask_img)

        return img
    except Exception:
        return None


def load_icons_from_pe(filepath: str):
    pe = pefile.PE(filepath)

    icon_data_by_id = {}
    if hasattr(pe, "DIRECTORY_ENTRY_RESOURCE"):
        for res_type in pe.DIRECTORY_ENTRY_RESOURCE.entries:
            if res_type.id == 3 and hasattr(res_type, "directory"):
                for icon_entry in res_type.directory.entries:
                    if hasattr(icon_entry, "directory"):
                        for lang in icon_entry.directory.entries:
                            rva = lang.data.struct.OffsetToData
                            size = lang.data.struct.Size
                            icon_data_by_id[icon_entry.id] = pe.get_data(rva, size)

    groups = []
    if hasattr(pe, "DIRECTORY_ENTRY_RESOURCE"):
        for res_type in pe.DIRECTORY_ENTRY_RESOURCE.entries:
            if res_type.id == 14 and hasattr(res_type, "directory"):
                for group_entry in res_type.directory.entries:
                    if hasattr(group_entry, "directory"):
                        for lang in group_entry.directory.entries:
                            rva = lang.data.struct.OffsetToData
                            size = lang.data.struct.Size
                            grp_data = pe.get_data(rva, size)
                            entries = parse_grp_icon_dir(grp_data)
                            images = []
                            for e in entries:
                                raw = icon_data_by_id.get(e["icon_id"])
                                if raw:
                                    img = icon_resource_to_image(raw)
                                    if img:
                                        images.append({"entry": e, "image": img})
                            groups.append({
                                "group_id": group_entry.id,
                                "group_name": str(group_entry.name) if group_entry.name else None,
                                "images": images,
                            })
    pe.close()
    return groups


# ---------------------------------------------------------------------------
# Qt helpers
# ---------------------------------------------------------------------------

def pil_to_qpixmap(pil_img: Image.Image) -> QPixmap:
    """Convert PIL RGBA image to QPixmap."""
    data = pil_img.tobytes("raw", "RGBA")
    qimg = QImage(data, pil_img.width, pil_img.height, 4 * pil_img.width, QImage.Format.Format_RGBA8888)
    return QPixmap.fromImage(qimg)


CHECKERBOARD_CSS = """
    background-color: #2a2a3c;
    background-image:
        linear-gradient(45deg, #353548 25%, transparent 25%),
        linear-gradient(-45deg, #353548 25%, transparent 25%),
        linear-gradient(45deg, transparent 75%, #353548 75%),
        linear-gradient(-45deg, transparent 75%, #353548 75%);
    background-size: 16px 16px;
    background-position: 0 0, 0 8px, 8px -8px, -8px 0px;
"""


# ---------------------------------------------------------------------------
# Widgets
# ---------------------------------------------------------------------------

class IconThumbnail(QFrame):
    """Clickable icon card for the grid."""

    def __init__(self, group, pixmap, on_click):
        super().__init__()
        self.group = group
        self._on_click = on_click
        self._selected = False
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        self.setFixedSize(110, 120)
        self._update_style()

        layout = QVBoxLayout(self)
        layout.setContentsMargins(6, 6, 6, 6)
        layout.setSpacing(2)

        img_label = QLabel()
        img_label.setPixmap(pixmap.scaled(
            QSize(64, 64), Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation
        ))
        img_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        img_label.setStyleSheet(CHECKERBOARD_CSS + "border-radius: 4px;")
        img_label.setFixedSize(68, 68)
        layout.addWidget(img_label, alignment=Qt.AlignmentFlag.AlignCenter)

        best = group["images"][0]["entry"] if group["images"] else None
        size_text = f'{best["width"]}x{best["height"]}' if best else "?"
        count = len(group["images"])

        # Show resource name if available, otherwise show ID
        name = group.get("group_name")
        if name:
            display_name = name if len(name) <= 14 else name[:12] + "…"
        else:
            display_name = f'#{group["group_id"]}'

        text_label = QLabel(f'{display_name}\n{size_text}  ({count})')
        text_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        text_label.setStyleSheet("color: #cdd6f4; font-size: 10px; background: transparent;")
        # Tooltip shows full name and ID
        tooltip = f'{name or "unnamed"}  (ID: {group["group_id"]})'
        text_label.setToolTip(tooltip)
        layout.addWidget(text_label)

    def mousePressEvent(self, event):
        self._on_click(self)

    def set_selected(self, selected):
        self._selected = selected
        self._update_style()

    def _update_style(self):
        bg = "#585b70" if self._selected else "#313244"
        self.setStyleSheet(f"""
            IconThumbnail {{
                background: {bg}; border-radius: 8px;
            }}
            IconThumbnail:hover {{
                background: {"#585b70" if self._selected else "#45475a"};
            }}
        """)


class DetailPanel(QScrollArea):
    """Right panel showing all size variants of the selected icon."""

    def __init__(self):
        super().__init__()
        self.setWidgetResizable(True)
        self.setStyleSheet("QScrollArea { border: none; background: #1e1e2e; }")

        self._container = QWidget()
        self._container.setStyleSheet("background: #1e1e2e;")
        self._layout = QVBoxLayout(self._container)
        self._layout.setAlignment(Qt.AlignmentFlag.AlignTop)
        self.setWidget(self._container)

        self._title = QLabel("Select an icon")
        self._title.setStyleSheet("color: #89b4fa; font-size: 16px; font-weight: bold;")
        self._layout.addWidget(self._title)

        self._pil_images = []  # keep refs for export

    def show_group(self, group, export_callback):
        # Clear old rows (keep title)
        while self._layout.count() > 1:
            item = self._layout.takeAt(1)
            if item.widget():
                item.widget().deleteLater()

        self._pil_images.clear()

        name = group.get("group_name") or f'Icon #{group["group_id"]}'
        self._title.setText(f'{name}  —  {len(group["images"])} variant(s)')

        for item in sorted(group["images"], key=lambda i: -i["entry"]["width"]):
            entry = item["entry"]
            pil_img = item["image"]
            self._pil_images.append(pil_img)

            row = QFrame()
            row.setStyleSheet("QFrame { background: #313244; border-radius: 8px; }")
            row_layout = QHBoxLayout(row)
            row_layout.setContentsMargins(12, 8, 12, 8)

            # Icon preview (up to 128px display, checkerboard bg)
            display_size = min(entry["width"], 128)
            pixmap = pil_to_qpixmap(pil_img)
            img_label = QLabel()
            img_label.setPixmap(pixmap.scaled(
                QSize(display_size, display_size),
                Qt.AspectRatioMode.KeepAspectRatio,
                Qt.TransformationMode.SmoothTransformation,
            ))
            img_label.setFixedSize(display_size + 8, display_size + 8)
            img_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            img_label.setStyleSheet(CHECKERBOARD_CSS + "border-radius: 4px;")
            row_layout.addWidget(img_label)

            # Info
            info = QVBoxLayout()
            info.setSpacing(2)

            bits = entry["bit_count"]
            depth = f"{bits}-bit" if bits else "?"
            dim_label = QLabel(f'{entry["width"]} \u00d7 {entry["height"]}  |  {depth}')
            dim_label.setStyleSheet("color: #cdd6f4; font-size: 13px; background: transparent;")
            info.addWidget(dim_label)

            meta_label = QLabel(f'{entry["bytes_in_res"]:,} bytes  |  resource {entry["icon_id"]}')
            meta_label.setStyleSheet("color: #a6adc8; font-size: 11px; background: transparent;")
            info.addWidget(meta_label)

            btn = QPushButton("Export PNG")
            btn.setCursor(Qt.CursorShape.PointingHandCursor)
            btn.setFixedWidth(100)
            btn.setStyleSheet("""
                QPushButton {
                    background: #89b4fa; color: #1e1e2e; border: none;
                    border-radius: 4px; padding: 4px 8px; font-weight: bold; font-size: 11px;
                }
                QPushButton:hover { background: #b4d0fb; }
            """)
            btn.clicked.connect(lambda _, g=group, i=item: export_callback(g, i))
            info.addWidget(btn, alignment=Qt.AlignmentFlag.AlignLeft)

            row_layout.addLayout(info, stretch=1)
            self._layout.addWidget(row)


class ICLViewer(QMainWindow):
    def __init__(self, initial_path=None):
        super().__init__()
        self.setWindowTitle("ICL Viewer")
        self.resize(1100, 720)
        self.setMinimumSize(800, 480)
        self.setStyleSheet("QMainWindow { background: #1e1e2e; }")

        self._groups = []
        self._cards = []
        self._selected_card = None

        self._build_ui()

        if initial_path and os.path.isfile(initial_path):
            self._open_file(initial_path)

    def _build_ui(self):
        central = QWidget()
        self.setCentralWidget(central)
        root_layout = QVBoxLayout(central)
        root_layout.setContentsMargins(0, 0, 0, 0)
        root_layout.setSpacing(0)

        # Toolbar
        toolbar = QFrame()
        toolbar.setFixedHeight(48)
        toolbar.setStyleSheet("QFrame { background: #181825; }")
        tb_layout = QHBoxLayout(toolbar)
        tb_layout.setContentsMargins(10, 0, 10, 0)

        btn_open = QPushButton("Open ICL...")
        btn_open.setCursor(Qt.CursorShape.PointingHandCursor)
        btn_open.setStyleSheet("""
            QPushButton {
                background: #89b4fa; color: #1e1e2e; border: none;
                border-radius: 5px; padding: 6px 16px; font-weight: bold; font-size: 13px;
            }
            QPushButton:hover { background: #b4d0fb; }
        """)
        btn_open.clicked.connect(self._on_open)
        tb_layout.addWidget(btn_open)

        btn_export = QPushButton("Export All PNGs...")
        btn_export.setCursor(Qt.CursorShape.PointingHandCursor)
        btn_export.setStyleSheet("""
            QPushButton {
                background: #a6e3a1; color: #1e1e2e; border: none;
                border-radius: 5px; padding: 6px 16px; font-weight: bold; font-size: 13px;
            }
            QPushButton:hover { background: #c6f0c4; }
        """)
        btn_export.clicked.connect(self._on_export_all)
        tb_layout.addWidget(btn_export)

        self._file_label = QLabel("No file loaded")
        self._file_label.setStyleSheet("color: #a6adc8; font-size: 12px; padding-left: 12px;")
        tb_layout.addWidget(self._file_label, stretch=1)

        root_layout.addWidget(toolbar)

        # Splitter: grid | detail
        splitter = QSplitter(Qt.Orientation.Horizontal)
        splitter.setStyleSheet("""
            QSplitter::handle { background: #313244; width: 3px; }
        """)

        # Left: scrollable icon grid
        left_scroll = QScrollArea()
        left_scroll.setWidgetResizable(True)
        left_scroll.setStyleSheet("QScrollArea { border: none; background: #1e1e2e; }")
        self._grid_widget = QWidget()
        self._grid_widget.setStyleSheet("background: #1e1e2e;")
        self._grid_layout = QGridLayout(self._grid_widget)
        self._grid_layout.setAlignment(Qt.AlignmentFlag.AlignTop | Qt.AlignmentFlag.AlignLeft)
        self._grid_layout.setSpacing(8)
        self._grid_layout.setContentsMargins(10, 10, 10, 10)
        left_scroll.setWidget(self._grid_widget)
        splitter.addWidget(left_scroll)

        # Right: detail panel
        self._detail = DetailPanel()
        splitter.addWidget(self._detail)

        splitter.setSizes([550, 500])
        root_layout.addWidget(splitter, stretch=1)

    def _on_open(self):
        path, _ = QFileDialog.getOpenFileName(
            self, "Open Icon Library", "",
            "Icon Libraries (*.icl);;DLL / EXE (*.dll *.exe);;All Files (*)"
        )
        if path:
            self._open_file(path)

    def _open_file(self, path: str):
        try:
            groups = load_icons_from_pe(path)
        except Exception as exc:
            QMessageBox.critical(self, "Error", f"Failed to parse file:\n{exc}")
            return
        if not groups:
            QMessageBox.information(self, "Empty", "No icon resources found in this file.")
            return

        self._groups = groups
        name = Path(path).name
        self._file_label.setText(f"{name}  —  {len(groups)} icon group(s)")
        self.setWindowTitle(f"ICL Viewer — {name}")
        self._populate_grid()

    def _populate_grid(self):
        # Clear grid
        while self._grid_layout.count():
            item = self._grid_layout.takeAt(0)
            if item.widget():
                item.widget().deleteLater()
        self._cards.clear()
        self._selected_card = None

        cols = 5
        for i, group in enumerate(self._groups):
            best_img = None
            best_diff = 99999
            for item in group["images"]:
                diff = abs(item["entry"]["width"] - 64)
                if diff < best_diff:
                    best_diff = diff
                    best_img = item["image"]
            if best_img is None:
                continue

            pixmap = pil_to_qpixmap(best_img)
            card = IconThumbnail(group, pixmap, self._on_card_click)
            self._cards.append(card)
            self._grid_layout.addWidget(card, i // cols, i % cols)

        if self._cards:
            self._on_card_click(self._cards[0])

    def _on_card_click(self, card):
        if self._selected_card:
            self._selected_card.set_selected(False)
        card.set_selected(True)
        self._selected_card = card
        self._detail.show_group(card.group, self._export_single)

    def _export_single(self, group, item):
        entry = item["entry"]
        name = group.get("group_name") or f'icon_{group["group_id"]}'
        default = f'{name}_{entry["width"]}x{entry["height"]}.png'
        path, _ = QFileDialog.getSaveFileName(
            self, "Export Icon", default, "PNG Image (*.png);;ICO File (*.ico)"
        )
        if path:
            item["image"].save(path)

    def _on_export_all(self):
        if not self._groups:
            return
        folder = QFileDialog.getExistingDirectory(self, "Choose export folder")
        if not folder:
            return
        count = 0
        for group in self._groups:
            for item in group["images"]:
                e = item["entry"]
                base_name = group.get("group_name") or f'icon_{group["group_id"]}'
                filename = f'{base_name}_{e["width"]}x{e["height"]}.png'
                item["image"].save(os.path.join(folder, filename))
                count += 1
        QMessageBox.information(self, "Done", f"Exported {count} icon(s) to:\n{folder}")


# ---------------------------------------------------------------------------
# Entry
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    # Dark palette
    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor("#1e1e2e"))
    palette.setColor(QPalette.ColorRole.WindowText, QColor("#cdd6f4"))
    palette.setColor(QPalette.ColorRole.Base, QColor("#1e1e2e"))
    palette.setColor(QPalette.ColorRole.Text, QColor("#cdd6f4"))
    palette.setColor(QPalette.ColorRole.Button, QColor("#313244"))
    palette.setColor(QPalette.ColorRole.ButtonText, QColor("#cdd6f4"))
    palette.setColor(QPalette.ColorRole.Highlight, QColor("#89b4fa"))
    app.setPalette(palette)

    initial = sys.argv[1] if len(sys.argv) > 1 else None
    viewer = ICLViewer(initial_path=initial)
    viewer.show()
    sys.exit(app.exec())
