*  Refactor class `ImeManagerComposer`:
	* remove all notfiy function: replace by `TaskQueue`. `ImeManagerComposer` will send task to `IME` thread or main thread according to the focus type;
	* refactor the almost functions return value handle;
* Set `ErrorNotifier` message level;