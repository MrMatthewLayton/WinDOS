#include "Forms.hpp"
#include "../../Drawing/Drawing.hpp"

namespace System::Windows::Forms
{

/******************************************************************************/
/*    SpatialGrid Implementation                                              */
/******************************************************************************/

SpatialGrid::SpatialGrid()
    : _cellsX(Int32(0))
    , _cellsY(Int32(0))
    , _screenWidth(Int32(0))
    , _screenHeight(Int32(0))
{
}

void SpatialGrid::Initialize(Int32 screenWidth, Int32 screenHeight)
{
    _screenWidth = screenWidth;
    _screenHeight = screenHeight;
    Int32 sw = _screenWidth;
    Int32 sh = _screenHeight;
    _cellsX = Int32((static_cast<int>(sw) + CELL_SIZE - 1) / CELL_SIZE);
    _cellsY = Int32((static_cast<int>(sh) + CELL_SIZE - 1) / CELL_SIZE);

    // Clamp to max
    if (static_cast<int>(_cellsX) > MAX_CELLS_X)
    {
        _cellsX = Int32(MAX_CELLS_X);
    }
    if (static_cast<int>(_cellsY) > MAX_CELLS_Y)
    {
        _cellsY = Int32(MAX_CELLS_Y);
    }

    Clear();
}

void SpatialGrid::Clear()
{
    for (Int32 y = Int32(0); static_cast<int>(y) < MAX_CELLS_Y; y += 1)
    {
        for (Int32 x = Int32(0); static_cast<int>(x) < MAX_CELLS_X; x += 1)
        {
            _cells[static_cast<int>(y)][static_cast<int>(x)].count = 0;
            for (Int32 i = Int32(0); static_cast<int>(i) < MAX_CONTROLS_PER_CELL; i += 1)
            {
                _cells[static_cast<int>(y)][static_cast<int>(x)].controls[static_cast<int>(i)] = nullptr;
            }
        }
    }
}

void SpatialGrid::GetCellIndex(Int32 x, Int32 y, Int32& cellX, Int32& cellY) const
{
    cellX = Int32(static_cast<int>(x) / CELL_SIZE);
    cellY = Int32(static_cast<int>(y) / CELL_SIZE);
    if (static_cast<int>(cellX) < 0)
    {
        cellX = Int32(0);
    }
    if (static_cast<int>(cellY) < 0)
    {
        cellY = Int32(0);
    }
    if (cellX >= _cellsX)
    {
        cellX = _cellsX - Int32(1);
    }
    if (cellY >= _cellsY)
    {
        cellY = _cellsY - Int32(1);
    }
}

void SpatialGrid::GetCellRange(const Rectangle& bounds, Int32& minX, Int32& minY,
                               Int32& maxX, Int32& maxY) const
{
    Int32 bx = bounds.x;
    Int32 by = bounds.y;
    Int32 bw = bounds.width;
    Int32 bh = bounds.height;

    GetCellIndex(bx, by, minX, minY);
    GetCellIndex(Int32(static_cast<int>(bx) + static_cast<int>(bw) - 1),
                 Int32(static_cast<int>(by) + static_cast<int>(bh) - 1), maxX, maxY);
}

void SpatialGrid::Insert(Control* control, const Rectangle& bounds)
{
    if (!control)
    {
        return;
    }

    Int32 minX, minY, maxX, maxY;
    GetCellRange(bounds, minX, minY, maxX, maxY);

    for (Int32 y = minY; y <= maxY; y += 1)
    {
        for (Int32 x = minX; x <= maxX; x += 1)
        {
            Cell& cell = _cells[static_cast<int>(y)][static_cast<int>(x)];
            if (cell.count < MAX_CONTROLS_PER_CELL)
            {
                // Check if already in cell
                Boolean found = Boolean(false);
                for (Int32 i = Int32(0); static_cast<int>(i) < cell.count; i += 1)
                {
                    if (cell.controls[static_cast<int>(i)] == control)
                    {
                        found = Boolean(true);
                        break;
                    }
                }
                if (!static_cast<bool>(found))
                {
                    cell.controls[cell.count++] = control;
                }
            }
        }
    }
}

void SpatialGrid::Remove(Control* control)
{
    if (!control)
    {
        return;
    }

    for (Int32 y = Int32(0); y < _cellsY; y += 1)
    {
        for (Int32 x = Int32(0); x < _cellsX; x += 1)
        {
            Cell& cell = _cells[static_cast<int>(y)][static_cast<int>(x)];
            for (Int32 i = Int32(0); static_cast<int>(i) < cell.count; i += 1)
            {
                if (cell.controls[static_cast<int>(i)] == control)
                {
                    // Shift remaining controls down
                    for (Int32 j = i; static_cast<int>(j) < cell.count - 1; j += 1)
                    {
                        cell.controls[static_cast<int>(j)] = cell.controls[static_cast<int>(j) + 1];
                    }
                    cell.controls[--cell.count] = nullptr;
                    break;
                }
            }
        }
    }
}

Control* SpatialGrid::HitTest(Int32 x, Int32 y) const
{
    Int32 px = x;
    Int32 py = y;

    if (static_cast<int>(px) < 0 || static_cast<int>(py) < 0 ||
        px >= _screenWidth || py >= _screenHeight)
    {
        return nullptr;
    }

    Int32 cellX, cellY;
    GetCellIndex(px, py, cellX, cellY);

    const Cell& cell = _cells[static_cast<int>(cellY)][static_cast<int>(cellX)];

    // Check controls in reverse order (last added = highest z-order)
    for (Int32 i = Int32(cell.count - 1); static_cast<int>(i) >= 0; i -= 1)
    {
        Control* ctrl = cell.controls[static_cast<int>(i)];
        if (ctrl && static_cast<bool>(ctrl->HitTest(px, py)))
        {
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
    , _isInvalid(true)
    , _visible(true)
{
}

Control::Control(Control* parent, const Rectangle& bounds)
    : _children()
    , _parent(parent)
    , _bounds(bounds)
    , _clientBounds()
    , _isInvalid(true)
    , _visible(true)
{
    UpdateClientBounds();
    if (parent)
    {
        parent->AddChild(this);
    }
}

Control::~Control()
{
    // Delete all children
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
    {
        delete _children[static_cast<int>(i)];
    }
}

void Control::UpdateClientBounds()
{
    // By default, client bounds is the full control area (no decoration)
    // Client bounds are relative to the control's origin (0,0)
    _clientBounds = Rectangle(0, 0, _bounds.width, _bounds.height);
}

Control* Control::GetChild(Int32 index) const
{
    Int32 idx = index;
    if (static_cast<int>(idx) >= 0 && static_cast<int>(idx) < _children.Length())
    {
        return _children[static_cast<int>(idx)];
    }
    return nullptr;
}

Rectangle Control::ScreenBounds() const
{
    if (!_parent)
    {
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

Rectangle Control::ScreenClientBounds() const
{
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

Rectangle Control::VisibleBounds() const
{
    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 sw = screen.width;
    Int32 sh = screen.height;

    // Get screen dimensions from current display mode
    Display current = Display::GetCurrent();
    Int32 screenWidth = Int32(static_cast<int>(current.Width()));
    Int32 screenHeight = Int32(static_cast<int>(current.Height()));

    if (!_parent)
    {
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

    if (right > parentRight)
    {
        right = parentRight;
    }
    if (bottom > parentBottom)
    {
        bottom = parentBottom;
    }

    // Also clip to screen bounds
    if (left < Int32(0))
    {
        left = Int32(0);
    }
    if (top < Int32(0))
    {
        top = Int32(0);
    }
    if (right > screenWidth)
    {
        right = screenWidth;
    }
    if (bottom > screenHeight)
    {
        bottom = screenHeight;
    }

    if (right <= left || bottom <= top)
    {
        return Rectangle(Int32(0), Int32(0), Int32(0), Int32(0));  // Not visible
    }

    return Rectangle(left, top, Int32(static_cast<int>(right) - static_cast<int>(left)),
                     Int32(static_cast<int>(bottom) - static_cast<int>(top)));
}

void Control::SetBounds(const Rectangle& bounds)
{
    _bounds = bounds;
    UpdateClientBounds();
    Invalidate();
}

void Control::SetBounds(Int32 x, Int32 y, Int32 width, Int32 height)
{
    SetBounds(Rectangle(x, y, width, height));
}

void Control::AddChild(Control* child)
{
    if (child)
    {
        Int32 oldLen = Int32(_children.Length());
        _children.Resize(static_cast<int>(oldLen) + 1);
        _children[static_cast<int>(oldLen)] = child;
        child->_parent = this;
        Invalidate();
    }
}

void Control::RemoveChild(Control* child)
{
    if (child)
    {
        Int32 index = Int32(_children.IndexOf(child));
        if (static_cast<int>(index) >= 0)
        {
            // Shift elements down
            for (Int32 i = index; static_cast<int>(i) < _children.Length() - 1; i += 1)
            {
                _children[static_cast<int>(i)] = _children[static_cast<int>(i) + 1];
            }
            _children.Resize(_children.Length() - 1);
            child->_parent = nullptr;
            Invalidate();
        }
    }
}

void Control::OnPaint(PaintEventArgs& e)
{
    // Base implementation: draw nothing, just call OnPaintClient
    OnPaintClient(e);
}

void Control::OnPaintClient(PaintEventArgs& e)
{
    // Get parent's client bounds in screen coordinates for clipping
    Rectangle parentClientScreen = ScreenClientBounds();

    // Base implementation: paint children with clipping
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
    {
        Control* child = _children[static_cast<int>(i)];
        if (child && static_cast<bool>(child->IsVisible()))
        {
            // Calculate clip region as intersection of parent's client area and child bounds
            // Also intersect with any existing clip region from parent
            Int32 clipLeft = static_cast<int>(parentClientScreen.x);
            Int32 clipTop = static_cast<int>(parentClientScreen.y);
            Int32 clipRight = static_cast<int>(parentClientScreen.x) + static_cast<int>(parentClientScreen.width);
            Int32 clipBottom = static_cast<int>(parentClientScreen.y) + static_cast<int>(parentClientScreen.height);

            // Intersect with incoming clip bounds from parent
            if (static_cast<int>(e.clipBounds.width) > 0 && static_cast<int>(e.clipBounds.height) > 0)
            {
                Int32 eLeft = static_cast<int>(e.clipBounds.x);
                Int32 eTop = static_cast<int>(e.clipBounds.y);
                Int32 eRight = static_cast<int>(e.clipBounds.x) + static_cast<int>(e.clipBounds.width);
                Int32 eBottom = static_cast<int>(e.clipBounds.y) + static_cast<int>(e.clipBounds.height);

                if (eLeft > clipLeft) clipLeft = eLeft;
                if (eTop > clipTop) clipTop = eTop;
                if (eRight < clipRight) clipRight = eRight;
                if (eBottom < clipBottom) clipBottom = eBottom;
            }

            // Create clip rectangle
            Rectangle childClip(Int32(clipLeft), Int32(clipTop),
                                Int32(clipRight - clipLeft), Int32(clipBottom - clipTop));

            // Only paint if clip region is valid
            if (static_cast<int>(childClip.width) > 0 && static_cast<int>(childClip.height) > 0)
            {
                PaintEventArgs childArgs(e.graphics, child->Bounds(), childClip);
                child->OnPaint(childArgs);
            }
        }
    }
}

void Control::Invalidate()
{
    _isInvalid = true;
    // Propagate up to parent
    if (_parent)
    {
        _parent->Invalidate();
    }
}

void Control::Update()
{
    if (_isInvalid)
    {
        GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
        if (fb)
        {
            Graphics g(BufferMode::Single, _bounds);
            PaintEventArgs e(&g, _bounds);
            OnPaint(e);
        }
        _isInvalid = false;
    }
}

void Control::OnMouse(MouseEventArgs& e)
{
    // Base implementation: do nothing
    (void)e;
}

void Control::OnKeyboard(KeyboardEventArgs& e)
{
    // Base implementation: do nothing
    (void)e;
}

void Control::NotifyMouse(MouseEventArgs& e)
{
    Int32 ex = e.x;
    Int32 ey = e.y;
    // First check if any child contains the point (in reverse order for z-order)
    for (Int32 i = Int32(_children.Length() - 1); static_cast<int>(i) >= 0; i -= 1)
    {
        Control* child = _children[static_cast<int>(i)];
        if (child && static_cast<bool>(child->HitTest(ex, ey)))
        {
            child->NotifyMouse(e);
            return;
        }
    }
    // No child hit, handle ourselves
    OnMouse(e);
}

void Control::NotifyKeyboard(KeyboardEventArgs& e)
{
    // Pass to all children (typically only focused window handles it)
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
    {
        Control* child = _children[static_cast<int>(i)];
        if (child)
        {
            child->NotifyKeyboard(e);
        }
    }
    OnKeyboard(e);
}

Boolean Control::HitTest(Int32 x, Int32 y) const
{
    return ScreenBounds().Contains(x, y);
}

/******************************************************************************/
/*    Control Layout Implementation                                           */
/******************************************************************************/

MeasureResult Control::GetPreferredSize() const
{
    // Base implementation: return current bounds size or min size
    Int32 w = _bounds.width;
    Int32 h = _bounds.height;

    // Apply min constraints
    Int32 minW = _layout.minWidth;
    Int32 minH = _layout.minHeight;
    if (w < minW)
    {
        w = minW;
    }
    if (h < minH)
    {
        h = minH;
    }

    return MeasureResult(w, h);
}

MeasureResult Control::Measure(Int32 availableWidth, Int32 availableHeight)
{
    Int32 avW = availableWidth;
    Int32 avH = availableHeight;

    // Subtract margins from available space
    Int32 marginH = Int32(static_cast<int>(_layout.marginLeft) + static_cast<int>(_layout.marginRight));
    Int32 marginV = Int32(static_cast<int>(_layout.marginTop) + static_cast<int>(_layout.marginBottom));
    avW = Int32(static_cast<int>(avW) - static_cast<int>(marginH));
    avH = Int32(static_cast<int>(avH) - static_cast<int>(marginV));
    if (avW < Int32(0))
    {
        avW = Int32(0);
    }
    if (avH < Int32(0))
    {
        avH = Int32(0);
    }

    Int32 resultW = Int32(0);
    Int32 resultH = Int32(0);

    // Handle different size modes
    if (_layout.widthMode == SizeMode::Fixed)
    {
        resultW = _bounds.width;
    }
    else if (_layout.widthMode == SizeMode::Fill)
    {
        resultW = avW;
    }
    // Auto mode: calculate from content/children

    if (_layout.heightMode == SizeMode::Fixed)
    {
        resultH = _bounds.height;
    }
    else if (_layout.heightMode == SizeMode::Fill)
    {
        resultH = avH;
    }
    // Always measure children - their _measuredSize is needed for Arrange phase
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
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
    {
        Control* child = _children[static_cast<int>(i)];
        if (child && child->_layout.participatesInLayout)
        {
            participatingCount += 1;
        }
    }

    // Measure children
    Boolean isRow = Boolean(_layout.direction == FlexDirection::Row);

    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
    {
        Control* child = _children[static_cast<int>(i)];
        if (!child || !child->_layout.participatesInLayout)
        {
            continue;
        }

        // Measure child - this populates child->_measuredSize
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

        if (static_cast<bool>(isRow))
        {
            // Row: width accumulates, height is max
            contentW = Int32(static_cast<int>(contentW) + static_cast<int>(cw));
            if (ch > contentH)
            {
                contentH = ch;
            }
        }
        else
        {
            // Column: height accumulates, width is max
            contentH = Int32(static_cast<int>(contentH) + static_cast<int>(ch));
            if (cw > contentW)
            {
                contentW = cw;
            }
        }
    }

    // Add gaps between children
    if (participatingCount > Int32(1))
    {
        if (static_cast<bool>(isRow))
        {
            contentW = Int32(static_cast<int>(contentW) + static_cast<int>(gap) * (static_cast<int>(participatingCount) - 1));
        }
        else
        {
            contentH = Int32(static_cast<int>(contentH) + static_cast<int>(gap) * (static_cast<int>(participatingCount) - 1));
        }
    }

    // Add padding
    contentW = Int32(static_cast<int>(contentW) + static_cast<int>(padL) + static_cast<int>(padR));
    contentH = Int32(static_cast<int>(contentH) + static_cast<int>(padT) + static_cast<int>(padB));

    // Use content size for Auto modes
    if (_layout.widthMode == SizeMode::Auto)
    {
        resultW = contentW;
    }
    if (_layout.heightMode == SizeMode::Auto)
    {
        resultH = contentH;
    }

    // Apply self preferred size if no children
    if (resultW == Int32(0) && resultH == Int32(0))
    {
        MeasureResult pref = GetPreferredSize();
        if (_layout.widthMode == SizeMode::Auto)
        {
            resultW = pref.preferredWidth;
        }
        if (_layout.heightMode == SizeMode::Auto)
        {
            resultH = pref.preferredHeight;
        }
    }

    // Apply min/max constraints
    Int32 minW = _layout.minWidth;
    Int32 minH = _layout.minHeight;
    Int32 maxW = _layout.maxWidth;
    Int32 maxH = _layout.maxHeight;

    if (resultW < minW)
    {
        resultW = minW;
    }
    if (resultH < minH)
    {
        resultH = minH;
    }
    if (resultW > maxW)
    {
        resultW = maxW;
    }
    if (resultH > maxH)
    {
        resultH = maxH;
    }

    // Store and return result
    _measuredSize = MeasureResult(resultW, resultH);
    return _measuredSize;
}

void Control::Arrange(const Rectangle& finalBounds)
{
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

    if (cw < Int32(0))
    {
        cw = Int32(0);
    }
    if (ch < Int32(0))
    {
        ch = Int32(0);
    }

    Rectangle contentArea(cx, cy, cw, ch);

    // Arrange children using flex layout
    ArrangeFlexChildren(contentArea);

    // Mark layout as clean
    _layout.needsLayout = false;
}

void Control::ArrangeFlexChildren(const Rectangle& contentArea)
{
    Int32 cx = contentArea.x;
    Int32 cy = contentArea.y;
    Int32 cw = contentArea.width;
    Int32 ch = contentArea.height;
    Int32 gap = _layout.gap;
    Boolean isRow = Boolean(_layout.direction == FlexDirection::Row);
    Boolean shouldWrap = Boolean(_layout.wrap == FlexWrap::Wrap);

    // For wrap mode, we need a different approach: process children line by line
    if (static_cast<bool>(shouldWrap))
    {
        // Track current position in content area
        Int32 mainPos = Int32(0);      // Position along main axis within current line
        Int32 crossPos = Int32(0);     // Position along cross axis (which line/column we're on)
        Int32 lineMaxCross = Int32(0); // Max cross-axis size in current line

        Int32 mainAxisSize = static_cast<bool>(isRow) ? cw : ch;
        // Note: crossAxisSize not needed in wrap mode - we track lineMaxCross instead

        for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
        {
            Control* child = _children[static_cast<int>(i)];
            if (!child || !child->_layout.participatesInLayout)
            {
                continue;
            }

            // Get measured size
            Int32 childW = child->_measuredSize.preferredWidth;
            Int32 childH = child->_measuredSize.preferredHeight;

            // Get margins
            Int32 mTop = child->_layout.marginTop;
            Int32 mRight = child->_layout.marginRight;
            Int32 mBottom = child->_layout.marginBottom;
            Int32 mLeft = child->_layout.marginLeft;

            // Calculate total size this child needs along main axis
            Int32 childMainSize = static_cast<bool>(isRow)
                ? Int32(static_cast<int>(childW) + static_cast<int>(mLeft) + static_cast<int>(mRight))
                : Int32(static_cast<int>(childH) + static_cast<int>(mTop) + static_cast<int>(mBottom));

            // Calculate total size this child needs along cross axis
            Int32 childCrossSize = static_cast<bool>(isRow)
                ? Int32(static_cast<int>(childH) + static_cast<int>(mTop) + static_cast<int>(mBottom))
                : Int32(static_cast<int>(childW) + static_cast<int>(mLeft) + static_cast<int>(mRight));

            // Check if we need to wrap (if this child would exceed main axis)
            // Note: mainPos already includes the gap from the previous child,
            // so we only check if the child itself fits
            Int32 neededSpace = childMainSize;

            if (mainPos > Int32(0) && Int32(static_cast<int>(mainPos) + static_cast<int>(neededSpace)) > mainAxisSize)
            {
                // Wrap to next line/column
                crossPos = Int32(static_cast<int>(crossPos) + static_cast<int>(lineMaxCross) + static_cast<int>(gap));
                mainPos = Int32(0);
                lineMaxCross = Int32(0);
            }

            // Track max cross size for this line
            if (childCrossSize > lineMaxCross)
            {
                lineMaxCross = childCrossSize;
            }

            // Calculate position
            Int32 childX, childY;

            if (static_cast<bool>(isRow))
            {
                // Row with wrap: main axis is X, cross axis is Y
                childX = Int32(static_cast<int>(cx) + static_cast<int>(mainPos) + static_cast<int>(mLeft));
                childY = Int32(static_cast<int>(cy) + static_cast<int>(crossPos) + static_cast<int>(mTop));
            }
            else
            {
                // Column with wrap: main axis is Y, cross axis is X
                childY = Int32(static_cast<int>(cy) + static_cast<int>(mainPos) + static_cast<int>(mTop));
                childX = Int32(static_cast<int>(cx) + static_cast<int>(crossPos) + static_cast<int>(mLeft));
            }

            // Advance main axis position
            mainPos = Int32(static_cast<int>(mainPos) + static_cast<int>(childMainSize) + static_cast<int>(gap));

            // Arrange child
            Rectangle childBounds(childX, childY, childW, childH);
            child->Arrange(childBounds);
        }
    }
    else
    {
        // Original non-wrap code path
        // First pass: gather info about participating children
        Int32 participatingCount = Int32(0);
        Int32 totalMainSize = Int32(0);
        Int32 totalFlexGrow = Int32(0);
        Int32 maxCrossSize = Int32(0);

        for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
        {
            Control* child = _children[static_cast<int>(i)];
            if (!child || !child->_layout.participatesInLayout)
            {
                continue;
            }

            participatingCount += 1;

            // Get measured size
            Int32 childW = child->_measuredSize.preferredWidth;
            Int32 childH = child->_measuredSize.preferredHeight;

            // Add margins to size
            Int32 marginH = Int32(static_cast<int>(child->_layout.marginLeft) +
                          static_cast<int>(child->_layout.marginRight));
            Int32 marginV = Int32(static_cast<int>(child->_layout.marginTop) +
                          static_cast<int>(child->_layout.marginBottom));

            if (static_cast<bool>(isRow))
            {
                totalMainSize = Int32(static_cast<int>(totalMainSize) + static_cast<int>(childW) + static_cast<int>(marginH));
                Int32 crossSize = Int32(static_cast<int>(childH) + static_cast<int>(marginV));
                if (crossSize > maxCrossSize)
                {
                    maxCrossSize = crossSize;
                }
            }
            else
            {
                totalMainSize = Int32(static_cast<int>(totalMainSize) + static_cast<int>(childH) + static_cast<int>(marginV));
                Int32 crossSize = Int32(static_cast<int>(childW) + static_cast<int>(marginH));
                if (crossSize > maxCrossSize)
                {
                    maxCrossSize = crossSize;
                }
            }

            totalFlexGrow = Int32(static_cast<int>(totalFlexGrow) + static_cast<int>(child->_layout.flexGrow));
        }

        // Add gaps
        if (participatingCount > Int32(1))
        {
            totalMainSize = Int32(static_cast<int>(totalMainSize) + static_cast<int>(gap) * (static_cast<int>(participatingCount) - 1));
        }

        if (participatingCount == Int32(0))
        {
            return;
        }

        // Calculate available space for distribution
        Int32 mainAxisSize = static_cast<bool>(isRow) ? cw : ch;
        Int32 crossAxisSize = static_cast<bool>(isRow) ? ch : cw;
        Int32 extraSpace = Int32(static_cast<int>(mainAxisSize) - static_cast<int>(totalMainSize));
        if (extraSpace < Int32(0))
        {
            extraSpace = Int32(0);
        }

        // Calculate starting position based on JustifyContent
        Int32 mainPos = Int32(0);
        Int32 spaceBetween = Int32(0);
        Int32 spaceAround = Int32(0);

        switch (_layout.justifyContent)
        {
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
                if (participatingCount > Int32(1))
                {
                    spaceBetween = Int32(static_cast<int>(extraSpace) / (static_cast<int>(participatingCount) - 1));
                }
                break;
            case JustifyContent::SpaceAround:
                if (participatingCount > Int32(0))
                {
                    spaceAround = Int32(static_cast<int>(extraSpace) / (static_cast<int>(participatingCount) * 2));
                    mainPos = spaceAround;
                }
                break;
        }

        // Second pass: arrange children
        for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
        {
            Control* child = _children[static_cast<int>(i)];
            if (!child || !child->_layout.participatesInLayout)
            {
                continue;
            }

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
            if (totalFlexGrow > Int32(0) && flexGrow > Int32(0) && extraSpace > Int32(0))
            {
                growAmount = Int32((static_cast<int>(extraSpace) * static_cast<int>(flexGrow)) / static_cast<int>(totalFlexGrow));
            }

            // Calculate final size
            Int32 finalW = childW;
            Int32 finalH = childH;

            if (static_cast<bool>(isRow))
            {
                finalW = Int32(static_cast<int>(finalW) + static_cast<int>(growAmount));
                // Handle AlignItems for cross axis (height)
                switch (_layout.alignItems)
                {
                    case AlignItems::Stretch:
                        finalH = Int32(static_cast<int>(crossAxisSize) - static_cast<int>(mTop) - static_cast<int>(mBottom));
                        break;
                    case AlignItems::Start:
                    case AlignItems::Center:
                    case AlignItems::End:
                        // Keep measured height
                        break;
                }
            }
            else
            {
                finalH = Int32(static_cast<int>(finalH) + static_cast<int>(growAmount));
                // Handle AlignItems for cross axis (width)
                switch (_layout.alignItems)
                {
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

            if (finalW < minW)
            {
                finalW = minW;
            }
            if (finalH < minH)
            {
                finalH = minH;
            }
            if (finalW > maxW)
            {
                finalW = maxW;
            }
            if (finalH > maxH)
            {
                finalH = maxH;
            }

            // Calculate position
            Int32 childX, childY;

            if (static_cast<bool>(isRow))
            {
                childX = Int32(static_cast<int>(cx) + static_cast<int>(mainPos) + static_cast<int>(mLeft));

                // Cross axis position (Y) based on AlignItems
                switch (_layout.alignItems)
                {
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
            }
            else
            {
                childY = Int32(static_cast<int>(cy) + static_cast<int>(mainPos) + static_cast<int>(mTop));

                // Cross axis position (X) based on AlignItems
                switch (_layout.alignItems)
                {
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
    }

    // Arrange non-participating children (they keep their current bounds)
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
    {
        Control* child = _children[static_cast<int>(i)];
        if (child && !child->_layout.participatesInLayout)
        {
            // Just trigger their own layout with current bounds
            child->Arrange(child->_bounds);
        }
    }
}

void Control::PerformLayout()
{
    if (!_layout.needsLayout)
    {
        return;
    }

    // Measure pass (bottom-up)
    Int32 availW = _bounds.width;
    Int32 availH = _bounds.height;
    Measure(availW, availH);

    // Arrange pass (top-down)
    Arrange(_bounds);
}

void Control::InvalidateLayout()
{
    _layout.needsLayout = true;
    // Also invalidate visual
    Invalidate();
}

/******************************************************************************/
/*    DesktopIconControl Implementation                                       */
/******************************************************************************/

// Helper to load icon font (MS Sans Serif, not bold)
static Font LoadIconFont()
{
    try
    {
        return Font::SystemFont();  // MS Sans Serif 8pt
    }
    catch (...)
    {
        return Font();  // Return invalid font on failure
    }
}

DesktopIconControl::DesktopIconControl(Control* parent, const Image& icon)
    : Control(parent, Rectangle(Int32(0), Int32(0), Int32(CELL_WIDTH), Int32(CELL_HEIGHT)))
    , _icon(icon)
    , _text()
    , _font(LoadIconFont())
    , _isSelected(false)
{
    // Configure for fixed cell size
    _layout.widthMode = SizeMode::Fixed;
    _layout.heightMode = SizeMode::Fixed;
}

DesktopIconControl::DesktopIconControl(Control* parent, const Image& icon, const String& text)
    : Control(parent, Rectangle(Int32(0), Int32(0), Int32(CELL_WIDTH), Int32(CELL_HEIGHT)))
    , _icon(icon)
    , _text(text)
    , _font(LoadIconFont())
    , _isSelected(false)
{
    // Configure for fixed cell size
    _layout.widthMode = SizeMode::Fixed;
    _layout.heightMode = SizeMode::Fixed;
}

String DesktopIconControl::TruncateWithEllipsis(const String& text, Int32 maxWidth) const
{
    if (!static_cast<bool>(_font.IsValid()))
    {
        return text;
    }

    Size textSize = _font.MeasureString(text);
    if (textSize.width <= maxWidth)
    {
        return text;
    }

    // Binary search for the longest substring that fits with "..."
    String ellipsis("...");
    Size ellipsisSize = _font.MeasureString(ellipsis);
    Int32 availWidth = Int32(static_cast<int>(maxWidth) - static_cast<int>(ellipsisSize.width));

    if (availWidth <= Int32(0))
    {
        return ellipsis;
    }

    // Find longest prefix that fits
    Int32 len = text.Length();
    for (Int32 i = len - Int32(1); i >= Int32(0); i -= 1)
    {
        String sub = text.Substring(Int32(0), i);
        Size subSize = _font.MeasureString(sub);
        if (subSize.width <= availWidth)
        {
            return sub + ellipsis;
        }
    }

    return ellipsis;
}

void DesktopIconControl::OnPaint(PaintEventArgs& e)
{
    // Get our screen bounds
    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb)
    {
        return;
    }

    Image& img = fb->GetImage();

    // Get icon dimensions
    Int32 iconW = _icon.Width();
    Int32 iconH = _icon.Height();

    // Draw icon centered in top area (64x64 with 16px padding = 32x32 icon area)
    if (iconW > Int32(0) && iconH > Int32(0))
    {
        // Center icon in the icon area (top 64px)
        Int32 iconAreaCenterX = Int32(static_cast<int>(sx) + CELL_WIDTH / 2);
        Int32 iconAreaCenterY = Int32(static_cast<int>(sy) + ICON_AREA_HEIGHT / 2);
        Int32 iconX = Int32(static_cast<int>(iconAreaCenterX) - static_cast<int>(iconW) / 2);
        Int32 iconY = Int32(static_cast<int>(iconAreaCenterY) - static_cast<int>(iconH) / 2);

        // Use clip bounds if valid
        if (static_cast<int>(e.clipBounds.width) > 0 && static_cast<int>(e.clipBounds.height) > 0)
        {
            img.CopyFromWithAlphaClipped(_icon, iconX, iconY, e.clipBounds);
        }
        else
        {
            img.CopyFromWithAlpha(_icon, iconX, iconY);
        }
    }

    // Draw text below icon, aligned to top of text area
    if (_text.Length() > Int32(0) && static_cast<bool>(_font.IsValid()))
    {
        // Measure and possibly truncate text
        String displayText = TruncateWithEllipsis(_text, Int32(CELL_WIDTH - 4));  // 2px margin each side
        Size textSize = _font.MeasureString(displayText);

        // Calculate text position in LOCAL coordinates (relative to control bounds)
        // Graphics adds screen offset automatically when drawing to framebuffer
        Int32 localTextX = Int32((CELL_WIDTH - static_cast<int>(textSize.width)) / 2);
        // Position text 2px below the icon area
        Int32 localTextY = Int32(ICON_AREA_HEIGHT + 2);

        // Draw text using Graphics with screen bounds - it handles offset internally
        Graphics g(BufferMode::Single, screen);
        g.DrawString(displayText, _font, Color::White, localTextX, localTextY);
    }

    // Draw selection visual if selected
    if (_isSelected)
    {
        Color navyBlue(UInt8(0), UInt8(0), UInt8(128));  // Navy blue 0x000080

        // Draw navy blue outline around the cell (1 pixel thick)
        Graphics g(BufferMode::Single, screen);
        Int32 right = Int32(CELL_WIDTH - 1);
        Int32 bottom = Int32(CELL_HEIGHT - 1);

        // Top, bottom, left, right lines
        g.DrawLine(Int32(0), Int32(0), right, Int32(0), navyBlue);
        g.DrawLine(Int32(0), bottom, right, bottom, navyBlue);
        g.DrawLine(Int32(0), Int32(0), Int32(0), bottom, navyBlue);
        g.DrawLine(right, Int32(0), right, bottom, navyBlue);

        // Draw 25% translucent blue overlay using alpha blending
        Int32 imgW = img.Width();
        Int32 imgH = img.Height();
        Int32 screenX = sx;
        Int32 screenY = sy;

        // Navy blue RGB components
        const int blueR = 0;
        const int blueG = 0;
        const int blueB = 128;

        for (Int32 py = Int32(1); static_cast<int>(py) < CELL_HEIGHT - 1; py += 1)
        {
            for (Int32 px = Int32(1); static_cast<int>(px) < CELL_WIDTH - 1; px += 1)
            {
                Int32 absX = Int32(static_cast<int>(screenX) + static_cast<int>(px));
                Int32 absY = Int32(static_cast<int>(screenY) + static_cast<int>(py));

                // Bounds check
                if (absX >= Int32(0) && absX < imgW && absY >= Int32(0) && absY < imgH)
                {
                    // Get existing pixel
                    Color existing = img.GetPixel(absX, absY);

                    // Alpha blend at 25% opacity: result = blue * 0.25 + existing * 0.75
                    int newR = (blueR * 64 + static_cast<int>(existing.R()) * 192) / 256;
                    int newG = (blueG * 64 + static_cast<int>(existing.G()) * 192) / 256;
                    int newB = (blueB * 64 + static_cast<int>(existing.B()) * 192) / 256;

                    img.SetPixel(absX, absY, Color(UInt8(newR), UInt8(newG), UInt8(newB)));
                }
            }
        }
    }
}

MeasureResult DesktopIconControl::GetPreferredSize() const
{
    return MeasureResult(Int32(CELL_WIDTH), Int32(CELL_HEIGHT));
}

void DesktopIconControl::OnMouse(MouseEventArgs& e)
{
    // Check for click (left button pressed)
    if (static_cast<bool>(e.leftButton))
    {
        // Find the Desktop parent to notify of selection
        Control* p = Parent();
        while (p)
        {
            Desktop* desktop = p->AsDesktop();
            if (desktop)
            {
                desktop->SelectIcon(this);
                break;
            }
            p = p->Parent();
        }
    }
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
    , _startMenu(nullptr)
    , _iconContainer(nullptr)
    , _selectedIcon(nullptr)
    , _iconLibrary(nullptr)
{
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

    // Update client bounds (excludes taskbar)
    UpdateClientBounds();

    // Initialize cursor save buffer
    for (Int32 i = Int32(0); static_cast<int>(i) < CURSOR_SIZE * CURSOR_SIZE; i += 1)
    {
        _cursorSave[static_cast<int>(i)] = 0;
    }

    // Create icon container for flexbox-based icon layout
    // Container fills the client area (above taskbar)
    _iconContainer = new Control(this, _clientBounds);
    _iconContainer->Layout()
        .SetDirection(FlexDirection::Column)
        .SetWrap(FlexWrap::Wrap)
        .SetJustifyContent(JustifyContent::Start)
        .SetAlignItems(AlignItems::Start)
        .SetPadding(Int32(ICON_MARGIN_Y), Int32(ICON_MARGIN_X), Int32(0), Int32(ICON_MARGIN_X))
        .SetGap(Int32(0));  // No gap - icons have internal padding

    // Icon container does participate in layout but has fixed bounds
    _iconContainer->Layout().widthMode = SizeMode::Fixed;
    _iconContainer->Layout().heightMode = SizeMode::Fixed;
}

void Desktop::UpdateClientBounds()
{
    // Client area = everything except taskbar at bottom
    _clientBounds = Rectangle(
        Int32(0),
        Int32(0),
        _screenWidth,
        Int32(static_cast<int>(_screenHeight) - TASKBAR_HEIGHT)
    );
}

Desktop::~Desktop()
{
    delete _iconLibrary;
    _iconLibrary = nullptr;
}

void Desktop::SetIconLibrary(Drawing::IconLibrary* library)
{
    delete _iconLibrary;
    _iconLibrary = library;
}

void Desktop::SetCursor(const Image& cursorImage)
{
    _cursorImage = cursorImage;
}

void Desktop::LoadCursorFromLibrary(const char* path, Int32 iconIndex)
{
    _cursorImage = Image::FromIconLibrary(path, iconIndex, Size::IconMedium);
}

void Desktop::AddIcon(const Image& icon)
{
    AddIcon(icon, String());  // Call overload with empty text
}

void Desktop::AddIcon(const Image& icon, const String& text)
{
    // Create DesktopIconControl as a child of the icon container
    // The flexbox layout system handles positioning automatically
    if (_iconContainer)
    {
        new DesktopIconControl(_iconContainer, icon, text);
        _iconContainer->InvalidateLayout();
        _iconContainer->PerformLayout();
    }

    // Also maintain legacy _icons array for backward compatibility
    Int32 taskBarHeight = Int32(TASKBAR_HEIGHT);
    Int32 maxY = Int32(static_cast<int>(_screenHeight) - static_cast<int>(taskBarHeight) - ICON_CELL_HEIGHT - ICON_MARGIN_Y);

    Int32 oldLen = Int32(_icons.Length());
    _icons.Resize(static_cast<int>(oldLen) + 1);
    _icons[static_cast<int>(oldLen)] = DesktopIcon(icon, static_cast<int>(_nextIconX), static_cast<int>(_nextIconY));

    // Update legacy position tracking using new cell dimensions
    _nextIconY = Int32(static_cast<int>(_nextIconY) + ICON_CELL_HEIGHT);
    if (_nextIconY > maxY)
    {
        _nextIconY = Int32(ICON_MARGIN_Y);
        _nextIconX = Int32(static_cast<int>(_nextIconX) + ICON_CELL_WIDTH);
    }

    Invalidate();
}

void Desktop::AddIconFromLibrary(const char* path, Int32 iconIndex)
{
    Image icon = Image::FromIconLibrary(path, iconIndex, Size::IconMedium);
    AddIcon(icon);
}

void Desktop::AddIconFromLibrary(const char* path, Int32 iconIndex, const String& text)
{
    Image icon = Image::FromIconLibrary(path, iconIndex, Size::IconMedium);
    AddIcon(icon, text);
}

void Desktop::LoadCursorFromLibrary(const char* path, const char* iconName)
{
    _cursorImage = Image::FromIconLibrary(path, iconName, Size::IconMedium);
}

void Desktop::AddIconFromLibrary(const char* path, const char* iconName)
{
    Image icon = Image::FromIconLibrary(path, iconName, Size::IconMedium);
    AddIcon(icon);
}

void Desktop::AddIconFromLibrary(const char* path, const char* iconName, const String& text)
{
    Image icon = Image::FromIconLibrary(path, iconName, Size::IconMedium);
    AddIcon(icon, text);
}

void Desktop::SelectIcon(DesktopIconControl* icon)
{
    // Deselect previous icon if different
    if (_selectedIcon && _selectedIcon != icon)
    {
        _selectedIcon->SetSelected(Boolean(false));
    }

    // Select new icon
    _selectedIcon = icon;
    if (_selectedIcon)
    {
        _selectedIcon->SetSelected(Boolean(true));
    }

    Invalidate();
}

void Desktop::DrawIcons()
{
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb)
    {
        return;
    }

    Image& img = fb->GetImage();

    for (Int32 i = Int32(0); static_cast<int>(i) < _icons.Length(); i += 1)
    {
        const DesktopIcon& icon = _icons[static_cast<int>(i)];
        Int32 iw = icon.image.Width();
        Int32 ih = icon.image.Height();

        if (iw > Int32(0) && ih > Int32(0))
        {
            // Draw icon with alpha blending
            img.CopyFromWithAlpha(icon.image, icon.x, icon.y);
        }
    }
}

void Desktop::SetFocusedWindow(Window* window)
{
    if (_focusedWindow == window)
    {
        return;
    }

    // Unfocus previous window
    if (_focusedWindow)
    {
        _focusedWindow->SetFocused(Boolean(false));
    }

    _focusedWindow = window;

    // Focus new window and bring to front (move to end of children array)
    if (_focusedWindow)
    {
        _focusedWindow->SetFocused(Boolean(true));

        // Find and move to end for z-order
        Int32 idx = Int32(_children.IndexOf(static_cast<Control*>(_focusedWindow)));
        if (static_cast<int>(idx) >= 0 && static_cast<int>(idx) < _children.Length() - 1)
        {
            // Shift elements down
            for (Int32 i = idx; static_cast<int>(i) < _children.Length() - 1; i += 1)
            {
                _children[static_cast<int>(i)] = _children[static_cast<int>(i) + 1];
            }
            _children[_children.Length() - 1] = _focusedWindow;
        }

        // Update spatial grid after z-order change
        UpdateSpatialGrid();
    }

    // Refresh taskbar buttons to show focused state
    if (_taskBar)
    {
        _taskBar->RefreshWindowButtons();
    }

    Invalidate();
}

void Desktop::AddChild(Control* child)
{
    if (child)
    {
        // Call base class
        Control::AddChild(child);

        // Add to spatial grid
        _spatialGrid.Insert(child, child->ScreenBounds());

        // If this is a window, add a taskbar button
        Window* win = child->AsWindow();
        if (win && _taskBar)
        {
            _taskBar->AddWindowButton(win);
        }
    }
}

void Desktop::RemoveChild(Control* child)
{
    if (child)
    {
        // Remove from spatial grid
        _spatialGrid.Remove(child);

        // If this is a window, remove taskbar button
        Window* win = child->AsWindow();
        if (win && _taskBar)
        {
            _taskBar->RemoveWindowButton(win);
        }

        // Call base class
        Control::RemoveChild(child);
    }
}

void Desktop::UpdateSpatialGrid()
{
    _spatialGrid.Clear();
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
    {
        Control* child = _children[static_cast<int>(i)];
        if (child)
        {
            _spatialGrid.Insert(child, child->ScreenBounds());
        }
    }
}

void Desktop::SetWallpaper(const Image& wallpaper)
{
    // Scale wallpaper to fit screen if needed
    if (wallpaper.Width() != _screenWidth || wallpaper.Height() != _screenHeight)
    {
        _wallpaper = wallpaper.ScaleTo(_screenWidth, _screenHeight);
    }
    else
    {
        _wallpaper = wallpaper;
    }
    Invalidate();
}

void Desktop::OnPaint(PaintEventArgs& e)
{
    // Draw wallpaper if present, otherwise fill with background color
    if (_wallpaper.Width() > Int32(0) && _wallpaper.Height() > Int32(0))
    {
        GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
        if (fb)
        {
            Image& fbImg = fb->GetImage();
            fbImg.CopyFrom(_wallpaper, 0, 0);
        }
    }
    else
    {
        e.graphics->FillRectangle(_bounds, _backgroundColor);
    }

    // Icons are now painted as children of _iconContainer via flexbox layout
    // The legacy DrawIcons() is kept but only used if _iconContainer is null
    if (!_iconContainer)
    {
        DrawIcons();
    }

    // Set up clip bounds for children (the full screen)
    Rectangle screenClip(Int32(0), Int32(0), _screenWidth, _screenHeight);

    // Paint children sorted by z-index (lowest first)
    // First, paint all children with alwaysOnTop=false (normal controls)
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
    {
        Control* child = _children[static_cast<int>(i)];
        if (child && static_cast<bool>(child->IsVisible()) && !child->Layout().alwaysOnTop &&
            child != static_cast<Control*>(_taskBar) &&
            child != static_cast<Control*>(_startMenu))
        {
            PaintEventArgs childArgs(e.graphics, child->Bounds(), screenClip);
            child->OnPaint(childArgs);
        }
    }

    // Then paint alwaysOnTop children (except taskbar and start menu)
    for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
    {
        Control* child = _children[static_cast<int>(i)];
        if (child && static_cast<bool>(child->IsVisible()) && child->Layout().alwaysOnTop &&
            child != static_cast<Control*>(_taskBar) &&
            child != static_cast<Control*>(_startMenu))
        {
            PaintEventArgs childArgs(e.graphics, child->Bounds(), screenClip);
            child->OnPaint(childArgs);
        }
    }

    // Paint taskbar AFTER all windows (always on top of windows)
    if (_taskBar)
    {
        PaintEventArgs taskBarArgs(e.graphics, _taskBar->Bounds(), screenClip);
        _taskBar->OnPaint(taskBarArgs);
    }

    // Paint start menu LAST so it appears on top of everything
    if (_startMenu && static_cast<bool>(_startMenu->IsVisible()))
    {
        PaintEventArgs menuArgs(e.graphics, _startMenu->Bounds(), screenClip);
        _startMenu->OnPaint(menuArgs);
    }
}

void Desktop::SaveUnderCursor()
{
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb)
    {
        return;
    }

    Image& img = fb->GetImage();
    for (Int32 dy = Int32(0); static_cast<int>(dy) < CURSOR_SIZE; dy += 1)
    {
        for (Int32 dx = Int32(0); static_cast<int>(dx) < CURSOR_SIZE; dx += 1)
        {
            Int32 px = Int32(static_cast<int>(_cursorX) + static_cast<int>(dx));
            Int32 py = Int32(static_cast<int>(_cursorY) + static_cast<int>(dy));
            if (px >= Int32(0) && px < _screenWidth && py >= Int32(0) && py < _screenHeight)
            {
                _cursorSave[static_cast<int>(dy) * CURSOR_SIZE + static_cast<int>(dx)] = static_cast<unsigned int>(img.GetPixel(px, py));
            }
        }
    }
    _prevCursorX = _cursorX;
    _prevCursorY = _cursorY;
    _cursorSaved = true;
}

void Desktop::RestoreCursor()
{
    if (!_cursorSaved)
    {
        return;
    }

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb)
    {
        return;
    }

    Image& img = fb->GetImage();
    for (Int32 dy = Int32(0); static_cast<int>(dy) < CURSOR_SIZE; dy += 1)
    {
        for (Int32 dx = Int32(0); static_cast<int>(dx) < CURSOR_SIZE; dx += 1)
        {
            Int32 px = Int32(static_cast<int>(_prevCursorX) + static_cast<int>(dx));
            Int32 py = Int32(static_cast<int>(_prevCursorY) + static_cast<int>(dy));
            if (px >= Int32(0) && px < _screenWidth && py >= Int32(0) && py < _screenHeight)
            {
                img.SetPixel(px, py, Color(_cursorSave[static_cast<int>(dy) * CURSOR_SIZE + static_cast<int>(dx)]));
            }
        }
    }
}

void Desktop::DrawCursor()
{
    if (!_cursorVisible)
    {
        return;
    }

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb)
    {
        return;
    }

    Image& img = fb->GetImage();

    Int32 cursorW = _cursorImage.Width();
    Int32 cursorH = _cursorImage.Height();

    // If we have a cursor image, draw it with alpha blending
    if (cursorW > Int32(0) && cursorH > Int32(0))
    {
        for (Int32 dy = Int32(0); static_cast<int>(dy) < static_cast<int>(cursorH) && static_cast<int>(dy) < CURSOR_SIZE; dy += 1)
        {
            for (Int32 dx = Int32(0); static_cast<int>(dx) < static_cast<int>(cursorW) && static_cast<int>(dx) < CURSOR_SIZE; dx += 1)
            {
                Int32 px = Int32(static_cast<int>(_cursorX) + static_cast<int>(dx));
                Int32 py = Int32(static_cast<int>(_cursorY) + static_cast<int>(dy));
                if (px >= Int32(0) && px < _screenWidth && py >= Int32(0) && py < _screenHeight)
                {
                    Color pixel = _cursorImage.GetPixel(dx, dy);
                    // Only draw non-transparent pixels (alpha >= 128)
                    UInt32 alpha = UInt32(static_cast<unsigned int>(pixel.A()));
                    if (static_cast<unsigned int>(alpha) >= 128)
                    {
                        img.SetPixel(px, py, pixel);
                    }
                }
            }
        }
    }
    else
    {
        // Fallback: draw a simple arrow cursor
        for (Int32 dy = Int32(0); static_cast<int>(dy) < CURSOR_SIZE; dy += 1)
        {
            for (Int32 dx = Int32(0); static_cast<int>(dx) < CURSOR_SIZE; dx += 1)
            {
                Int32 px = Int32(static_cast<int>(_cursorX) + static_cast<int>(dx));
                Int32 py = Int32(static_cast<int>(_cursorY) + static_cast<int>(dy));
                if (px >= Int32(0) && px < _screenWidth && py >= Int32(0) && py < _screenHeight)
                {
                    // Simple arrow shape
                    Boolean isArrow = Boolean(static_cast<int>(dx) <= static_cast<int>(dy) && static_cast<int>(dx) < 12 && static_cast<int>(dy) < 18);
                    Boolean isBorder = Boolean(static_cast<bool>(isArrow) && (static_cast<int>(dx) == 0 || dx == dy || static_cast<int>(dy) == 17));
                    if (static_cast<bool>(isArrow))
                    {
                        img.SetPixel(px, py, static_cast<bool>(isBorder) ? Color::Black : Color::White);
                    }
                }
            }
        }
    }
}

void Desktop::CaptureWindowBitmap(Window* win)
{
    if (!win)
    {
        return;
    }

    // Store the window's starting position for offset calculation
    Rectangle screen = win->ScreenBounds();
    _dragStartX = screen.x;
    _dragStartY = screen.y;

    // We don't actually capture a bitmap anymore - we'll paint the window
    // directly during the drag loop. This fixes the clipping issue where
    // windows dragged off-screen and back would have black portions.
}

void Desktop::DrawDragBitmap()
{
    if (!_isDragging || !_dragWindow)
    {
        return;
    }

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb)
    {
        return;
    }

    // Calculate current drag position
    Int32 newX = Int32(static_cast<int>(_cursorX) - static_cast<int>(_dragOffsetX));
    Int32 newY = Int32(static_cast<int>(_cursorY) - static_cast<int>(_dragOffsetY));

    // Save the window's original bounds
    Rectangle originalBounds = _dragWindow->Bounds();

    // Temporarily move the window to the drag position
    _dragWindow->SetBounds(newX, newY, originalBounds.width, originalBounds.height);

    // Paint the window at its new position with screen clipping
    Graphics g(BufferMode::Single, _bounds);
    Rectangle screenClip(Int32(0), Int32(0), _screenWidth, _screenHeight);
    PaintEventArgs e(&g, _dragWindow->Bounds(), screenClip);
    _dragWindow->OnPaint(e);

    // Restore the original bounds (the position will be updated on drop)
    _dragWindow->SetBounds(originalBounds);
}

void Desktop::OnKeyboard(KeyboardEventArgs& e)
{
    // ESC key exits
    if (static_cast<char>(e.key) == 27)  // 27 = ESC
    {
        Stop();
    }
}

void Desktop::HandleMouse(MouseEventArgs& e)
{
    Int32 ex = e.x;
    Int32 ey = e.y;
    Boolean leftButton = e.leftButton;
    Boolean isNewClick = Boolean(static_cast<bool>(leftButton) && !_wasMouseDown);

    // Handle ongoing drag
    if (_isDragging && _dragWindow && static_cast<bool>(leftButton))
    {
        // During drag, just update position - bitmap is drawn in Run()
        _wasMouseDown = static_cast<bool>(leftButton);
        return;
    }

    // End drag if mouse released
    if (!static_cast<bool>(leftButton) && _isDragging && _dragWindow)
    {
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
    if (_startMenu && static_cast<bool>(_startMenu->IsVisible()))
    {
        Boolean mouseOnMenu = _startMenu->HitTest(ex, ey);
        Boolean clickOnStartButton = Boolean(_taskBar && _taskBar->StartButton() &&
            static_cast<bool>(_taskBar->StartButton()->HitTest(ex, ey)));

        // Hide menu if clicked outside
        if (static_cast<bool>(isNewClick) && !static_cast<bool>(mouseOnMenu) && !static_cast<bool>(clickOnStartButton))
        {
            _startMenu->Hide();
            Invalidate();
        }

        // Always send mouse events to menu for hover tracking
        _startMenu->OnMouse(e);

        // If mouse is on menu, don't process other controls
        if (static_cast<bool>(mouseOnMenu))
        {
            _wasMouseDown = static_cast<bool>(leftButton);
            return;
        }
    }

    // Find child under cursor (for event propagation)
    Control* hitChild = _spatialGrid.HitTest(ex, ey);

    // Fall back to linear search if spatial grid misses (e.g., for TaskBar)
    if (!hitChild)
    {
        for (Int32 i = Int32(_children.Length() - 1); static_cast<int>(i) >= 0; i -= 1)
        {
            Control* child = _children[static_cast<int>(i)];
            if (child && static_cast<bool>(child->HitTest(ex, ey)))
            {
                hitChild = child;
                break;
            }
        }
    }

    // Handle window focus and drag initiation on new click only
    if (static_cast<bool>(isNewClick) && hitChild)
    {
        Window* win = hitChild->AsWindow();
        if (win)
        {
            Rectangle screen = hitChild->ScreenBounds();
            Int32 sy = screen.y;
            Int32 sx = screen.x;

            SetFocusedWindow(win);

            // Check if click is on title bar (top 22 pixels)
            if (static_cast<int>(ey) < static_cast<int>(sy) + 22)
            {
                _dragWindow = win;
                _dragOffsetX = Int32(static_cast<int>(ex) - static_cast<int>(sx));
                _dragOffsetY = Int32(static_cast<int>(ey) - static_cast<int>(sy));

                // Capture the window as a bitmap before starting drag
                Invalidate();
                // Force repaint to get clean image
                GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
                if (fb)
                {
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

    // Deselect desktop icon if clicking outside of icon container
    // The icon's OnMouse will re-select if an icon was actually clicked
    if (static_cast<bool>(isNewClick))
    {
        // Check if click was outside the icon container
        bool clickedOnIconContainer = (hitChild == _iconContainer);

        // Also check if we clicked on a child of the icon container
        if (!clickedOnIconContainer && hitChild)
        {
            Control* parent = hitChild->Parent();
            while (parent)
            {
                if (parent == _iconContainer)
                {
                    clickedOnIconContainer = true;
                    break;
                }
                parent = parent->Parent();
            }
        }

        // If click was not on icon container or its children, deselect
        if (!clickedOnIconContainer && _selectedIcon)
        {
            SelectIcon(nullptr);
        }
    }

    // ALWAYS propagate mouse events to children (for button state tracking)
    if (hitChild)
    {
        hitChild->NotifyMouse(e);
    }

    _wasMouseDown = static_cast<bool>(leftButton);
}

void Desktop::CheckForUpdates()
{
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
    if (static_cast<bool>(Keyboard::IsKeyPressed()))
    {
        Char key = Keyboard::ReadKey();
        KeyboardStatus ks = Keyboard::GetStatus();
        KeyboardEventArgs keyArgs(key, ks.altPressed, ks.ctrlPressed, ks.shiftPressed);

        // Route to focused window first, then handle ourselves
        if (_focusedWindow)
        {
            _focusedWindow->OnKeyboard(keyArgs);
        }
        OnKeyboard(keyArgs);
    }
}

void Desktop::Run()
{
    _running = true;
    _isInvalid = true;  // Force initial paint

    // Do initial paint before fade in
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (fb)
    {
        Graphics g(BufferMode::Single, _bounds);
        PaintEventArgs e(&g, _bounds);
        OnPaint(e);
        SaveUnderCursor();
        DrawCursor();
        GraphicsBuffer::FlushFrameBuffer();
    }
    _isInvalid = false;


    while (_running)
    {
        // Wait for vertical sync to limit frame rate (~60fps)
        Display::WaitForVSync();

        // Check for input
        CheckForUpdates();

        if (_isDragging && _dragWindow)
        {
            // During drag: repaint scene without dragged window, then draw bitmap
            RestoreCursor();
            _cursorSaved = false;

            GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
            if (fb)
            {
                // Paint everything except the dragged window
                Graphics g(BufferMode::Single, _bounds);
                Rectangle screenClip(Int32(0), Int32(0), _screenWidth, _screenHeight);

                // Fill background
                g.FillRectangle(_bounds, _backgroundColor);

                // Draw desktop icons - only use legacy method if no icon container
                if (!_iconContainer)
                {
                    DrawIcons();
                }

                // Paint children except dragged window, respecting z-order
                // First paint non-alwaysOnTop children
                for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
                {
                    Control* child = _children[static_cast<int>(i)];
                    if (child && child != _dragWindow &&
                        static_cast<bool>(child->IsVisible()) &&
                        !child->Layout().alwaysOnTop &&
                        child != static_cast<Control*>(_taskBar) &&
                        child != static_cast<Control*>(_startMenu))
                    {
                        PaintEventArgs childArgs(&g, child->ScreenBounds(), screenClip);
                        child->OnPaint(childArgs);
                    }
                }

                // Paint alwaysOnTop children except taskbar/startmenu
                for (Int32 i = Int32(0); static_cast<int>(i) < _children.Length(); i += 1)
                {
                    Control* child = _children[static_cast<int>(i)];
                    if (child && child != _dragWindow &&
                        static_cast<bool>(child->IsVisible()) &&
                        child->Layout().alwaysOnTop &&
                        child != static_cast<Control*>(_taskBar) &&
                        child != static_cast<Control*>(_startMenu))
                    {
                        PaintEventArgs childArgs(&g, child->ScreenBounds(), screenClip);
                        child->OnPaint(childArgs);
                    }
                }

                // Draw the drag bitmap at current mouse position
                DrawDragBitmap();

                // Paint taskbar AFTER dragged window (taskbar is always on top)
                if (_taskBar && _taskBar != static_cast<Control*>(_dragWindow))
                {
                    PaintEventArgs taskBarArgs(&g, _taskBar->ScreenBounds(), screenClip);
                    _taskBar->OnPaint(taskBarArgs);
                }

                // Paint start menu LAST if visible
                if (_startMenu && static_cast<bool>(_startMenu->IsVisible()))
                {
                    PaintEventArgs menuArgs(&g, _startMenu->ScreenBounds(), screenClip);
                    _startMenu->OnPaint(menuArgs);
                }
            }

            SaveUnderCursor();
            DrawCursor();
            GraphicsBuffer::FlushFrameBuffer();
        }
        else if (_isInvalid)
        {
            // Normal repaint
            RestoreCursor();
            _cursorSaved = false;

            GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
            if (fb)
            {
                Graphics g(BufferMode::Single, _bounds);
                PaintEventArgs e(&g, _bounds);
                OnPaint(e);
            }
            _isInvalid = false;

            SaveUnderCursor();
            DrawCursor();
            GraphicsBuffer::FlushFrameBuffer();
        }
        else
        {
            // Just update cursor position
            Boolean cursorMoved = Boolean(_cursorX != _prevCursorX || _cursorY != _prevCursorY);
            if (static_cast<bool>(cursorMoved))
            {
                RestoreCursor();
                SaveUnderCursor();
                DrawCursor();
                GraphicsBuffer::FlushFrameBuffer();
            }
        }
    }

}

void Desktop::Stop()
{
    _running = false;
}

/******************************************************************************/
/*    Window Implementation                                                   */
/******************************************************************************/

// Helper function to load window title font
// Tries TTF first, falls back to FON (MS Sans Serif Bold)
static Font LoadWindowFont()
{
    try
    {
        // Try ProggyClean TTF at 13 pixels
        return Font::FromTrueType("PROGGY.TTF", 13, FontStyle::Bold);
    }
    catch (...)
    {
        // Fall back to MS Sans Serif Bold
        return Font::SystemFontBold();
    }
}

Window::Window(Control* parent, const Rectangle& bounds)
    : Control(parent, bounds)
    , _isFocused(false)
    , _title()
    , _font(LoadWindowFont())
    , _backColor(Color::Gray)
    , _isMaximized(false)
    , _isMinimized(false)
    , _restoreBounds(bounds)
    , _borderStyle(BorderStyle::RaisedDouble)
    , _closeIcon()
    , _maximizeIcon()
    , _minimizeIcon()
    , _restoreIcon()
{
    // Must call UpdateClientBounds again because virtual dispatch doesn't work
    // from base class constructor (Control called Control::UpdateClientBounds)
    UpdateClientBounds();

    // Windows are floating - they don't participate in parent's layout
    _layout.participatesInLayout = false;

    // Register with taskbar - must be done here because virtual dispatch doesn't
    // work during base class constructor (AsWindow() returns nullptr there)
    if (parent && parent->GetControlType() == ControlType::Desktop)
    {
        Desktop* desktop = static_cast<Desktop*>(parent);
        TaskBar* taskBar = desktop->GetTaskBar();
        if (taskBar)
        {
            taskBar->AddWindowButton(this);
        }
    }
}

Window::~Window()
{
}

void Window::UpdateClientBounds()
{
    // Client area is relative to window bounds (not screen)
    // Normal: Outer frame: 2 pixels, title bar: 20 pixels, client sunken border: 1 pixel
    // Maximized: No frame, title bar: 20 pixels, client sunken border: 1 pixel
    Int32 bw = _bounds.width;
    Int32 bh = _bounds.height;
    int frame = _isMaximized ? 0 : FRAME_WIDTH;
    _clientBounds = Rectangle(
        Int32(frame),
        Int32(TITLE_BAR_HEIGHT + frame),
        Int32(static_cast<int>(bw) - frame * 2),
        Int32(static_cast<int>(bh) - TITLE_BAR_HEIGHT - frame * 2)
    );
}

Rectangle Window::GetCloseButtonRect() const
{
    Rectangle screen = ScreenBounds();
    int frame = _isMaximized ? 0 : FRAME_WIDTH;
    // Close button is rightmost, frame+2px from right edge of title bar
    Int32 btnX = Int32(static_cast<int>(screen.x) + static_cast<int>(screen.width) - BUTTON_SIZE - frame - 2);
    Int32 btnY = Int32(static_cast<int>(screen.y) + frame + (TITLE_BAR_HEIGHT - BUTTON_SIZE) / 2);
    return Rectangle(btnX, btnY, Int32(BUTTON_SIZE), Int32(BUTTON_SIZE));
}

Rectangle Window::GetMaximizeButtonRect() const
{
    Rectangle closeBtn = GetCloseButtonRect();
    // Maximize button is left of close button
    Int32 btnX = Int32(static_cast<int>(closeBtn.x) - BUTTON_SIZE - BUTTON_SPACING);
    return Rectangle(btnX, closeBtn.y, Int32(BUTTON_SIZE), Int32(BUTTON_SIZE));
}

Rectangle Window::GetMinimizeButtonRect() const
{
    Rectangle maxBtn = GetMaximizeButtonRect();
    // Minimize button is left of maximize button
    Int32 btnX = Int32(static_cast<int>(maxBtn.x) - BUTTON_SIZE - BUTTON_SPACING);
    return Rectangle(btnX, maxBtn.y, Int32(BUTTON_SIZE), Int32(BUTTON_SIZE));
}

void Window::Minimize()
{
    if (!_isMinimized)
    {
        _isMinimized = true;
        SetVisible(Boolean(false));
        Invalidate();
    }
}

void Window::Maximize()
{
    if (!_isMaximized)
    {
        // Save current bounds for restore
        _restoreBounds = _bounds;
        _isMaximized = true;

        // Change border style to None when maximized (no frame)
        _borderStyle = BorderStyle::None;

        // Get desktop client bounds (excludes taskbar)
        Control* p = Parent();
        if (p && p->GetControlType() == ControlType::Desktop)
        {
            Desktop* desktop = static_cast<Desktop*>(p);
            Rectangle clientArea = desktop->ClientBounds();
            // Maximize to fill desktop client area, starting at (0,0)
            SetBounds(Int32(0), Int32(0), clientArea.width, clientArea.height);
        }
        Invalidate();
    }
}

void Window::Restore()
{
    if (_isMaximized)
    {
        _isMaximized = false;
        // Restore border style to RaisedDouble
        _borderStyle = BorderStyle::RaisedDouble;
        SetBounds(_restoreBounds.x, _restoreBounds.y, _restoreBounds.width, _restoreBounds.height);
        Invalidate();
    }
    if (_isMinimized)
    {
        _isMinimized = false;
        SetVisible(Boolean(true));
        Invalidate();
    }
}

void Window::Close()
{
    // Remove from parent
    Control* p = Parent();
    if (p)
    {
        // Remove from taskbar if applicable
        if (p->GetControlType() == ControlType::Desktop)
        {
            Desktop* desktop = static_cast<Desktop*>(p);
            TaskBar* taskBar = desktop->GetTaskBar();
            if (taskBar)
            {
                taskBar->RemoveWindowButton(this);
            }
            // Clear focus if this window had it
            if (desktop->FocusedWindow() == this)
            {
                desktop->SetFocusedWindow(nullptr);
            }
        }
        p->RemoveChild(this);
    }
    // Note: Window is now orphaned. Caller should delete it or it will leak.
    // In a real implementation, we might want to mark it for deletion.
}

void Window::LoadButtonIcons()
{
    // Get desktop's icon library
    Control* p = Parent();
    if (!p || p->GetControlType() != ControlType::Desktop)
    {
        return;
    }

    Desktop* desktop = static_cast<Desktop*>(p);
    Drawing::IconLibrary* icons = desktop->GetIconLibrary();
    if (!icons)
    {
        return;
    }

    // Load 16x16 button icons
    try
    {
        _closeIcon = icons->FromName("ui-close", Drawing::IconSize::Small);
    }
    catch (...)
    {
        // Icon loading failed - will use fallback
    }

    try
    {
        _maximizeIcon = icons->FromName("ui-maximize", Drawing::IconSize::Small);
    }
    catch (...)
    {
        // Icon loading failed - will use fallback
    }

    try
    {
        _minimizeIcon = icons->FromName("ui-minimize", Drawing::IconSize::Small);
    }
    catch (...)
    {
        // Icon loading failed - will use fallback
    }

    try
    {
        _restoreIcon = icons->FromName("ui-restore", Drawing::IconSize::Small);
    }
    catch (...)
    {
        // Icon loading failed - will use fallback
    }

    Invalidate();
}

void Window::OnPaint(PaintEventArgs& e)
{
    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 sw = screen.width;
    Int32 sh = screen.height;

    // Frame offset: 0 when maximized (no frame), FRAME_WIDTH when normal
    int frame = _isMaximized ? 0 : FRAME_WIDTH;

    // Draw window frame with current border style
    // Maximized windows use None (no frame), normal windows use RaisedDouble (Window style)
    BorderStyle frameStyle = _isMaximized ? BorderStyle::None : BorderStyle::Window;
    e.graphics->FillRectangle(screen, frameStyle);

    // Draw title bar - gradient if focused, solid dark gray if not
    Rectangle titleBar(Int32(static_cast<int>(sx) + frame), Int32(static_cast<int>(sy) + frame),
                       Int32(static_cast<int>(sw) - frame * 2), Int32(TITLE_BAR_HEIGHT));
    if (_isFocused)
    {
        // Horizontal gradient from dark blue (left) to lighter blue (right)
        Color leftColor(UInt32(0xFF000080));   // Dark blue
        Color rightColor(UInt32(0xFF1084D0));  // Lighter blue
        Int32 titleX = Int32(static_cast<int>(sx) + frame);
        Int32 titleY = Int32(static_cast<int>(sy) + frame);
        Int32 titleW = Int32(static_cast<int>(sw) - frame * 2);
        Int32 titleH = Int32(TITLE_BAR_HEIGHT);

        // Draw vertical lines with interpolated colors
        for (Int32 col = Int32(0); static_cast<int>(col) < static_cast<int>(titleW); col += 1)
        {
            Float32 t = static_cast<float>(static_cast<int>(col)) / static_cast<float>(static_cast<int>(titleW) - 1);
            Color lineColor = Color::Lerp(leftColor, rightColor, t);
            Int32 lineX = Int32(static_cast<int>(titleX) + static_cast<int>(col));
            e.graphics->DrawLine(lineX, titleY, lineX, Int32(static_cast<int>(titleY) + static_cast<int>(titleH) - 1), lineColor);
        }
    }
    else
    {
        e.graphics->FillRectangle(titleBar, Color::DarkGray);
    }

    // Draw title text (white, centered vertically, left-aligned with padding)
    if (_title.Length() > Int32(0) && static_cast<bool>(_font.IsValid()))
    {
        Int32 textX = Int32(static_cast<int>(sx) + frame + 4);  // Left padding
        Int32 textY = Int32(static_cast<int>(sy) + frame + (TITLE_BAR_HEIGHT - static_cast<int>(_font.Height())) / 2);
        e.graphics->DrawString(_title, _font, Color::White, textX, textY);
    }

    // Draw title bar buttons (minimize, maximize, close)
    Rectangle closeBtn = GetCloseButtonRect();
    Rectangle maxBtn = GetMaximizeButtonRect();
    Rectangle minBtn = GetMinimizeButtonRect();

    // Draw buttons with raised 3D effect
    e.graphics->FillRectangle(minBtn, BorderStyle::Raised);
    e.graphics->FillRectangle(maxBtn, BorderStyle::Raised);
    e.graphics->FillRectangle(closeBtn, BorderStyle::Raised);

    // Get framebuffer for icon drawing
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();

    // Draw button icons if loaded, otherwise fall back to line symbols
    if (_minimizeIcon.Width() > Int32(0) && fb)
    {
        Image& fbImg = fb->GetImage();
        fbImg.CopyFromWithAlpha(_minimizeIcon, static_cast<int>(minBtn.x), static_cast<int>(minBtn.y));
    }
    else
    {
        // Fallback: horizontal line at bottom
        Color btnColor = Color::Black;
        Int32 minLineY = Int32(static_cast<int>(minBtn.y) + static_cast<int>(minBtn.height) - 5);
        e.graphics->DrawLine(Int32(static_cast<int>(minBtn.x) + 4), minLineY,
                             Int32(static_cast<int>(minBtn.x) + static_cast<int>(minBtn.width) - 5), minLineY, btnColor);
    }

    // Maximize/Restore button - use restore icon if maximized
    const Image& maxIcon = _isMaximized ? _restoreIcon : _maximizeIcon;
    if (maxIcon.Width() > Int32(0) && fb)
    {
        Image& fbImg = fb->GetImage();
        fbImg.CopyFromWithAlpha(maxIcon, static_cast<int>(maxBtn.x), static_cast<int>(maxBtn.y));
    }
    else
    {
        // Fallback: rectangle outline
        Color btnColor = Color::Black;
        Int32 maxLeft = Int32(static_cast<int>(maxBtn.x) + 3);
        Int32 maxTop = Int32(static_cast<int>(maxBtn.y) + 3);
        Int32 maxRight = Int32(static_cast<int>(maxBtn.x) + static_cast<int>(maxBtn.width) - 4);
        Int32 maxBottom = Int32(static_cast<int>(maxBtn.y) + static_cast<int>(maxBtn.height) - 4);
        e.graphics->DrawLine(maxLeft, maxTop, maxRight, maxTop, btnColor);
        e.graphics->DrawLine(maxLeft, Int32(static_cast<int>(maxTop) + 1), maxRight, Int32(static_cast<int>(maxTop) + 1), btnColor);
        e.graphics->DrawLine(maxLeft, maxTop, maxLeft, maxBottom, btnColor);
        e.graphics->DrawLine(maxRight, maxTop, maxRight, maxBottom, btnColor);
        e.graphics->DrawLine(maxLeft, maxBottom, maxRight, maxBottom, btnColor);
    }

    // Close button
    if (_closeIcon.Width() > Int32(0) && fb)
    {
        Image& fbImg = fb->GetImage();
        fbImg.CopyFromWithAlpha(_closeIcon, static_cast<int>(closeBtn.x), static_cast<int>(closeBtn.y));
    }
    else
    {
        // Fallback: X shape
        Color btnColor = Color::Black;
        Int32 closeLeft = Int32(static_cast<int>(closeBtn.x) + 4);
        Int32 closeTop = Int32(static_cast<int>(closeBtn.y) + 4);
        Int32 closeRight = Int32(static_cast<int>(closeBtn.x) + static_cast<int>(closeBtn.width) - 5);
        Int32 closeBottom = Int32(static_cast<int>(closeBtn.y) + static_cast<int>(closeBtn.height) - 5);
        e.graphics->DrawLine(closeLeft, closeTop, closeRight, closeBottom, btnColor);
        e.graphics->DrawLine(closeRight, closeTop, closeLeft, closeBottom, btnColor);
    }

    // Draw client area with sunken border effect
    Rectangle clientFrame(Int32(static_cast<int>(sx) + frame),
                          Int32(static_cast<int>(sy) + TITLE_BAR_HEIGHT + frame),
                          Int32(static_cast<int>(sw) - frame * 2),
                          Int32(static_cast<int>(sh) - TITLE_BAR_HEIGHT - frame * 2));
    e.graphics->FillRectangle(clientFrame, BorderStyle::Sunken);

    // Fill client area interior with back color (inside the sunken border)
    Rectangle clientInterior(Int32(static_cast<int>(sx) + frame + 1),
                             Int32(static_cast<int>(sy) + TITLE_BAR_HEIGHT + frame + 1),
                             Int32(static_cast<int>(sw) - frame * 2 - 2),
                             Int32(static_cast<int>(sh) - TITLE_BAR_HEIGHT - frame * 2 - 2));
    e.graphics->FillRectangle(clientInterior, _backColor);

    // Paint children in client area
    OnPaintClient(e);
}

void Window::OnMouse(MouseEventArgs& e)
{
    // Check for button clicks (only on mouse down)
    if (static_cast<bool>(e.leftButton))
    {
        Int32 mx = e.x;
        Int32 my = e.y;

        // Check close button
        Rectangle closeBtn = GetCloseButtonRect();
        if (static_cast<int>(mx) >= static_cast<int>(closeBtn.x) &&
            static_cast<int>(mx) < static_cast<int>(closeBtn.x) + static_cast<int>(closeBtn.width) &&
            static_cast<int>(my) >= static_cast<int>(closeBtn.y) &&
            static_cast<int>(my) < static_cast<int>(closeBtn.y) + static_cast<int>(closeBtn.height))
        {
            Close();
            return;
        }

        // Check maximize button
        Rectangle maxBtn = GetMaximizeButtonRect();
        if (static_cast<int>(mx) >= static_cast<int>(maxBtn.x) &&
            static_cast<int>(mx) < static_cast<int>(maxBtn.x) + static_cast<int>(maxBtn.width) &&
            static_cast<int>(my) >= static_cast<int>(maxBtn.y) &&
            static_cast<int>(my) < static_cast<int>(maxBtn.y) + static_cast<int>(maxBtn.height))
        {
            if (_isMaximized)
            {
                Restore();
            }
            else
            {
                Maximize();
            }
            return;
        }

        // Check minimize button
        Rectangle minBtn = GetMinimizeButtonRect();
        if (static_cast<int>(mx) >= static_cast<int>(minBtn.x) &&
            static_cast<int>(mx) < static_cast<int>(minBtn.x) + static_cast<int>(minBtn.width) &&
            static_cast<int>(my) >= static_cast<int>(minBtn.y) &&
            static_cast<int>(my) < static_cast<int>(minBtn.y) + static_cast<int>(minBtn.height))
        {
            Minimize();
            return;
        }
    }
}

/******************************************************************************/
/*    TaskBar Implementation                                                  */
/******************************************************************************/

void TaskBar::OnStartButtonClick(Button* sender, void* userData)
{
    (void)sender;
    TaskBar* taskBar = static_cast<TaskBar*>(userData);
    if (taskBar && taskBar->_startMenu)
    {
        taskBar->_startMenu->Toggle();
        if (taskBar->_desktop)
        {
            taskBar->_desktop->Invalidate();
        }
    }
}

TaskBar::TaskBar(Control* parent, StartMenu* startMenu)
    : Control(parent, Rectangle(Int32(0), Int32(0), Int32(0), Int32(32)))  // Temporary bounds, updated below
    , _startButton(nullptr)
    , _startMenu(startMenu)
    , _desktop(nullptr)
    , _windowButtons()
    , _taskTray(nullptr)
{
    // Get screen dimensions from current display mode
    Display current = Display::GetCurrent();
    Int32 screenWidth = Int32(static_cast<int>(current.Width()));
    Int32 screenHeight = Int32(static_cast<int>(current.Height()));
    Int32 taskBarHeight = Int32(32);

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

    // TaskBar is always on top with highest z-index
    _layout.alwaysOnTop = true;
    _layout.zIndex = Int32(1000);

    // Create Start button (positioned relative to taskbar, not screen)
    // Make it wider to fit icon + bold text
    _startButton = new Button(this, Rectangle(Int32(4), Int32(4), Int32(65), Int32(24)));
    _startButton->SetText("Start");
    _startButton->SetOnClick(OnStartButtonClick, this);

    // Use bold font for "Start"
    _startButton->SetFont(Font::SystemFontBold());

    // Start button has fixed size
    _startButton->Layout().widthMode = SizeMode::Fixed;
    _startButton->Layout().heightMode = SizeMode::Fixed;

    // Create task tray on the right side
    _taskTray = new TaskTray(this);

    // Note: Icons are loaded later via LoadIcons() when the icon library is available

    // Position task tray on the right side
    Int32 trayWidth = _taskTray->CalculateWidth();
    Int32 trayHeight = Int32(24);  // Same height as buttons
    Int32 trayX = Int32(static_cast<int>(screenWidth) - static_cast<int>(trayWidth) - 4);  // 4px right margin
    Int32 trayY = Int32(4);  // 4px top margin
    _taskTray->SetBounds(trayX, trayY, trayWidth, trayHeight);

    // Task tray has fixed size and doesn't participate in layout
    _taskTray->Layout().participatesInLayout = false;
}

TaskBar::~TaskBar()
{
    // Children are deleted by Control destructor
    // _windowButtons are children, so they get deleted too
}

void TaskBar::LoadIcons()
{
    if (!_desktop)
    {
        return;
    }

    Drawing::IconLibrary* icons = _desktop->GetIconLibrary();
    if (!icons)
    {
        return;
    }

    // Load Start button icon
    try
    {
        Image startIcon = icons->FromIndex(Int32(0), Drawing::IconSize::Small);
        _startButton->SetIcon(startIcon);
    }
    catch (...)
    {
        // If icon loading fails, continue without icon
    }

    // Load task tray icons
    try
    {
        Image soundIcon = icons->FromName("sound", Drawing::IconSize::Small);
        _taskTray->AddIcon(soundIcon);

        Image txSendIcon = icons->FromName("tx-send", Drawing::IconSize::Small);
        _taskTray->AddIcon(txSendIcon);

        Image networkIcon = icons->FromName("network-signal-2", Drawing::IconSize::Small);
        _taskTray->AddIcon(networkIcon);

        Image shieldIcon = icons->FromName("shield-danger", Drawing::IconSize::Small);
        _taskTray->AddIcon(shieldIcon);
    }
    catch (...)
    {
        // If icon loading fails, continue without icons
    }

    // Reposition task tray now that icons are loaded
    Display current = Display::GetCurrent();
    Int32 screenWidth = Int32(static_cast<int>(current.Width()));
    Int32 trayWidth = _taskTray->CalculateWidth();
    Int32 trayHeight = Int32(24);
    Int32 trayX = Int32(static_cast<int>(screenWidth) - static_cast<int>(trayWidth) - 4);
    Int32 trayY = Int32(4);
    _taskTray->SetBounds(trayX, trayY, trayWidth, trayHeight);

    Invalidate();
}

/******************************************************************************/
/*    TaskTray Implementation                                                 */
/******************************************************************************/

TaskTray::TaskTray(Control* parent)
    : Control(parent, Rectangle(Int32(0), Int32(0), Int32(0), Int32(24)))
    , _icons()
{
}

void TaskTray::AddIcon(const Image& icon)
{
    Int32 oldLen = Int32(_icons.Length());
    _icons.Resize(static_cast<int>(oldLen) + 1);
    _icons[static_cast<int>(oldLen)] = icon;
    Invalidate();
}

void TaskTray::OnPaint(PaintEventArgs& e)
{
    Rectangle screen = ScreenBounds();

    // Draw sunken border
    e.graphics->FillRectangle(screen, BorderStyle::Sunken);

    // Draw icons
    Int32 iconX = Int32(static_cast<int>(screen.x) + PADDING);
    Int32 iconY = Int32(static_cast<int>(screen.y) + (static_cast<int>(screen.height) - ICON_SIZE) / 2);

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (fb)
    {
        Image& targetImg = fb->GetImage();
        for (int i = 0; i < _icons.Length(); i++)
        {
            targetImg.CopyFromWithAlpha(_icons[i], iconX, iconY);
            iconX = Int32(static_cast<int>(iconX) + ICON_SIZE + ICON_SPACING);
        }
    }

    // Paint children
    OnPaintClient(e);
}

MeasureResult TaskTray::GetPreferredSize() const
{
    return MeasureResult(CalculateWidth(), Int32(24));
}

void TaskBar::AddWindowButton(Window* window)
{
    if (!window)
    {
        return;
    }

    // Check if button already exists
    for (Int32 i = Int32(0); static_cast<int>(i) < _windowButtons.Length(); i += 1)
    {
        if (_windowButtons[static_cast<int>(i)]->GetWindow() == window)
        {
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

void TaskBar::RemoveWindowButton(Window* window)
{
    if (!window)
    {
        return;
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < _windowButtons.Length(); i += 1)
    {
        if (_windowButtons[static_cast<int>(i)]->GetWindow() == window)
        {
            TaskBarButton* btn = _windowButtons[static_cast<int>(i)];

            // Remove from array
            for (Int32 j = i; static_cast<int>(j) < _windowButtons.Length() - 1; j += 1)
            {
                _windowButtons[static_cast<int>(j)] = _windowButtons[static_cast<int>(j) + 1];
            }
            _windowButtons.Resize(_windowButtons.Length() - 1);

            // Remove from control hierarchy and delete
            RemoveChild(btn);
            delete btn;

            // Reposition remaining buttons (relative to taskbar)
            for (Int32 k = Int32(0); static_cast<int>(k) < _windowButtons.Length(); k += 1)
            {
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

void TaskBar::RefreshWindowButtons()
{
    if (!_desktop)
    {
        return;
    }

    Window* focused = _desktop->FocusedWindow();
    for (Int32 i = Int32(0); static_cast<int>(i) < _windowButtons.Length(); i += 1)
    {
        TaskBarButton* btn = _windowButtons[static_cast<int>(i)];
        btn->SetPressed(Boolean(btn->GetWindow() == focused));
    }
    Invalidate();
}

TaskBarButton* TaskBar::FindButtonForWindow(Window* window) const
{
    for (Int32 i = Int32(0); static_cast<int>(i) < _windowButtons.Length(); i += 1)
    {
        if (_windowButtons[static_cast<int>(i)]->GetWindow() == window)
        {
            return _windowButtons[static_cast<int>(i)];
        }
    }
    return nullptr;
}

void TaskBar::OnPaint(PaintEventArgs& e)
{
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
    , _font(Font::SystemFont())
    , _icon()
{
}

Button::~Button()
{
}

void Button::SetOnClick(ClickEventHandler handler, void* userData)
{
    _onClick = handler;
    _onClickUserData = userData;
}

void Button::OnPaint(PaintEventArgs& e)
{
    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 sw = screen.width;
    Int32 sh = screen.height;

    // Draw with appropriate border style based on pressed state
    // Visual state is toggled OR mouse down
    Boolean visualPressed = Boolean(_isToggled || _isMouseDown);
    if (static_cast<bool>(visualPressed))
    {
        e.graphics->FillRectangle(screen, BorderStyle::SunkenDouble);
    }
    else
    {
        e.graphics->FillRectangle(screen, BorderStyle::RaisedDouble);
    }

    // Calculate content area (inside borders)
    Int32 contentX = Int32(static_cast<int>(sx) + 4);
    Int32 contentY = Int32(static_cast<int>(sy) + 2);
    Int32 contentH = Int32(static_cast<int>(sh) - 4);

    // Track icon dimensions
    Int32 iconW = _icon.Width();
    Int32 iconH = _icon.Height();
    Int32 hasIcon = (static_cast<int>(iconW) > 0 && static_cast<int>(iconH) > 0) ? 1 : 0;

    // Measure text
    Drawing::Size textSize = _font.MeasureString(_text);
    Int32 textH = _text.Length() > Int32(0) ? textSize.height : Int32(0);

    // Gap between icon and text
    Int32 gap = hasIcon && _text.Length() > Int32(0) ? Int32(3) : Int32(0);

    // Left-align content with small padding (better for icon buttons)
    Int32 startX = Int32(static_cast<int>(contentX) + 2);

    // Offset by 1 pixel when pressed for 3D effect
    if (static_cast<bool>(visualPressed))
    {
        startX = Int32(static_cast<int>(startX) + 1);
        contentY = Int32(static_cast<int>(contentY) + 1);
    }

    // Draw icon if present
    if (hasIcon)
    {
        Int32 iconY = Int32(static_cast<int>(contentY) + (static_cast<int>(contentH) - static_cast<int>(iconH)) / 2);
        GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
        if (fb)
        {
            fb->GetImage().CopyFromWithAlpha(_icon, startX, iconY);
        }
        startX = Int32(static_cast<int>(startX) + static_cast<int>(iconW) + static_cast<int>(gap));
    }

    // Draw button text
    if (_text.Length() > Int32(0) && static_cast<bool>(_font.IsValid()))
    {
        Int32 textY = Int32(static_cast<int>(contentY) + (static_cast<int>(contentH) - static_cast<int>(textH)) / 2);
        e.graphics->DrawString(_text, _font, Color::Black, startX, textY);
    }

    // Paint children
    OnPaintClient(e);
}

void Button::OnMouse(MouseEventArgs& e)
{
    Boolean wasVisuallyPressed = Boolean(_isToggled || _isMouseDown);
    Int32 ex = e.x;
    Int32 ey = e.y;
    Boolean isOver = HitTest(ex, ey);
    Boolean leftDown = e.leftButton;

    // Only track mouse-down state, don't affect toggle state
    _isMouseDown = static_cast<bool>(leftDown) && static_cast<bool>(isOver);

    // Detect click: was pressed, now released while still over button
    if (_wasMouseDown && !static_cast<bool>(leftDown) && static_cast<bool>(isOver))
    {
        // Fire click event
        if (_onClick)
        {
            _onClick(this, _onClickUserData);
        }
    }

    _wasMouseDown = static_cast<bool>(leftDown) && static_cast<bool>(isOver);

    Boolean nowVisuallyPressed = Boolean(_isToggled || _isMouseDown);
    if (static_cast<bool>(nowVisuallyPressed) != static_cast<bool>(wasVisuallyPressed))
    {
        Invalidate();
    }
}

MeasureResult Button::GetPreferredSize() const
{
    // Buttons use their initial bounds size as preferred size
    return MeasureResult(_bounds.width, _bounds.height);
}

/******************************************************************************/
/*    Picture Implementation                                                  */
/******************************************************************************/

Picture::Picture(Control* parent, const Rectangle& bounds)
    : Control(parent, bounds)
    , _image()
{
}

Picture::Picture(Control* parent, const Rectangle& bounds, const Image& image)
    : Control(parent, bounds)
    , _image(image)
{
}

Picture::~Picture()
{
}

void Picture::SetImage(const Image& image)
{
    _image = image;
    Invalidate();
}

void Picture::OnPaint(PaintEventArgs& e)
{
    Int32 iw = _image.Width();
    Int32 ih = _image.Height();

    if (iw > Int32(0) && ih > Int32(0))
    {
        Rectangle screen = ScreenBounds();
        Rectangle visible = VisibleBounds();

        Int32 vw = visible.width;
        Int32 vh = visible.height;

        // Check if anything is visible
        if (vw <= Int32(0) || vh <= Int32(0))
        {
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
        if (Int32(static_cast<int>(srcX) + static_cast<int>(drawWidth)) > iw)
        {
            drawWidth = Int32(static_cast<int>(iw) - static_cast<int>(srcX));
        }
        if (Int32(static_cast<int>(srcY) + static_cast<int>(drawHeight)) > ih)
        {
            drawHeight = Int32(static_cast<int>(ih) - static_cast<int>(srcY));
        }

        if (drawWidth > Int32(0) && drawHeight > Int32(0) && srcX >= Int32(0) && srcY >= Int32(0))
        {
            Image region = _image.GetRegion(srcX, srcY, drawWidth, drawHeight);
            e.graphics->DrawImage(region, vx, vy);
        }
    }

    // Paint children
    OnPaintClient(e);
}

MeasureResult Picture::GetPreferredSize() const
{
    // Pictures prefer their image dimensions if available
    Int32 w = _image.Width();
    Int32 h = _image.Height();

    // Fall back to bounds if no image
    if (w <= Int32(0) || h <= Int32(0))
    {
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
    , _gradient()
{
    RegenerateGradient();
}

SpectrumControl::~SpectrumControl()
{
}

void SpectrumControl::SetBaseColor(const Color& color)
{
    _baseColor = color;
    RegenerateGradient();
    Invalidate();
}

void SpectrumControl::RegenerateGradient()
{
    Int32 w = _bounds.width;
    Int32 h = _bounds.height;

    if (w <= Int32(0) || h <= Int32(0))
    {
        return;
    }

    _gradient = Image(w, h);

    // Generate gradient: white at top, base color in middle, black at bottom
    Int32 midY = Int32(static_cast<int>(h) / 2);

    for (Int32 y = Int32(0); y < h; y += 1)
    {
        Color lineColor;

        if (y <= midY)
        {
            // Top half: white to base color
            float t = (static_cast<int>(midY) > 0) ? static_cast<float>(static_cast<int>(y)) / static_cast<int>(midY) : 0.0f;
            lineColor = Color::Lerp(Color::White, _baseColor, t);
        }
        else
        {
            // Bottom half: base color to black
            float t = (static_cast<int>(h) - 1 - static_cast<int>(midY) > 0) ?
                      static_cast<float>(static_cast<int>(y) - static_cast<int>(midY)) / (static_cast<int>(h) - 1 - static_cast<int>(midY)) : 0.0f;
            lineColor = Color::Lerp(_baseColor, Color::Black, t);
        }

        // Fill entire row with this color
        for (Int32 x = Int32(0); x < w; x += 1)
        {
            _gradient.SetPixel(x, y, lineColor);
        }
    }
}

Color SpectrumControl::GetColorAtY(Int32 y) const
{
    Int32 yi = y;
    Int32 h = _bounds.height;

    if (yi < Int32(0))
    {
        yi = Int32(0);
    }
    if (yi >= h)
    {
        yi = Int32(static_cast<int>(h) - 1);
    }
    if (h <= Int32(0))
    {
        return _baseColor;
    }

    Int32 midY = Int32(static_cast<int>(h) / 2);

    if (yi <= midY)
    {
        float t = (static_cast<int>(midY) > 0) ? static_cast<float>(static_cast<int>(yi)) / static_cast<int>(midY) : 0.0f;
        return Color::Lerp(Color::White, _baseColor, t);
    }
    else
    {
        float t = (static_cast<int>(h) - 1 - static_cast<int>(midY) > 0) ?
                  static_cast<float>(static_cast<int>(yi) - static_cast<int>(midY)) / (static_cast<int>(h) - 1 - static_cast<int>(midY)) : 0.0f;
        return Color::Lerp(_baseColor, Color::Black, t);
    }
}

void SpectrumControl::OnPaint(PaintEventArgs& e)
{
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb)
    {
        return;
    }

    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 gw = _gradient.Width();
    Int32 gh = _gradient.Height();

    // Copy gradient directly to framebuffer image with clipping
    // The buffer writer will handle dithering if needed for VGA modes
    Image& img = fb->GetImage();
    if (gw > Int32(0) && gh > Int32(0))
    {
        // Use clip bounds from PaintEventArgs if valid
        if (static_cast<int>(e.clipBounds.width) > 0 && static_cast<int>(e.clipBounds.height) > 0)
        {
            img.CopyFromClipped(_gradient, sx, sy, e.clipBounds);
        }
        else
        {
            img.CopyFrom(_gradient, sx, sy);
        }
    }

    // Paint children (if any)
    OnPaintClient(e);
}

/******************************************************************************/
/*    TaskBarButton Implementation                                            */
/******************************************************************************/

void TaskBarButton::OnTaskBarButtonClick(Button* sender, void* userData)
{
    TaskBarButton* btn = static_cast<TaskBarButton*>(sender);
    (void)userData;

    if (btn && btn->_window)
    {
        // If window is minimized, restore it
        if (static_cast<bool>(btn->_window->IsMinimized()))
        {
            btn->_window->Restore();
        }

        // Find the desktop through the control hierarchy and focus the window
        Control* parent = btn->Parent();
        while (parent)
        {
            if (parent->GetControlType() == ControlType::TaskBar)
            {
                TaskBar* taskBar = parent->AsTaskBar();
                if (taskBar && taskBar->GetDesktop())
                {
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
    , _window(window)
{
    SetOnClick(OnTaskBarButtonClick, nullptr);
}

TaskBarButton::~TaskBarButton()
{
}

void TaskBarButton::OnPaint(PaintEventArgs& e)
{
    Rectangle screen = ScreenBounds();
    Int32 x = screen.x;
    Int32 y = screen.y;
    Int32 w = screen.width;
    Int32 h = screen.height;

    // Check if button is visually pressed (toggled OR mouse down)
    Boolean visualPressed = IsPressed();

    if (static_cast<bool>(visualPressed))
    {
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
    }
    else
    {
        // Normal state: use standard raised button style
        e.graphics->FillRectangle(screen, BorderStyle::RaisedDouble);
    }

    // Draw window title text (left-aligned with padding, vertically centered)
    if (_window)
    {
        const String& title = _window->Title();
        const Font& font = _window->GetFont();
        if (title.Length() > Int32(0) && static_cast<bool>(font.IsValid()))
        {
            Int32 textX = Int32(static_cast<int>(x) + 4);  // Left padding
            Int32 textY = Int32(static_cast<int>(y) + (static_cast<int>(h) - static_cast<int>(font.Height())) / 2);
            // Offset by 1 pixel when pressed for 3D effect
            if (static_cast<bool>(visualPressed))
            {
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
    , _text()
    , _isHighlighted(false)
    , _isSeparator(false)
    , _wasPressed(false)
    , _onClick(nullptr)
    , _onClickUserData(nullptr)
    , _itemIndex(itemIndex)
{
}

MenuItem::~MenuItem()
{
}

void MenuItem::SetIcon(const Image& icon)
{
    _icon = icon;
    Invalidate();
}

void MenuItem::SetText(const String& text)
{
    _text = text;
    Invalidate();
}

void MenuItem::SetSeparator(Boolean isSeparator)
{
    _isSeparator = static_cast<bool>(isSeparator);
    Invalidate();
}

void MenuItem::SetOnClick(ClickEventHandler handler, void* userData)
{
    _onClick = handler;
    _onClickUserData = userData;
}

void MenuItem::OnPaint(PaintEventArgs& e)
{
    Rectangle screen = ScreenBounds();
    Int32 sx = screen.x;
    Int32 sy = screen.y;
    Int32 sw = screen.width;
    Int32 sh = screen.height;

    // Handle separator items
    if (_isSeparator)
    {
        // Draw background
        e.graphics->FillRectangle(screen, Color::Gray);

        // Draw 3D separator line (dark grey on top, white on bottom)
        Int32 lineY = Int32(static_cast<int>(sy) + static_cast<int>(sh) / 2 - 1);
        Int32 lineX1 = Int32(static_cast<int>(sx) + ICON_MARGIN);
        Int32 lineX2 = Int32(static_cast<int>(sx) + static_cast<int>(sw) - ICON_MARGIN);

        // Top line: dark grey
        e.graphics->DrawLine(lineX1, lineY, lineX2, lineY, Color::DarkGray);
        // Bottom line: white
        e.graphics->DrawLine(lineX1, Int32(static_cast<int>(lineY) + 1), lineX2, Int32(static_cast<int>(lineY) + 1), Color::White);
        return;
    }

    // Draw background
    Color bgColor = _isHighlighted ? Color::DarkBlue : Color::Gray;
    Color textColor = _isHighlighted ? Color::White : Color::Black;
    e.graphics->FillRectangle(screen, bgColor);

    // Draw icon if present
    Int32 iw = _icon.Width();
    Int32 ih = _icon.Height();
    Int32 textX = Int32(static_cast<int>(sx) + ICON_MARGIN + ICON_SIZE + TEXT_MARGIN);

    if (iw > Int32(0) && ih > Int32(0))
    {
        // Center icon vertically in item
        Int32 iconY = Int32(static_cast<int>(sy) + (static_cast<int>(sh) - static_cast<int>(ih)) / 2);
        Int32 iconX = Int32(static_cast<int>(sx) + ICON_MARGIN);

        GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
        if (fb)
        {
            Image& fbImg = fb->GetImage();
            fbImg.CopyFromWithAlpha(_icon, static_cast<int>(iconX), static_cast<int>(iconY));
        }
    }

    // Draw text if present
    if (_text.Length() > Int32(0))
    {
        Font sysFont = Font::SystemFont();
        Int32 textY = Int32(static_cast<int>(sy) + (static_cast<int>(sh) - static_cast<int>(sysFont.Height())) / 2);
        e.graphics->DrawString(_text, sysFont, textColor, textX, textY);
    }

    // Paint children
    OnPaintClient(e);
}

bool MenuItem::HandleMouseUpdate(MouseEventArgs& e)
{
    // Skip separators - return false (no change)
    if (_isSeparator)
    {
        return false;
    }

    Int32 ex = e.x;
    Int32 ey = e.y;
    Boolean isOver = HitTest(ex, ey);
    Boolean leftDown = e.leftButton;

    bool wasHighlighted = _isHighlighted;
    _isHighlighted = static_cast<bool>(isOver);

    // Fire click on release while over
    if (_wasPressed && !static_cast<bool>(leftDown) && static_cast<bool>(isOver) && _onClick)
    {
        _onClick(reinterpret_cast<Button*>(this), _onClickUserData);
    }
    _wasPressed = static_cast<bool>(leftDown) && static_cast<bool>(isOver);

    // Return true if highlight state changed (caller handles repaint)
    return _isHighlighted != wasHighlighted;
}

void MenuItem::OnMouse(MouseEventArgs& e)
{
    // StartMenu handles all mouse events for menu items via HandleMouseUpdate
    // This method exists for compatibility but defers to HandleMouseUpdate
    // without any painting (StartMenu batches repaints for performance)
    HandleMouseUpdate(e);
}

/******************************************************************************/
/*    StartMenu Implementation                                                */
/******************************************************************************/

StartMenu::StartMenu(Desktop* desktop)
    : Control()  // No parent yet - will be added to desktop separately
    , _desktop(desktop)
    , _isVisible(false)
    , _items()
{
    // Get screen dimensions
    Display current = Display::GetCurrent();
    Int32 screenHeight = Int32(static_cast<int>(current.Height()));
    Int32 taskBarHeight = Int32(32);

    // Menu structure: items 3 and 8 (0-indexed) are separators
    // Total height: 9 regular items (40px each) + 2 separators (8px each) = 376px + 4px border
    Int32 menuHeight = Int32(9 * ITEM_HEIGHT + 2 * SEPARATOR_HEIGHT + 4);  // +4 for border
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

    // StartMenu is always on top, above taskbar
    _layout.alwaysOnTop = true;
    _layout.zIndex = Int32(1001);

    // Create menu items with RELATIVE positioning (to parent's client area)
    // Items 3 and 8 (0-indexed) are separators with smaller height
    _items.Resize(ITEM_COUNT);
    Int32 currentY = Int32(0);

    for (Int32 i = Int32(0); static_cast<int>(i) < ITEM_COUNT; i += 1)
    {
        bool isSeparator = (static_cast<int>(i) == 3 || static_cast<int>(i) == 8);
        Int32 itemHeight = isSeparator ? Int32(SEPARATOR_HEIGHT) : Int32(ITEM_HEIGHT);

        _items[static_cast<int>(i)] = new MenuItem(this, Rectangle(Int32(0), currentY,
            Int32(MENU_WIDTH - SIDEBAR_WIDTH - 2), itemHeight), static_cast<int>(i));

        if (isSeparator)
        {
            _items[static_cast<int>(i)]->SetSeparator(Boolean(true));
        }

        // Menu items have fixed height
        _items[static_cast<int>(i)]->Layout().heightMode = SizeMode::Fixed;
        _items[static_cast<int>(i)]->Layout().widthMode = SizeMode::Fill;

        currentY = Int32(static_cast<int>(currentY) + static_cast<int>(itemHeight));
    }

    // Load icons
    LoadIcons();

    // Add to desktop (but hidden)
    if (desktop)
    {
        // Don't use AddChild - just set parent directly to avoid spatial grid
        _parent = desktop;
    }
}

StartMenu::~StartMenu()
{
    // MenuItems are children, deleted by Control destructor
}

// Shutdown handler for start menu
static void OnShutdownClick(Button* sender, void* userData)
{
    (void)sender;
    Desktop* desktop = static_cast<Desktop*>(userData);
    if (desktop)
    {
        desktop->Stop();
    }
}

void StartMenu::LoadIcons()
{
    if (!_desktop)
    {
        return;
    }

    Drawing::IconLibrary* icons = _desktop->GetIconLibrary();
    if (!icons)
    {
        return;
    }

    // Menu item definitions: icon name, display text
    // Items 3 and 8 are separators (no icon/text needed)
    struct MenuItemDef
    {
        const char* iconName;
        const char* text;
    };

    static const MenuItemDef menuItems[ITEM_COUNT] =
    {
        { "computer",      "Computer" },        // 0
        { "folder-library", "Documents" },      // 1
        { "settings",      "Settings" },        // 2
        { nullptr,         nullptr },           // 3 - separator
        { "app-winfx-2",   "Application 1" },   // 4
        { "app-winfx-1",   "Application 2" },   // 5
        { "app-windos",    "Application 3" },   // 6
        { "app-msdos",     "Command Prompt" },  // 7
        { nullptr,         nullptr },           // 8 - separator
        { "sys-logout",    "Log off..." },      // 9
        { "sys-shutdown",  "Shut down" }        // 10
    };

    for (Int32 i = Int32(0); static_cast<int>(i) < ITEM_COUNT && static_cast<int>(i) < _items.Length(); i += 1)
    {
        int idx = static_cast<int>(i);
        const MenuItemDef& def = menuItems[idx];

        // Skip separators
        if (def.iconName == nullptr)
        {
            continue;
        }

        // Set text
        _items[idx]->SetText(String(def.text));

        // Set icon (32x32)
        try
        {
            Image icon = icons->FromName(def.iconName, Drawing::IconSize::Medium);
            _items[idx]->SetIcon(icon);
        }
        catch (...)
        {
            // Icon loading failed - item will show without icon
        }
    }

    // Set shutdown handler on last item
    if (_items.Length() > 0)
    {
        _items[ITEM_COUNT - 1]->SetOnClick(OnShutdownClick, _desktop);
    }
}

void StartMenu::Show()
{
    _isVisible = true;
    // Update Start button to show pressed state
    if (_desktop)
    {
        TaskBar* taskBar = _desktop->GetTaskBar();
        if (taskBar && taskBar->StartButton())
        {
            taskBar->StartButton()->SetPressed(Boolean(true));
        }
    }
    Invalidate();
}

void StartMenu::Hide()
{
    _isVisible = false;
    // Update Start button to show normal state
    if (_desktop)
    {
        TaskBar* taskBar = _desktop->GetTaskBar();
        if (taskBar && taskBar->StartButton())
        {
            taskBar->StartButton()->SetPressed(Boolean(false));
        }
    }
    Invalidate();
}

void StartMenu::Toggle()
{
    if (_isVisible)
    {
        Hide();
    }
    else
    {
        Show();
    }
}

void StartMenu::OnPaint(PaintEventArgs& e)
{
    if (!_isVisible)
    {
        return;
    }

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

void StartMenu::OnMouse(MouseEventArgs& e)
{
    // Propagate mouse events to ALL menu items so they can update highlight state
    // (not just the one under cursor, so items can un-highlight when mouse leaves)
    // Track if any item changed state to batch the framebuffer flush
    bool anyChanged = false;
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();

    for (Int32 i = Int32(0); static_cast<int>(i) < _items.Length(); i += 1)
    {
        MenuItem* item = _items[static_cast<int>(i)];
        if (item)
        {
            // HandleMouseUpdate returns true if highlight changed
            if (item->HandleMouseUpdate(e))
            {
                // Repaint just this item (without flush)
                if (fb)
                {
                    // Use full framebuffer bounds for Graphics to avoid coordinate clipping issues
                    // (item->OnPaint uses screen coordinates which must not be clipped to item bounds)
                    Image& fbImg = fb->GetImage();
                    Rectangle fbBounds(Int32(0), Int32(0), fbImg.Width(), fbImg.Height());
                    Graphics g(BufferMode::Single, fbBounds);
                    PaintEventArgs paintArgs(&g, item->ScreenBounds());
                    item->OnPaint(paintArgs);
                }
                anyChanged = true;
            }
        }
    }

    // Single flush after all items are repainted
    if (anyChanged)
    {
        GraphicsBuffer::FlushFrameBuffer();
    }
}

} // namespace System::Windows::Forms
