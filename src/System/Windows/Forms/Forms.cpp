#include "Forms.hpp"

namespace System { namespace Windows { namespace Forms {

/******************************************************************************/
/*    SpatialGrid Implementation                                              */
/******************************************************************************/

SpatialGrid::SpatialGrid()
    : _cellsX(0)
    , _cellsY(0)
    , _screenWidth(0)
    , _screenHeight(0) {
}

void SpatialGrid::Initialize(Int32 screenWidth, Int32 screenHeight) {
    _screenWidth = static_cast<int>(screenWidth);
    _screenHeight = static_cast<int>(screenHeight);
    _cellsX = (_screenWidth + CELL_SIZE - 1) / CELL_SIZE;
    _cellsY = (_screenHeight + CELL_SIZE - 1) / CELL_SIZE;

    // Clamp to max
    if (_cellsX > MAX_CELLS_X) _cellsX = MAX_CELLS_X;
    if (_cellsY > MAX_CELLS_Y) _cellsY = MAX_CELLS_Y;

    Clear();
}

void SpatialGrid::Clear() {
    for (int y = 0; y < MAX_CELLS_Y; y++) {
        for (int x = 0; x < MAX_CELLS_X; x++) {
            _cells[y][x].count = 0;
            for (int i = 0; i < MAX_CONTROLS_PER_CELL; i++) {
                _cells[y][x].controls[i] = nullptr;
            }
        }
    }
}

void SpatialGrid::GetCellIndex(int x, int y, int& cellX, int& cellY) const {
    cellX = x / CELL_SIZE;
    cellY = y / CELL_SIZE;
    if (cellX < 0) cellX = 0;
    if (cellY < 0) cellY = 0;
    if (cellX >= _cellsX) cellX = _cellsX - 1;
    if (cellY >= _cellsY) cellY = _cellsY - 1;
}

void SpatialGrid::GetCellRange(const Rectangle& bounds, int& minX, int& minY,
                               int& maxX, int& maxY) const {
    int bx = static_cast<int>(bounds.x);
    int by = static_cast<int>(bounds.y);
    int bw = static_cast<int>(bounds.width);
    int bh = static_cast<int>(bounds.height);

    GetCellIndex(bx, by, minX, minY);
    GetCellIndex(bx + bw - 1, by + bh - 1, maxX, maxY);
}

void SpatialGrid::Insert(Control* control, const Rectangle& bounds) {
    if (!control) return;

    int minX, minY, maxX, maxY;
    GetCellRange(bounds, minX, minY, maxX, maxY);

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            Cell& cell = _cells[y][x];
            if (cell.count < MAX_CONTROLS_PER_CELL) {
                // Check if already in cell
                bool found = false;
                for (int i = 0; i < cell.count; i++) {
                    if (cell.controls[i] == control) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    cell.controls[cell.count++] = control;
                }
            }
        }
    }
}

void SpatialGrid::Remove(Control* control) {
    if (!control) return;

    for (int y = 0; y < _cellsY; y++) {
        for (int x = 0; x < _cellsX; x++) {
            Cell& cell = _cells[y][x];
            for (int i = 0; i < cell.count; i++) {
                if (cell.controls[i] == control) {
                    // Shift remaining controls down
                    for (int j = i; j < cell.count - 1; j++) {
                        cell.controls[j] = cell.controls[j + 1];
                    }
                    cell.controls[--cell.count] = nullptr;
                    break;
                }
            }
        }
    }
}

Control* SpatialGrid::HitTest(Int32 x, Int32 y) const {
    int px = static_cast<int>(x);
    int py = static_cast<int>(y);

    if (px < 0 || py < 0 || px >= _screenWidth || py >= _screenHeight) {
        return nullptr;
    }

    int cellX, cellY;
    GetCellIndex(px, py, cellX, cellY);

    const Cell& cell = _cells[cellY][cellX];

    // Check controls in reverse order (last added = highest z-order)
    for (int i = cell.count - 1; i >= 0; i--) {
        Control* ctrl = cell.controls[i];
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
    for (int i = 0; i < _children.Length(); i++) {
        delete _children[i];
    }
}

void Control::UpdateClientBounds() {
    // By default, client bounds is the full control area (no decoration)
    // Client bounds are relative to the control's origin (0,0)
    _clientBounds = Rectangle(0, 0, _bounds.width, _bounds.height);
}

Control* Control::GetChild(Int32 index) const {
    int idx = static_cast<int>(index);
    if (idx >= 0 && idx < _children.Length()) {
        return _children[idx];
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
    int px = static_cast<int>(parentClient.x);
    int py = static_cast<int>(parentClient.y);
    int bx = static_cast<int>(_bounds.x);
    int by = static_cast<int>(_bounds.y);
    int bw = static_cast<int>(_bounds.width);
    int bh = static_cast<int>(_bounds.height);
    return Rectangle(px + bx, py + by, bw, bh);
}

Rectangle Control::ScreenClientBounds() const {
    Rectangle screenBounds = ScreenBounds();
    int sx = static_cast<int>(screenBounds.x);
    int sy = static_cast<int>(screenBounds.y);
    int cx = static_cast<int>(_clientBounds.x);
    int cy = static_cast<int>(_clientBounds.y);
    int cw = static_cast<int>(_clientBounds.width);
    int ch = static_cast<int>(_clientBounds.height);
    return Rectangle(sx + cx, sy + cy, cw, ch);
}

Rectangle Control::VisibleBounds() const {
    Rectangle screen = ScreenBounds();
    int sx = static_cast<int>(screen.x);
    int sy = static_cast<int>(screen.y);
    int sw = static_cast<int>(screen.width);
    int sh = static_cast<int>(screen.height);

    // Get screen dimensions from current display mode
    Display current = Display::GetCurrent();
    int screenWidth = static_cast<int>(current.Width());
    int screenHeight = static_cast<int>(current.Height());

    if (!_parent) {
        // Root control - clip to screen
        int left = sx < 0 ? 0 : sx;
        int top = sy < 0 ? 0 : sy;
        int right = sx + sw > screenWidth ? screenWidth : sx + sw;
        int bottom = sy + sh > screenHeight ? screenHeight : sy + sh;
        return Rectangle(left, top, right - left, bottom - top);
    }

    // Clip to parent's client area
    Rectangle parentClient = _parent->ScreenClientBounds();
    int pcx = static_cast<int>(parentClient.x);
    int pcy = static_cast<int>(parentClient.y);
    int pcw = static_cast<int>(parentClient.width);
    int pch = static_cast<int>(parentClient.height);

    int left = sx < pcx ? pcx : sx;
    int top = sy < pcy ? pcy : sy;
    int right = sx + sw;
    int bottom = sy + sh;

    int parentRight = pcx + pcw;
    int parentBottom = pcy + pch;

    if (right > parentRight) right = parentRight;
    if (bottom > parentBottom) bottom = parentBottom;

    // Also clip to screen bounds
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right > screenWidth) right = screenWidth;
    if (bottom > screenHeight) bottom = screenHeight;

    if (right <= left || bottom <= top) {
        return Rectangle(0, 0, 0, 0);  // Not visible
    }

    return Rectangle(left, top, right - left, bottom - top);
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
        int oldLen = _children.Length();
        _children.Resize(oldLen + 1);
        _children[oldLen] = child;
        child->_parent = this;
        Invalidate();
    }
}

void Control::RemoveChild(Control* child) {
    if (child) {
        int index = _children.IndexOf(child);
        if (index >= 0) {
            // Shift elements down
            for (int i = index; i < _children.Length() - 1; i++) {
                _children[i] = _children[i + 1];
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
    for (int i = 0; i < _children.Length(); i++) {
        Control* child = _children[i];
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
    int ex = static_cast<int>(e.x);
    int ey = static_cast<int>(e.y);
    // First check if any child contains the point (in reverse order for z-order)
    for (int i = _children.Length() - 1; i >= 0; i--) {
        Control* child = _children[i];
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
    for (int i = 0; i < _children.Length(); i++) {
        Control* child = _children[i];
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
    int w = static_cast<int>(_bounds.width);
    int h = static_cast<int>(_bounds.height);

    // Apply min constraints
    int minW = static_cast<int>(_layout.minWidth);
    int minH = static_cast<int>(_layout.minHeight);
    if (w < minW) w = minW;
    if (h < minH) h = minH;

    return MeasureResult(Int32(w), Int32(h));
}

MeasureResult Control::Measure(Int32 availableWidth, Int32 availableHeight) {
    int avW = static_cast<int>(availableWidth);
    int avH = static_cast<int>(availableHeight);

    // Subtract margins from available space
    int marginH = static_cast<int>(_layout.marginLeft) + static_cast<int>(_layout.marginRight);
    int marginV = static_cast<int>(_layout.marginTop) + static_cast<int>(_layout.marginBottom);
    avW -= marginH;
    avH -= marginV;
    if (avW < 0) avW = 0;
    if (avH < 0) avH = 0;

    int resultW = 0;
    int resultH = 0;

    // Handle different size modes
    if (_layout.widthMode == SizeMode::Fixed) {
        resultW = static_cast<int>(_bounds.width);
    } else if (_layout.widthMode == SizeMode::Fill) {
        resultW = avW;
    }
    // Auto mode: calculate from content/children

    if (_layout.heightMode == SizeMode::Fixed) {
        resultH = static_cast<int>(_bounds.height);
    } else if (_layout.heightMode == SizeMode::Fill) {
        resultH = avH;
    }
    // Auto mode: calculate from content/children

    // For Auto mode, measure based on children
    if (_layout.widthMode == SizeMode::Auto || _layout.heightMode == SizeMode::Auto) {
        // Get padding
        int padL = static_cast<int>(_layout.paddingLeft);
        int padR = static_cast<int>(_layout.paddingRight);
        int padT = static_cast<int>(_layout.paddingTop);
        int padB = static_cast<int>(_layout.paddingBottom);

        int contentW = 0;
        int contentH = 0;
        int gap = static_cast<int>(_layout.gap);

        // Count participating children
        int participatingCount = 0;
        for (int i = 0; i < _children.Length(); i++) {
            Control* child = _children[i];
            if (child && child->_layout.participatesInLayout) {
                participatingCount++;
            }
        }

        // Measure children
        bool isRow = (_layout.direction == FlexDirection::Row);

        for (int i = 0; i < _children.Length(); i++) {
            Control* child = _children[i];
            if (!child || !child->_layout.participatesInLayout) continue;

            // Measure child
            MeasureResult childSize = child->Measure(
                Int32(avW - padL - padR),
                Int32(avH - padT - padB)
            );

            int cw = static_cast<int>(childSize.preferredWidth);
            int ch = static_cast<int>(childSize.preferredHeight);

            // Add child margins
            cw += static_cast<int>(child->_layout.marginLeft) +
                  static_cast<int>(child->_layout.marginRight);
            ch += static_cast<int>(child->_layout.marginTop) +
                  static_cast<int>(child->_layout.marginBottom);

            if (isRow) {
                // Row: width accumulates, height is max
                contentW += cw;
                if (ch > contentH) contentH = ch;
            } else {
                // Column: height accumulates, width is max
                contentH += ch;
                if (cw > contentW) contentW = cw;
            }
        }

        // Add gaps between children
        if (participatingCount > 1) {
            if (isRow) {
                contentW += gap * (participatingCount - 1);
            } else {
                contentH += gap * (participatingCount - 1);
            }
        }

        // Add padding
        contentW += padL + padR;
        contentH += padT + padB;

        // Use content size for Auto modes
        if (_layout.widthMode == SizeMode::Auto) {
            resultW = contentW;
        }
        if (_layout.heightMode == SizeMode::Auto) {
            resultH = contentH;
        }
    }

    // Apply self preferred size if no children
    if (resultW == 0 && resultH == 0) {
        MeasureResult pref = GetPreferredSize();
        if (_layout.widthMode == SizeMode::Auto) {
            resultW = static_cast<int>(pref.preferredWidth);
        }
        if (_layout.heightMode == SizeMode::Auto) {
            resultH = static_cast<int>(pref.preferredHeight);
        }
    }

    // Apply min/max constraints
    int minW = static_cast<int>(_layout.minWidth);
    int minH = static_cast<int>(_layout.minHeight);
    int maxW = static_cast<int>(_layout.maxWidth);
    int maxH = static_cast<int>(_layout.maxHeight);

    if (resultW < minW) resultW = minW;
    if (resultH < minH) resultH = minH;
    if (resultW > maxW) resultW = maxW;
    if (resultH > maxH) resultH = maxH;

    // Store and return result
    _measuredSize = MeasureResult(Int32(resultW), Int32(resultH));
    return _measuredSize;
}

void Control::Arrange(const Rectangle& finalBounds) {
    // Update our bounds
    _bounds = finalBounds;
    UpdateClientBounds();

    // Calculate content area (bounds minus padding)
    int padL = static_cast<int>(_layout.paddingLeft);
    int padR = static_cast<int>(_layout.paddingRight);
    int padT = static_cast<int>(_layout.paddingTop);
    int padB = static_cast<int>(_layout.paddingBottom);

    int cx = static_cast<int>(_clientBounds.x) + padL;
    int cy = static_cast<int>(_clientBounds.y) + padT;
    int cw = static_cast<int>(_clientBounds.width) - padL - padR;
    int ch = static_cast<int>(_clientBounds.height) - padT - padB;

    if (cw < 0) cw = 0;
    if (ch < 0) ch = 0;

    Rectangle contentArea(cx, cy, cw, ch);

    // Arrange children using flex layout
    ArrangeFlexChildren(contentArea);

    // Mark layout as clean
    _layout.needsLayout = false;
}

void Control::ArrangeFlexChildren(const Rectangle& contentArea) {
    int cx = static_cast<int>(contentArea.x);
    int cy = static_cast<int>(contentArea.y);
    int cw = static_cast<int>(contentArea.width);
    int ch = static_cast<int>(contentArea.height);
    int gap = static_cast<int>(_layout.gap);
    bool isRow = (_layout.direction == FlexDirection::Row);

    // First pass: gather info about participating children
    int participatingCount = 0;
    int totalMainSize = 0;
    int totalFlexGrow = 0;
    int maxCrossSize = 0;

    for (int i = 0; i < _children.Length(); i++) {
        Control* child = _children[i];
        if (!child || !child->_layout.participatesInLayout) continue;

        participatingCount++;

        // Get measured size
        int childW = static_cast<int>(child->_measuredSize.preferredWidth);
        int childH = static_cast<int>(child->_measuredSize.preferredHeight);

        // Add margins to size
        int marginH = static_cast<int>(child->_layout.marginLeft) +
                      static_cast<int>(child->_layout.marginRight);
        int marginV = static_cast<int>(child->_layout.marginTop) +
                      static_cast<int>(child->_layout.marginBottom);

        if (isRow) {
            totalMainSize += childW + marginH;
            if (childH + marginV > maxCrossSize) {
                maxCrossSize = childH + marginV;
            }
        } else {
            totalMainSize += childH + marginV;
            if (childW + marginH > maxCrossSize) {
                maxCrossSize = childW + marginH;
            }
        }

        totalFlexGrow += static_cast<int>(child->_layout.flexGrow);
    }

    // Add gaps
    if (participatingCount > 1) {
        totalMainSize += gap * (participatingCount - 1);
    }

    if (participatingCount == 0) return;

    // Calculate available space for distribution
    int mainAxisSize = isRow ? cw : ch;
    int crossAxisSize = isRow ? ch : cw;
    int extraSpace = mainAxisSize - totalMainSize;
    if (extraSpace < 0) extraSpace = 0;

    // Calculate starting position based on JustifyContent
    int mainPos = 0;
    int spaceBetween = 0;
    int spaceAround = 0;

    switch (_layout.justifyContent) {
        case JustifyContent::Start:
            mainPos = 0;
            break;
        case JustifyContent::End:
            mainPos = extraSpace;
            break;
        case JustifyContent::Center:
            mainPos = extraSpace / 2;
            break;
        case JustifyContent::SpaceBetween:
            mainPos = 0;
            if (participatingCount > 1) {
                spaceBetween = extraSpace / (participatingCount - 1);
            }
            break;
        case JustifyContent::SpaceAround:
            if (participatingCount > 0) {
                spaceAround = extraSpace / (participatingCount * 2);
                mainPos = spaceAround;
            }
            break;
    }

    // Second pass: arrange children
    for (int i = 0; i < _children.Length(); i++) {
        Control* child = _children[i];
        if (!child || !child->_layout.participatesInLayout) continue;

        // Get measured size
        int childW = static_cast<int>(child->_measuredSize.preferredWidth);
        int childH = static_cast<int>(child->_measuredSize.preferredHeight);

        // Get margins
        int mTop = static_cast<int>(child->_layout.marginTop);
        int mRight = static_cast<int>(child->_layout.marginRight);
        int mBottom = static_cast<int>(child->_layout.marginBottom);
        int mLeft = static_cast<int>(child->_layout.marginLeft);

        // Calculate flex grow contribution
        int flexGrow = static_cast<int>(child->_layout.flexGrow);
        int growAmount = 0;
        if (totalFlexGrow > 0 && flexGrow > 0 && extraSpace > 0) {
            growAmount = (extraSpace * flexGrow) / totalFlexGrow;
        }

        // Calculate final size
        int finalW = childW;
        int finalH = childH;

        if (isRow) {
            finalW += growAmount;
            // Handle AlignItems for cross axis (height)
            switch (_layout.alignItems) {
                case AlignItems::Stretch:
                    finalH = crossAxisSize - mTop - mBottom;
                    break;
                case AlignItems::Start:
                case AlignItems::Center:
                case AlignItems::End:
                    // Keep measured height
                    break;
            }
        } else {
            finalH += growAmount;
            // Handle AlignItems for cross axis (width)
            switch (_layout.alignItems) {
                case AlignItems::Stretch:
                    finalW = crossAxisSize - mLeft - mRight;
                    break;
                case AlignItems::Start:
                case AlignItems::Center:
                case AlignItems::End:
                    // Keep measured width
                    break;
            }
        }

        // Apply min/max constraints
        int minW = static_cast<int>(child->_layout.minWidth);
        int minH = static_cast<int>(child->_layout.minHeight);
        int maxW = static_cast<int>(child->_layout.maxWidth);
        int maxH = static_cast<int>(child->_layout.maxHeight);

        if (finalW < minW) finalW = minW;
        if (finalH < minH) finalH = minH;
        if (finalW > maxW) finalW = maxW;
        if (finalH > maxH) finalH = maxH;

        // Calculate position
        int childX, childY;

        if (isRow) {
            childX = cx + mainPos + mLeft;

            // Cross axis position (Y) based on AlignItems
            switch (_layout.alignItems) {
                case AlignItems::Start:
                    childY = cy + mTop;
                    break;
                case AlignItems::End:
                    childY = cy + crossAxisSize - finalH - mBottom;
                    break;
                case AlignItems::Center:
                    childY = cy + (crossAxisSize - finalH - mTop - mBottom) / 2 + mTop;
                    break;
                case AlignItems::Stretch:
                default:
                    childY = cy + mTop;
                    break;
            }

            // Advance main axis position
            mainPos += finalW + mLeft + mRight + gap + spaceBetween + spaceAround * 2;
        } else {
            childY = cy + mainPos + mTop;

            // Cross axis position (X) based on AlignItems
            switch (_layout.alignItems) {
                case AlignItems::Start:
                    childX = cx + mLeft;
                    break;
                case AlignItems::End:
                    childX = cx + crossAxisSize - finalW - mRight;
                    break;
                case AlignItems::Center:
                    childX = cx + (crossAxisSize - finalW - mLeft - mRight) / 2 + mLeft;
                    break;
                case AlignItems::Stretch:
                default:
                    childX = cx + mLeft;
                    break;
            }

            // Advance main axis position
            mainPos += finalH + mTop + mBottom + gap + spaceBetween + spaceAround * 2;
        }

        // Recursively arrange child
        Rectangle childBounds(childX, childY, finalW, finalH);
        child->Arrange(childBounds);
    }

    // Arrange non-participating children (they keep their current bounds)
    for (int i = 0; i < _children.Length(); i++) {
        Control* child = _children[i];
        if (child && !child->_layout.participatesInLayout) {
            // Just trigger their own layout with current bounds
            child->Arrange(child->_bounds);
        }
    }
}

void Control::PerformLayout() {
    if (!_layout.needsLayout) return;

    // Measure pass (bottom-up)
    int availW = static_cast<int>(_bounds.width);
    int availH = static_cast<int>(_bounds.height);
    Measure(Int32(availW), Int32(availH));

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
    , _dragOffsetX(0)
    , _dragOffsetY(0)
    , _dragStartX(0)
    , _dragStartY(0)
    , _dragBitmap()
    , _cursorImage()
    , _icons()
    , _isDragging(false)
    , _running(false)
    , _cursorX(0)
    , _cursorY(0)
    , _prevCursorX(0)
    , _prevCursorY(0)
    , _cursorVisible(true)
    , _wasMouseDown(false)
    , _cursorSaved(false)
    , _screenWidth(0)
    , _screenHeight(0)
    , _nextIconX(ICON_MARGIN_X)
    , _nextIconY(ICON_MARGIN_Y)
    , _spatialGrid()
    , _taskBar(nullptr)
    , _startMenu(nullptr) {
    // Get screen dimensions from current display mode
    Display current = Display::GetCurrent();
    _screenWidth = static_cast<int>(current.Width());
    _screenHeight = static_cast<int>(current.Height());

    // Initialize spatial grid
    _spatialGrid.Initialize(Int32(_screenWidth), Int32(_screenHeight));

    // Center cursor initially
    _cursorX = _screenWidth / 2;
    _cursorY = _screenHeight / 2;
    _prevCursorX = _cursorX;
    _prevCursorY = _cursorY;

    // Desktop fills the screen
    _bounds = Rectangle(0, 0, _screenWidth, _screenHeight);
    _clientBounds = Rectangle(0, 0, _screenWidth, _screenHeight);
    // Initialize cursor save buffer
    for (int i = 0; i < CURSOR_SIZE * CURSOR_SIZE; i++) {
        _cursorSave[i] = 0;
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
    int taskBarHeight = 28;  // Reserve space for taskbar
    int maxY = _screenHeight - taskBarHeight - ICON_SIZE - ICON_MARGIN_Y;

    // Add the icon at current position
    int oldLen = _icons.Length();
    _icons.Resize(oldLen + 1);
    _icons[oldLen] = DesktopIcon(icon, _nextIconX, _nextIconY);

    // Move to next position (flow down, then right)
    _nextIconY += ICON_SPACING_Y;
    if (_nextIconY > maxY) {
        _nextIconY = ICON_MARGIN_Y;
        _nextIconX += ICON_SPACING_X;
    }

    Invalidate();
}

void Desktop::AddIconFromLibrary(const char* path, Int32 iconIndex) {
    Image icon = Image::FromIconLibrary(path, iconIndex, Size::IconMedium);
    AddIcon(icon);
}

void Desktop::DrawIcons() {
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    Image& img = fb->GetImage();

    for (int i = 0; i < _icons.Length(); i++) {
        const DesktopIcon& icon = _icons[i];
        int iw = static_cast<int>(icon.image.Width());
        int ih = static_cast<int>(icon.image.Height());

        if (iw > 0 && ih > 0) {
            // Draw icon with alpha blending
            img.CopyFromWithAlpha(icon.image, icon.x, icon.y);
        }
    }
}

void Desktop::SetFocusedWindow(Window* window) {
    if (_focusedWindow == window) return;

    // Unfocus previous window
    if (_focusedWindow) {
        _focusedWindow->SetFocused(false);
    }

    _focusedWindow = window;

    // Focus new window and bring to front (move to end of children array)
    if (_focusedWindow) {
        _focusedWindow->SetFocused(true);

        // Find and move to end for z-order
        int idx = _children.IndexOf(static_cast<Control*>(_focusedWindow));
        if (idx >= 0 && idx < _children.Length() - 1) {
            // Shift elements down
            for (int i = idx; i < _children.Length() - 1; i++) {
                _children[i] = _children[i + 1];
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
    for (int i = 0; i < _children.Length(); i++) {
        Control* child = _children[i];
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
    for (int i = 0; i < _children.Length(); i++) {
        Control* child = _children[i];
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
    for (int dy = 0; dy < CURSOR_SIZE; dy++) {
        for (int dx = 0; dx < CURSOR_SIZE; dx++) {
            int px = _cursorX + dx;
            int py = _cursorY + dy;
            if (px >= 0 && px < _screenWidth && py >= 0 && py < _screenHeight) {
                _cursorSave[dy * CURSOR_SIZE + dx] = static_cast<unsigned int>(img.GetPixel(px, py));
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
    for (int dy = 0; dy < CURSOR_SIZE; dy++) {
        for (int dx = 0; dx < CURSOR_SIZE; dx++) {
            int px = _prevCursorX + dx;
            int py = _prevCursorY + dy;
            if (px >= 0 && px < _screenWidth && py >= 0 && py < _screenHeight) {
                img.SetPixel(px, py, Color(_cursorSave[dy * CURSOR_SIZE + dx]));
            }
        }
    }
}

void Desktop::DrawCursor() {
    if (!_cursorVisible) return;

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    Image& img = fb->GetImage();

    int cursorW = static_cast<int>(_cursorImage.Width());
    int cursorH = static_cast<int>(_cursorImage.Height());

    // If we have a cursor image, draw it with alpha blending
    if (cursorW > 0 && cursorH > 0) {
        for (int dy = 0; dy < cursorH && dy < CURSOR_SIZE; dy++) {
            for (int dx = 0; dx < cursorW && dx < CURSOR_SIZE; dx++) {
                int px = _cursorX + dx;
                int py = _cursorY + dy;
                if (px >= 0 && px < _screenWidth && py >= 0 && py < _screenHeight) {
                    Color pixel = _cursorImage.GetPixel(dx, dy);
                    // Only draw non-transparent pixels (alpha >= 128)
                    unsigned char alpha = static_cast<unsigned char>(pixel.A());
                    if (alpha >= 128) {
                        img.SetPixel(px, py, pixel);
                    }
                }
            }
        }
    } else {
        // Fallback: draw a simple arrow cursor
        for (int dy = 0; dy < CURSOR_SIZE; dy++) {
            for (int dx = 0; dx < CURSOR_SIZE; dx++) {
                int px = _cursorX + dx;
                int py = _cursorY + dy;
                if (px >= 0 && px < _screenWidth && py >= 0 && py < _screenHeight) {
                    // Simple arrow shape
                    bool isArrow = (dx <= dy && dx < 12 && dy < 18);
                    bool isBorder = isArrow && (dx == 0 || dx == dy || dy == 17);
                    if (isArrow) {
                        img.SetPixel(px, py, isBorder ? Color::Black : Color::White);
                    }
                }
            }
        }
    }
}

void Desktop::CaptureWindowBitmap(Window* win) {
    if (!win) return;

    Rectangle screen = win->ScreenBounds();
    int sw = static_cast<int>(screen.width);
    int sh = static_cast<int>(screen.height);
    int sx = static_cast<int>(screen.x);
    int sy = static_cast<int>(screen.y);
    _dragBitmap = Image(sw, sh);

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    // Copy window region from frame buffer
    const Image& fbImg = fb->GetImage();
    for (int y = 0; y < sh; y++) {
        for (int x = 0; x < sw; x++) {
            int srcX = sx + x;
            int srcY = sy + y;
            if (srcX >= 0 && srcX < _screenWidth && srcY >= 0 && srcY < _screenHeight) {
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
    int newX = _cursorX - _dragOffsetX;
    int newY = _cursorY - _dragOffsetY;

    int dh = static_cast<int>(_dragBitmap.Height());
    int dw = static_cast<int>(_dragBitmap.Width());

    // Draw the captured bitmap at new position
    for (int y = 0; y < dh; y++) {
        for (int x = 0; x < dw; x++) {
            int dstX = newX + x;
            int dstY = newY + y;
            if (dstX >= 0 && dstX < _screenWidth && dstY >= 0 && dstY < _screenHeight) {
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
    int ex = static_cast<int>(e.x);
    int ey = static_cast<int>(e.y);
    bool leftButton = static_cast<bool>(e.leftButton);
    bool isNewClick = leftButton && !_wasMouseDown;

    // Handle ongoing drag
    if (_isDragging && _dragWindow && leftButton) {
        // During drag, just update position - bitmap is drawn in Run()
        _wasMouseDown = leftButton;
        return;
    }

    // End drag if mouse released
    if (!leftButton && _isDragging && _dragWindow) {
        // Apply final position
        int newX = _cursorX - _dragOffsetX;
        int newY = _cursorY - _dragOffsetY;
        int bw = static_cast<int>(_dragWindow->Bounds().width);
        int bh = static_cast<int>(_dragWindow->Bounds().height);
        _dragWindow->SetBounds(newX, newY, bw, bh);

        _isDragging = false;
        _dragWindow = nullptr;
        _dragBitmap = Image();  // Free the bitmap
        UpdateSpatialGrid();  // Update spatial grid after move
        Invalidate();  // Redraw everything
    }

    // Handle start menu - continuous mouse tracking for hover effects
    if (_startMenu && static_cast<bool>(_startMenu->IsVisible())) {
        bool mouseOnMenu = static_cast<bool>(_startMenu->HitTest(ex, ey));
        bool clickOnStartButton = _taskBar && _taskBar->StartButton() &&
            static_cast<bool>(_taskBar->StartButton()->HitTest(ex, ey));

        // Hide menu if clicked outside
        if (isNewClick && !mouseOnMenu && !clickOnStartButton) {
            _startMenu->Hide();
            Invalidate();
        }

        // Always send mouse events to menu for hover tracking
        _startMenu->OnMouse(e);

        // If mouse is on menu, don't process other controls
        if (mouseOnMenu) {
            _wasMouseDown = leftButton;
            return;
        }
    }

    // Find child under cursor (for event propagation)
    Control* hitChild = _spatialGrid.HitTest(Int32(ex), Int32(ey));

    // Fall back to linear search if spatial grid misses (e.g., for TaskBar)
    if (!hitChild) {
        for (int i = _children.Length() - 1; i >= 0; i--) {
            Control* child = _children[i];
            if (child && static_cast<bool>(child->HitTest(ex, ey))) {
                hitChild = child;
                break;
            }
        }
    }

    // Handle window focus and drag initiation on new click only
    if (isNewClick && hitChild) {
        Window* win = hitChild->AsWindow();
        if (win) {
            Rectangle screen = hitChild->ScreenBounds();
            int sy = static_cast<int>(screen.y);
            int sx = static_cast<int>(screen.x);

            SetFocusedWindow(win);

            // Check if click is on title bar (top 22 pixels)
            if (ey < sy + 22) {
                _dragWindow = win;
                _dragOffsetX = ex - sx;
                _dragOffsetY = ey - sy;

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

    _wasMouseDown = leftButton;
}

void Desktop::CheckForUpdates() {
    // Check mouse
    MouseStatus ms = Mouse::GetStatus();

    // Update cursor position
    _cursorX = static_cast<int>(ms.x);
    _cursorY = static_cast<int>(ms.y);

    // Create mouse event and handle (always call to track drag state)
    MouseEventArgs mouseArgs(ms.x, ms.y, ms.leftButton, ms.rightButton);
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
    Display::FadeIn(500);

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
                for (int i = 0; i < _children.Length(); i++) {
                    Control* child = _children[i];
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
            bool cursorMoved = (_cursorX != _prevCursorX || _cursorY != _prevCursorY);
            if (cursorMoved) {
                RestoreCursor();
                SaveUnderCursor();
                DrawCursor();
                GraphicsBuffer::FlushFrameBuffer();
            }
        }
    }

    // Fade out to black
    Display::FadeOut(500);
}

void Desktop::Stop() {
    _running = false;
}

/******************************************************************************/
/*    Window Implementation                                                   */
/******************************************************************************/

Window::Window(Control* parent, const Rectangle& bounds)
    : Control(parent, bounds)
    , _isFocused(false) {
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
    int bw = static_cast<int>(_bounds.width);
    int bh = static_cast<int>(_bounds.height);
    _clientBounds = Rectangle(
        FRAME_WIDTH,
        TITLE_BAR_HEIGHT + FRAME_WIDTH,
        bw - FRAME_WIDTH * 2,
        bh - TITLE_BAR_HEIGHT - FRAME_WIDTH * 2
    );
}

void Window::OnPaint(PaintEventArgs& e) {
    Rectangle screen = ScreenBounds();
    int sx = static_cast<int>(screen.x);
    int sy = static_cast<int>(screen.y);
    int sw = static_cast<int>(screen.width);
    int sh = static_cast<int>(screen.height);

    // Draw window frame with Window border style (2-pixel thick 3D border)
    e.graphics->FillRectangle(screen, BorderStyle::Window);

    // Draw title bar - blue if focused, dark gray if not
    Rectangle titleBar(sx + 2, sy + 2, sw - 4, TITLE_BAR_HEIGHT);
    Color titleColor = _isFocused ? Color::DarkBlue : Color::DarkGray;
    e.graphics->FillRectangle(titleBar, titleColor);

    // Draw client area with sunken border effect
    Rectangle clientFrame(sx + 2, sy + TITLE_BAR_HEIGHT + 2, sw - 4, sh - TITLE_BAR_HEIGHT - 4);
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
    : Control(parent, Rectangle(0, 0, 0, 28))  // Temporary bounds, updated below
    , _startButton(nullptr)
    , _startMenu(startMenu)
    , _desktop(nullptr)
    , _windowButtons() {
    // Get screen dimensions from current display mode
    Display current = Display::GetCurrent();
    int screenWidth = static_cast<int>(current.Width());
    int screenHeight = static_cast<int>(current.Height());
    int taskBarHeight = 28;

    // Position taskbar at bottom of screen
    SetBounds(0, screenHeight - taskBarHeight, screenWidth, taskBarHeight);

    // Configure layout: row direction, fixed height, items centered vertically
    _layout.direction = FlexDirection::Row;
    _layout.alignItems = AlignItems::Center;
    _layout.gap = 2;
    _layout.heightMode = SizeMode::Fixed;
    _layout.paddingLeft = 4;
    _layout.paddingTop = 4;
    _layout.paddingBottom = 4;

    // Create Start button (positioned relative to taskbar, not screen)
    _startButton = new Button(this, Rectangle(4, 4, 54, 20));
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
    for (int i = 0; i < _windowButtons.Length(); i++) {
        if (_windowButtons[i]->GetWindow() == window) {
            return;
        }
    }

    // Calculate button position (relative to taskbar, not screen)
    int buttonX = WINDOW_BUTTON_START_X + _windowButtons.Length() *
                  (WINDOW_BUTTON_WIDTH + WINDOW_BUTTON_SPACING);
    int buttonY = 4;  // 4 pixels from top of taskbar

    // Create new button
    TaskBarButton* btn = new TaskBarButton(this,
        Rectangle(buttonX, buttonY, WINDOW_BUTTON_WIDTH, WINDOW_BUTTON_HEIGHT),
        window);

    // Add to array
    int oldLen = _windowButtons.Length();
    _windowButtons.Resize(oldLen + 1);
    _windowButtons[oldLen] = btn;

    RefreshWindowButtons();
    Invalidate();
}

void TaskBar::RemoveWindowButton(Window* window) {
    if (!window) return;

    for (int i = 0; i < _windowButtons.Length(); i++) {
        if (_windowButtons[i]->GetWindow() == window) {
            TaskBarButton* btn = _windowButtons[i];

            // Remove from array
            for (int j = i; j < _windowButtons.Length() - 1; j++) {
                _windowButtons[j] = _windowButtons[j + 1];
            }
            _windowButtons.Resize(_windowButtons.Length() - 1);

            // Remove from control hierarchy and delete
            RemoveChild(btn);
            delete btn;

            // Reposition remaining buttons (relative to taskbar)
            for (int k = 0; k < _windowButtons.Length(); k++) {
                int buttonX = WINDOW_BUTTON_START_X + k *
                              (WINDOW_BUTTON_WIDTH + WINDOW_BUTTON_SPACING);
                _windowButtons[k]->SetBounds(buttonX, 4,
                    WINDOW_BUTTON_WIDTH, WINDOW_BUTTON_HEIGHT);
            }

            Invalidate();
            break;
        }
    }
}

void TaskBar::RefreshWindowButtons() {
    if (!_desktop) return;

    Window* focused = _desktop->FocusedWindow();
    for (int i = 0; i < _windowButtons.Length(); i++) {
        TaskBarButton* btn = _windowButtons[i];
        btn->SetPressed(Boolean(btn->GetWindow() == focused));
    }
    Invalidate();
}

TaskBarButton* TaskBar::FindButtonForWindow(Window* window) const {
    for (int i = 0; i < _windowButtons.Length(); i++) {
        if (_windowButtons[i]->GetWindow() == window) {
            return _windowButtons[i];
        }
    }
    return nullptr;
}

void TaskBar::OnPaint(PaintEventArgs& e) {
    Rectangle screen = ScreenBounds();
    int sx = static_cast<int>(screen.x);
    int sy = static_cast<int>(screen.y);
    int sw = static_cast<int>(screen.width);

    // Draw taskbar background (gray)
    e.graphics->FillRectangle(screen, Color::Gray);

    // Draw white highlight line at top
    e.graphics->DrawLine(sx, sy, sx + sw - 1, sy, Color::White);

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
    , _onClickUserData(nullptr) {
}

Button::~Button() {
}

void Button::SetOnClick(ClickEventHandler handler, void* userData) {
    _onClick = handler;
    _onClickUserData = userData;
}

void Button::OnPaint(PaintEventArgs& e) {
    Rectangle screen = ScreenBounds();

    // Draw with appropriate border style based on pressed state
    // Visual state is toggled OR mouse down
    bool visualPressed = _isToggled || _isMouseDown;
    if (visualPressed) {
        e.graphics->FillRectangle(screen, BorderStyle::SunkenDouble);
    } else {
        e.graphics->FillRectangle(screen, BorderStyle::RaisedDouble);
    }

    // Paint children
    OnPaintClient(e);
}

void Button::OnMouse(MouseEventArgs& e) {
    bool wasVisuallyPressed = _isToggled || _isMouseDown;
    int ex = static_cast<int>(e.x);
    int ey = static_cast<int>(e.y);
    bool isOver = static_cast<bool>(HitTest(ex, ey));
    bool leftDown = static_cast<bool>(e.leftButton);

    // Only track mouse-down state, don't affect toggle state
    _isMouseDown = leftDown && isOver;

    // Detect click: was pressed, now released while still over button
    if (_wasMouseDown && !leftDown && isOver) {
        // Fire click event
        if (_onClick) {
            _onClick(this, _onClickUserData);
        }
    }

    _wasMouseDown = leftDown && isOver;

    bool nowVisuallyPressed = _isToggled || _isMouseDown;
    if (nowVisuallyPressed != wasVisuallyPressed) {
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
    int iw = static_cast<int>(_image.Width());
    int ih = static_cast<int>(_image.Height());

    if (iw > 0 && ih > 0) {
        Rectangle screen = ScreenBounds();
        Rectangle visible = VisibleBounds();

        int vw = static_cast<int>(visible.width);
        int vh = static_cast<int>(visible.height);

        // Check if anything is visible
        if (vw <= 0 || vh <= 0) {
            return;
        }

        int sx = static_cast<int>(screen.x);
        int sy = static_cast<int>(screen.y);
        int vx = static_cast<int>(visible.x);
        int vy = static_cast<int>(visible.y);

        // Calculate which part of the image to draw
        int srcX = vx - sx;
        int srcY = vy - sy;
        int drawWidth = vw;
        int drawHeight = vh;

        // Clamp to image dimensions
        if (srcX + drawWidth > iw) {
            drawWidth = iw - srcX;
        }
        if (srcY + drawHeight > ih) {
            drawHeight = ih - srcY;
        }

        if (drawWidth > 0 && drawHeight > 0 && srcX >= 0 && srcY >= 0) {
            Image region = _image.GetRegion(srcX, srcY, drawWidth, drawHeight);
            e.graphics->DrawImage(region, vx, vy);
        }
    }

    // Paint children
    OnPaintClient(e);
}

MeasureResult Picture::GetPreferredSize() const {
    // Pictures prefer their image dimensions if available
    int w = static_cast<int>(_image.Width());
    int h = static_cast<int>(_image.Height());

    // Fall back to bounds if no image
    if (w <= 0 || h <= 0) {
        w = static_cast<int>(_bounds.width);
        h = static_cast<int>(_bounds.height);
    }

    return MeasureResult(Int32(w), Int32(h));
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
    int w = static_cast<int>(_bounds.width);
    int h = static_cast<int>(_bounds.height);

    if (w <= 0 || h <= 0) return;

    _gradient = Image(Int32(w), Int32(h));

    // Generate gradient: white at top, base color in middle, black at bottom
    int midY = h / 2;

    for (int y = 0; y < h; y++) {
        Color lineColor;

        if (y <= midY) {
            // Top half: white to base color
            float t = (midY > 0) ? static_cast<float>(y) / midY : 0.0f;
            lineColor = Color::Lerp(Color::White, _baseColor, t);
        } else {
            // Bottom half: base color to black
            float t = (h - 1 - midY > 0) ?
                      static_cast<float>(y - midY) / (h - 1 - midY) : 0.0f;
            lineColor = Color::Lerp(_baseColor, Color::Black, t);
        }

        // Fill entire row with this color
        for (int x = 0; x < w; x++) {
            _gradient.SetPixel(Int32(x), Int32(y), lineColor);
        }
    }
}

Color SpectrumControl::GetColorAtY(Int32 y) const {
    int yi = static_cast<int>(y);
    int h = static_cast<int>(_bounds.height);

    if (yi < 0) yi = 0;
    if (yi >= h) yi = h - 1;
    if (h <= 0) return _baseColor;

    int midY = h / 2;

    if (yi <= midY) {
        float t = (midY > 0) ? static_cast<float>(yi) / midY : 0.0f;
        return Color::Lerp(Color::White, _baseColor, t);
    } else {
        float t = (h - 1 - midY > 0) ?
                  static_cast<float>(yi - midY) / (h - 1 - midY) : 0.0f;
        return Color::Lerp(_baseColor, Color::Black, t);
    }
}

void SpectrumControl::OnPaint(PaintEventArgs& e) {
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    Rectangle screen = ScreenBounds();
    int sx = static_cast<int>(screen.x);
    int sy = static_cast<int>(screen.y);
    int gw = static_cast<int>(_gradient.Width());
    int gh = static_cast<int>(_gradient.Height());

    // Copy gradient directly to framebuffer image
    // The buffer writer will handle dithering if needed for VGA modes
    Image& img = fb->GetImage();
    if (gw > 0 && gh > 0) {
        img.CopyFrom(_gradient, Int32(sx), Int32(sy));
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
    int sx = static_cast<int>(screen.x);
    int sy = static_cast<int>(screen.y);
    int sh = static_cast<int>(screen.height);

    // Draw background
    Color bgColor = _isHighlighted ? Color::DarkBlue : Color::Gray;
    e.graphics->FillRectangle(screen, bgColor);

    // Draw icon if present
    int iw = static_cast<int>(_icon.Width());
    int ih = static_cast<int>(_icon.Height());
    if (iw > 0 && ih > 0) {
        // Center icon vertically in item
        int iconY = sy + (sh - ih) / 2;
        int iconX = sx + ICON_MARGIN;

        GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
        if (fb) {
            Image& fbImg = fb->GetImage();
            fbImg.CopyFromWithAlpha(_icon, iconX, iconY);
        }
    }

    // Paint children
    OnPaintClient(e);
}

void MenuItem::OnMouse(MouseEventArgs& e) {
    int ex = static_cast<int>(e.x);
    int ey = static_cast<int>(e.y);
    bool isOver = static_cast<bool>(HitTest(ex, ey));
    bool leftDown = static_cast<bool>(e.leftButton);

    bool wasHighlighted = _isHighlighted;
    _isHighlighted = isOver;

    // Fire click on release while over
    static bool wasPressed = false;
    if (wasPressed && !leftDown && isOver && _onClick) {
        _onClick(reinterpret_cast<Button*>(this), _onClickUserData);
    }
    wasPressed = leftDown && isOver;

    if (_isHighlighted != wasHighlighted) {
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
    int screenHeight = static_cast<int>(current.Height());
    int taskBarHeight = 28;

    // Calculate menu height and position
    int menuHeight = ITEM_COUNT * ITEM_HEIGHT + 4;  // +4 for border
    int menuX = 0;
    int menuY = screenHeight - taskBarHeight - menuHeight;

    // Set bounds (positioned above taskbar, aligned with Start button)
    _bounds = Rectangle(menuX, menuY, MENU_WIDTH, menuHeight);
    _clientBounds = Rectangle(SIDEBAR_WIDTH, 2, MENU_WIDTH - SIDEBAR_WIDTH - 2, menuHeight - 4);

    // Configure layout: column direction for menu items
    _layout.direction = FlexDirection::Column;
    _layout.alignItems = AlignItems::Stretch;
    _layout.gap = 0;
    _layout.paddingLeft = SIDEBAR_WIDTH;
    _layout.paddingTop = 2;
    _layout.paddingRight = 2;
    _layout.paddingBottom = 2;

    // Create menu items with RELATIVE positioning (to parent's client area)
    _items.Resize(ITEM_COUNT);
    for (int i = 0; i < ITEM_COUNT; i++) {
        // Items positioned relative to client area (after sidebar)
        int itemY = i * ITEM_HEIGHT;  // Relative Y position within client
        int itemX = 0;                // Relative X position within client
        _items[i] = new MenuItem(this, Rectangle(itemX, itemY,
            MENU_WIDTH - SIDEBAR_WIDTH - 2, ITEM_HEIGHT), i);

        // Menu items have fixed height
        _items[i]->Layout().heightMode = SizeMode::Fixed;
        _items[i]->Layout().widthMode = SizeMode::Fill;
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
    // Icon indices from sysicons.icl to use (various system icons)
    static const int iconIndices[ITEM_COUNT] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
    };

    for (int i = 0; i < ITEM_COUNT && i < _items.Length(); i++) {
        try {
            Image icon = Image::FromIconLibrary("sysicons.icl", iconIndices[i], Size::IconSmall);
            _items[i]->SetIcon(icon);
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
            taskBar->StartButton()->SetPressed(true);
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
            taskBar->StartButton()->SetPressed(false);
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
    int sx = static_cast<int>(screen.x);
    int sy = static_cast<int>(screen.y);
    int sh = static_cast<int>(screen.height);

    // Draw raised border
    e.graphics->FillRectangle(screen, BorderStyle::RaisedDouble);

    // Draw blue sidebar on left (Windows 95 style)
    Rectangle sidebar(sx + 2, sy + 2, SIDEBAR_WIDTH - 2, sh - 4);
    e.graphics->FillRectangle(sidebar, Color::DarkBlue);

    // Paint menu items
    OnPaintClient(e);
}

void StartMenu::OnMouse(MouseEventArgs& e) {
    // Propagate mouse events to ALL menu items so they can update highlight state
    // (not just the one under cursor, so items can un-highlight when mouse leaves)
    for (int i = 0; i < _items.Length(); i++) {
        if (_items[i]) {
            _items[i]->OnMouse(e);
        }
    }
}

}}} // namespace System::Windows::Forms
