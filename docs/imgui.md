## Table Header Font Size
1. Need custom draw Table header
2. Must call `PushFont` after call `NextColumn` because `skipItems` be set true in `TableNextRow`;

```c++
ImGui::TableNextRow(ImGuiTableRowFlags_Headers);  
for (int colIndex = 0; colIndex < 6; ++colIndex)  
{  
    ImGui::TableSetColumnIndex(colIndex);  
    ImGuiScope::FontSize fontSize(Setting::UiSetting::FONT_SIZE_TITLE_3);  
    ImGui::TableHeader(ImGui::TableGetColumnName(colIndex));  
}
```
