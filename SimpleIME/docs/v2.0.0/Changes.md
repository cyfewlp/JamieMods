##  Consume MouseEvent when IME is inputting #status/done #type/feature

* if IME is inputting and mouse click area is not ImeMenu:
	* consume event
	* abort IME
## Use game Cursor in our menus #status/todo #type/feature 

* Use game cursor:
	* avoid trigger `MenuOpenCloseEventSink::FixInconsistentTextEntryCount` to disable IME when open `ToolWindowMenu`;
## Support choose candidate by mouse wheel #status/todo

## Optimize obtain scaleform caret pos #status/todo

* Avoid IME UI render area exceed game window size; 
* Avoid IME UI overlap text entry;
* We haven't handled `scroll` yet, because multi-line text scenarios are limited; #status/investigating 
*  Old logic that translate point (0,0) to screen space will get incorrect result in console menu;
* ⭐Retrieve char boundaries and call `GFxMovieView#TranslateLocalToScreen`;

## Implement Unicode Paste in flash? #

**NO**. The method `pasteFromClipboard` in GFx TextField class won't work as expected.
And implement in SkyrimSE can support all UI that implemented from `IMenu` interface.

## Support preview & choose installed font

- Support preview & choose installed font; #status/done 
- Support preview & choose font from a specified font path;
- store font name and index that in the `IDWriteFontSet`. Retrive font file path when user choosed a font in UI. #status/done 
- `IDWriteFactory3` : Minimum supported client: Windows 10
- Support choose different fonts for latin, CJK and emoji. #status/doing
- Debounce optimization:
	- Update the preview font 200ms after the user selects a font;
## Update ImThemes #status/todo 

## Consume Scaleform event when `ToolWindowMenu` showing #status/done 

