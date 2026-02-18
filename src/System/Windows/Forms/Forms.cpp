#include "Forms.hpp"
#include "../../Drawing/Drawing.hpp"

namespace System { namespace Windows { namespace Forms {

/******************************************************************************/
/*    SpatialGrid Implementation                                              */
/******************************************************************************/

SpatialGrid::SpatialGrid()
    : _cellsX(Int32(0))
    , _cellsY(Int32(0))
    , _screenWidth(Int32(0))
    , _screenHeight(Int32(0)) {
}

void SpatialGrid::Initialize(Int32 screenWidth, Int32 screenHeight) {
    _screenWidth = screenWidth;
    _screenHeight = screenHeight;
    Int32 sw = _screenWidth;
    Int32 sh = _screenHeight;
    _cellsX = Int32((static_cast<int>(sw) + CELL_SIZE - 1) / CELL_SIZE);
    _cellsY = Int32((static_cast<int>(sh) + CELL_SIZE - 1) / CELL_SIZE);

    // Clamp to max
    if (static_cast<int>(_cellsX) > MAX_CELLS_X) _cellsX = Int32(MAX_CELLS_X);
    if (static_cast<int>(_cellsY) > MAX_CELLS_Y) _cellsY = Int32(MAX_CELLS_Y);

    Clear();
}

void SpatialGrid::Clear() {
    for (Int32 y = Int32(0); static_cast<int>(y) < MAX_CELLS_Y; y += 1) {
        for (Int32 x = Int32(0); static_cast<int>(x) < MAX_CELLS_X; x += 1) {
            _cells[static_cast<int>(y)][static_cast<int>(x)].count = 0;
            for (Int32 i = Int32(0); static_cast<int>(i) < MAX_CONTROLS_PER_CELL; i += 1) {
                _cells[static_cast<int>(y)][static_cast<int>(x)].controls[static_cast<int>(i)] = nullptr;
            }
        }
    }
}

void SpatialGrid::GetCellIndex(Int32 x, Int32 y, Int32& cellX, Int32& cellY) const {
    cellX = Int32(static_cast<int>(x) / CELL_SIZE);
    cellY = Int32(static_cast<int>(y) / CELL_SIZE);
    if (static_cast<int>(cellX) < 0) cellX = Int32(0);
    if (static_cast<int>(cellY) < 0) cellY = Int32(0);
    if (cellX >= _cellsX) cellX = _cellsX - Int32(1);
    if (cellY >= _cellsY) cellY = _cellsY - Int32(1);
}

void SpatialGrid::GetCellRange(const Rectangle& bounds, Int32& minX, Int32& minY,
                               Int32& maxX, Int32& maxY) const {
    Int32 bx = bounds.x;
    Int32 by = bounds.y;
    Int32 bw = bounds.width;
    Int32 bh = bounds.height;

    GetCellIndex(bx, by, minX, minY);
    GetCellIndex(Int32(static_cast<int>(bx) + static_cast<int>(bw) - 1),
                 Int32(static_cast<int>(by) + static_cast<int>(bh) - 1), maxX, maxY);
}

void SpatialGrid::Insert(Control* control, const Rectangle& bounds) {
    if (!control) return;

    Int32 minX, minY, maxX, maxY;
    GetCellRange(bounds, minX, minY, maxX, maxY);

    for (Int32 y = minY; y <= maxY; y += 1) {
        for (Int32 x = minX; x <= maxX; x += 1) {
            Cell& cell = _cells[static_cast<int>(y)][static_cast<int>(x)];
            if (cell.count < MAX_CONTROLS_PER_CELL) {
                // Check if already in cell
                Boolean found = Boolean(false);
                for (Int32 i = Int32(0); static_cast<int>(i) < cell.count; i += 1) {
                    if (cell.controls[static_cast<int>(i)] == control) {
                        found = Boolean(true);
                        break;
                    }
                }
                if (!static_cast<bool>(found)) {
                    cell.controls[cell.count++] = control;
                }
            }
        }
    }
}

void SpatialGrid::Remove(Control* control) {
    if (!control) return;

    for (Int32 y = Int32(0); y < _cellsY; y += 1) {
        for (Int32 x = Int32(0); x < _cellsX; x += 1) {
            Cell& cell = _cells[static_cast<int>(y)][static_cast<int>(x)];
            for (Int32 i = Int32(0); static_cast<int>(i) < cell.count; i += 1) {
                if (cell.controls[static_cast<int>(i)] == control) {
                    // Shift remaining controls down
                    for (Int32 j = i; static_cast<int>(j) < cell.count - 1; j += 1) {
                        cell.controls[static_cast<int>(j)] = cell.controls[static_cast<int>(j) + 1];
                    }
                    cell.controls[--cell.count] = nullptr;
                    break;
                }
            }
        }
    }
}

Control* SpatialGrid::HitTest(Int32 x, Int32 y) const {
    Int32 px = x;
    Int32 py = y;

    if (static_cast<int>(px) < 0 || static_cast<int>(py) < 0 ||
        px >= _screenWidth || py >= _screenHeight) {
        return nullptr;
    }

    Int32 cellX, cellY;
    GetCellIndex(px, py, cellX, cellY);

    const Cell& cell = _cells[static_cast<int>(cellY)][static_cast<int>(cellX)];

    // Check controls in reverse order (last added = highest z-order)
    for (Int32 i = Int32(cell.count - 1); static_cast<int>(i) >= 0; i -= 1) {
        Control* ctrl = cell.controls[static_cast<int>(i)];
        if (ctrl && static_cast<bool>(ctrl->HitTest(px, py))) {
            return ctrl;
        }
    }

    return nullptr;
}

/******************************************************************************/
/*    Control Implementation                                                  */
/******************************************************************************/

Control::Control()
    : _children()
    , _parent(nullptr)
    , _bounds()
    , _clientBounds()
    , _isInvalid(true) {
}

Control::Control(Control* parent, const Rectangle& bounds)
    : _children()
    , _parent(parent)
    , _bounds(bounds)
    , _clientBounds()
    , _isInvalid(true) {
    UpdateClientBounds();
    if (parent) {
        parent->AddChild(this);
    }
}

Control::~Control() {
    // Delete all children
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
        delete _children[static_cast<int>(i)];
    }
}

void Control::UpdateClientBounds() {
    // By default, client bounds is the full control area (no decoration)
    // Client bounds are relative to the control's origin (0,0)
    _clientBounds = Rectangle(0, 0, _bounds.width, _bounds.height);
}

Control* Control::GetChild(Int32 index) const {
    Int32 idx = index;
    if (static_cast<int>(idx) >= 0 && static_cast<int>(idx) < _children.Length()) {
        return _children[static_cast<int>(idx)];
    }
    return nullptr;
}

Rectangle Control::ScreenBounds() const {
    if (!_parent) {
        // Root control (Desktop) - bounds are already screen coordinates
        return _bounds;
    }

    // Get parent's screen client bounds and offset our bounds
    Rectangle parentClient = _parent->ScreenClientBounds();
    Int32 px = parentClient.x;
    Int32 py = parentClient.y;
    Int32 bx = _bounds.x;
    Int32 by = _bounds.y;
    Int32 bw = _bounds.width;
    Int32 bh = _bounds.height;
    return Rectangle(Int32(static_cast<int>(px) + static_cast<int>(bx)),
                     Int32(static_cast<int>(py) + static_cast<int>(by)), bw, bh);
}

Rectangle Control::ScreenClientBounds() const {
    Rectangle screenBounds = ScreenBounds();
    Int32 sx = screenBounds.x;
    Int32 sy = screenBounds.y;
    Int32 cx = _clientBounds.x;
    Int32 cy = _clientBounds.y;
    Int32 cw = _clientBounds.width;
    Int32 ch = _clientBounds.height;
    return Rectangle(Int32(static_cast<int>(sx) + static_cast<int>(cx)),
                     Int32(static_cast<int>(sy) + static_cast<int>(cy)), cw, ch);
}

Rectangle Control::VisibleBounds() const {
    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 sw = screen.width;
    Int32 sh = screen.height;

    // Get screen dimensions from current display mode
    Display current = Display::GetCurrent();
    Int32 screenWidth = Int32(static_cast<int>(current.Width()));
    Int32 screenHeight = Int32(static_cast<int>(current.Height()));

    if (!_parent) {
        // Root control - clip to screen
        Int32 left = sx < Int32(0) ? Int32(0) : sx;
        Int32 top = sy < Int32(0) ? Int32(0) : sy;
        Int32 right = Int32(static_cast<int>(sx) + static_cast<int>(sw)) > screenWidth ?
                      screenWidth : Int32(static_cast<int>(sx) + static_cast<int>(sw));
        Int32 bottom = Int32(static_cast<int>(sy) + static_cast<int>(sh)) > screenHeight ?
                       screenHeight : Int32(static_cast<int>(sy) + static_cast<int>(sh));
        return Rectangle(left, top, Int32(static_cast<int>(right) - static_cast<int>(left)),
                         Int32(static_cast<int>(bottom) - static_cast<int>(top)));
    }

    // Clip to parent's client area
    Rectangle parentClient = _parent->ScreenClientBounds();
    Int32 pcx = parentClient.x;
    Int32 pcy = parentClient.y;
    Int32 pcw = parentClient.width;
    Int32 pch = parentClient.height;

    Int32 left = sx < pcx ? pcx : sx;
    Int32 top = sy < pcy ? pcy : sy;
    Int32 right = Int32(static_cast<int>(sx) + static_cast<int>(sw));
    Int32 bottom = Int32(static_cast<int>(sy) + static_cast<int>(sh));

    Int32 parentRight = Int32(static_cast<int>(pcx) + static_cast<int>(pcw));
    Int32 parentBottom = Int32(static_cast<int>(pcy) + static_cast<int>(pch));

    if (right > parentRight) right = parentRight;
    if (bottom > parentBottom) bottom = parentBottom;

    // Also clip to screen bounds
    if (left < Int32(0)) left = Int32(0);
    if (top < Int32(0)) top = Int32(0);
    if (right > screenWidth) right = screenWidth;
    if (bottom > screenHeight) bottom = screenHeight;

    if (right <= left || bottom <= top) {
        return Rectangle(Int32(0), Int32(0), Int32(0), Int32(0));  // Not visible
    }

    return Rectangle(left, top, Int32(static_cast<int>(right) - static_cast<int>(left)),
                     Int32(static_cast<int>(bottom) - static_cast<int>(top)));
}

void Control::SetBounds(const Rectangle& bounds) {
    _bounds = bounds;
    UpdateClientBounds();
    Invalidate();
}

void Control::SetBounds(Int32 x, Int32 y, Int32 width, Int32 height) {
    SetBounds(Rectangle(x, y, width, height));
}

void Control::AddChild(Control* child) {
    if (child) {
        Int32 oldLen = Int32(_children.Length());
        _children.Resize(static_cast<int>(oldLen) + 1);
        _children[static_cast<int>(oldLen)] = child;
        child->_parent = this;
        Invalidate();
    }
}

void Control::RemoveChild(Control* child) {
    if (child) {
        Int32 index = Int32(_children.IndexOf(child));
        if (static_cast<int>(index) >= 0) {
            // Shift elements down
            for (Int32 i = index; static_cast<int>(i) < _children.Length() - 1; i += 1) {
                _children[static_cast<int>(i)] = _children[static_cast<int>(i) + 1];
            }
            _children.Resize(_children.Length() - 1);
            child->_parent = nullptr;
            Invalidate();
        }
    }
}

void Control::OnPaint(PaintEventArgs& e) {
    // Base implementation: draw nothing, just call OnPaintClient
    OnPaintClient(e);
}

void Control::OnPaintClient(PaintEventArgs& e) {
    // Base implementation: paint children
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
        Control* child = _children[static_cast<int>(i)];
        if (child) {
            PaintEventArgs childArgs(e.graphics, child->Bounds());
            child->OnPaint(childArgs);
        }
    }
}

void Control::Invalidate() {
    _isInvalid = true;
    // Propagate up to parent
    if (_parent) {
        _parent->Invalidate();
    }
}

void Control::Update() {
    if (_isInvalid) {
        GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
        if (fb) {
            Graphics g(BufferMode::Single, _bounds);
            PaintEventArgs e(&g, _bounds);
            OnPaint(e);
        }
        _isInvalid = false;
    }
}

void Control::OnMouse(MouseEventArgs& e) {
    // Base implementation: do nothing
    (void)e;
}

void Control::OnKeyboard(KeyboardEventArgs& e) {
    // Base implementation: do nothing
    (void)e;
}

void Control::NotifyMouse(MouseEventArgs& e) {
    Int32 ex = e.x;
    Int32 ey = e.y;
    // First check if any child contains the point (in reverse order for z-order)
    for (Int32 i = Int32(_children.Length() - 1); static_cast<int>(i) >= 0; i -= 1) {
        Control* child = _children[static_cast<int>(i)];
        if (child && static_cast<bool>(child->HitTest(ex, ey))) {
            child->NotifyMouse(e);
            return;
        }
    }
    // No child hit, handle ourselves
    OnMouse(e);
}

void Control::NotifyKeyboard(KeyboardEventArgs& e) {
    // Pass to all children (typically only focused window handles it)
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
        Control* child = _children[static_cast<int>(i)];
        if (child) {
            child->NotifyKeyboard(e);
        }
    }
    OnKeyboard(e);
}

Boolean Control::HitTest(Int32 x, Int32 y) const {
    return ScreenBounds().Contains(x, y);
}

/******************************************************************************/
/*    Control Layout Implementation                                           */
/******************************************************************************/

MeasureResult Control::GetPreferredSize() const {
    // Base implementation: return current bounds size or min size
    Int32 w = _bounds.width;
    Int32 h = _bounds.height;

    // Apply min constraints
    Int32 minW = _layout.minWidth;
    Int32 minH = _layout.minHeight;
    if (w < minW) w = minW;
    if (h < minH) h = minH;

    return MeasureResult(w, h);
}

MeasureResult Control::Measure(Int32 availableWidth, Int32 availableHeight) {
    Int32 avW = availableWidth;
    Int32 avH = availableHeight;

    // Subtract margins from available space
    Int32 marginH = Int32(static_cast<int>(_layout.marginLeft) + static_cast<int>(_layout.marginRight));
    Int32 marginV = Int32(static_cast<int>(_layout.marginTop) + static_cast<int>(_layout.marginBottom));
    avW = Int32(static_cast<int>(avW) - static_cast<int>(marginH));
    avH = Int32(static_cast<int>(avH) - static_cast<int>(marginV));
    if (avW < Int32(0)) avW = Int32(0);
    if (avH < Int32(0)) avH = Int32(0);

    Int32 resultW = Int32(0);
    Int32 resultH = Int32(0);

    // Handle different size modes
    if (_layout.widthMode == SizeMode::Fixed) {
        resultW = _bounds.width;
    } else if (_layout.widthMode == SizeMode::Fill) {
        resultW = avW;
    }
    // Auto mode: calculate from content/children

    if (_layout.heightMode == SizeMode::Fixed) {
        resultH = _bounds.height;
    } else if (_layout.heightMode == SizeMode::Fill) {
        resultH = avH;
    }
    // Auto mode: calculate from content/children

    // For Auto mode, measure based on children
    if (_layout.widthMode == SizeMode::Auto || _layout.heightMode == SizeMode::Auto) {
        // Get padding
        Int32 padL = _layout.paddingLeft;
        Int32 padR = _layout.paddingRight;
        Int32 padT = _layout.paddingTop;
        Int32 padB = _layout.paddingBottom;

        Int32 contentW = Int32(0);
        Int32 contentH = Int32(0);
        Int32 gap = _layout.gap;

        // Count participating children
        Int32 participatingCount = Int32(0);
        for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
            Control* child = _children[static_cast<int>(i)];
            if (child && child->_layout.participatesInLayout) {
                participatingCount += 1;
            }
        }

        // Measure children
        Boolean isRow = Boolean(_layout.direction == FlexDirection::Row);

        for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
            Control* child = _children[static_cast<int>(i)];
            if (!child || !child->_layout.participatesInLayout) continue;

            // Measure child
            MeasureResult childSize = child->Measure(
                Int32(static_cast<int>(avW) - static_cast<int>(padL) - static_cast<int>(padR)),
                Int32(static_cast<int>(avH) - static_cast<int>(padT) - static_cast<int>(padB))
            );

            Int32 cw = childSize.preferredWidth;
            Int32 ch = childSize.preferredHeight;

            // Add child margins
            cw = Int32(static_cast<int>(cw) + static_cast<int>(child->_layout.marginLeft) +
                  static_cast<int>(child->_layout.marginRight));
            ch = Int32(static_cast<int>(ch) + static_cast<int>(child->_layout.marginTop) +
                  static_cast<int>(child->_layout.marginBottom));

            if (static_cast<bool>(isRow)) {
                // Row: width accumulates, height is max
                contentW = Int32(static_cast<int>(contentW) + static_cast<int>(cw));
                if (ch > contentH) contentH = ch;
            } else {
                // Column: height accumulates, width is max
                contentH = Int32(static_cast<int>(contentH) + static_cast<int>(ch));
                if (cw > contentW) contentW = cw;
            }
        }

        // Add gaps between children
        if (participatingCount > Int32(1)) {
            if (static_cast<bool>(isRow)) {
                contentW = Int32(static_cast<int>(contentW) + static_cast<int>(gap) * (static_cast<int>(participatingCount) - 1));
            } else {
                contentH = Int32(static_cast<int>(contentH) + static_cast<int>(gap) * (static_cast<int>(participatingCount) - 1));
            }
        }

        // Add padding
        contentW = Int32(static_cast<int>(contentW) + static_cast<int>(padL) + static_cast<int>(padR));
        contentH = Int32(static_cast<int>(contentH) + static_cast<int>(padT) + static_cast<int>(padB));

        // Use content size for Auto modes
        if (_layout.widthMode == SizeMode::Auto) {
            resultW = contentW;
        }
        if (_layout.heightMode == SizeMode::Auto) {
            resultH = contentH;
        }
    }

    // Apply self preferred size if no children
    if (resultW == Int32(0) && resultH == Int32(0)) {
        MeasureResult pref = GetPreferredSize();
        if (_layout.widthMode == SizeMode::Auto) {
            resultW = pref.preferredWidth;
        }
        if (_layout.heightMode == SizeMode::Auto) {
            resultH = pref.preferredHeight;
        }
    }

    // Apply min/max constraints
    Int32 minW = _layout.minWidth;
    Int32 minH = _layout.minHeight;
    Int32 maxW = _layout.maxWidth;
    Int32 maxH = _layout.maxHeight;

    if (resultW < minW) resultW = minW;
    if (resultH < minH) resultH = minH;
    if (resultW > maxW) resultW = maxW;
    if (resultH > maxH) resultH = maxH;

    // Store and return result
    _measuredSize = MeasureResult(resultW, resultH);
    return _measuredSize;
}

void Control::Arrange(const Rectangle& finalBounds) {
    // Update our bounds
    _bounds = finalBounds;
    UpdateClientBounds();

    // Calculate content area (bounds minus padding)
    Int32 padL = _layout.paddingLeft;
    Int32 padR = _layout.paddingRight;
    Int32 padT = _layout.paddingTop;
    Int32 padB = _layout.paddingBottom;

    Int32 cx = Int32(static_cast<int>(_clientBounds.x) + static_cast<int>(padL));
    Int32 cy = Int32(static_cast<int>(_clientBounds.y) + static_cast<int>(padT));
    Int32 cw = Int32(static_cast<int>(_clientBounds.width) - static_cast<int>(padL) - static_cast<int>(padR));
    Int32 ch = Int32(static_cast<int>(_clientBounds.height) - static_cast<int>(padT) - static_cast<int>(padB));

    if (cw < Int32(0)) cw = Int32(0);
    if (ch < Int32(0)) ch = Int32(0);

    Rectangle contentArea(cx, cy, cw, ch);

    // Arrange children using flex layout
    ArrangeFlexChildren(contentArea);

    // Mark layout as clean
    _layout.needsLayout = false;
}

void Control::ArrangeFlexChildren(const Rectangle& contentArea) {
    Int32 cx = contentArea.x;
    Int32 cy = contentArea.y;
    Int32 cw = contentArea.width;
    Int32 ch = contentArea.height;
    Int32 gap = _layout.gap;
    Boolean isRow = Boolean(_layout.direction == FlexDirection::Row);

    // First pass: gather info about participating children
    Int32 participatingCount = Int32(0);
    Int32 totalMainSize = Int32(0);
    Int32 totalFlexGrow = Int32(0);
    Int32 maxCrossSize = Int32(0);

    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
        Control* child = _children[static_cast<int>(i)];
        if (!child || !child->_layout.participatesInLayout) continue;

        participatingCount += 1;

        // Get measured size
        Int32 childW = child->_measuredSize.preferredWidth;
        Int32 childH = child->_measuredSize.preferredHeight;

        // Add margins to size
        Int32 marginH = Int32(static_cast<int>(child->_layout.marginLeft) +
                      static_cast<int>(child->_layout.marginRight));
        Int32 marginV = Int32(static_cast<int>(child->_layout.marginTop) +
                      static_cast<int>(child->_layout.marginBottom));

        if (static_cast<bool>(isRow)) {
            totalMainSize = Int32(static_cast<int>(totalMainSize) + static_cast<int>(childW) + static_cast<int>(marginH));
            Int32 crossSize = Int32(static_cast<int>(childH) + static_cast<int>(marginV));
            if (crossSize > maxCrossSize) {
                maxCrossSize = crossSize;
            }
        } else {
            totalMainSize = Int32(static_cast<int>(totalMainSize) + static_cast<int>(childH) + static_cast<int>(marginV));
            Int32 crossSize = Int32(static_cast<int>(childW) + static_cast<int>(marginH));
            if (crossSize > maxCrossSize) {
                maxCrossSize = crossSize;
            }
        }

        totalFlexGrow = Int32(static_cast<int>(totalFlexGrow) + static_cast<int>(child->_layout.flexGrow));
    }

    // Add gaps
    if (participatingCount > Int32(1)) {
        totalMainSize = Int32(static_cast<int>(totalMainSize) + static_cast<int>(gap) * (static_cast<int>(participatingCount) - 1));
    }

    if (participatingCount == Int32(0)) return;

    // Calculate available space for distribution
    Int32 mainAxisSize = static_cast<bool>(isRow) ? cw : ch;
    Int32 crossAxisSize = static_cast<bool>(isRow) ? ch : cw;
    Int32 extraSpace = Int32(static_cast<int>(mainAxisSize) - static_cast<int>(totalMainSize));
    if (extraSpace < Int32(0)) extraSpace = Int32(0);

    // Calculate starting position based on JustifyContent
    Int32 mainPos = Int32(0);
    Int32 spaceBetween = Int32(0);
    Int32 spaceAround = Int32(0);

    switch (_layout.justifyContent) {
        case JustifyContent::Start:
            mainPos = Int32(0);
            break;
        case JustifyContent::End:
            mainPos = extraSpace;
            break;
        case JustifyContent::Center:
            mainPos = Int32(static_cast<int>(extraSpace) / 2);
            break;
        case JustifyContent::SpaceBetween:
            mainPos = Int32(0);
            if (participatingCount > Int32(1)) {
                spaceBetween = Int32(static_cast<int>(extraSpace) / (static_cast<int>(participatingCount) - 1));
            }
            break;
        case JustifyContent::SpaceAround:
            if (participatingCount > Int32(0)) {
                spaceAround = Int32(static_cast<int>(extraSpace) / (static_cast<int>(participatingCount) * 2));
                mainPos = spaceAround;
            }
            break;
    }

    // Second pass: arrange children
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
        Control* child = _children[static_cast<int>(i)];
        if (!child || !child->_layout.participatesInLayout) continue;

        // Get measured size
        Int32 childW = child->_measuredSize.preferredWidth;
        Int32 childH = child->_measuredSize.preferredHeight;

        // Get margins
        Int32 mTop = child->_layout.marginTop;
        Int32 mRight = child->_layout.marginRight;
        Int32 mBottom = child->_layout.marginBottom;
        Int32 mLeft = child->_layout.marginLeft;

        // Calculate flex grow contribution
        Int32 flexGrow = child->_layout.flexGrow;
        Int32 growAmount = Int32(0);
        if (totalFlexGrow > Int32(0) && flexGrow > Int32(0) && extraSpace > Int32(0)) {
            growAmount = Int32((static_cast<int>(extraSpace) * static_cast<int>(flexGrow)) / static_cast<int>(totalFlexGrow));
        }

        // Calculate final size
        Int32 finalW = childW;
        Int32 finalH = childH;

        if (static_cast<bool>(isRow)) {
            finalW = Int32(static_cast<int>(finalW) + static_cast<int>(growAmount));
            // Handle AlignItems for cross axis (height)
            switch (_layout.alignItems) {
                case AlignItems::Stretch:
                    finalH = Int32(static_cast<int>(crossAxisSize) - static_cast<int>(mTop) - static_cast<int>(mBottom));
                    break;
                case AlignItems::Start:
                case AlignItems::Center:
                case AlignItems::End:
                    // Keep measured height
                    break;
            }
        } else {
            finalH = Int32(static_cast<int>(finalH) + static_cast<int>(growAmount));
            // Handle AlignItems for cross axis (width)
            switch (_layout.alignItems) {
                case AlignItems::Stretch:
                    finalW = Int32(static_cast<int>(crossAxisSize) - static_cast<int>(mLeft) - static_cast<int>(mRight));
                    break;
                case AlignItems::Start:
                case AlignItems::Center:
                case AlignItems::End:
                    // Keep measured width
                    break;
            }
        }

        // Apply min/max constraints
        Int32 minW = child->_layout.minWidth;
        Int32 minH = child->_layout.minHeight;
        Int32 maxW = child->_layout.maxWidth;
        Int32 maxH = child->_layout.maxHeight;

        if (finalW < minW) finalW = minW;
        if (finalH < minH) finalH = minH;
        if (finalW > maxW) finalW = maxW;
        if (finalH > maxH) finalH = maxH;

        // Calculate position
        Int32 childX, childY;

        if (static_cast<bool>(isRow)) {
            childX = Int32(static_cast<int>(cx) + static_cast<int>(mainPos) + static_cast<int>(mLeft));

            // Cross axis position (Y) based on AlignItems
            switch (_layout.alignItems) {
                case AlignItems::Start:
                    childY = Int32(static_cast<int>(cy) + static_cast<int>(mTop));
                    break;
                case AlignItems::End:
                    childY = Int32(static_cast<int>(cy) + static_cast<int>(crossAxisSize) - static_cast<int>(finalH) - static_cast<int>(mBottom));
                    break;
                case AlignItems::Center:
                    childY = Int32(static_cast<int>(cy) + (static_cast<int>(crossAxisSize) - static_cast<int>(finalH) - static_cast<int>(mTop) - static_cast<int>(mBottom)) / 2 + static_cast<int>(mTop));
                    break;
                case AlignItems::Stretch:
                default:
                    childY = Int32(static_cast<int>(cy) + static_cast<int>(mTop));
                    break;
            }

            // Advance main axis position
            mainPos = Int32(static_cast<int>(mainPos) + static_cast<int>(finalW) + static_cast<int>(mLeft) + static_cast<int>(mRight) + static_cast<int>(gap) + static_cast<int>(spaceBetween) + static_cast<int>(spaceAround) * 2);
        } else {
            childY = Int32(static_cast<int>(cy) + static_cast<int>(mainPos) + static_cast<int>(mTop));

            // Cross axis position (X) based on AlignItems
            switch (_layout.alignItems) {
                case AlignItems::Start:
                    childX = Int32(static_cast<int>(cx) + static_cast<int>(mLeft));
                    break;
                case AlignItems::End:
                    childX = Int32(static_cast<int>(cx) + static_cast<int>(crossAxisSize) - static_cast<int>(finalW) - static_cast<int>(mRight));
                    break;
                case AlignItems::Center:
                    childX = Int32(static_cast<int>(cx) + (static_cast<int>(crossAxisSize) - static_cast<int>(finalW) - static_cast<int>(mLeft) - static_cast<int>(mRight)) / 2 + static_cast<int>(mLeft));
                    break;
                case AlignItems::Stretch:
                default:
                    childX = Int32(static_cast<int>(cx) + static_cast<int>(mLeft));
                    break;
            }

            // Advance main axis position
            mainPos = Int32(static_cast<int>(mainPos) + static_cast<int>(finalH) + static_cast<int>(mTop) + static_cast<int>(mBottom) + static_cast<int>(gap) + static_cast<int>(spaceBetween) + static_cast<int>(spaceAround) * 2);
        }

        // Recursively arrange child
        Rectangle childBounds(childX, childY, finalW, finalH);
        child->Arrange(childBounds);
    }

    // Arrange non-participating children (they keep their current bounds)
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
        Control* child = _children[static_cast<int>(i)];
        if (child && !child->_layout.participatesInLayout) {
            // Just trigger their own layout with current bounds
            child->Arrange(child->_bounds);
        }
    }
}

void Control::PerformLayout() {
    if (!_layout.needsLayout) return;

    // Measure pass (bottom-up)
    Int32 availW = _bounds.width;
    Int32 availH = _bounds.height;
    Measure(availW, availH);

    // Arrange pass (top-down)
    Arrange(_bounds);
}

void Control::InvalidateLayout() {
    _layout.needsLayout = true;
    // Also invalidate visual
    Invalidate();
}

/******************************************************************************/
/*    Desktop Implementation                                                  */
/******************************************************************************/

Desktop::Desktop(const Color& backgroundColor)
    : Control()
    , _backgroundColor(backgroundColor)
    , _focusedWindow(nullptr)
    , _dragWindow(nullptr)
    , _dragOffsetX(Int32(0))
    , _dragOffsetY(Int32(0))
    , _dragStartX(Int32(0))
    , _dragStartY(Int32(0))
    , _dragBitmap()
    , _cursorImage()
    , _icons()
    , _isDragging(false)
    , _running(false)
    , _cursorX(Int32(0))
    , _cursorY(Int32(0))
    , _prevCursorX(Int32(0))
    , _prevCursorY(Int32(0))
    , _cursorVisible(true)
    , _wasMouseDown(false)
    , _cursorSaved(false)
    , _screenWidth(Int32(0))
    , _screenHeight(Int32(0))
    , _nextIconX(Int32(ICON_MARGIN_X))
    , _nextIconY(Int32(ICON_MARGIN_Y))
    , _spatialGrid()
    , _taskBar(nullptr)
    , _startMenu(nullptr) {
    // Get screen dimensions from current display mode
    Display current = Display::GetCurrent();
    _screenWidth = Int32(static_cast<int>(current.Width()));
    _screenHeight = Int32(static_cast<int>(current.Height()));

    // Initialize spatial grid
    _spatialGrid.Initialize(_screenWidth, _screenHeight);

    // Center cursor initially
    _cursorX = Int32(static_cast<int>(_screenWidth) / 2);
    _cursorY = Int32(static_cast<int>(_screenHeight) / 2);
    _prevCursorX = _cursorX;
    _prevCursorY = _cursorY;

    // Desktop fills the screen
    _bounds = Rectangle(Int32(0), Int32(0), _screenWidth, _screenHeight);
    _clientBounds = Rectangle(Int32(0), Int32(0), _screenWidth, _screenHeight);
    // Initialize cursor save buffer
    for (Int32 i = Int32(0); static_cast<int>(i) < CURSOR_SIZE * CURSOR_SIZE; i += 1) {
        _cursorSave[static_cast<int>(i)] = 0;
    }
}

Desktop::~Desktop() {
}

void Desktop::SetCursor(const Image& cursorImage) {
    _cursorImage = cursorImage;
}

void Desktop::LoadCursorFromLibrary(const char* path, Int32 iconIndex) {
    _cursorImage = Image::FromIconLibrary(path, iconIndex, Size::IconCursor);
}

void Desktop::AddIcon(const Image& icon) {
    // Calculate position for new icon (arrange in columns)
    Int32 taskBarHeight = Int32(28);  // Reserve space for taskbar
    Int32 maxY = Int32(static_cast<int>(_screenHeight) - static_cast<int>(taskBarHeight) - ICON_SIZE - ICON_MARGIN_Y);

    // Add the icon at current position
    Int32 oldLen = Int32(_icons.Length());
    _icons.Resize(static_cast<int>(oldLen) + 1);
    _icons[static_cast<int>(oldLen)] = DesktopIcon(icon, static_cast<int>(_nextIconX), static_cast<int>(_nextIconY));

    // Move to next position (flow down, then right)
    _nextIconY = Int32(static_cast<int>(_nextIconY) + ICON_SPACING_Y);
    if (_nextIconY > maxY) {
        _nextIconY = Int32(ICON_MARGIN_Y);
        _nextIconX = Int32(static_cast<int>(_nextIconX) + ICON_SPACING_X);
    }

    Invalidate();
}

void Desktop::AddIconFromLibrary(const char* path, Int32 iconIndex) {
    Image icon = Image::FromIconLibrary(path, iconIndex, Size::IconMedium);
    AddIcon(icon);
}

void Desktop::LoadCursorFromLibrary(const char* path, const char* iconName) {
    _cursorImage = Image::FromIconLibrary(path, iconName, Size::IconCursor);
}

void Desktop::AddIconFromLibrary(const char* path, const char* iconName) {
    Image icon = Image::FromIconLibrary(path, iconName, Size::IconMedium);
    AddIcon(icon);
}

void Desktop::DrawIcons() {
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    Image& img = fb->GetImage();

    for (Int32 i = Int32(0); static_cast<int>(i) < _icons.Length(); i += 1) {
        const DesktopIcon& icon = _icons[static_cast<int>(i)];
        Int32 iw = icon.image.Width();
        Int32 ih = icon.image.Height();

        if (iw > Int32(0) && ih > Int32(0)) {
            // Draw icon with alpha blending
            img.CopyFromWithAlpha(icon.image, icon.x, icon.y);
        }
    }
}

void Desktop::SetFocusedWindow(Window* window) {
    if (_focusedWindow == window) return;

    // Unfocus previous window
    if (_focusedWindow) {
        _focusedWindow->SetFocused(Boolean(false));
    }

    _focusedWindow = window;

    // Focus new window and bring to front (move to end of children array)
    if (_focusedWindow) {
        _focusedWindow->SetFocused(Boolean(true));

        // Find and move to end for z-order
        Int32 idx = Int32(_children.IndexOf(static_cast<Control*>(_focusedWindow)));
        if (static_cast<int>(idx) >= 0 && static_cast<int>(idx) < _children.Length() - 1) {
            // Shift elements down
            for (Int32 i = idx; static_cast<int>(i) < _children.Length() - 1; i += 1) {
                _children[static_cast<int>(i)] = _children[static_cast<int>(i) + 1];
            }
            _children[_children.Length() - 1] = _focusedWindow;
        }

        // Update spatial grid after z-order change
        UpdateSpatialGrid();
    }

    // Refresh taskbar buttons to show focused state
    if (_taskBar) {
        _taskBar->RefreshWindowButtons();
    }

    Invalidate();
}

void Desktop::AddChild(Control* child) {
    if (child) {
        // Call base class
        Control::AddChild(child);

        // Add to spatial grid
        _spatialGrid.Insert(child, child->ScreenBounds());

        // If this is a window, add a taskbar button
        Window* win = child->AsWindow();
        if (win && _taskBar) {
            _taskBar->AddWindowButton(win);
        }
    }
}

void Desktop::RemoveChild(Control* child) {
    if (child) {
        // Remove from spatial grid
        _spatialGrid.Remove(child);

        // If this is a window, remove taskbar button
        Window* win = child->AsWindow();
        if (win && _taskBar) {
            _taskBar->RemoveWindowButton(win);
        }

        // Call base class
        Control::RemoveChild(child);
    }
}

void Desktop::UpdateSpatialGrid() {
    _spatialGrid.Clear();
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
        Control* child = _children[static_cast<int>(i)];
        if (child) {
            _spatialGrid.Insert(child, child->ScreenBounds());
        }
    }
}

void Desktop::OnPaint(PaintEventArgs& e) {
    // Fill background
    e.graphics->FillRectangle(_bounds, _backgroundColor);

    // Draw desktop icons
    DrawIcons();

    // Paint all children (windows, taskbar, etc.) EXCEPT start menu
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
        Control* child = _children[static_cast<int>(i)];
        if (child && child != static_cast<Control*>(_startMenu)) {
            PaintEventArgs childArgs(e.graphics, child->Bounds());
            child->OnPaint(childArgs);
        }
    }

    // Paint start menu LAST so it appears on top
    if (_startMenu && static_cast<bool>(_startMenu->IsVisible())) {
        PaintEventArgs menuArgs(e.graphics, _startMenu->Bounds());
        _startMenu->OnPaint(menuArgs);
    }
}

void Desktop::SaveUnderCursor() {
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    Image& img = fb->GetImage();
    for (Int32 dy = Int32(0); static_cast<int>(dy) < CURSOR_SIZE; dy += 1) {
        for (Int32 dx = Int32(0); static_cast<int>(dx) < CURSOR_SIZE; dx += 1) {
            Int32 px = Int32(static_cast<int>(_cursorX) + static_cast<int>(dx));
            Int32 py = Int32(static_cast<int>(_cursorY) + static_cast<int>(dy));
            if (px >= Int32(0) && px < _screenWidth && py >= Int32(0) && py < _screenHeight) {
                _cursorSave[static_cast<int>(dy) * CURSOR_SIZE + static_cast<int>(dx)] = static_cast<unsigned int>(img.GetPixel(px, py));
            }
        }
    }
    _prevCursorX = _cursorX;
    _prevCursorY = _cursorY;
    _cursorSaved = true;
}

void Desktop::RestoreCursor() {
    if (!_cursorSaved) return;

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    Image& img = fb->GetImage();
    for (Int32 dy = Int32(0); static_cast<int>(dy) < CURSOR_SIZE; dy += 1) {
        for (Int32 dx = Int32(0); static_cast<int>(dx) < CURSOR_SIZE; dx += 1) {
            Int32 px = Int32(static_cast<int>(_prevCursorX) + static_cast<int>(dx));
            Int32 py = Int32(static_cast<int>(_prevCursorY) + static_cast<int>(dy));
            if (px >= Int32(0) && px < _screenWidth && py >= Int32(0) && py < _screenHeight) {
                img.SetPixel(px, py, Color(_cursorSave[static_cast<int>(dy) * CURSOR_SIZE + static_cast<int>(dx)]));
            }
        }
    }
}

void Desktop::DrawCursor() {
    if (!_cursorVisible) return;

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    Image& img = fb->GetImage();

    Int32 cursorW = _cursorImage.Width();
    Int32 cursorH = _cursorImage.Height();

    // If we have a cursor image, draw it with alpha blending
    if (cursorW > Int32(0) && cursorH > Int32(0)) {
        for (Int32 dy = Int32(0); static_cast<int>(dy) < static_cast<int>(cursorH) && static_cast<int>(dy) < CURSOR_SIZE; dy += 1) {
            for (Int32 dx = Int32(0); static_cast<int>(dx) < static_cast<int>(cursorW) && static_cast<int>(dx) < CURSOR_SIZE; dx += 1) {
                Int32 px = Int32(static_cast<int>(_cursorX) + static_cast<int>(dx));
                Int32 py = Int32(static_cast<int>(_cursorY) + static_cast<int>(dy));
                if (px >= Int32(0) && px < _screenWidth && py >= Int32(0) && py < _screenHeight) {
                    Color pixel = _cursorImage.GetPixel(dx, dy);
                    // Only draw non-transparent pixels (alpha >= 128)
                    UInt32 alpha = UInt32(static_cast<unsigned int>(pixel.A()));
                    if (static_cast<unsigned int>(alpha) >= 128) {
                        img.SetPixel(px, py, pixel);
                    }
                }
            }
        }
    } else {
        // Fallback: draw a simple arrow cursor
        for (Int32 dy = Int32(0); static_cast<int>(dy) < CURSOR_SIZE; dy += 1) {
            for (Int32 dx = Int32(0); static_cast<int>(dx) < CURSOR_SIZE; dx += 1) {
                Int32 px = Int32(static_cast<int>(_cursorX) + static_cast<int>(dx));
                Int32 py = Int32(static_cast<int>(_cursorY) + static_cast<int>(dy));
                if (px >= Int32(0) && px < _screenWidth && py >= Int32(0) && py < _screenHeight) {
                    // Simple arrow shape
                    Boolean isArrow = Boolean(static_cast<int>(dx) <= static_cast<int>(dy) && static_cast<int>(dx) < 12 && static_cast<int>(dy) < 18);
                    Boolean isBorder = Boolean(static_cast<bool>(isArrow) && (static_cast<int>(dx) == 0 || dx == dy || static_cast<int>(dy) == 17));
                    if (static_cast<bool>(isArrow)) {
                        img.SetPixel(px, py, static_cast<bool>(isBorder) ? Color::Black : Color::White);
                    }
                }
            }
        }
    }
}

void Desktop::CaptureWindowBitmap(Window* win) {
    if (!win) return;

    Rectangle screen = win->ScreenBounds();
    Int32 sw = screen.width;
    Int32 sh = screen.height;
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    _dragBitmap = Image(sw, sh);

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    // Copy window region from frame buffer
    const Image& fbImg = fb->GetImage();
    for (Int32 y = Int32(0); y < sh; y += 1) {
        for (Int32 x = Int32(0); x < sw; x += 1) {
            Int32 srcX = Int32(static_cast<int>(sx) + static_cast<int>(x));
            Int32 srcY = Int32(static_cast<int>(sy) + static_cast<int>(y));
            if (srcX >= Int32(0) && srcX < _screenWidth && srcY >= Int32(0) && srcY < _screenHeight) {
                _dragBitmap.SetPixel(x, y, fbImg.GetPixel(srcX, srcY));
            }
        }
    }

    _dragStartX = sx;
    _dragStartY = sy;
}

void Desktop::DrawDragBitmap() {
    if (!_isDragging || !_dragWindow) return;

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    Image& img = fb->GetImage();

    // Calculate current drag position
    Int32 newX = Int32(static_cast<int>(_cursorX) - static_cast<int>(_dragOffsetX));
    Int32 newY = Int32(static_cast<int>(_cursorY) - static_cast<int>(_dragOffsetY));

    Int32 dh = _dragBitmap.Height();
    Int32 dw = _dragBitmap.Width();

    // Draw the captured bitmap at new position
    for (Int32 y = Int32(0); y < dh; y += 1) {
        for (Int32 x = Int32(0); x < dw; x += 1) {
            Int32 dstX = Int32(static_cast<int>(newX) + static_cast<int>(x));
            Int32 dstY = Int32(static_cast<int>(newY) + static_cast<int>(y));
            if (dstX >= Int32(0) && dstX < _screenWidth && dstY >= Int32(0) && dstY < _screenHeight) {
                img.SetPixel(dstX, dstY, _dragBitmap.GetPixel(x, y));
            }
        }
    }
}

void Desktop::OnKeyboard(KeyboardEventArgs& e) {
    // ESC key exits
    if (static_cast<char>(e.key) == 27) {  // 27 = ESC
        Stop();
    }
}

void Desktop::HandleMouse(MouseEventArgs& e) {
    Int32 ex = e.x;
    Int32 ey = e.y;
    Boolean leftButton = e.leftButton;
    Boolean isNewClick = Boolean(static_cast<bool>(leftButton) && !_wasMouseDown);

    // Handle ongoing drag
    if (_isDragging && _dragWindow && static_cast<bool>(leftButton)) {
        // During drag, just update position - bitmap is drawn in Run()
        _wasMouseDown = static_cast<bool>(leftButton);
        return;
    }

    // End drag if mouse released
    if (!static_cast<bool>(leftButton) && _isDragging && _dragWindow) {
        // Apply final position
        Int32 newX = Int32(static_cast<int>(_cursorX) - static_cast<int>(_dragOffsetX));
        Int32 newY = Int32(static_cast<int>(_cursorY) - static_cast<int>(_dragOffsetY));
        Int32 bw = _dragWindow->Bounds().width;
        Int32 bh = _dragWindow->Bounds().height;
        _dragWindow->SetBounds(newX, newY, bw, bh);

        _isDragging = false;
        _dragWindow = nullptr;
        _dragBitmap = Image();  // Free the bitmap
        UpdateSpatialGrid();  // Update spatial grid after move
        Invalidate();  // Redraw everything
    }

    // Handle start menu - continuous mouse tracking for hover effects
    if (_startMenu && static_cast<bool>(_startMenu->IsVisible())) {
        Boolean mouseOnMenu = _startMenu->HitTest(ex, ey);
        Boolean clickOnStartButton = Boolean(_taskBar && _taskBar->StartButton() &&
            static_cast<bool>(_taskBar->StartButton()->HitTest(ex, ey)));

        // Hide menu if clicked outside
        if (static_cast<bool>(isNewClick) && !static_cast<bool>(mouseOnMenu) && !static_cast<bool>(clickOnStartButton)) {
            _startMenu->Hide();
            Invalidate();
        }

        // Always send mouse events to menu for hover tracking
        _startMenu->OnMouse(e);

        // If mouse is on menu, don't process other controls
        if (static_cast<bool>(mouseOnMenu)) {
            _wasMouseDown = static_cast<bool>(leftButton);
            return;
        }
    }

    // Find child under cursor (for event propagation)
    Control* hitChild = _spatialGrid.HitTest(ex, ey);

    // Fall back to linear search if spatial grid misses (e.g., for TaskBar)
    if (!hitChild) {
        for (Int32 i = Int32(_children.Length() - 1); static_cast<int>(i) >= 0; i -= 1) {
            Control* child = _children[static_cast<int>(i)];
            if (child && static_cast<bool>(child->HitTest(ex, ey))) {
                hitChild = child;
                break;
            }
        }
    }

    // Handle window focus and drag initiation on new click only
    if (static_cast<bool>(isNewClick) && hitChild) {
        Window* win = hitChild->AsWindow();
        if (win) {
            Rectangle screen = hitChild->ScreenBounds();
            Int32 sy = screen.y;
            Int32 sx = screen.x;

            SetFocusedWindow(win);

            // Check if click is on title bar (top 22 pixels)
            if (static_cast<int>(ey) < static_cast<int>(sy) + 22) {
                _dragWindow = win;
                _dragOffsetX = Int32(static_cast<int>(ex) - static_cast<int>(sx));
                _dragOffsetY = Int32(static_cast<int>(ey) - static_cast<int>(sy));

                // Capture the window as a bitmap before starting drag
                Invalidate();
                // Force repaint to get clean image
                GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
                if (fb) {
                    Graphics g(BufferMode::Single, _bounds);
                    PaintEventArgs pe(&g, _bounds);
                    OnPaint(pe);
                }
                CaptureWindowBitmap(win);
                _isDragging = true;
            }
            Invalidate();
        }
    }

    // ALWAYS propagate mouse events to children (for button state tracking)
    if (hitChild) {
        hitChild->NotifyMouse(e);
    }

    _wasMouseDown = static_cast<bool>(leftButton);
}

void Desktop::CheckForUpdates() {
    // Check mouse - create temporary int variables for platform function
    int tempX, tempY;
    bool tempLeftBtn, tempRightBtn;
    MouseStatus ms = Mouse::GetStatus();
    tempX = static_cast<int>(ms.x);
    tempY = static_cast<int>(ms.y);
    tempLeftBtn = static_cast<bool>(ms.leftButton);
    tempRightBtn = static_cast<bool>(ms.rightButton);

    // Update cursor position
    _cursorX = Int32(tempX);
    _cursorY = Int32(tempY);

    // Create mouse event and handle (always call to track drag state)
    MouseEventArgs mouseArgs{Int32(tempX), Int32(tempY), Boolean(tempLeftBtn), Boolean(tempRightBtn)};
    HandleMouse(mouseArgs);

    // Check keyboard
    if (static_cast<bool>(Keyboard::IsKeyPressed())) {
        Char key = Keyboard::ReadKey();
        KeyboardStatus ks = Keyboard::GetStatus();
        KeyboardEventArgs keyArgs(key, ks.altPressed, ks.ctrlPressed, ks.shiftPressed);

        // Route to focused window first, then handle ourselves
        if (_focusedWindow) {
            _focusedWindow->OnKeyboard(keyArgs);
        }
        OnKeyboard(keyArgs);
    }
}

void Desktop::Run() {
    _running = true;
    _isInvalid = true;  // Force initial paint

    // Do initial paint before fade in
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (fb) {
        Graphics g(BufferMode::Single, _bounds);
        PaintEventArgs e(&g, _bounds);
        OnPaint(e);
        SaveUnderCursor();
        DrawCursor();
        GraphicsBuffer::FlushFrameBuffer();
    }
    _isInvalid = false;

    // Fade in from black
    Display::FadeIn(Int32(500));

    while (_running) {
        // Wait for vertical sync to limit frame rate (~60fps)
        Display::WaitForVSync();

        // Check for input
        CheckForUpdates();

        if (_isDragging && _dragWindow) {
            // During drag: repaint scene without dragged window, then draw bitmap
            RestoreCursor();
            _cursorSaved = false;

            GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
            if (fb) {
                // Paint everything except the dragged window
                Graphics g(BufferMode::Single, _bounds);
                PaintEventArgs e(&g, _bounds);

                // Fill background
                g.FillRectangle(_bounds, _backgroundColor);

                // Draw desktop icons
                DrawIcons();

                // Paint children except dragged window
                for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1) {
                    Control* child = _children[static_cast<int>(i)];
                    if (child && child != _dragWindow) {
                        PaintEventArgs childArgs(&g, child->ScreenBounds());
                        child->OnPaint(childArgs);
                    }
                }

                // Draw the drag bitmap at current mouse position
                DrawDragBitmap();
            }

            SaveUnderCursor();
            DrawCursor();
            GraphicsBuffer::FlushFrameBuffer();
        } else if (_isInvalid) {
            // Normal repaint
            RestoreCursor();
            _cursorSaved = false;

            GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
            if (fb) {
                Graphics g(BufferMode::Single, _bounds);
                PaintEventArgs e(&g, _bounds);
                OnPaint(e);
            }
            _isInvalid = false;

            SaveUnderCursor();
            DrawCursor();
            GraphicsBuffer::FlushFrameBuffer();
        } else {
            // Just update cursor position
            Boolean cursorMoved = Boolean(_cursorX != _prevCursorX || _cursorY != _prevCursorY);
            if (static_cast<bool>(cursorMoved)) {
                RestoreCursor();
                SaveUnderCursor();
                DrawCursor();
                GraphicsBuffer::FlushFrameBuffer();
            }
        }
    }

    // Fade out to black
    Display::FadeOut(Int32(500));
}

void Desktop::Stop() {
    _running = false;
}

/******************************************************************************/
/*    Window Implementation                                                   */
/******************************************************************************/

// Helper function to load window title font
// Tries TTF first, falls back to FON (MS Sans Serif Bold)
static Font LoadWindowFont() {
    try {
        // Try ProggyClean TTF at 13 pixels
        return Font::FromTrueType("PROGGY.TTF", 13, FontStyle::Bold);
    } catch (...) {
        // Fall back to MS Sans Serif Bold
        return Font::SystemFontBold();
    }
}

Window::Window(Control* parent, const Rectangle& bounds)
    : Control(parent, bounds)
    , _isFocused(false)
    , _title()
    , _font(LoadWindowFont()) {
    // Must call UpdateClientBounds again because virtual dispatch doesn't work
    // from base class constructor (Control called Control::UpdateClientBounds)
    UpdateClientBounds();

    // Windows are floating - they don't participate in parent's layout
    _layout.participatesInLayout = false;

    // Register with taskbar - must be done here because virtual dispatch doesn't
    // work during base class constructor (AsWindow() returns nullptr there)
    if (parent && parent->GetControlType() == ControlType::Desktop) {
        Desktop* desktop = static_cast<Desktop*>(parent);
        TaskBar* taskBar = desktop->GetTaskBar();
        if (taskBar) {
            taskBar->AddWindowButton(this);
        }
    }
}

Window::~Window() {
}

void Window::UpdateClientBounds() {
    // Client area is relative to window bounds (not screen)
    // Outer frame: 2 pixels, title bar: 20 pixels, client sunken border: 1 pixel
    Int32 bw = _bounds.width;
    Int32 bh = _bounds.height;
    _clientBounds = Rectangle(
        Int32(FRAME_WIDTH),
        Int32(TITLE_BAR_HEIGHT + FRAME_WIDTH),
        Int32(static_cast<int>(bw) - FRAME_WIDTH * 2),
        Int32(static_cast<int>(bh) - TITLE_BAR_HEIGHT - FRAME_WIDTH * 2)
    );
}

void Window::OnPaint(PaintEventArgs& e) {
    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 sw = screen.width;
    Int32 sh = screen.height;

    // Draw window frame with Window border style (2-pixel thick 3D border)
    e.graphics->FillRectangle(screen, BorderStyle::Window);

    // Draw title bar - blue if focused, dark gray if not
    Rectangle titleBar(Int32(static_cast<int>(sx) + 2), Int32(static_cast<int>(sy) + 2),
                       Int32(static_cast<int>(sw) - 4), Int32(TITLE_BAR_HEIGHT));
    Color titleColor = _isFocused ? Color::DarkBlue : Color::DarkGray;
    e.graphics->FillRectangle(titleBar, titleColor);

    // Draw title text (white, centered vertically, left-aligned with padding)
    if (_title.Length() > Int32(0) && static_cast<bool>(_font.IsValid())) {
        Int32 textX = Int32(static_cast<int>(sx) + 6);  // Left padding
        Int32 textY = Int32(static_cast<int>(sy) + 2 + (TITLE_BAR_HEIGHT - static_cast<int>(_font.Height())) / 2);
        e.graphics->DrawString(_title, _font, Color::White, textX, textY);
    }

    // Draw client area with sunken border effect
    Rectangle clientFrame(Int32(static_cast<int>(sx) + 2),
                          Int32(static_cast<int>(sy) + TITLE_BAR_HEIGHT + 2),
                          Int32(static_cast<int>(sw) - 4),
                          Int32(static_cast<int>(sh) - TITLE_BAR_HEIGHT - 4));
    e.graphics->FillRectangle(clientFrame, BorderStyle::Sunken);

    // Paint children in client area
    OnPaintClient(e);
}

void Window::OnMouse(MouseEventArgs& e) {
    // Window click handling - focus is managed by Desktop
    (void)e;
}

/******************************************************************************/
/*    TaskBar Implementation                                                  */
/******************************************************************************/

void TaskBar::OnStartButtonClick(Button* sender, void* userData) {
    (void)sender;
    TaskBar* taskBar = static_cast<TaskBar*>(userData);
    if (taskBar && taskBar->_startMenu) {
        taskBar->_startMenu->Toggle();
        if (taskBar->_desktop) {
            taskBar->_desktop->Invalidate();
        }
    }
}

TaskBar::TaskBar(Control* parent, StartMenu* startMenu)
    : Control(parent, Rectangle(Int32(0), Int32(0), Int32(0), Int32(28)))  // Temporary bounds, updated below
    , _startButton(nullptr)
    , _startMenu(startMenu)
    , _desktop(nullptr)
    , _windowButtons() {
    // Get screen dimensions from current display mode
    Display current = Display::GetCurrent();
    Int32 screenWidth = Int32(static_cast<int>(current.Width()));
    Int32 screenHeight = Int32(static_cast<int>(current.Height()));
    Int32 taskBarHeight = Int32(28);

    // Position taskbar at bottom of screen
    SetBounds(Int32(0), Int32(static_cast<int>(screenHeight) - static_cast<int>(taskBarHeight)),
              screenWidth, taskBarHeight);

    // Configure layout: row direction, fixed height, items centered vertically
    _layout.direction = FlexDirection::Row;
    _layout.alignItems = AlignItems::Center;
    _layout.gap = Int32(2);
    _layout.heightMode = SizeMode::Fixed;
    _layout.paddingLeft = Int32(4);
    _layout.paddingTop = Int32(4);
    _layout.paddingBottom = Int32(4);

    // Create Start button (positioned relative to taskbar, not screen)
    _startButton = new Button(this, Rectangle(Int32(4), Int32(4), Int32(54), Int32(20)));
    _startButton->SetText("Start");
    _startButton->SetOnClick(OnStartButtonClick, this);

    // Start button has fixed size
    _startButton->Layout().widthMode = SizeMode::Fixed;
    _startButton->Layout().heightMode = SizeMode::Fixed;
}

TaskBar::~TaskBar() {
    // Children are deleted by Control destructor
    // _windowButtons are children, so they get deleted too
}

void TaskBar::AddWindowButton(Window* window) {
    if (!window) return;

    // Check if button already exists
    for (Int32 i = Int32(0); static_cast<int>(i) < _windowButtons.Length(); i += 1) {
        if (_windowButtons[static_cast<int>(i)]->GetWindow() == window) {
            return;
        }
    }

    // Calculate button position (relative to taskbar, not screen)
    Int32 buttonX = Int32(WINDOW_BUTTON_START_X + _windowButtons.Length() *
                  (WINDOW_BUTTON_WIDTH + WINDOW_BUTTON_SPACING));
    Int32 buttonY = Int32(4);  // 4 pixels from top of taskbar

    // Create new button
    TaskBarButton* btn = new TaskBarButton(this,
        Rectangle(buttonX, buttonY, Int32(WINDOW_BUTTON_WIDTH), Int32(WINDOW_BUTTON_HEIGHT)),
        window);

    // Add to array
    Int32 oldLen = Int32(_windowButtons.Length());
    _windowButtons.Resize(static_cast<int>(oldLen) + 1);
    _windowButtons[static_cast<int>(oldLen)] = btn;

    RefreshWindowButtons();
    Invalidate();
}

void TaskBar::RemoveWindowButton(Window* window) {
    if (!window) return;

    for (Int32 i = Int32(0); static_cast<int>(i) < _windowButtons.Length(); i += 1) {
        if (_windowButtons[static_cast<int>(i)]->GetWindow() == window) {
            TaskBarButton* btn = _windowButtons[static_cast<int>(i)];

            // Remove from array
            for (Int32 j = i; static_cast<int>(j) < _windowButtons.Length() - 1; j += 1) {
                _windowButtons[static_cast<int>(j)] = _windowButtons[static_cast<int>(j) + 1];
            }
            _windowButtons.Resize(_windowButtons.Length() - 1);

            // Remove from control hierarchy and delete
            RemoveChild(btn);
            delete btn;

            // Reposition remaining buttons (relative to taskbar)
            for (Int32 k = Int32(0); static_cast<int>(k) < _windowButtons.Length(); k += 1) {
                Int32 buttonX = Int32(WINDOW_BUTTON_START_X + static_cast<int>(k) *
                              (WINDOW_BUTTON_WIDTH + WINDOW_BUTTON_SPACING));
                _windowButtons[static_cast<int>(k)]->SetBounds(buttonX, Int32(4),
                    Int32(WINDOW_BUTTON_WIDTH), Int32(WINDOW_BUTTON_HEIGHT));
            }

            Invalidate();
            break;
        }
    }
}

void TaskBar::RefreshWindowButtons() {
    if (!_desktop) return;

    Window* focused = _desktop->FocusedWindow();
    for (Int32 i = Int32(0); static_cast<int>(i) < _windowButtons.Length(); i += 1) {
        TaskBarButton* btn = _windowButtons[static_cast<int>(i)];
        btn->SetPressed(Boolean(btn->GetWindow() == focused));
    }
    Invalidate();
}

TaskBarButton* TaskBar::FindButtonForWindow(Window* window) const {
    for (Int32 i = Int32(0); static_cast<int>(i) < _windowButtons.Length(); i += 1) {
        if (_windowButtons[static_cast<int>(i)]->GetWindow() == window) {
            return _windowButtons[static_cast<int>(i)];
        }
    }
    return nullptr;
}

void TaskBar::OnPaint(PaintEventArgs& e) {
    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 sw = screen.width;

    // Draw taskbar background (gray)
    e.graphics->FillRectangle(screen, Color::Gray);

    // Draw white highlight line at top
    e.graphics->DrawLine(sx, sy, Int32(static_cast<int>(sx) + static_cast<int>(sw) - 1), sy, Color::White);

    // Paint children (Start button, window buttons, etc.)
    OnPaintClient(e);
}

/******************************************************************************/
/*    Button Implementation                                                   */
/******************************************************************************/

Button::Button(Control* parent, const Rectangle& bounds)
    : Control(parent, bounds)
    , _isToggled(false)
    , _isMouseDown(false)
    , _wasMouseDown(false)
    , _onClick(nullptr)
    , _onClickUserData(nullptr)
    , _text()
    , _font(Font::SystemFont()) {
}

Button::~Button() {
}

void Button::SetOnClick(ClickEventHandler handler, void* userData) {
    _onClick = handler;
    _onClickUserData = userData;
}

void Button::OnPaint(PaintEventArgs& e) {
    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 sw = screen.width;
    Int32 sh = screen.height;

    // Draw with appropriate border style based on pressed state
    // Visual state is toggled OR mouse down
    Boolean visualPressed = Boolean(_isToggled || _isMouseDown);
    if (static_cast<bool>(visualPressed)) {
        e.graphics->FillRectangle(screen, BorderStyle::SunkenDouble);
    } else {
        e.graphics->FillRectangle(screen, BorderStyle::RaisedDouble);
    }

    // Draw button text (centered)
    if (_text.Length() > Int32(0) && static_cast<bool>(_font.IsValid())) {
        Drawing::Size textSize = _font.MeasureString(_text);
        Int32 textW = textSize.width;
        Int32 textH = textSize.height;
        Int32 textX = Int32(static_cast<int>(sx) + (static_cast<int>(sw) - static_cast<int>(textW)) / 2);
        Int32 textY = Int32(static_cast<int>(sy) + (static_cast<int>(sh) - static_cast<int>(textH)) / 2);
        // Offset by 1 pixel when pressed for 3D effect
        if (static_cast<bool>(visualPressed)) {
            textX = Int32(static_cast<int>(textX) + 1);
            textY = Int32(static_cast<int>(textY) + 1);
        }
        e.graphics->DrawString(_text, _font, Color::Black, textX, textY);
    }

    // Paint children
    OnPaintClient(e);
}

void Button::OnMouse(MouseEventArgs& e) {
    Boolean wasVisuallyPressed = Boolean(_isToggled || _isMouseDown);
    Int32 ex = e.x;
    Int32 ey = e.y;
    Boolean isOver = HitTest(ex, ey);
    Boolean leftDown = e.leftButton;

    // Only track mouse-down state, don't affect toggle state
    _isMouseDown = static_cast<bool>(leftDown) && static_cast<bool>(isOver);

    // Detect click: was pressed, now released while still over button
    if (_wasMouseDown && !static_cast<bool>(leftDown) && static_cast<bool>(isOver)) {
        // Fire click event
        if (_onClick) {
            _onClick(this, _onClickUserData);
        }
    }

    _wasMouseDown = static_cast<bool>(leftDown) && static_cast<bool>(isOver);

    Boolean nowVisuallyPressed = Boolean(_isToggled || _isMouseDown);
    if (static_cast<bool>(nowVisuallyPressed) != static_cast<bool>(wasVisuallyPressed)) {
        Invalidate();
    }
}

MeasureResult Button::GetPreferredSize() const {
    // Buttons use their initial bounds size as preferred size
    return MeasureResult(_bounds.width, _bounds.height);
}

/******************************************************************************/
/*    Picture Implementation                                                  */
/******************************************************************************/

Picture::Picture(Control* parent, const Rectangle& bounds)
    : Control(parent, bounds)
    , _image() {
}

Picture::Picture(Control* parent, const Rectangle& bounds, const Image& image)
    : Control(parent, bounds)
    , _image(image) {
}

Picture::~Picture() {
}

void Picture::SetImage(const Image& image) {
    _image = image;
    Invalidate();
}

void Picture::OnPaint(PaintEventArgs& e) {
    Int32 iw = _image.Width();
    Int32 ih = _image.Height();

    if (iw > Int32(0) && ih > Int32(0)) {
        Rectangle screen = ScreenBounds();
        Rectangle visible = VisibleBounds();

        Int32 vw = visible.width;
        Int32 vh = visible.height;

        // Check if anything is visible
        if (vw <= Int32(0) || vh <= Int32(0)) {
            return;
        }

        Int32 sx = screen.x;
        Int32 sy = screen.y;
        Int32 vx = visible.x;
        Int32 vy = visible.y;

        // Calculate which part of the image to draw
        Int32 srcX = Int32(static_cast<int>(vx) - static_cast<int>(sx));
        Int32 srcY = Int32(static_cast<int>(vy) - static_cast<int>(sy));
        Int32 drawWidth = vw;
        Int32 drawHeight = vh;

        // Clamp to image dimensions
        if (Int32(static_cast<int>(srcX) + static_cast<int>(drawWidth)) > iw) {
            drawWidth = Int32(static_cast<int>(iw) - static_cast<int>(srcX));
        }
        if (Int32(static_cast<int>(srcY) + static_cast<int>(drawHeight)) > ih) {
            drawHeight = Int32(static_cast<int>(ih) - static_cast<int>(srcY));
        }

        if (drawWidth > Int32(0) && drawHeight > Int32(0) && srcX >= Int32(0) && srcY >= Int32(0)) {
            Image region = _image.GetRegion(srcX, srcY, drawWidth, drawHeight);
            e.graphics->DrawImage(region, vx, vy);
        }
    }

    // Paint children
    OnPaintClient(e);
}

MeasureResult Picture::GetPreferredSize() const {
    // Pictures prefer their image dimensions if available
    Int32 w = _image.Width();
    Int32 h = _image.Height();

    // Fall back to bounds if no image
    if (w <= Int32(0) || h <= Int32(0)) {
        w = _bounds.width;
        h = _bounds.height;
    }

    return MeasureResult(w, h);
}

/******************************************************************************/
/*    SpectrumControl Implementation                                          */
/******************************************************************************/

SpectrumControl::SpectrumControl(Control* parent, const Rectangle& bounds,
                                  const Color& baseColor)
    : Control(parent, bounds)
    , _baseColor(baseColor)
    , _gradient() {
    RegenerateGradient();
}

SpectrumControl::~SpectrumControl() {
}

void SpectrumControl::SetBaseColor(const Color& color) {
    _baseColor = color;
    RegenerateGradient();
    Invalidate();
}

void SpectrumControl::RegenerateGradient() {
    Int32 w = _bounds.width;
    Int32 h = _bounds.height;

    if (w <= Int32(0) || h <= Int32(0)) return;

    _gradient = Image(w, h);

    // Generate gradient: white at top, base color in middle, black at bottom
    Int32 midY = Int32(static_cast<int>(h) / 2);

    for (Int32 y = Int32(0); y < h; y += 1) {
        Color lineColor;

        if (y <= midY) {
            // Top half: white to base color
            float t = (static_cast<int>(midY) > 0) ? static_cast<float>(static_cast<int>(y)) / static_cast<int>(midY) : 0.0f;
            lineColor = Color::Lerp(Color::White, _baseColor, t);
        } else {
            // Bottom half: base color to black
            float t = (static_cast<int>(h) - 1 - static_cast<int>(midY) > 0) ?
                      static_cast<float>(static_cast<int>(y) - static_cast<int>(midY)) / (static_cast<int>(h) - 1 - static_cast<int>(midY)) : 0.0f;
            lineColor = Color::Lerp(_baseColor, Color::Black, t);
        }

        // Fill entire row with this color
        for (Int32 x = Int32(0); x < w; x += 1) {
            _gradient.SetPixel(x, y, lineColor);
        }
    }
}

Color SpectrumControl::GetColorAtY(Int32 y) const {
    Int32 yi = y;
    Int32 h = _bounds.height;

    if (yi < Int32(0)) yi = Int32(0);
    if (yi >= h) yi = Int32(static_cast<int>(h) - 1);
    if (h <= Int32(0)) return _baseColor;

    Int32 midY = Int32(static_cast<int>(h) / 2);

    if (yi <= midY) {
        float t = (static_cast<int>(midY) > 0) ? static_cast<float>(static_cast<int>(yi)) / static_cast<int>(midY) : 0.0f;
        return Color::Lerp(Color::White, _baseColor, t);
    } else {
        float t = (static_cast<int>(h) - 1 - static_cast<int>(midY) > 0) ?
                  static_cast<float>(static_cast<int>(yi) - static_cast<int>(midY)) / (static_cast<int>(h) - 1 - static_cast<int>(midY)) : 0.0f;
        return Color::Lerp(_baseColor, Color::Black, t);
    }
}

void SpectrumControl::OnPaint(PaintEventArgs& e) {
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 gw = _gradient.Width();
    Int32 gh = _gradient.Height();

    // Copy gradient directly to framebuffer image
    // The buffer writer will handle dithering if needed for VGA modes
    Image& img = fb->GetImage();
    if (gw > Int32(0) && gh > Int32(0)) {
        img.CopyFrom(_gradient, sx, sy);
    }

    // Paint children (if any)
    OnPaintClient(e);
}

/******************************************************************************/
/*    TaskBarButton Implementation                                            */
/******************************************************************************/

void TaskBarButton::OnTaskBarButtonClick(Button* sender, void* userData) {
    TaskBarButton* btn = static_cast<TaskBarButton*>(sender);
    (void)userData;

    if (btn && btn->_window) {
        // Find the desktop through the control hierarchy
        Control* parent = btn->Parent();
        while (parent) {
            if (parent->GetControlType() == ControlType::TaskBar) {
                TaskBar* taskBar = parent->AsTaskBar();
                if (taskBar && taskBar->GetDesktop()) {
                    taskBar->GetDesktop()->SetFocusedWindow(btn->_window);
                }
                break;
            }
            parent = parent->Parent();
        }
    }
}

TaskBarButton::TaskBarButton(Control* parent, const Rectangle& bounds, Window* window)
    : Button(parent, bounds)
    , _window(window) {
    SetOnClick(OnTaskBarButtonClick, nullptr);
}

TaskBarButton::~TaskBarButton() {
}

void TaskBarButton::OnPaint(PaintEventArgs& e) {
    Rectangle screen = ScreenBounds();
    Int32 x = screen.x;
    Int32 y = screen.y;
    Int32 w = screen.width;
    Int32 h = screen.height;

    // Check if button is visually pressed (toggled OR mouse down)
    Boolean visualPressed = IsPressed();

    if (static_cast<bool>(visualPressed)) {
        // Pressed state: use checkerboard hatch pattern (Windows 95 style)
        // Fill with hatch pattern: 50% checkerboard with Gray and White
        e.graphics->FillRectangle(x, y, w, h,
            HatchStyle::Percent50, Color::Gray, Color::White);

        // Draw sunken border on top of hatch pattern
        // Outer border: Black top/left, White bottom/right
        e.graphics->DrawLine(x, y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Color::Black);           // Top outer
        e.graphics->DrawLine(x, y, x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::Black);           // Left outer
        e.graphics->DrawLine(Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::White);   // Right outer
        e.graphics->DrawLine(x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::White);   // Bottom outer
        // Inner border: DarkGray top/left, Gray bottom/right
        e.graphics->DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + 1), Color::DarkGray);    // Top inner
        e.graphics->DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::DarkGray);    // Left inner
        e.graphics->DrawLine(Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::Gray);     // Right inner
        e.graphics->DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::Gray);     // Bottom inner
    } else {
        // Normal state: use standard raised button style
        e.graphics->FillRectangle(screen, BorderStyle::RaisedDouble);
    }

    // Draw window title text (left-aligned with padding, vertically centered)
    if (_window) {
        const String& title = _window->Title();
        const Font& font = _window->GetFont();
        if (title.Length() > Int32(0) && static_cast<bool>(font.IsValid())) {
            Int32 textX = Int32(static_cast<int>(x) + 4);  // Left padding
            Int32 textY = Int32(static_cast<int>(y) + (static_cast<int>(h) - static_cast<int>(font.Height())) / 2);
            // Offset by 1 pixel when pressed for 3D effect
            if (static_cast<bool>(visualPressed)) {
                textX = Int32(static_cast<int>(textX) + 1);
                textY = Int32(static_cast<int>(textY) + 1);
            }
            e.graphics->DrawString(title, font, Color::Black, textX, textY);
        }
    }

    // Paint children
    OnPaintClient(e);
}

/******************************************************************************/
/*    MenuItem Implementation                                                 */
/******************************************************************************/

MenuItem::MenuItem(Control* parent, const Rectangle& bounds, int itemIndex)
    : Control(parent, bounds)
    , _icon()
    , _isHighlighted(false)
    , _onClick(nullptr)
    , _onClickUserData(nullptr)
    , _itemIndex(itemIndex) {
}

MenuItem::~MenuItem() {
}

void MenuItem::SetIcon(const Image& icon) {
    _icon = icon;
    Invalidate();
}

void MenuItem::SetOnClick(ClickEventHandler handler, void* userData) {
    _onClick = handler;
    _onClickUserData = userData;
}

void MenuItem::OnPaint(PaintEventArgs& e) {
    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 sh = screen.height;

    // Draw background
    Color bgColor = _isHighlighted ? Color::DarkBlue : Color::Gray;
    e.graphics->FillRectangle(screen, bgColor);

    // Draw icon if present
    Int32 iw = _icon.Width();
    Int32 ih = _icon.Height();
    if (iw > Int32(0) && ih > Int32(0)) {
        // Center icon vertically in item
        Int32 iconY = Int32(static_cast<int>(sy) + (static_cast<int>(sh) - static_cast<int>(ih)) / 2);
        Int32 iconX = Int32(static_cast<int>(sx) + ICON_MARGIN);

        GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
        if (fb) {
            Image& fbImg = fb->GetImage();
            fbImg.CopyFromWithAlpha(_icon, static_cast<int>(iconX), static_cast<int>(iconY));
        }
    }

    // Paint children
    OnPaintClient(e);
}

void MenuItem::OnMouse(MouseEventArgs& e) {
    Int32 ex = e.x;
    Int32 ey = e.y;
    Boolean isOver = HitTest(ex, ey);
    Boolean leftDown = e.leftButton;

    Boolean wasHighlighted = Boolean(_isHighlighted);
    _isHighlighted = static_cast<bool>(isOver);

    // Fire click on release while over
    static bool wasPressed = false;
    if (wasPressed && !static_cast<bool>(leftDown) && static_cast<bool>(isOver) && _onClick) {
        _onClick(reinterpret_cast<Button*>(this), _onClickUserData);
    }
    wasPressed = static_cast<bool>(leftDown) && static_cast<bool>(isOver);

    if (_isHighlighted != static_cast<bool>(wasHighlighted)) {
        Invalidate();
    }
}

/******************************************************************************/
/*    StartMenu Implementation                                                */
/******************************************************************************/

StartMenu::StartMenu(Desktop* desktop)
    : Control()  // No parent yet - will be added to desktop separately
    , _desktop(desktop)
    , _isVisible(false)
    , _items() {
    // Get screen dimensions
    Display current = Display::GetCurrent();
    Int32 screenHeight = Int32(static_cast<int>(current.Height()));
    Int32 taskBarHeight = Int32(28);

    // Calculate menu height and position
    Int32 menuHeight = Int32(ITEM_COUNT * ITEM_HEIGHT + 4);  // +4 for border
    Int32 menuX = Int32(0);
    Int32 menuY = Int32(static_cast<int>(screenHeight) - static_cast<int>(taskBarHeight) - static_cast<int>(menuHeight));

    // Set bounds (positioned above taskbar, aligned with Start button)
    _bounds = Rectangle(menuX, menuY, Int32(MENU_WIDTH), menuHeight);
    _clientBounds = Rectangle(Int32(SIDEBAR_WIDTH), Int32(2), Int32(MENU_WIDTH - SIDEBAR_WIDTH - 2), Int32(static_cast<int>(menuHeight) - 4));

    // Configure layout: column direction for menu items
    _layout.direction = FlexDirection::Column;
    _layout.alignItems = AlignItems::Stretch;
    _layout.gap = Int32(0);
    _layout.paddingLeft = Int32(SIDEBAR_WIDTH);
    _layout.paddingTop = Int32(2);
    _layout.paddingRight = Int32(2);
    _layout.paddingBottom = Int32(2);

    // Create menu items with RELATIVE positioning (to parent's client area)
    _items.Resize(ITEM_COUNT);
    for (Int32 i = Int32(0); static_cast<int>(i) < ITEM_COUNT; i += 1) {
        // Items positioned relative to client area (after sidebar)
        Int32 itemY = Int32(static_cast<int>(i) * ITEM_HEIGHT);  // Relative Y position within client
        Int32 itemX = Int32(0);                // Relative X position within client
        _items[static_cast<int>(i)] = new MenuItem(this, Rectangle(itemX, itemY,
            Int32(MENU_WIDTH - SIDEBAR_WIDTH - 2), Int32(ITEM_HEIGHT)), static_cast<int>(i));

        // Menu items have fixed height
        _items[static_cast<int>(i)]->Layout().heightMode = SizeMode::Fixed;
        _items[static_cast<int>(i)]->Layout().widthMode = SizeMode::Fill;
    }

    // Load icons
    LoadIcons();

    // Add to desktop (but hidden)
    if (desktop) {
        // Don't use AddChild - just set parent directly to avoid spatial grid
        _parent = desktop;
    }
}

StartMenu::~StartMenu() {
    // MenuItems are children, deleted by Control destructor
}

void StartMenu::LoadIcons() {
    // Map menu items to system icons by name
    static const char* iconNames[ITEM_COUNT] = {
        Drawing::SystemIcons::FolderApps,     // Programs
        Drawing::SystemIcons::FolderDocs,     // Documents
        Drawing::SystemIcons::DisplaySettings1,// Settings
        Drawing::SystemIcons::FolderOpen,     // Find
        Drawing::SystemIcons::DialogInfo1,    // Help
        Drawing::SystemIcons::AppWindos,      // Run
        Drawing::SystemIcons::FolderLibrary,  // Favorites
        Drawing::SystemIcons::FolderOpenFiles,// Recent
        Drawing::SystemIcons::Computer,       // My Computer
        Drawing::SystemIcons::ComputerNet,    // Network
        Drawing::SystemIcons::DialogWarning1, // Shut Down
        Drawing::SystemIcons::DialogQuestion1 // Log Off
    };

    for (Int32 i = Int32(0); static_cast<int>(i) < ITEM_COUNT && static_cast<int>(i) < _items.Length(); i += 1) {
        try {
            Image icon = Drawing::SystemIcons::Load(iconNames[static_cast<int>(i)], Size::IconSmall);
            _items[static_cast<int>(i)]->SetIcon(icon);
        } catch (...) {
            // Icon loading failed - item will show without icon
        }
    }
}

void StartMenu::Show() {
    _isVisible = true;
    // Update Start button to show pressed state
    if (_desktop) {
        TaskBar* taskBar = _desktop->GetTaskBar();
        if (taskBar && taskBar->StartButton()) {
            taskBar->StartButton()->SetPressed(Boolean(true));
        }
    }
    Invalidate();
}

void StartMenu::Hide() {
    _isVisible = false;
    // Update Start button to show normal state
    if (_desktop) {
        TaskBar* taskBar = _desktop->GetTaskBar();
        if (taskBar && taskBar->StartButton()) {
            taskBar->StartButton()->SetPressed(Boolean(false));
        }
    }
    Invalidate();
}

void StartMenu::Toggle() {
    if (_isVisible) {
        Hide();
    } else {
        Show();
    }
}

void StartMenu::OnPaint(PaintEventArgs& e) {
    if (!_isVisible) return;

    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 sh = screen.height;

    // Draw raised border
    e.graphics->FillRectangle(screen, BorderStyle::RaisedDouble);

    // Draw blue sidebar on left (Windows 95 style)
    Rectangle sidebar(Int32(static_cast<int>(sx) + 2), Int32(static_cast<int>(sy) + 2),
                      Int32(SIDEBAR_WIDTH - 2), Int32(static_cast<int>(sh) - 4));
    e.graphics->FillRectangle(sidebar, Color::DarkBlue);

    // Paint menu items
    OnPaintClient(e);
}

void StartMenu::OnMouse(MouseEventArgs& e) {
    // Propagate mouse events to ALL menu items so they can update highlight state
    // (not just the one under cursor, so items can un-highlight when mouse leaves)
    for (Int32 i = Int32(0); static_cast<int>(i) < _items.Length(); i += 1) {
        if (_items[static_cast<int>(i)]) {
            _items[static_cast<int>(i)]->OnMouse(e);
        }
    }
}

}}} // namespace System::Windows::Forms
