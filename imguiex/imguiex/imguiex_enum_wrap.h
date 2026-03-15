#pragma once

namespace ImGuiEx
{
template <typename Derived, typename EnumType>
class FlagBuilder
{
    EnumType flags_ = static_cast<EnumType>(0);

public:
    constexpr FlagBuilder() = default;

    constexpr explicit FlagBuilder(EnumType f) : flags_(f) {}

    constexpr operator EnumType() const { return flags_; }

    constexpr Derived set(EnumType f) const { return Derived(static_cast<EnumType>(flags_ | f)); }
};

/////////////////////////////////////////////////
/// ... ImGuiWindowFlags ...
/////////////////////////////////////////////////

class WindowFlags : public FlagBuilder<WindowFlags, ImGuiWindowFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiWindowFlags_None
    constexpr auto None() const -> WindowFlags { return set(ImGuiWindowFlags_None); }
    //! ImGuiWindowFlags_NoTitleBar
    constexpr auto NoTitleBar() const -> WindowFlags { return set(ImGuiWindowFlags_NoTitleBar); }
    //! ImGuiWindowFlags_NoResize
    constexpr auto NoResize() const -> WindowFlags { return set(ImGuiWindowFlags_NoResize); }
    //! ImGuiWindowFlags_NoMove
    constexpr auto NoMove() const -> WindowFlags { return set(ImGuiWindowFlags_NoMove); }
    //! ImGuiWindowFlags_NoScrollbar
    constexpr auto NoScrollbar() const -> WindowFlags { return set(ImGuiWindowFlags_NoScrollbar); }
    //! ImGuiWindowFlags_NoScrollWithMouse
    constexpr auto NoScrollWithMouse() const -> WindowFlags { return set(ImGuiWindowFlags_NoScrollWithMouse); }
    //! ImGuiWindowFlags_NoCollapse
    constexpr auto NoCollapse() const -> WindowFlags { return set(ImGuiWindowFlags_NoCollapse); }
    //! ImGuiWindowFlags_AlwaysAutoResize
    constexpr auto AlwaysAutoResize() const -> WindowFlags { return set(ImGuiWindowFlags_AlwaysAutoResize); }
    //! ImGuiWindowFlags_NoBackground
    constexpr auto NoBackground() const -> WindowFlags { return set(ImGuiWindowFlags_NoBackground); }
    //! ImGuiWindowFlags_NoSavedSettings
    constexpr auto NoSavedSettings() const -> WindowFlags { return set(ImGuiWindowFlags_NoSavedSettings); }
    //! ImGuiWindowFlags_NoMouseInputs
    constexpr auto NoMouseInputs() const -> WindowFlags { return set(ImGuiWindowFlags_NoMouseInputs); }
    //! ImGuiWindowFlags_MenuBar
    constexpr auto MenuBar() const -> WindowFlags { return set(ImGuiWindowFlags_MenuBar); }
    //! ImGuiWindowFlags_HorizontalScrollbar
    constexpr auto HorizontalScrollbar() const -> WindowFlags { return set(ImGuiWindowFlags_HorizontalScrollbar); }
    //! ImGuiWindowFlags_NoFocusOnAppearing
    constexpr auto NoFocusOnAppearing() const -> WindowFlags { return set(ImGuiWindowFlags_NoFocusOnAppearing); }
    //! ImGuiWindowFlags_NoBringToFrontOnFocus
    constexpr auto NoBringToFrontOnFocus() const -> WindowFlags { return set(ImGuiWindowFlags_NoBringToFrontOnFocus); }
    //! ImGuiWindowFlags_AlwaysVerticalScrollbar
    constexpr auto AlwaysVerticalScrollbar() const -> WindowFlags { return set(ImGuiWindowFlags_AlwaysVerticalScrollbar); }
    //! ImGuiWindowFlags_AlwaysHorizontalScrollbar
    constexpr auto AlwaysHorizontalScrollbar() const -> WindowFlags { return set(ImGuiWindowFlags_AlwaysHorizontalScrollbar); }
    //! ImGuiWindowFlags_NoNavInputs
    constexpr auto NoNavInputs() const -> WindowFlags { return set(ImGuiWindowFlags_NoNavInputs); }
    //! ImGuiWindowFlags_NoNavFocus
    constexpr auto NoNavFocus() const -> WindowFlags { return set(ImGuiWindowFlags_NoNavFocus); }
    //! ImGuiWindowFlags_UnsavedDocument
    constexpr auto UnsavedDocument() const -> WindowFlags { return set(ImGuiWindowFlags_UnsavedDocument); }
    //! ImGuiWindowFlags_NoNav
    constexpr auto NoNav() const -> WindowFlags { return set(ImGuiWindowFlags_NoNav); }
    //! ImGuiWindowFlags_NoDecoration
    constexpr auto NoDecoration() const -> WindowFlags { return set(ImGuiWindowFlags_NoDecoration); }
    //! ImGuiWindowFlags_NoInputs
    constexpr auto NoInputs() const -> WindowFlags { return set(ImGuiWindowFlags_NoInputs); }
    //! ImGuiWindowFlags_ChildWindow
    constexpr auto ChildWindow() const -> WindowFlags { return set(ImGuiWindowFlags_ChildWindow); }
    //! ImGuiWindowFlags_Tooltip
    constexpr auto Tooltip() const -> WindowFlags { return set(ImGuiWindowFlags_Tooltip); }
    //! ImGuiWindowFlags_Popup
    constexpr auto Popup() const -> WindowFlags { return set(ImGuiWindowFlags_Popup); }
    //! ImGuiWindowFlags_Modal
    constexpr auto Modal() const -> WindowFlags { return set(ImGuiWindowFlags_Modal); }
    //! ImGuiWindowFlags_ChildMenu
    constexpr auto ChildMenu() const -> WindowFlags { return set(ImGuiWindowFlags_ChildMenu); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiChildFlags ...
/////////////////////////////////////////////////

class ChildFlags : public FlagBuilder<ChildFlags, ImGuiChildFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiChildFlags_None
    constexpr auto None() const -> ChildFlags { return set(ImGuiChildFlags_None); }
    //! ImGuiChildFlags_Borders
    constexpr auto Borders() const -> ChildFlags { return set(ImGuiChildFlags_Borders); }
    //! ImGuiChildFlags_AlwaysUseWindowPadding
    constexpr auto AlwaysUseWindowPadding() const -> ChildFlags { return set(ImGuiChildFlags_AlwaysUseWindowPadding); }
    //! ImGuiChildFlags_ResizeX
    constexpr auto ResizeX() const -> ChildFlags { return set(ImGuiChildFlags_ResizeX); }
    //! ImGuiChildFlags_ResizeY
    constexpr auto ResizeY() const -> ChildFlags { return set(ImGuiChildFlags_ResizeY); }
    //! ImGuiChildFlags_AutoResizeX
    constexpr auto AutoResizeX() const -> ChildFlags { return set(ImGuiChildFlags_AutoResizeX); }
    //! ImGuiChildFlags_AutoResizeY
    constexpr auto AutoResizeY() const -> ChildFlags { return set(ImGuiChildFlags_AutoResizeY); }
    //! ImGuiChildFlags_AlwaysAutoResize
    constexpr auto AlwaysAutoResize() const -> ChildFlags { return set(ImGuiChildFlags_AlwaysAutoResize); }
    //! ImGuiChildFlags_FrameStyle
    constexpr auto FrameStyle() const -> ChildFlags { return set(ImGuiChildFlags_FrameStyle); }
    //! ImGuiChildFlags_NavFlattened
    constexpr auto NavFlattened() const -> ChildFlags { return set(ImGuiChildFlags_NavFlattened); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiItemFlags ...
/////////////////////////////////////////////////

class ItemFlags : public FlagBuilder<ItemFlags, ImGuiItemFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiItemFlags_None
    constexpr auto None() const -> ItemFlags { return set(ImGuiItemFlags_None); }
    //! ImGuiItemFlags_NoTabStop
    constexpr auto NoTabStop() const -> ItemFlags { return set(ImGuiItemFlags_NoTabStop); }
    //! ImGuiItemFlags_NoNav
    constexpr auto NoNav() const -> ItemFlags { return set(ImGuiItemFlags_NoNav); }
    //! ImGuiItemFlags_NoNavDefaultFocus
    constexpr auto NoNavDefaultFocus() const -> ItemFlags { return set(ImGuiItemFlags_NoNavDefaultFocus); }
    //! ImGuiItemFlags_ButtonRepeat
    constexpr auto ButtonRepeat() const -> ItemFlags { return set(ImGuiItemFlags_ButtonRepeat); }
    //! ImGuiItemFlags_AutoClosePopups
    constexpr auto AutoClosePopups() const -> ItemFlags { return set(ImGuiItemFlags_AutoClosePopups); }
    //! ImGuiItemFlags_AllowDuplicateId
    constexpr auto AllowDuplicateId() const -> ItemFlags { return set(ImGuiItemFlags_AllowDuplicateId); }
    //! ImGuiItemFlags_Disabled
    constexpr auto Disabled() const -> ItemFlags { return set(ImGuiItemFlags_Disabled); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiInputTextFlags ...
/////////////////////////////////////////////////

class InputTextFlags : public FlagBuilder<InputTextFlags, ImGuiInputTextFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiInputTextFlags_None
    constexpr auto None() const -> InputTextFlags { return set(ImGuiInputTextFlags_None); }
    //! ImGuiInputTextFlags_CharsDecimal
    constexpr auto CharsDecimal() const -> InputTextFlags { return set(ImGuiInputTextFlags_CharsDecimal); }
    //! ImGuiInputTextFlags_CharsHexadecimal
    constexpr auto CharsHexadecimal() const -> InputTextFlags { return set(ImGuiInputTextFlags_CharsHexadecimal); }
    //! ImGuiInputTextFlags_CharsScientific
    constexpr auto CharsScientific() const -> InputTextFlags { return set(ImGuiInputTextFlags_CharsScientific); }
    //! ImGuiInputTextFlags_CharsUppercase
    constexpr auto CharsUppercase() const -> InputTextFlags { return set(ImGuiInputTextFlags_CharsUppercase); }
    //! ImGuiInputTextFlags_CharsNoBlank
    constexpr auto CharsNoBlank() const -> InputTextFlags { return set(ImGuiInputTextFlags_CharsNoBlank); }
    //! ImGuiInputTextFlags_AllowTabInput
    constexpr auto AllowTabInput() const -> InputTextFlags { return set(ImGuiInputTextFlags_AllowTabInput); }
    //! ImGuiInputTextFlags_EnterReturnsTrue
    constexpr auto EnterReturnsTrue() const -> InputTextFlags { return set(ImGuiInputTextFlags_EnterReturnsTrue); }
    //! ImGuiInputTextFlags_EscapeClearsAll
    constexpr auto EscapeClearsAll() const -> InputTextFlags { return set(ImGuiInputTextFlags_EscapeClearsAll); }
    //! ImGuiInputTextFlags_CtrlEnterForNewLine
    constexpr auto CtrlEnterForNewLine() const -> InputTextFlags { return set(ImGuiInputTextFlags_CtrlEnterForNewLine); }
    //! ImGuiInputTextFlags_ReadOnly
    constexpr auto ReadOnly() const -> InputTextFlags { return set(ImGuiInputTextFlags_ReadOnly); }
    //! ImGuiInputTextFlags_Password
    constexpr auto Password() const -> InputTextFlags { return set(ImGuiInputTextFlags_Password); }
    //! ImGuiInputTextFlags_AlwaysOverwrite
    constexpr auto AlwaysOverwrite() const -> InputTextFlags { return set(ImGuiInputTextFlags_AlwaysOverwrite); }
    //! ImGuiInputTextFlags_AutoSelectAll
    constexpr auto AutoSelectAll() const -> InputTextFlags { return set(ImGuiInputTextFlags_AutoSelectAll); }
    //! ImGuiInputTextFlags_ParseEmptyRefVal
    constexpr auto ParseEmptyRefVal() const -> InputTextFlags { return set(ImGuiInputTextFlags_ParseEmptyRefVal); }
    //! ImGuiInputTextFlags_DisplayEmptyRefVal
    constexpr auto DisplayEmptyRefVal() const -> InputTextFlags { return set(ImGuiInputTextFlags_DisplayEmptyRefVal); }
    //! ImGuiInputTextFlags_NoHorizontalScroll
    constexpr auto NoHorizontalScroll() const -> InputTextFlags { return set(ImGuiInputTextFlags_NoHorizontalScroll); }
    //! ImGuiInputTextFlags_NoUndoRedo
    constexpr auto NoUndoRedo() const -> InputTextFlags { return set(ImGuiInputTextFlags_NoUndoRedo); }
    //! ImGuiInputTextFlags_ElideLeft
    constexpr auto ElideLeft() const -> InputTextFlags { return set(ImGuiInputTextFlags_ElideLeft); }
    //! ImGuiInputTextFlags_CallbackCompletion
    constexpr auto CallbackCompletion() const -> InputTextFlags { return set(ImGuiInputTextFlags_CallbackCompletion); }
    //! ImGuiInputTextFlags_CallbackHistory
    constexpr auto CallbackHistory() const -> InputTextFlags { return set(ImGuiInputTextFlags_CallbackHistory); }
    //! ImGuiInputTextFlags_CallbackAlways
    constexpr auto CallbackAlways() const -> InputTextFlags { return set(ImGuiInputTextFlags_CallbackAlways); }
    //! ImGuiInputTextFlags_CallbackCharFilter
    constexpr auto CallbackCharFilter() const -> InputTextFlags { return set(ImGuiInputTextFlags_CallbackCharFilter); }
    //! ImGuiInputTextFlags_CallbackResize
    constexpr auto CallbackResize() const -> InputTextFlags { return set(ImGuiInputTextFlags_CallbackResize); }
    //! ImGuiInputTextFlags_CallbackEdit
    constexpr auto CallbackEdit() const -> InputTextFlags { return set(ImGuiInputTextFlags_CallbackEdit); }
    //! ImGuiInputTextFlags_WordWrap
    constexpr auto WordWrap() const -> InputTextFlags { return set(ImGuiInputTextFlags_WordWrap); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiTreeNodeFlags ...
/////////////////////////////////////////////////

class TreeNodeFlags : public FlagBuilder<TreeNodeFlags, ImGuiTreeNodeFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiTreeNodeFlags_None
    constexpr auto None() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_None); }
    //! ImGuiTreeNodeFlags_Selected
    constexpr auto Selected() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_Selected); }
    //! ImGuiTreeNodeFlags_Framed
    constexpr auto Framed() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_Framed); }
    //! ImGuiTreeNodeFlags_AllowOverlap
    constexpr auto AllowOverlap() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_AllowOverlap); }
    //! ImGuiTreeNodeFlags_NoTreePushOnOpen
    constexpr auto NoTreePushOnOpen() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_NoTreePushOnOpen); }
    //! ImGuiTreeNodeFlags_NoAutoOpenOnLog
    constexpr auto NoAutoOpenOnLog() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_NoAutoOpenOnLog); }
    //! ImGuiTreeNodeFlags_DefaultOpen
    constexpr auto DefaultOpen() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_DefaultOpen); }
    //! ImGuiTreeNodeFlags_OpenOnDoubleClick
    constexpr auto OpenOnDoubleClick() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_OpenOnDoubleClick); }
    //! ImGuiTreeNodeFlags_OpenOnArrow
    constexpr auto OpenOnArrow() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_OpenOnArrow); }
    //! ImGuiTreeNodeFlags_Leaf
    constexpr auto Leaf() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_Leaf); }
    //! ImGuiTreeNodeFlags_Bullet
    constexpr auto Bullet() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_Bullet); }
    //! ImGuiTreeNodeFlags_FramePadding
    constexpr auto FramePadding() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_FramePadding); }
    //! ImGuiTreeNodeFlags_SpanAvailWidth
    constexpr auto SpanAvailWidth() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_SpanAvailWidth); }
    //! ImGuiTreeNodeFlags_SpanFullWidth
    constexpr auto SpanFullWidth() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_SpanFullWidth); }
    //! ImGuiTreeNodeFlags_SpanLabelWidth
    constexpr auto SpanLabelWidth() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_SpanLabelWidth); }
    //! ImGuiTreeNodeFlags_SpanAllColumns
    constexpr auto SpanAllColumns() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_SpanAllColumns); }
    //! ImGuiTreeNodeFlags_LabelSpanAllColumns
    constexpr auto LabelSpanAllColumns() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_LabelSpanAllColumns); }
    //! ImGuiTreeNodeFlags_NavLeftJumpsToParent
    constexpr auto NavLeftJumpsToParent() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_NavLeftJumpsToParent); }
    //! ImGuiTreeNodeFlags_CollapsingHeader
    constexpr auto CollapsingHeader() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_CollapsingHeader); }
    //! ImGuiTreeNodeFlags_DrawLinesNone
    constexpr auto DrawLinesNone() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_DrawLinesNone); }
    //! ImGuiTreeNodeFlags_DrawLinesFull
    constexpr auto DrawLinesFull() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_DrawLinesFull); }
    //! ImGuiTreeNodeFlags_DrawLinesToNodes
    constexpr auto DrawLinesToNodes() const -> TreeNodeFlags { return set(ImGuiTreeNodeFlags_DrawLinesToNodes); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiPopupFlags ...
/////////////////////////////////////////////////

class PopupFlags : public FlagBuilder<PopupFlags, ImGuiPopupFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiPopupFlags_None
    constexpr auto None() const -> PopupFlags { return set(ImGuiPopupFlags_None); }
    //! ImGuiPopupFlags_MouseButtonLeft
    constexpr auto MouseButtonLeft() const -> PopupFlags { return set(ImGuiPopupFlags_MouseButtonLeft); }
    //! ImGuiPopupFlags_MouseButtonRight
    constexpr auto MouseButtonRight() const -> PopupFlags { return set(ImGuiPopupFlags_MouseButtonRight); }
    //! ImGuiPopupFlags_MouseButtonMiddle
    constexpr auto MouseButtonMiddle() const -> PopupFlags { return set(ImGuiPopupFlags_MouseButtonMiddle); }
    //! ImGuiPopupFlags_NoReopen
    constexpr auto NoReopen() const -> PopupFlags { return set(ImGuiPopupFlags_NoReopen); }
    //! ImGuiPopupFlags_NoOpenOverExistingPopup
    constexpr auto NoOpenOverExistingPopup() const -> PopupFlags { return set(ImGuiPopupFlags_NoOpenOverExistingPopup); }
    //! ImGuiPopupFlags_NoOpenOverItems
    constexpr auto NoOpenOverItems() const -> PopupFlags { return set(ImGuiPopupFlags_NoOpenOverItems); }
    //! ImGuiPopupFlags_AnyPopupId
    constexpr auto AnyPopupId() const -> PopupFlags { return set(ImGuiPopupFlags_AnyPopupId); }
    //! ImGuiPopupFlags_AnyPopupLevel
    constexpr auto AnyPopupLevel() const -> PopupFlags { return set(ImGuiPopupFlags_AnyPopupLevel); }
    //! ImGuiPopupFlags_AnyPopup
    constexpr auto AnyPopup() const -> PopupFlags { return set(ImGuiPopupFlags_AnyPopup); }
    //! ImGuiPopupFlags_MouseButtonShift_
    constexpr auto MouseButtonShift_() const -> PopupFlags { return set(ImGuiPopupFlags_MouseButtonShift_); }
    //! ImGuiPopupFlags_MouseButtonMask_
    constexpr auto MouseButtonMask_() const -> PopupFlags { return set(ImGuiPopupFlags_MouseButtonMask_); }
    //! ImGuiPopupFlags_InvalidMask_
    constexpr auto InvalidMask_() const -> PopupFlags { return set(ImGuiPopupFlags_InvalidMask_); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiSelectableFlags ...
/////////////////////////////////////////////////

class SelectableFlags : public FlagBuilder<SelectableFlags, ImGuiSelectableFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiSelectableFlags_None
    constexpr auto None() const -> SelectableFlags { return set(ImGuiSelectableFlags_None); }
    //! ImGuiSelectableFlags_NoAutoClosePopups
    constexpr auto NoAutoClosePopups() const -> SelectableFlags { return set(ImGuiSelectableFlags_NoAutoClosePopups); }
    //! ImGuiSelectableFlags_SpanAllColumns
    constexpr auto SpanAllColumns() const -> SelectableFlags { return set(ImGuiSelectableFlags_SpanAllColumns); }
    //! ImGuiSelectableFlags_AllowDoubleClick
    constexpr auto AllowDoubleClick() const -> SelectableFlags { return set(ImGuiSelectableFlags_AllowDoubleClick); }
    //! ImGuiSelectableFlags_Disabled
    constexpr auto Disabled() const -> SelectableFlags { return set(ImGuiSelectableFlags_Disabled); }
    //! ImGuiSelectableFlags_AllowOverlap
    constexpr auto AllowOverlap() const -> SelectableFlags { return set(ImGuiSelectableFlags_AllowOverlap); }
    //! ImGuiSelectableFlags_Highlight
    constexpr auto Highlight() const -> SelectableFlags { return set(ImGuiSelectableFlags_Highlight); }
    //! ImGuiSelectableFlags_SelectOnNav
    constexpr auto SelectOnNav() const -> SelectableFlags { return set(ImGuiSelectableFlags_SelectOnNav); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiComboFlags ...
/////////////////////////////////////////////////

class ComboFlags : public FlagBuilder<ComboFlags, ImGuiComboFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiComboFlags_None
    constexpr auto None() const -> ComboFlags { return set(ImGuiComboFlags_None); }
    //! ImGuiComboFlags_PopupAlignLeft
    constexpr auto PopupAlignLeft() const -> ComboFlags { return set(ImGuiComboFlags_PopupAlignLeft); }
    //! ImGuiComboFlags_HeightSmall
    constexpr auto HeightSmall() const -> ComboFlags { return set(ImGuiComboFlags_HeightSmall); }
    //! ImGuiComboFlags_HeightRegular
    constexpr auto HeightRegular() const -> ComboFlags { return set(ImGuiComboFlags_HeightRegular); }
    //! ImGuiComboFlags_HeightLarge
    constexpr auto HeightLarge() const -> ComboFlags { return set(ImGuiComboFlags_HeightLarge); }
    //! ImGuiComboFlags_HeightLargest
    constexpr auto HeightLargest() const -> ComboFlags { return set(ImGuiComboFlags_HeightLargest); }
    //! ImGuiComboFlags_NoArrowButton
    constexpr auto NoArrowButton() const -> ComboFlags { return set(ImGuiComboFlags_NoArrowButton); }
    //! ImGuiComboFlags_NoPreview
    constexpr auto NoPreview() const -> ComboFlags { return set(ImGuiComboFlags_NoPreview); }
    //! ImGuiComboFlags_WidthFitPreview
    constexpr auto WidthFitPreview() const -> ComboFlags { return set(ImGuiComboFlags_WidthFitPreview); }
    //! ImGuiComboFlags_HeightMask_
    constexpr auto HeightMask_() const -> ComboFlags { return set(ImGuiComboFlags_HeightMask_); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiTabBarFlags ...
/////////////////////////////////////////////////

class TabBarFlags : public FlagBuilder<TabBarFlags, ImGuiTabBarFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiTabBarFlags_None
    constexpr auto None() const -> TabBarFlags { return set(ImGuiTabBarFlags_None); }
    //! ImGuiTabBarFlags_Reorderable
    constexpr auto Reorderable() const -> TabBarFlags { return set(ImGuiTabBarFlags_Reorderable); }
    //! ImGuiTabBarFlags_AutoSelectNewTabs
    constexpr auto AutoSelectNewTabs() const -> TabBarFlags { return set(ImGuiTabBarFlags_AutoSelectNewTabs); }
    //! ImGuiTabBarFlags_TabListPopupButton
    constexpr auto TabListPopupButton() const -> TabBarFlags { return set(ImGuiTabBarFlags_TabListPopupButton); }
    //! ImGuiTabBarFlags_NoCloseWithMiddleMouseButton
    constexpr auto NoCloseWithMiddleMouseButton() const -> TabBarFlags { return set(ImGuiTabBarFlags_NoCloseWithMiddleMouseButton); }
    //! ImGuiTabBarFlags_NoTabListScrollingButtons
    constexpr auto NoTabListScrollingButtons() const -> TabBarFlags { return set(ImGuiTabBarFlags_NoTabListScrollingButtons); }
    //! ImGuiTabBarFlags_NoTooltip
    constexpr auto NoTooltip() const -> TabBarFlags { return set(ImGuiTabBarFlags_NoTooltip); }
    //! ImGuiTabBarFlags_DrawSelectedOverline
    constexpr auto DrawSelectedOverline() const -> TabBarFlags { return set(ImGuiTabBarFlags_DrawSelectedOverline); }
    //! ImGuiTabBarFlags_FittingPolicyMixed
    constexpr auto FittingPolicyMixed() const -> TabBarFlags { return set(ImGuiTabBarFlags_FittingPolicyMixed); }
    //! ImGuiTabBarFlags_FittingPolicyShrink
    constexpr auto FittingPolicyShrink() const -> TabBarFlags { return set(ImGuiTabBarFlags_FittingPolicyShrink); }
    //! ImGuiTabBarFlags_FittingPolicyScroll
    constexpr auto FittingPolicyScroll() const -> TabBarFlags { return set(ImGuiTabBarFlags_FittingPolicyScroll); }
    //! ImGuiTabBarFlags_FittingPolicyMask_
    constexpr auto FittingPolicyMask_() const -> TabBarFlags { return set(ImGuiTabBarFlags_FittingPolicyMask_); }
    //! ImGuiTabBarFlags_FittingPolicyDefault_
    constexpr auto FittingPolicyDefault_() const -> TabBarFlags { return set(ImGuiTabBarFlags_FittingPolicyDefault_); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiTabItemFlags ...
/////////////////////////////////////////////////

class TabItemFlags : public FlagBuilder<TabItemFlags, ImGuiTabItemFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiTabItemFlags_None
    constexpr auto None() const -> TabItemFlags { return set(ImGuiTabItemFlags_None); }
    //! ImGuiTabItemFlags_UnsavedDocument
    constexpr auto UnsavedDocument() const -> TabItemFlags { return set(ImGuiTabItemFlags_UnsavedDocument); }
    //! ImGuiTabItemFlags_SetSelected
    constexpr auto SetSelected() const -> TabItemFlags { return set(ImGuiTabItemFlags_SetSelected); }
    //! ImGuiTabItemFlags_NoCloseWithMiddleMouseButton
    constexpr auto NoCloseWithMiddleMouseButton() const -> TabItemFlags { return set(ImGuiTabItemFlags_NoCloseWithMiddleMouseButton); }
    //! ImGuiTabItemFlags_NoPushId
    constexpr auto NoPushId() const -> TabItemFlags { return set(ImGuiTabItemFlags_NoPushId); }
    //! ImGuiTabItemFlags_NoTooltip
    constexpr auto NoTooltip() const -> TabItemFlags { return set(ImGuiTabItemFlags_NoTooltip); }
    //! ImGuiTabItemFlags_NoReorder
    constexpr auto NoReorder() const -> TabItemFlags { return set(ImGuiTabItemFlags_NoReorder); }
    //! ImGuiTabItemFlags_Leading
    constexpr auto Leading() const -> TabItemFlags { return set(ImGuiTabItemFlags_Leading); }
    //! ImGuiTabItemFlags_Trailing
    constexpr auto Trailing() const -> TabItemFlags { return set(ImGuiTabItemFlags_Trailing); }
    //! ImGuiTabItemFlags_NoAssumedClosure
    constexpr auto NoAssumedClosure() const -> TabItemFlags { return set(ImGuiTabItemFlags_NoAssumedClosure); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiFocusedFlags ...
/////////////////////////////////////////////////

class FocusedFlags : public FlagBuilder<FocusedFlags, ImGuiFocusedFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiFocusedFlags_None
    constexpr auto None() const -> FocusedFlags { return set(ImGuiFocusedFlags_None); }
    //! ImGuiFocusedFlags_ChildWindows
    constexpr auto ChildWindows() const -> FocusedFlags { return set(ImGuiFocusedFlags_ChildWindows); }
    //! ImGuiFocusedFlags_RootWindow
    constexpr auto RootWindow() const -> FocusedFlags { return set(ImGuiFocusedFlags_RootWindow); }
    //! ImGuiFocusedFlags_AnyWindow
    constexpr auto AnyWindow() const -> FocusedFlags { return set(ImGuiFocusedFlags_AnyWindow); }
    //! ImGuiFocusedFlags_NoPopupHierarchy
    constexpr auto NoPopupHierarchy() const -> FocusedFlags { return set(ImGuiFocusedFlags_NoPopupHierarchy); }
    //! ImGuiFocusedFlags_RootAndChildWindows
    constexpr auto RootAndChildWindows() const -> FocusedFlags { return set(ImGuiFocusedFlags_RootAndChildWindows); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiHoveredFlags ...
/////////////////////////////////////////////////

class HoveredFlags : public FlagBuilder<HoveredFlags, ImGuiHoveredFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiHoveredFlags_None
    constexpr auto None() const -> HoveredFlags { return set(ImGuiHoveredFlags_None); }
    //! ImGuiHoveredFlags_ChildWindows
    constexpr auto ChildWindows() const -> HoveredFlags { return set(ImGuiHoveredFlags_ChildWindows); }
    //! ImGuiHoveredFlags_RootWindow
    constexpr auto RootWindow() const -> HoveredFlags { return set(ImGuiHoveredFlags_RootWindow); }
    //! ImGuiHoveredFlags_AnyWindow
    constexpr auto AnyWindow() const -> HoveredFlags { return set(ImGuiHoveredFlags_AnyWindow); }
    //! ImGuiHoveredFlags_NoPopupHierarchy
    constexpr auto NoPopupHierarchy() const -> HoveredFlags { return set(ImGuiHoveredFlags_NoPopupHierarchy); }
    //! ImGuiHoveredFlags_AllowWhenBlockedByPopup
    constexpr auto AllowWhenBlockedByPopup() const -> HoveredFlags { return set(ImGuiHoveredFlags_AllowWhenBlockedByPopup); }
    //! ImGuiHoveredFlags_AllowWhenBlockedByActiveItem
    constexpr auto AllowWhenBlockedByActiveItem() const -> HoveredFlags { return set(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem); }
    //! ImGuiHoveredFlags_AllowWhenOverlappedByItem
    constexpr auto AllowWhenOverlappedByItem() const -> HoveredFlags { return set(ImGuiHoveredFlags_AllowWhenOverlappedByItem); }
    //! ImGuiHoveredFlags_AllowWhenOverlappedByWindow
    constexpr auto AllowWhenOverlappedByWindow() const -> HoveredFlags { return set(ImGuiHoveredFlags_AllowWhenOverlappedByWindow); }
    //! ImGuiHoveredFlags_AllowWhenDisabled
    constexpr auto AllowWhenDisabled() const -> HoveredFlags { return set(ImGuiHoveredFlags_AllowWhenDisabled); }
    //! ImGuiHoveredFlags_NoNavOverride
    constexpr auto NoNavOverride() const -> HoveredFlags { return set(ImGuiHoveredFlags_NoNavOverride); }
    //! ImGuiHoveredFlags_AllowWhenOverlapped
    constexpr auto AllowWhenOverlapped() const -> HoveredFlags { return set(ImGuiHoveredFlags_AllowWhenOverlapped); }
    //! ImGuiHoveredFlags_RectOnly
    constexpr auto RectOnly() const -> HoveredFlags { return set(ImGuiHoveredFlags_RectOnly); }
    //! ImGuiHoveredFlags_RootAndChildWindows
    constexpr auto RootAndChildWindows() const -> HoveredFlags { return set(ImGuiHoveredFlags_RootAndChildWindows); }
    //! ImGuiHoveredFlags_ForTooltip
    constexpr auto ForTooltip() const -> HoveredFlags { return set(ImGuiHoveredFlags_ForTooltip); }
    //! ImGuiHoveredFlags_Stationary
    constexpr auto Stationary() const -> HoveredFlags { return set(ImGuiHoveredFlags_Stationary); }
    //! ImGuiHoveredFlags_DelayNone
    constexpr auto DelayNone() const -> HoveredFlags { return set(ImGuiHoveredFlags_DelayNone); }
    //! ImGuiHoveredFlags_DelayShort
    constexpr auto DelayShort() const -> HoveredFlags { return set(ImGuiHoveredFlags_DelayShort); }
    //! ImGuiHoveredFlags_DelayNormal
    constexpr auto DelayNormal() const -> HoveredFlags { return set(ImGuiHoveredFlags_DelayNormal); }
    //! ImGuiHoveredFlags_NoSharedDelay
    constexpr auto NoSharedDelay() const -> HoveredFlags { return set(ImGuiHoveredFlags_NoSharedDelay); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiDragDropFlags ...
/////////////////////////////////////////////////

class DragDropFlags : public FlagBuilder<DragDropFlags, ImGuiDragDropFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiDragDropFlags_None
    constexpr auto None() const -> DragDropFlags { return set(ImGuiDragDropFlags_None); }
    //! ImGuiDragDropFlags_SourceNoPreviewTooltip
    constexpr auto SourceNoPreviewTooltip() const -> DragDropFlags { return set(ImGuiDragDropFlags_SourceNoPreviewTooltip); }
    //! ImGuiDragDropFlags_SourceNoDisableHover
    constexpr auto SourceNoDisableHover() const -> DragDropFlags { return set(ImGuiDragDropFlags_SourceNoDisableHover); }
    //! ImGuiDragDropFlags_SourceNoHoldToOpenOthers
    constexpr auto SourceNoHoldToOpenOthers() const -> DragDropFlags { return set(ImGuiDragDropFlags_SourceNoHoldToOpenOthers); }
    //! ImGuiDragDropFlags_SourceAllowNullID
    constexpr auto SourceAllowNullID() const -> DragDropFlags { return set(ImGuiDragDropFlags_SourceAllowNullID); }
    //! ImGuiDragDropFlags_SourceExtern
    constexpr auto SourceExtern() const -> DragDropFlags { return set(ImGuiDragDropFlags_SourceExtern); }
    //! ImGuiDragDropFlags_PayloadAutoExpire
    constexpr auto PayloadAutoExpire() const -> DragDropFlags { return set(ImGuiDragDropFlags_PayloadAutoExpire); }
    //! ImGuiDragDropFlags_PayloadNoCrossContext
    constexpr auto PayloadNoCrossContext() const -> DragDropFlags { return set(ImGuiDragDropFlags_PayloadNoCrossContext); }
    //! ImGuiDragDropFlags_PayloadNoCrossProcess
    constexpr auto PayloadNoCrossProcess() const -> DragDropFlags { return set(ImGuiDragDropFlags_PayloadNoCrossProcess); }
    //! ImGuiDragDropFlags_AcceptBeforeDelivery
    constexpr auto AcceptBeforeDelivery() const -> DragDropFlags { return set(ImGuiDragDropFlags_AcceptBeforeDelivery); }
    //! ImGuiDragDropFlags_AcceptNoDrawDefaultRect
    constexpr auto AcceptNoDrawDefaultRect() const -> DragDropFlags { return set(ImGuiDragDropFlags_AcceptNoDrawDefaultRect); }
    //! ImGuiDragDropFlags_AcceptNoPreviewTooltip
    constexpr auto AcceptNoPreviewTooltip() const -> DragDropFlags { return set(ImGuiDragDropFlags_AcceptNoPreviewTooltip); }
    //! ImGuiDragDropFlags_AcceptDrawAsHovered
    constexpr auto AcceptDrawAsHovered() const -> DragDropFlags { return set(ImGuiDragDropFlags_AcceptDrawAsHovered); }
    //! ImGuiDragDropFlags_AcceptPeekOnly
    constexpr auto AcceptPeekOnly() const -> DragDropFlags { return set(ImGuiDragDropFlags_AcceptPeekOnly); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiInputFlags ...
/////////////////////////////////////////////////

class InputFlags : public FlagBuilder<InputFlags, ImGuiInputFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiInputFlags_None
    constexpr auto None() const -> InputFlags { return set(ImGuiInputFlags_None); }
    //! ImGuiInputFlags_Repeat
    constexpr auto Repeat() const -> InputFlags { return set(ImGuiInputFlags_Repeat); }
    //! ImGuiInputFlags_RouteActive
    constexpr auto RouteActive() const -> InputFlags { return set(ImGuiInputFlags_RouteActive); }
    //! ImGuiInputFlags_RouteFocused
    constexpr auto RouteFocused() const -> InputFlags { return set(ImGuiInputFlags_RouteFocused); }
    //! ImGuiInputFlags_RouteGlobal
    constexpr auto RouteGlobal() const -> InputFlags { return set(ImGuiInputFlags_RouteGlobal); }
    //! ImGuiInputFlags_RouteAlways
    constexpr auto RouteAlways() const -> InputFlags { return set(ImGuiInputFlags_RouteAlways); }
    //! ImGuiInputFlags_RouteOverFocused
    constexpr auto RouteOverFocused() const -> InputFlags { return set(ImGuiInputFlags_RouteOverFocused); }
    //! ImGuiInputFlags_RouteOverActive
    constexpr auto RouteOverActive() const -> InputFlags { return set(ImGuiInputFlags_RouteOverActive); }
    //! ImGuiInputFlags_RouteUnlessBgFocused
    constexpr auto RouteUnlessBgFocused() const -> InputFlags { return set(ImGuiInputFlags_RouteUnlessBgFocused); }
    //! ImGuiInputFlags_RouteFromRootWindow
    constexpr auto RouteFromRootWindow() const -> InputFlags { return set(ImGuiInputFlags_RouteFromRootWindow); }
    //! ImGuiInputFlags_Tooltip
    constexpr auto Tooltip() const -> InputFlags { return set(ImGuiInputFlags_Tooltip); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiConfigFlags ...
/////////////////////////////////////////////////

class ConfigFlags : public FlagBuilder<ConfigFlags, ImGuiConfigFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiConfigFlags_None
    constexpr auto None() const -> ConfigFlags { return set(ImGuiConfigFlags_None); }
    //! ImGuiConfigFlags_NavEnableKeyboard
    constexpr auto NavEnableKeyboard() const -> ConfigFlags { return set(ImGuiConfigFlags_NavEnableKeyboard); }
    //! ImGuiConfigFlags_NavEnableGamepad
    constexpr auto NavEnableGamepad() const -> ConfigFlags { return set(ImGuiConfigFlags_NavEnableGamepad); }
    //! ImGuiConfigFlags_NoMouse
    constexpr auto NoMouse() const -> ConfigFlags { return set(ImGuiConfigFlags_NoMouse); }
    //! ImGuiConfigFlags_NoMouseCursorChange
    constexpr auto NoMouseCursorChange() const -> ConfigFlags { return set(ImGuiConfigFlags_NoMouseCursorChange); }
    //! ImGuiConfigFlags_NoKeyboard
    constexpr auto NoKeyboard() const -> ConfigFlags { return set(ImGuiConfigFlags_NoKeyboard); }
    //! ImGuiConfigFlags_IsSRGB
    constexpr auto IsSRGB() const -> ConfigFlags { return set(ImGuiConfigFlags_IsSRGB); }
    //! ImGuiConfigFlags_IsTouchScreen
    constexpr auto IsTouchScreen() const -> ConfigFlags { return set(ImGuiConfigFlags_IsTouchScreen); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiBackendFlags ...
/////////////////////////////////////////////////

class BackendFlags : public FlagBuilder<BackendFlags, ImGuiBackendFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiBackendFlags_None
    constexpr auto None() const -> BackendFlags { return set(ImGuiBackendFlags_None); }
    //! ImGuiBackendFlags_HasGamepad
    constexpr auto HasGamepad() const -> BackendFlags { return set(ImGuiBackendFlags_HasGamepad); }
    //! ImGuiBackendFlags_HasMouseCursors
    constexpr auto HasMouseCursors() const -> BackendFlags { return set(ImGuiBackendFlags_HasMouseCursors); }
    //! ImGuiBackendFlags_HasSetMousePos
    constexpr auto HasSetMousePos() const -> BackendFlags { return set(ImGuiBackendFlags_HasSetMousePos); }
    //! ImGuiBackendFlags_RendererHasVtxOffset
    constexpr auto RendererHasVtxOffset() const -> BackendFlags { return set(ImGuiBackendFlags_RendererHasVtxOffset); }
    //! ImGuiBackendFlags_RendererHasTextures
    constexpr auto RendererHasTextures() const -> BackendFlags { return set(ImGuiBackendFlags_RendererHasTextures); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiButtonFlags ...
/////////////////////////////////////////////////

class ButtonFlags : public FlagBuilder<ButtonFlags, ImGuiButtonFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiButtonFlags_None
    constexpr auto None() const -> ButtonFlags { return set(ImGuiButtonFlags_None); }
    //! ImGuiButtonFlags_MouseButtonLeft
    constexpr auto MouseButtonLeft() const -> ButtonFlags { return set(ImGuiButtonFlags_MouseButtonLeft); }
    //! ImGuiButtonFlags_MouseButtonRight
    constexpr auto MouseButtonRight() const -> ButtonFlags { return set(ImGuiButtonFlags_MouseButtonRight); }
    //! ImGuiButtonFlags_MouseButtonMiddle
    constexpr auto MouseButtonMiddle() const -> ButtonFlags { return set(ImGuiButtonFlags_MouseButtonMiddle); }
    //! ImGuiButtonFlags_MouseButtonMask_
    constexpr auto MouseButtonMask_() const -> ButtonFlags { return set(ImGuiButtonFlags_MouseButtonMask_); }
    //! ImGuiButtonFlags_EnableNav
    constexpr auto EnableNav() const -> ButtonFlags { return set(ImGuiButtonFlags_EnableNav); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiColorEditFlags ...
/////////////////////////////////////////////////

class ColorEditFlags : public FlagBuilder<ColorEditFlags, ImGuiColorEditFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiColorEditFlags_None
    constexpr auto None() const -> ColorEditFlags { return set(ImGuiColorEditFlags_None); }
    //! ImGuiColorEditFlags_NoAlpha
    constexpr auto NoAlpha() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoAlpha); }
    //! ImGuiColorEditFlags_NoPicker
    constexpr auto NoPicker() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoPicker); }
    //! ImGuiColorEditFlags_NoOptions
    constexpr auto NoOptions() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoOptions); }
    //! ImGuiColorEditFlags_NoSmallPreview
    constexpr auto NoSmallPreview() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoSmallPreview); }
    //! ImGuiColorEditFlags_NoInputs
    constexpr auto NoInputs() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoInputs); }
    //! ImGuiColorEditFlags_NoTooltip
    constexpr auto NoTooltip() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoTooltip); }
    //! ImGuiColorEditFlags_NoLabel
    constexpr auto NoLabel() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoLabel); }
    //! ImGuiColorEditFlags_NoSidePreview
    constexpr auto NoSidePreview() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoSidePreview); }
    //! ImGuiColorEditFlags_NoDragDrop
    constexpr auto NoDragDrop() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoDragDrop); }
    //! ImGuiColorEditFlags_NoBorder
    constexpr auto NoBorder() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoBorder); }
    //! ImGuiColorEditFlags_NoColorMarkers
    constexpr auto NoColorMarkers() const -> ColorEditFlags { return set(ImGuiColorEditFlags_NoColorMarkers); }
    //! ImGuiColorEditFlags_AlphaOpaque
    constexpr auto AlphaOpaque() const -> ColorEditFlags { return set(ImGuiColorEditFlags_AlphaOpaque); }
    //! ImGuiColorEditFlags_AlphaNoBg
    constexpr auto AlphaNoBg() const -> ColorEditFlags { return set(ImGuiColorEditFlags_AlphaNoBg); }
    //! ImGuiColorEditFlags_AlphaPreviewHalf
    constexpr auto AlphaPreviewHalf() const -> ColorEditFlags { return set(ImGuiColorEditFlags_AlphaPreviewHalf); }
    //! ImGuiColorEditFlags_AlphaBar
    constexpr auto AlphaBar() const -> ColorEditFlags { return set(ImGuiColorEditFlags_AlphaBar); }
    //! ImGuiColorEditFlags_HDR
    constexpr auto HDR() const -> ColorEditFlags { return set(ImGuiColorEditFlags_HDR); }
    //! ImGuiColorEditFlags_DisplayRGB
    constexpr auto DisplayRGB() const -> ColorEditFlags { return set(ImGuiColorEditFlags_DisplayRGB); }
    //! ImGuiColorEditFlags_DisplayHSV
    constexpr auto DisplayHSV() const -> ColorEditFlags { return set(ImGuiColorEditFlags_DisplayHSV); }
    //! ImGuiColorEditFlags_DisplayHex
    constexpr auto DisplayHex() const -> ColorEditFlags { return set(ImGuiColorEditFlags_DisplayHex); }
    //! ImGuiColorEditFlags_Uint8
    constexpr auto Uint8() const -> ColorEditFlags { return set(ImGuiColorEditFlags_Uint8); }
    //! ImGuiColorEditFlags_Float
    constexpr auto Float() const -> ColorEditFlags { return set(ImGuiColorEditFlags_Float); }
    //! ImGuiColorEditFlags_PickerHueBar
    constexpr auto PickerHueBar() const -> ColorEditFlags { return set(ImGuiColorEditFlags_PickerHueBar); }
    //! ImGuiColorEditFlags_PickerHueWheel
    constexpr auto PickerHueWheel() const -> ColorEditFlags { return set(ImGuiColorEditFlags_PickerHueWheel); }
    //! ImGuiColorEditFlags_InputRGB
    constexpr auto InputRGB() const -> ColorEditFlags { return set(ImGuiColorEditFlags_InputRGB); }
    //! ImGuiColorEditFlags_InputHSV
    constexpr auto InputHSV() const -> ColorEditFlags { return set(ImGuiColorEditFlags_InputHSV); }
    //! ImGuiColorEditFlags_DefaultOptions_
    constexpr auto DefaultOptions_() const -> ColorEditFlags { return set(ImGuiColorEditFlags_DefaultOptions_); }
    //! ImGuiColorEditFlags_AlphaMask_
    constexpr auto AlphaMask_() const -> ColorEditFlags { return set(ImGuiColorEditFlags_AlphaMask_); }
    //! ImGuiColorEditFlags_DisplayMask_
    constexpr auto DisplayMask_() const -> ColorEditFlags { return set(ImGuiColorEditFlags_DisplayMask_); }
    //! ImGuiColorEditFlags_DataTypeMask_
    constexpr auto DataTypeMask_() const -> ColorEditFlags { return set(ImGuiColorEditFlags_DataTypeMask_); }
    //! ImGuiColorEditFlags_PickerMask_
    constexpr auto PickerMask_() const -> ColorEditFlags { return set(ImGuiColorEditFlags_PickerMask_); }
    //! ImGuiColorEditFlags_InputMask_
    constexpr auto InputMask_() const -> ColorEditFlags { return set(ImGuiColorEditFlags_InputMask_); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiSliderFlags ...
/////////////////////////////////////////////////

class SliderFlags : public FlagBuilder<SliderFlags, ImGuiSliderFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiSliderFlags_None
    constexpr auto None() const -> SliderFlags { return set(ImGuiSliderFlags_None); }
    //! ImGuiSliderFlags_Logarithmic
    constexpr auto Logarithmic() const -> SliderFlags { return set(ImGuiSliderFlags_Logarithmic); }
    //! ImGuiSliderFlags_NoRoundToFormat
    constexpr auto NoRoundToFormat() const -> SliderFlags { return set(ImGuiSliderFlags_NoRoundToFormat); }
    //! ImGuiSliderFlags_NoInput
    constexpr auto NoInput() const -> SliderFlags { return set(ImGuiSliderFlags_NoInput); }
    //! ImGuiSliderFlags_WrapAround
    constexpr auto WrapAround() const -> SliderFlags { return set(ImGuiSliderFlags_WrapAround); }
    //! ImGuiSliderFlags_ClampOnInput
    constexpr auto ClampOnInput() const -> SliderFlags { return set(ImGuiSliderFlags_ClampOnInput); }
    //! ImGuiSliderFlags_ClampZeroRange
    constexpr auto ClampZeroRange() const -> SliderFlags { return set(ImGuiSliderFlags_ClampZeroRange); }
    //! ImGuiSliderFlags_NoSpeedTweaks
    constexpr auto NoSpeedTweaks() const -> SliderFlags { return set(ImGuiSliderFlags_NoSpeedTweaks); }
    //! ImGuiSliderFlags_ColorMarkers
    constexpr auto ColorMarkers() const -> SliderFlags { return set(ImGuiSliderFlags_ColorMarkers); }
    //! ImGuiSliderFlags_AlwaysClamp
    constexpr auto AlwaysClamp() const -> SliderFlags { return set(ImGuiSliderFlags_AlwaysClamp); }
    //! ImGuiSliderFlags_InvalidMask_
    constexpr auto InvalidMask_() const -> SliderFlags { return set(ImGuiSliderFlags_InvalidMask_); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiTableFlags ...
/////////////////////////////////////////////////

class TableFlags : public FlagBuilder<TableFlags, ImGuiTableFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiTableFlags_None
    constexpr auto None() const -> TableFlags { return set(ImGuiTableFlags_None); }
    //! ImGuiTableFlags_Resizable
    constexpr auto Resizable() const -> TableFlags { return set(ImGuiTableFlags_Resizable); }
    //! ImGuiTableFlags_Reorderable
    constexpr auto Reorderable() const -> TableFlags { return set(ImGuiTableFlags_Reorderable); }
    //! ImGuiTableFlags_Hideable
    constexpr auto Hideable() const -> TableFlags { return set(ImGuiTableFlags_Hideable); }
    //! ImGuiTableFlags_Sortable
    constexpr auto Sortable() const -> TableFlags { return set(ImGuiTableFlags_Sortable); }
    //! ImGuiTableFlags_NoSavedSettings
    constexpr auto NoSavedSettings() const -> TableFlags { return set(ImGuiTableFlags_NoSavedSettings); }
    //! ImGuiTableFlags_ContextMenuInBody
    constexpr auto ContextMenuInBody() const -> TableFlags { return set(ImGuiTableFlags_ContextMenuInBody); }
    //! ImGuiTableFlags_RowBg
    constexpr auto RowBg() const -> TableFlags { return set(ImGuiTableFlags_RowBg); }
    //! ImGuiTableFlags_BordersInnerH
    constexpr auto BordersInnerH() const -> TableFlags { return set(ImGuiTableFlags_BordersInnerH); }
    //! ImGuiTableFlags_BordersOuterH
    constexpr auto BordersOuterH() const -> TableFlags { return set(ImGuiTableFlags_BordersOuterH); }
    //! ImGuiTableFlags_BordersInnerV
    constexpr auto BordersInnerV() const -> TableFlags { return set(ImGuiTableFlags_BordersInnerV); }
    //! ImGuiTableFlags_BordersOuterV
    constexpr auto BordersOuterV() const -> TableFlags { return set(ImGuiTableFlags_BordersOuterV); }
    //! ImGuiTableFlags_BordersH
    constexpr auto BordersH() const -> TableFlags { return set(ImGuiTableFlags_BordersH); }
    //! ImGuiTableFlags_BordersV
    constexpr auto BordersV() const -> TableFlags { return set(ImGuiTableFlags_BordersV); }
    //! ImGuiTableFlags_BordersInner
    constexpr auto BordersInner() const -> TableFlags { return set(ImGuiTableFlags_BordersInner); }
    //! ImGuiTableFlags_BordersOuter
    constexpr auto BordersOuter() const -> TableFlags { return set(ImGuiTableFlags_BordersOuter); }
    //! ImGuiTableFlags_Borders
    constexpr auto Borders() const -> TableFlags { return set(ImGuiTableFlags_Borders); }
    //! ImGuiTableFlags_NoBordersInBody
    constexpr auto NoBordersInBody() const -> TableFlags { return set(ImGuiTableFlags_NoBordersInBody); }
    //! ImGuiTableFlags_NoBordersInBodyUntilResize
    constexpr auto NoBordersInBodyUntilResize() const -> TableFlags { return set(ImGuiTableFlags_NoBordersInBodyUntilResize); }
    //! ImGuiTableFlags_SizingFixedFit
    constexpr auto SizingFixedFit() const -> TableFlags { return set(ImGuiTableFlags_SizingFixedFit); }
    //! ImGuiTableFlags_SizingFixedSame
    constexpr auto SizingFixedSame() const -> TableFlags { return set(ImGuiTableFlags_SizingFixedSame); }
    //! ImGuiTableFlags_SizingStretchProp
    constexpr auto SizingStretchProp() const -> TableFlags { return set(ImGuiTableFlags_SizingStretchProp); }
    //! ImGuiTableFlags_SizingStretchSame
    constexpr auto SizingStretchSame() const -> TableFlags { return set(ImGuiTableFlags_SizingStretchSame); }
    //! ImGuiTableFlags_NoHostExtendX
    constexpr auto NoHostExtendX() const -> TableFlags { return set(ImGuiTableFlags_NoHostExtendX); }
    //! ImGuiTableFlags_NoHostExtendY
    constexpr auto NoHostExtendY() const -> TableFlags { return set(ImGuiTableFlags_NoHostExtendY); }
    //! ImGuiTableFlags_NoKeepColumnsVisible
    constexpr auto NoKeepColumnsVisible() const -> TableFlags { return set(ImGuiTableFlags_NoKeepColumnsVisible); }
    //! ImGuiTableFlags_PreciseWidths
    constexpr auto PreciseWidths() const -> TableFlags { return set(ImGuiTableFlags_PreciseWidths); }
    //! ImGuiTableFlags_NoClip
    constexpr auto NoClip() const -> TableFlags { return set(ImGuiTableFlags_NoClip); }
    //! ImGuiTableFlags_PadOuterX
    constexpr auto PadOuterX() const -> TableFlags { return set(ImGuiTableFlags_PadOuterX); }
    //! ImGuiTableFlags_NoPadOuterX
    constexpr auto NoPadOuterX() const -> TableFlags { return set(ImGuiTableFlags_NoPadOuterX); }
    //! ImGuiTableFlags_NoPadInnerX
    constexpr auto NoPadInnerX() const -> TableFlags { return set(ImGuiTableFlags_NoPadInnerX); }
    //! ImGuiTableFlags_ScrollX
    constexpr auto ScrollX() const -> TableFlags { return set(ImGuiTableFlags_ScrollX); }
    //! ImGuiTableFlags_ScrollY
    constexpr auto ScrollY() const -> TableFlags { return set(ImGuiTableFlags_ScrollY); }
    //! ImGuiTableFlags_SortMulti
    constexpr auto SortMulti() const -> TableFlags { return set(ImGuiTableFlags_SortMulti); }
    //! ImGuiTableFlags_SortTristate
    constexpr auto SortTristate() const -> TableFlags { return set(ImGuiTableFlags_SortTristate); }
    //! ImGuiTableFlags_HighlightHoveredColumn
    constexpr auto HighlightHoveredColumn() const -> TableFlags { return set(ImGuiTableFlags_HighlightHoveredColumn); }
    //! ImGuiTableFlags_SizingMask_
    constexpr auto SizingMask_() const -> TableFlags { return set(ImGuiTableFlags_SizingMask_); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiTableColumnFlags ...
/////////////////////////////////////////////////

class TableColumnFlags : public FlagBuilder<TableColumnFlags, ImGuiTableColumnFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiTableColumnFlags_None
    constexpr auto None() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_None); }
    //! ImGuiTableColumnFlags_Disabled
    constexpr auto Disabled() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_Disabled); }
    //! ImGuiTableColumnFlags_DefaultHide
    constexpr auto DefaultHide() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_DefaultHide); }
    //! ImGuiTableColumnFlags_DefaultSort
    constexpr auto DefaultSort() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_DefaultSort); }
    //! ImGuiTableColumnFlags_WidthStretch
    constexpr auto WidthStretch() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_WidthStretch); }
    //! ImGuiTableColumnFlags_WidthFixed
    constexpr auto WidthFixed() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_WidthFixed); }
    //! ImGuiTableColumnFlags_NoResize
    constexpr auto NoResize() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_NoResize); }
    //! ImGuiTableColumnFlags_NoReorder
    constexpr auto NoReorder() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_NoReorder); }
    //! ImGuiTableColumnFlags_NoHide
    constexpr auto NoHide() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_NoHide); }
    //! ImGuiTableColumnFlags_NoClip
    constexpr auto NoClip() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_NoClip); }
    //! ImGuiTableColumnFlags_NoSort
    constexpr auto NoSort() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_NoSort); }
    //! ImGuiTableColumnFlags_NoSortAscending
    constexpr auto NoSortAscending() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_NoSortAscending); }
    //! ImGuiTableColumnFlags_NoSortDescending
    constexpr auto NoSortDescending() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_NoSortDescending); }
    //! ImGuiTableColumnFlags_NoHeaderLabel
    constexpr auto NoHeaderLabel() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_NoHeaderLabel); }
    //! ImGuiTableColumnFlags_NoHeaderWidth
    constexpr auto NoHeaderWidth() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_NoHeaderWidth); }
    //! ImGuiTableColumnFlags_PreferSortAscending
    constexpr auto PreferSortAscending() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_PreferSortAscending); }
    //! ImGuiTableColumnFlags_PreferSortDescending
    constexpr auto PreferSortDescending() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_PreferSortDescending); }
    //! ImGuiTableColumnFlags_IndentEnable
    constexpr auto IndentEnable() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_IndentEnable); }
    //! ImGuiTableColumnFlags_IndentDisable
    constexpr auto IndentDisable() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_IndentDisable); }
    //! ImGuiTableColumnFlags_AngledHeader
    constexpr auto AngledHeader() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_AngledHeader); }
    //! ImGuiTableColumnFlags_IsEnabled
    constexpr auto IsEnabled() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_IsEnabled); }
    //! ImGuiTableColumnFlags_IsVisible
    constexpr auto IsVisible() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_IsVisible); }
    //! ImGuiTableColumnFlags_IsSorted
    constexpr auto IsSorted() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_IsSorted); }
    //! ImGuiTableColumnFlags_IsHovered
    constexpr auto IsHovered() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_IsHovered); }
    //! ImGuiTableColumnFlags_WidthMask_
    constexpr auto WidthMask_() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_WidthMask_); }
    //! ImGuiTableColumnFlags_IndentMask_
    constexpr auto IndentMask_() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_IndentMask_); }
    //! ImGuiTableColumnFlags_StatusMask_
    constexpr auto StatusMask_() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_StatusMask_); }
    //! ImGuiTableColumnFlags_NoDirectResize_
    constexpr auto NoDirectResize_() const -> TableColumnFlags { return set(ImGuiTableColumnFlags_NoDirectResize_); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiTableRowFlags ...
/////////////////////////////////////////////////

class TableRowFlags : public FlagBuilder<TableRowFlags, ImGuiTableRowFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiTableRowFlags_None
    constexpr auto None() const -> TableRowFlags { return set(ImGuiTableRowFlags_None); }
    //! ImGuiTableRowFlags_Headers
    constexpr auto Headers() const -> TableRowFlags { return set(ImGuiTableRowFlags_Headers); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiListClipperFlags ...
/////////////////////////////////////////////////

class ListClipperFlags : public FlagBuilder<ListClipperFlags, ImGuiListClipperFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiListClipperFlags_None
    constexpr auto None() const -> ListClipperFlags { return set(ImGuiListClipperFlags_None); }
    //! ImGuiListClipperFlags_NoSetTableRowCounters
    constexpr auto NoSetTableRowCounters() const -> ListClipperFlags { return set(ImGuiListClipperFlags_NoSetTableRowCounters); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiMultiSelectFlags ...
/////////////////////////////////////////////////

class MultiSelectFlags : public FlagBuilder<MultiSelectFlags, ImGuiMultiSelectFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiMultiSelectFlags_None
    constexpr auto None() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_None); }
    //! ImGuiMultiSelectFlags_SingleSelect
    constexpr auto SingleSelect() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_SingleSelect); }
    //! ImGuiMultiSelectFlags_NoSelectAll
    constexpr auto NoSelectAll() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_NoSelectAll); }
    //! ImGuiMultiSelectFlags_NoRangeSelect
    constexpr auto NoRangeSelect() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_NoRangeSelect); }
    //! ImGuiMultiSelectFlags_NoAutoSelect
    constexpr auto NoAutoSelect() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_NoAutoSelect); }
    //! ImGuiMultiSelectFlags_NoAutoClear
    constexpr auto NoAutoClear() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_NoAutoClear); }
    //! ImGuiMultiSelectFlags_NoAutoClearOnReselect
    constexpr auto NoAutoClearOnReselect() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_NoAutoClearOnReselect); }
    //! ImGuiMultiSelectFlags_BoxSelect1d
    constexpr auto BoxSelect1d() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_BoxSelect1d); }
    //! ImGuiMultiSelectFlags_BoxSelect2d
    constexpr auto BoxSelect2d() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_BoxSelect2d); }
    //! ImGuiMultiSelectFlags_BoxSelectNoScroll
    constexpr auto BoxSelectNoScroll() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_BoxSelectNoScroll); }
    //! ImGuiMultiSelectFlags_ClearOnEscape
    constexpr auto ClearOnEscape() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_ClearOnEscape); }
    //! ImGuiMultiSelectFlags_ClearOnClickVoid
    constexpr auto ClearOnClickVoid() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_ClearOnClickVoid); }
    //! ImGuiMultiSelectFlags_ScopeWindow
    constexpr auto ScopeWindow() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_ScopeWindow); }
    //! ImGuiMultiSelectFlags_ScopeRect
    constexpr auto ScopeRect() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_ScopeRect); }
    //! ImGuiMultiSelectFlags_SelectOnClick
    constexpr auto SelectOnClick() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_SelectOnClick); }
    //! ImGuiMultiSelectFlags_SelectOnClickRelease
    constexpr auto SelectOnClickRelease() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_SelectOnClickRelease); }
    //! ImGuiMultiSelectFlags_NavWrapX
    constexpr auto NavWrapX() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_NavWrapX); }
    //! ImGuiMultiSelectFlags_NoSelectOnRightClick
    constexpr auto NoSelectOnRightClick() const -> MultiSelectFlags { return set(ImGuiMultiSelectFlags_NoSelectOnRightClick); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImDrawFlags ...
/////////////////////////////////////////////////

class DrawFlags : public FlagBuilder<DrawFlags, ImDrawFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImDrawFlags_None
    constexpr auto None() const -> DrawFlags { return set(ImDrawFlags_None); }
    //! ImDrawFlags_Closed
    constexpr auto Closed() const -> DrawFlags { return set(ImDrawFlags_Closed); }
    //! ImDrawFlags_RoundCornersTopLeft
    constexpr auto RoundCornersTopLeft() const -> DrawFlags { return set(ImDrawFlags_RoundCornersTopLeft); }
    //! ImDrawFlags_RoundCornersTopRight
    constexpr auto RoundCornersTopRight() const -> DrawFlags { return set(ImDrawFlags_RoundCornersTopRight); }
    //! ImDrawFlags_RoundCornersBottomLeft
    constexpr auto RoundCornersBottomLeft() const -> DrawFlags { return set(ImDrawFlags_RoundCornersBottomLeft); }
    //! ImDrawFlags_RoundCornersBottomRight
    constexpr auto RoundCornersBottomRight() const -> DrawFlags { return set(ImDrawFlags_RoundCornersBottomRight); }
    //! ImDrawFlags_RoundCornersNone
    constexpr auto RoundCornersNone() const -> DrawFlags { return set(ImDrawFlags_RoundCornersNone); }
    //! ImDrawFlags_RoundCornersTop
    constexpr auto RoundCornersTop() const -> DrawFlags { return set(ImDrawFlags_RoundCornersTop); }
    //! ImDrawFlags_RoundCornersBottom
    constexpr auto RoundCornersBottom() const -> DrawFlags { return set(ImDrawFlags_RoundCornersBottom); }
    //! ImDrawFlags_RoundCornersLeft
    constexpr auto RoundCornersLeft() const -> DrawFlags { return set(ImDrawFlags_RoundCornersLeft); }
    //! ImDrawFlags_RoundCornersRight
    constexpr auto RoundCornersRight() const -> DrawFlags { return set(ImDrawFlags_RoundCornersRight); }
    //! ImDrawFlags_RoundCornersAll
    constexpr auto RoundCornersAll() const -> DrawFlags { return set(ImDrawFlags_RoundCornersAll); }
    //! ImDrawFlags_RoundCornersDefault_
    constexpr auto RoundCornersDefault_() const -> DrawFlags { return set(ImDrawFlags_RoundCornersDefault_); }
    //! ImDrawFlags_RoundCornersMask_
    constexpr auto RoundCornersMask_() const -> DrawFlags { return set(ImDrawFlags_RoundCornersMask_); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImDrawListFlags ...
/////////////////////////////////////////////////

class DrawListFlags : public FlagBuilder<DrawListFlags, ImDrawListFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImDrawListFlags_None
    constexpr auto None() const -> DrawListFlags { return set(ImDrawListFlags_None); }
    //! ImDrawListFlags_AntiAliasedLines
    constexpr auto AntiAliasedLines() const -> DrawListFlags { return set(ImDrawListFlags_AntiAliasedLines); }
    //! ImDrawListFlags_AntiAliasedLinesUseTex
    constexpr auto AntiAliasedLinesUseTex() const -> DrawListFlags { return set(ImDrawListFlags_AntiAliasedLinesUseTex); }
    //! ImDrawListFlags_AntiAliasedFill
    constexpr auto AntiAliasedFill() const -> DrawListFlags { return set(ImDrawListFlags_AntiAliasedFill); }
    //! ImDrawListFlags_AllowVtxOffset
    constexpr auto AllowVtxOffset() const -> DrawListFlags { return set(ImDrawListFlags_AllowVtxOffset); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImFontAtlasFlags ...
/////////////////////////////////////////////////

class FontAtlasFlags : public FlagBuilder<FontAtlasFlags, ImFontAtlasFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImFontAtlasFlags_None
    constexpr auto None() const -> FontAtlasFlags { return set(ImFontAtlasFlags_None); }
    //! ImFontAtlasFlags_NoPowerOfTwoHeight
    constexpr auto NoPowerOfTwoHeight() const -> FontAtlasFlags { return set(ImFontAtlasFlags_NoPowerOfTwoHeight); }
    //! ImFontAtlasFlags_NoMouseCursors
    constexpr auto NoMouseCursors() const -> FontAtlasFlags { return set(ImFontAtlasFlags_NoMouseCursors); }
    //! ImFontAtlasFlags_NoBakedLines
    constexpr auto NoBakedLines() const -> FontAtlasFlags { return set(ImFontAtlasFlags_NoBakedLines); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImFontFlags ...
/////////////////////////////////////////////////

class FontFlags : public FlagBuilder<FontFlags, ImFontFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImFontFlags_None
    constexpr auto None() const -> FontFlags { return set(ImFontFlags_None); }
    //! ImFontFlags_NoLoadError
    constexpr auto NoLoadError() const -> FontFlags { return set(ImFontFlags_NoLoadError); }
    //! ImFontFlags_NoLoadGlyphs
    constexpr auto NoLoadGlyphs() const -> FontFlags { return set(ImFontFlags_NoLoadGlyphs); }
    //! ImFontFlags_LockBakedSizes
    constexpr auto LockBakedSizes() const -> FontFlags { return set(ImFontFlags_LockBakedSizes); }

    // clang-format on
};

/////////////////////////////////////////////////
/// ... ImGuiViewportFlags ...
/////////////////////////////////////////////////

class ViewportFlags : public FlagBuilder<ViewportFlags, ImGuiViewportFlags>
{
    using FlagBuilder::FlagBuilder;

public:
    // clang-format off
    //! ImGuiViewportFlags_None
    constexpr auto None() const -> ViewportFlags { return set(ImGuiViewportFlags_None); }
    //! ImGuiViewportFlags_IsPlatformWindow
    constexpr auto IsPlatformWindow() const -> ViewportFlags { return set(ImGuiViewportFlags_IsPlatformWindow); }
    //! ImGuiViewportFlags_IsPlatformMonitor
    constexpr auto IsPlatformMonitor() const -> ViewportFlags { return set(ImGuiViewportFlags_IsPlatformMonitor); }
    //! ImGuiViewportFlags_OwnedByApp
    constexpr auto OwnedByApp() const -> ViewportFlags { return set(ImGuiViewportFlags_OwnedByApp); }

    // clang-format on
};

} // namespace ImGuiEx
