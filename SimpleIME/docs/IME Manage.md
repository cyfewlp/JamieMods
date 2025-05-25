For avoid the inconsistent `textEntryCount`: the `CursorMenu` hide but text entry count not equal zero.
	* When `CursorMenu` is hide and text entry count > 0, try disable IME
	* When `CursorMenu` is show and text entry count > 0, try enable IME

### `Console Native UI Menu`

When Open/Close `Steam Overlay` will open/close the "Console Native UI Menu". 
So, we can according to this menu event to disable IME when `Steam Overlay` is showing;
