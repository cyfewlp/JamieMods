#v2_0_0

## Support preview & choose installed font #type/feature 

- Support preview & choose installed font; #status/done 
- Support preview & choose font from a specified font path;
- store font name and index that in the `IDWriteFontSet`. Retrive font file path when user choosed a font in UI. #status/done 
- `IDWriteFactory3` : Minimum supported client: Windows 10
- Support choose different fonts for latin, CJK and emoji. #status/done
- Debounce optimization: #status/done 
	- Update the preview font 200ms after the user selects a font;
- Search/Filter font #status/done 
- Automatically merge the nerd font when the font appied to the default font; #status/done 
	- **Do not add your own icon fonts!** `SymbolsNerdFont` is already included via automatic merging. #userWarning
- Save the FontBuilder configuration #status/done 
- Check font if duplicate?
  - Get the unique font id from DWrite. #status/investigating 
- CLeanup when ToolWindow closed #status/todo 
## Following Google's Material 3 Design Specification

- Remove `fontSize` configruation from UI and config file:
	- Use the **fixed** `fontSize` + scale factor to control UI.