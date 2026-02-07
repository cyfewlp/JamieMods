#pragma once

namespace ImGuiEx
{
#define FLAGS_CLASS_BODY(className, enumName)                                                                          \
    class className                                                                                                    \
    {                                                                                                                  \
        enumName flags = 0;                                                                                            \
                                                                                                                       \
    public:                                                                                                            \
        constexpr className() = default;                                                                               \
        constexpr explicit className(enumName f) : flags(f) {}

#define FLAGS_CLASS_FUNCTION(className, enumName, flagName, is_deprecated)                                             \
    IMGUI_OPTIONAL_DEPRECATED(is_deprecated)                                                                           \
    constexpr auto flagName() -> className &                                                                           \
    {                                                                                                                  \
        flags |= enumName##_##flagName;                                                                                \
        return *this;                                                                                                  \
    }

#define FLAGS_CLASS_END(enumName)                                                                                      \
    constexpr operator enumName() const                                                                                \
    {                                                                                                                  \
        return flags;                                                                                                  \
    }                                                                                                                  \
    }                                                                                                                  \
    ;

#define IMGUI_OPTIONAL_DEPRECATED(is_deprecated) IMGUI_DEPRECATED_##is_deprecated
#define IMGUI_DEPRECATED_True                    [[deprecated("This flag is obsolete in ImGui.")]]
#define IMGUI_DEPRECATED_False

//////////////////////////////////////////////////
/// ImGuiWindowFlags

FLAGS_CLASS_BODY(WindowFlags, ImGuiWindowFlags)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, None, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoTitleBar, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoResize, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoMove, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoScrollbar, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoScrollWithMouse, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoCollapse, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, AlwaysAutoResize, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoBackground, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoSavedSettings, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoMouseInputs, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, MenuBar, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, HorizontalScrollbar, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoFocusOnAppearing, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoBringToFrontOnFocus, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, AlwaysVerticalScrollbar, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, AlwaysHorizontalScrollbar, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoNavInputs, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoNavFocus, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, UnsavedDocument, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoNav, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoDecoration, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, NoInputs, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, ChildWindow, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, Tooltip, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, Popup, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, Modal, False)
FLAGS_CLASS_FUNCTION(WindowFlags, ImGuiWindowFlags, ChildMenu, False)
FLAGS_CLASS_END(ImGuiWindowFlags)

//////////////////////////////////////////////////
/// ImGuiChildFlags

FLAGS_CLASS_BODY(ChildFlags, ImGuiChildFlags)
FLAGS_CLASS_FUNCTION(ChildFlags, ImGuiChildFlags, None, False)
FLAGS_CLASS_FUNCTION(ChildFlags, ImGuiChildFlags, Borders, False)
FLAGS_CLASS_FUNCTION(ChildFlags, ImGuiChildFlags, AlwaysUseWindowPadding, False)
FLAGS_CLASS_FUNCTION(ChildFlags, ImGuiChildFlags, ResizeX, False)
FLAGS_CLASS_FUNCTION(ChildFlags, ImGuiChildFlags, ResizeY, False)
FLAGS_CLASS_FUNCTION(ChildFlags, ImGuiChildFlags, AutoResizeX, False)
FLAGS_CLASS_FUNCTION(ChildFlags, ImGuiChildFlags, AutoResizeY, False)
FLAGS_CLASS_FUNCTION(ChildFlags, ImGuiChildFlags, AlwaysAutoResize, False)
FLAGS_CLASS_FUNCTION(ChildFlags, ImGuiChildFlags, FrameStyle, False)
FLAGS_CLASS_FUNCTION(ChildFlags, ImGuiChildFlags, NavFlattened, False)
FLAGS_CLASS_END(ImGuiChildFlags)

//////////////////////////////////////////////////
/// ImGuiItemFlags

FLAGS_CLASS_BODY(ItemFlags, ImGuiItemFlags)
FLAGS_CLASS_FUNCTION(ItemFlags, ImGuiItemFlags, None, False)
FLAGS_CLASS_FUNCTION(ItemFlags, ImGuiItemFlags, NoTabStop, False)
FLAGS_CLASS_FUNCTION(ItemFlags, ImGuiItemFlags, NoNav, False)
FLAGS_CLASS_FUNCTION(ItemFlags, ImGuiItemFlags, NoNavDefaultFocus, False)
FLAGS_CLASS_FUNCTION(ItemFlags, ImGuiItemFlags, ButtonRepeat, False)
FLAGS_CLASS_FUNCTION(ItemFlags, ImGuiItemFlags, AutoClosePopups, False)
FLAGS_CLASS_FUNCTION(ItemFlags, ImGuiItemFlags, AllowDuplicateId, False)
FLAGS_CLASS_FUNCTION(ItemFlags, ImGuiItemFlags, Disabled, False)
FLAGS_CLASS_END(ImGuiItemFlags)

//////////////////////////////////////////////////
/// ImGuiInputTextFlags

FLAGS_CLASS_BODY(InputTextFlags, ImGuiInputTextFlags)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, None, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CharsDecimal, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CharsHexadecimal, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CharsScientific, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CharsUppercase, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CharsNoBlank, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, AllowTabInput, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, EnterReturnsTrue, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, EscapeClearsAll, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CtrlEnterForNewLine, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, ReadOnly, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, Password, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, AlwaysOverwrite, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, AutoSelectAll, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, ParseEmptyRefVal, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, DisplayEmptyRefVal, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, NoHorizontalScroll, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, NoUndoRedo, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, ElideLeft, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CallbackCompletion, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CallbackHistory, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CallbackAlways, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CallbackCharFilter, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CallbackResize, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, CallbackEdit, False)
FLAGS_CLASS_FUNCTION(InputTextFlags, ImGuiInputTextFlags, WordWrap, False)
FLAGS_CLASS_END(ImGuiInputTextFlags)

//////////////////////////////////////////////////
/// ImGuiTreeNodeFlags

FLAGS_CLASS_BODY(TreeNodeFlags, ImGuiTreeNodeFlags)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, None, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, Selected, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, Framed, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, AllowOverlap, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, NoTreePushOnOpen, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, NoAutoOpenOnLog, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, DefaultOpen, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, OpenOnDoubleClick, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, OpenOnArrow, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, Leaf, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, Bullet, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, FramePadding, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, SpanAvailWidth, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, SpanFullWidth, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, SpanLabelWidth, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, SpanAllColumns, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, LabelSpanAllColumns, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, NavLeftJumpsToParent, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, CollapsingHeader, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, DrawLinesNone, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, DrawLinesFull, False)
FLAGS_CLASS_FUNCTION(TreeNodeFlags, ImGuiTreeNodeFlags, DrawLinesToNodes, False)
FLAGS_CLASS_END(ImGuiTreeNodeFlags)

//////////////////////////////////////////////////
/// ImGuiPopupFlags

FLAGS_CLASS_BODY(PopupFlags, ImGuiPopupFlags)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, None, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, MouseButtonLeft, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, MouseButtonRight, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, MouseButtonMiddle, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, NoReopen, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, NoOpenOverExistingPopup, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, NoOpenOverItems, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, AnyPopupId, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, AnyPopupLevel, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, AnyPopup, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, MouseButtonShift_, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, MouseButtonMask_, False)
FLAGS_CLASS_FUNCTION(PopupFlags, ImGuiPopupFlags, InvalidMask_, False)
FLAGS_CLASS_END(ImGuiPopupFlags)

//////////////////////////////////////////////////
/// ImGuiSelectableFlags

FLAGS_CLASS_BODY(SelectableFlags, ImGuiSelectableFlags)
FLAGS_CLASS_FUNCTION(SelectableFlags, ImGuiSelectableFlags, None, False)
FLAGS_CLASS_FUNCTION(SelectableFlags, ImGuiSelectableFlags, NoAutoClosePopups, False)
FLAGS_CLASS_FUNCTION(SelectableFlags, ImGuiSelectableFlags, SpanAllColumns, False)
FLAGS_CLASS_FUNCTION(SelectableFlags, ImGuiSelectableFlags, AllowDoubleClick, False)
FLAGS_CLASS_FUNCTION(SelectableFlags, ImGuiSelectableFlags, Disabled, False)
FLAGS_CLASS_FUNCTION(SelectableFlags, ImGuiSelectableFlags, AllowOverlap, False)
FLAGS_CLASS_FUNCTION(SelectableFlags, ImGuiSelectableFlags, Highlight, False)
FLAGS_CLASS_FUNCTION(SelectableFlags, ImGuiSelectableFlags, SelectOnNav, False)
FLAGS_CLASS_END(ImGuiSelectableFlags)

//////////////////////////////////////////////////
/// ImGuiComboFlags

FLAGS_CLASS_BODY(ComboFlags, ImGuiComboFlags)
FLAGS_CLASS_FUNCTION(ComboFlags, ImGuiComboFlags, None, False)
FLAGS_CLASS_FUNCTION(ComboFlags, ImGuiComboFlags, PopupAlignLeft, False)
FLAGS_CLASS_FUNCTION(ComboFlags, ImGuiComboFlags, HeightSmall, False)
FLAGS_CLASS_FUNCTION(ComboFlags, ImGuiComboFlags, HeightRegular, False)
FLAGS_CLASS_FUNCTION(ComboFlags, ImGuiComboFlags, HeightLarge, False)
FLAGS_CLASS_FUNCTION(ComboFlags, ImGuiComboFlags, HeightLargest, False)
FLAGS_CLASS_FUNCTION(ComboFlags, ImGuiComboFlags, NoArrowButton, False)
FLAGS_CLASS_FUNCTION(ComboFlags, ImGuiComboFlags, NoPreview, False)
FLAGS_CLASS_FUNCTION(ComboFlags, ImGuiComboFlags, WidthFitPreview, False)
FLAGS_CLASS_FUNCTION(ComboFlags, ImGuiComboFlags, HeightMask_, False)
FLAGS_CLASS_END(ImGuiComboFlags)

//////////////////////////////////////////////////
/// ImGuiTabBarFlags

FLAGS_CLASS_BODY(TabBarFlags, ImGuiTabBarFlags)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, None, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, Reorderable, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, AutoSelectNewTabs, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, TabListPopupButton, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, NoCloseWithMiddleMouseButton, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, NoTabListScrollingButtons, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, NoTooltip, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, DrawSelectedOverline, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, FittingPolicyMixed, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, FittingPolicyShrink, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, FittingPolicyScroll, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, FittingPolicyMask_, False)
FLAGS_CLASS_FUNCTION(TabBarFlags, ImGuiTabBarFlags, FittingPolicyDefault_, False)
FLAGS_CLASS_END(ImGuiTabBarFlags)

//////////////////////////////////////////////////
/// ImGuiTabItemFlags

FLAGS_CLASS_BODY(TabItemFlags, ImGuiTabItemFlags)
FLAGS_CLASS_FUNCTION(TabItemFlags, ImGuiTabItemFlags, None, False)
FLAGS_CLASS_FUNCTION(TabItemFlags, ImGuiTabItemFlags, UnsavedDocument, False)
FLAGS_CLASS_FUNCTION(TabItemFlags, ImGuiTabItemFlags, SetSelected, False)
FLAGS_CLASS_FUNCTION(TabItemFlags, ImGuiTabItemFlags, NoCloseWithMiddleMouseButton, False)
FLAGS_CLASS_FUNCTION(TabItemFlags, ImGuiTabItemFlags, NoPushId, False)
FLAGS_CLASS_FUNCTION(TabItemFlags, ImGuiTabItemFlags, NoTooltip, False)
FLAGS_CLASS_FUNCTION(TabItemFlags, ImGuiTabItemFlags, NoReorder, False)
FLAGS_CLASS_FUNCTION(TabItemFlags, ImGuiTabItemFlags, Leading, False)
FLAGS_CLASS_FUNCTION(TabItemFlags, ImGuiTabItemFlags, Trailing, False)
FLAGS_CLASS_FUNCTION(TabItemFlags, ImGuiTabItemFlags, NoAssumedClosure, False)
FLAGS_CLASS_END(ImGuiTabItemFlags)

//////////////////////////////////////////////////
/// ImGuiFocusedFlags

FLAGS_CLASS_BODY(FocusedFlags, ImGuiFocusedFlags)
FLAGS_CLASS_FUNCTION(FocusedFlags, ImGuiFocusedFlags, None, False)
FLAGS_CLASS_FUNCTION(FocusedFlags, ImGuiFocusedFlags, ChildWindows, False)
FLAGS_CLASS_FUNCTION(FocusedFlags, ImGuiFocusedFlags, RootWindow, False)
FLAGS_CLASS_FUNCTION(FocusedFlags, ImGuiFocusedFlags, AnyWindow, False)
FLAGS_CLASS_FUNCTION(FocusedFlags, ImGuiFocusedFlags, NoPopupHierarchy, False)
FLAGS_CLASS_FUNCTION(FocusedFlags, ImGuiFocusedFlags, RootAndChildWindows, False)
FLAGS_CLASS_END(ImGuiFocusedFlags)

//////////////////////////////////////////////////
/// ImGuiHoveredFlags

FLAGS_CLASS_BODY(HoveredFlags, ImGuiHoveredFlags)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, None, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, ChildWindows, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, RootWindow, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, AnyWindow, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, NoPopupHierarchy, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, AllowWhenBlockedByPopup, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, AllowWhenBlockedByActiveItem, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, AllowWhenOverlappedByItem, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, AllowWhenOverlappedByWindow, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, AllowWhenDisabled, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, NoNavOverride, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, AllowWhenOverlapped, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, RectOnly, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, RootAndChildWindows, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, ForTooltip, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, Stationary, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, DelayNone, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, DelayShort, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, DelayNormal, False)
FLAGS_CLASS_FUNCTION(HoveredFlags, ImGuiHoveredFlags, NoSharedDelay, False)
FLAGS_CLASS_END(ImGuiHoveredFlags)

//////////////////////////////////////////////////
/// ImGuiDragDropFlags

FLAGS_CLASS_BODY(DragDropFlags, ImGuiDragDropFlags)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, None, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, SourceNoPreviewTooltip, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, SourceNoDisableHover, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, SourceNoHoldToOpenOthers, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, SourceAllowNullID, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, SourceExtern, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, PayloadAutoExpire, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, PayloadNoCrossContext, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, PayloadNoCrossProcess, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, AcceptBeforeDelivery, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, AcceptNoDrawDefaultRect, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, AcceptNoPreviewTooltip, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, AcceptDrawAsHovered, False)
FLAGS_CLASS_FUNCTION(DragDropFlags, ImGuiDragDropFlags, AcceptPeekOnly, False)
FLAGS_CLASS_END(ImGuiDragDropFlags)

//////////////////////////////////////////////////
/// ImGuiInputFlags

FLAGS_CLASS_BODY(InputFlags, ImGuiInputFlags)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, None, False)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, Repeat, False)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, RouteActive, False)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, RouteFocused, False)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, RouteGlobal, False)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, RouteAlways, False)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, RouteOverFocused, False)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, RouteOverActive, False)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, RouteUnlessBgFocused, False)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, RouteFromRootWindow, False)
FLAGS_CLASS_FUNCTION(InputFlags, ImGuiInputFlags, Tooltip, False)
FLAGS_CLASS_END(ImGuiInputFlags)

//////////////////////////////////////////////////
/// ImGuiConfigFlags

FLAGS_CLASS_BODY(ConfigFlags, ImGuiConfigFlags)
FLAGS_CLASS_FUNCTION(ConfigFlags, ImGuiConfigFlags, None, False)
FLAGS_CLASS_FUNCTION(ConfigFlags, ImGuiConfigFlags, NavEnableKeyboard, False)
FLAGS_CLASS_FUNCTION(ConfigFlags, ImGuiConfigFlags, NavEnableGamepad, False)
FLAGS_CLASS_FUNCTION(ConfigFlags, ImGuiConfigFlags, NoMouse, False)
FLAGS_CLASS_FUNCTION(ConfigFlags, ImGuiConfigFlags, NoMouseCursorChange, False)
FLAGS_CLASS_FUNCTION(ConfigFlags, ImGuiConfigFlags, NoKeyboard, False)
FLAGS_CLASS_FUNCTION(ConfigFlags, ImGuiConfigFlags, IsSRGB, False)
FLAGS_CLASS_FUNCTION(ConfigFlags, ImGuiConfigFlags, IsTouchScreen, False)
FLAGS_CLASS_END(ImGuiConfigFlags)

//////////////////////////////////////////////////
/// ImGuiBackendFlags

FLAGS_CLASS_BODY(BackendFlags, ImGuiBackendFlags)
FLAGS_CLASS_FUNCTION(BackendFlags, ImGuiBackendFlags, None, False)
FLAGS_CLASS_FUNCTION(BackendFlags, ImGuiBackendFlags, HasGamepad, False)
FLAGS_CLASS_FUNCTION(BackendFlags, ImGuiBackendFlags, HasMouseCursors, False)
FLAGS_CLASS_FUNCTION(BackendFlags, ImGuiBackendFlags, HasSetMousePos, False)
FLAGS_CLASS_FUNCTION(BackendFlags, ImGuiBackendFlags, RendererHasVtxOffset, False)
FLAGS_CLASS_FUNCTION(BackendFlags, ImGuiBackendFlags, RendererHasTextures, False)
FLAGS_CLASS_END(ImGuiBackendFlags)

//////////////////////////////////////////////////
/// ImGuiButtonFlags

FLAGS_CLASS_BODY(ButtonFlags, ImGuiButtonFlags)
FLAGS_CLASS_FUNCTION(ButtonFlags, ImGuiButtonFlags, None, False)
FLAGS_CLASS_FUNCTION(ButtonFlags, ImGuiButtonFlags, MouseButtonLeft, False)
FLAGS_CLASS_FUNCTION(ButtonFlags, ImGuiButtonFlags, MouseButtonRight, False)
FLAGS_CLASS_FUNCTION(ButtonFlags, ImGuiButtonFlags, MouseButtonMiddle, False)
FLAGS_CLASS_FUNCTION(ButtonFlags, ImGuiButtonFlags, MouseButtonMask_, False)
FLAGS_CLASS_FUNCTION(ButtonFlags, ImGuiButtonFlags, EnableNav, False)
FLAGS_CLASS_END(ImGuiButtonFlags)

//////////////////////////////////////////////////
/// ImGuiColorEditFlags

FLAGS_CLASS_BODY(ColorEditFlags, ImGuiColorEditFlags)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, None, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoAlpha, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoPicker, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoOptions, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoSmallPreview, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoInputs, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoTooltip, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoLabel, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoSidePreview, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoDragDrop, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoBorder, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, NoColorMarkers, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, AlphaOpaque, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, AlphaNoBg, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, AlphaPreviewHalf, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, AlphaBar, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, HDR, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, DisplayRGB, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, DisplayHSV, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, DisplayHex, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, Uint8, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, Float, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, PickerHueBar, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, PickerHueWheel, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, InputRGB, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, InputHSV, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, DefaultOptions_, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, AlphaMask_, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, DisplayMask_, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, DataTypeMask_, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, PickerMask_, False)
FLAGS_CLASS_FUNCTION(ColorEditFlags, ImGuiColorEditFlags, InputMask_, False)
FLAGS_CLASS_END(ImGuiColorEditFlags)

//////////////////////////////////////////////////
/// ImGuiSliderFlags

FLAGS_CLASS_BODY(SliderFlags, ImGuiSliderFlags)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, None, False)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, Logarithmic, False)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, NoRoundToFormat, False)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, NoInput, False)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, WrapAround, False)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, ClampOnInput, False)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, ClampZeroRange, False)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, NoSpeedTweaks, False)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, ColorMarkers, False)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, AlwaysClamp, False)
FLAGS_CLASS_FUNCTION(SliderFlags, ImGuiSliderFlags, InvalidMask_, False)
FLAGS_CLASS_END(ImGuiSliderFlags)

//////////////////////////////////////////////////
/// ImGuiTableFlags

FLAGS_CLASS_BODY(TableFlags, ImGuiTableFlags)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, None, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, Resizable, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, Reorderable, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, Hideable, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, Sortable, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, NoSavedSettings, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, ContextMenuInBody, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, RowBg, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, BordersInnerH, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, BordersOuterH, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, BordersInnerV, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, BordersOuterV, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, BordersH, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, BordersV, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, BordersInner, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, BordersOuter, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, Borders, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, NoBordersInBody, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, NoBordersInBodyUntilResize, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, SizingFixedFit, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, SizingFixedSame, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, SizingStretchProp, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, SizingStretchSame, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, NoHostExtendX, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, NoHostExtendY, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, NoKeepColumnsVisible, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, PreciseWidths, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, NoClip, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, PadOuterX, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, NoPadOuterX, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, NoPadInnerX, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, ScrollX, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, ScrollY, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, SortMulti, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, SortTristate, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, HighlightHoveredColumn, False)
FLAGS_CLASS_FUNCTION(TableFlags, ImGuiTableFlags, SizingMask_, False)
FLAGS_CLASS_END(ImGuiTableFlags)

//////////////////////////////////////////////////
/// ImGuiTableColumnFlags

FLAGS_CLASS_BODY(TableColumnFlags, ImGuiTableColumnFlags)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, None, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, Disabled, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, DefaultHide, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, DefaultSort, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, WidthStretch, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, WidthFixed, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, NoResize, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, NoReorder, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, NoHide, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, NoClip, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, NoSort, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, NoSortAscending, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, NoSortDescending, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, NoHeaderLabel, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, NoHeaderWidth, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, PreferSortAscending, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, PreferSortDescending, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, IndentEnable, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, IndentDisable, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, AngledHeader, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, IsEnabled, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, IsVisible, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, IsSorted, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, IsHovered, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, WidthMask_, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, IndentMask_, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, StatusMask_, False)
FLAGS_CLASS_FUNCTION(TableColumnFlags, ImGuiTableColumnFlags, NoDirectResize_, False)
FLAGS_CLASS_END(ImGuiTableColumnFlags)

//////////////////////////////////////////////////
/// ImGuiTableRowFlags

FLAGS_CLASS_BODY(TableRowFlags, ImGuiTableRowFlags)
FLAGS_CLASS_FUNCTION(TableRowFlags, ImGuiTableRowFlags, None, False)
FLAGS_CLASS_FUNCTION(TableRowFlags, ImGuiTableRowFlags, Headers, False)
FLAGS_CLASS_END(ImGuiTableRowFlags)

//////////////////////////////////////////////////
/// ImGuiListClipperFlags

FLAGS_CLASS_BODY(ListClipperFlags, ImGuiListClipperFlags)
FLAGS_CLASS_FUNCTION(ListClipperFlags, ImGuiListClipperFlags, None, False)
FLAGS_CLASS_FUNCTION(ListClipperFlags, ImGuiListClipperFlags, NoSetTableRowCounters, False)
FLAGS_CLASS_END(ImGuiListClipperFlags)

//////////////////////////////////////////////////
/// ImGuiMultiSelectFlags

FLAGS_CLASS_BODY(MultiSelectFlags, ImGuiMultiSelectFlags)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, None, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, SingleSelect, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, NoSelectAll, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, NoRangeSelect, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, NoAutoSelect, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, NoAutoClear, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, NoAutoClearOnReselect, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, BoxSelect1d, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, BoxSelect2d, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, BoxSelectNoScroll, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, ClearOnEscape, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, ClearOnClickVoid, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, ScopeWindow, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, ScopeRect, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, SelectOnClick, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, SelectOnClickRelease, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, NavWrapX, False)
FLAGS_CLASS_FUNCTION(MultiSelectFlags, ImGuiMultiSelectFlags, NoSelectOnRightClick, False)
FLAGS_CLASS_END(ImGuiMultiSelectFlags)

//////////////////////////////////////////////////
/// ImDrawFlags

FLAGS_CLASS_BODY(DrawFlags, ImDrawFlags)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, None, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, Closed, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersTopLeft, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersTopRight, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersBottomLeft, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersBottomRight, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersNone, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersTop, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersBottom, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersLeft, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersRight, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersAll, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersDefault_, False)
FLAGS_CLASS_FUNCTION(DrawFlags, ImDrawFlags, RoundCornersMask_, False)
FLAGS_CLASS_END(ImDrawFlags)

//////////////////////////////////////////////////
/// ImDrawListFlags

FLAGS_CLASS_BODY(DrawListFlags, ImDrawListFlags)
FLAGS_CLASS_FUNCTION(DrawListFlags, ImDrawListFlags, None, False)
FLAGS_CLASS_FUNCTION(DrawListFlags, ImDrawListFlags, AntiAliasedLines, False)
FLAGS_CLASS_FUNCTION(DrawListFlags, ImDrawListFlags, AntiAliasedLinesUseTex, False)
FLAGS_CLASS_FUNCTION(DrawListFlags, ImDrawListFlags, AntiAliasedFill, False)
FLAGS_CLASS_FUNCTION(DrawListFlags, ImDrawListFlags, AllowVtxOffset, False)
FLAGS_CLASS_END(ImDrawListFlags)

//////////////////////////////////////////////////
/// ImFontAtlasFlags

FLAGS_CLASS_BODY(FontAtlasFlags, ImFontAtlasFlags)
FLAGS_CLASS_FUNCTION(FontAtlasFlags, ImFontAtlasFlags, None, False)
FLAGS_CLASS_FUNCTION(FontAtlasFlags, ImFontAtlasFlags, NoPowerOfTwoHeight, False)
FLAGS_CLASS_FUNCTION(FontAtlasFlags, ImFontAtlasFlags, NoMouseCursors, False)
FLAGS_CLASS_FUNCTION(FontAtlasFlags, ImFontAtlasFlags, NoBakedLines, False)
FLAGS_CLASS_END(ImFontAtlasFlags)

//////////////////////////////////////////////////
/// ImFontFlags

FLAGS_CLASS_BODY(FontFlags, ImFontFlags)
FLAGS_CLASS_FUNCTION(FontFlags, ImFontFlags, None, False)
FLAGS_CLASS_FUNCTION(FontFlags, ImFontFlags, NoLoadError, False)
FLAGS_CLASS_FUNCTION(FontFlags, ImFontFlags, NoLoadGlyphs, False)
FLAGS_CLASS_FUNCTION(FontFlags, ImFontFlags, LockBakedSizes, False)
FLAGS_CLASS_END(ImFontFlags)

//////////////////////////////////////////////////
/// ImGuiViewportFlags

FLAGS_CLASS_BODY(ViewportFlags, ImGuiViewportFlags)
FLAGS_CLASS_FUNCTION(ViewportFlags, ImGuiViewportFlags, None, False)
FLAGS_CLASS_FUNCTION(ViewportFlags, ImGuiViewportFlags, IsPlatformWindow, False)
FLAGS_CLASS_FUNCTION(ViewportFlags, ImGuiViewportFlags, IsPlatformMonitor, False)
FLAGS_CLASS_FUNCTION(ViewportFlags, ImGuiViewportFlags, OwnedByApp, False)
FLAGS_CLASS_END(ImGuiViewportFlags)

} // namespace ImGuiEx
