## TODO

- [x] [[improvement]] Add a `side bar`(like IDEA, VS code) to show/hide [[OutfitListView]],

1. Add new window to draw sidebar;
2. Adjust `DockSpace` window horizonal position offset

```c++
if (ImGui::Begin(
        "##MainSidebar", nullptr, ImGuiUtil::WindowFlags().AlwaysAutoResize().NoDecoration().NoDocking().NoMove()
    ))
{
    width = ImGui::GetWindowWidth();
}
ImGui::End();
return width;

void SosGui::DockSpace()
{
    float sideBarWidth = DrawSidebar();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({sideBarWidth, viewport->WorkPos.y});
}
```
