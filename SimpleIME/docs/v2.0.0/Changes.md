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
