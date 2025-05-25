*  Refactor class `ImeManagerComposer`:
	* remove all notfiy function: replace by `TaskQueue`. `ImeManagerComposer` will send task to `IME` thread or main thread according to the focus type;
	* refactor the almost functions return value handle;
* Set `ErrorNotifier` message level;
* [x] Add a parameter to `SyncImeState` to incidate whetehr the IME should be enabled;
	* because we need handle the `keepImeOpen` flag;
* [x] Refactor `ImeUIWidgets#m_uiUint32Vars` or remove;
* [x] Translate: Add embed english translation;
* [x] Ignore judgement the `::SetFocus` call whether failed according to the return value . Because we no need the *last focused HWND* and the `NULL` return value **not means** `::SetFocus` failed!  We can ensure `::SetFocus` call in the HWND's message thread(by custom message `CM_EXECUTE_TASK` );
* [ ] ~~use boost:parser?~~
* [x] mark settings dirty if any field changed;
* [x] Optimize `Settings` dependencies;
* [ ] ~~Optimize std::string fields ?~~
* [x] setup `ErrorNotifier` message level
* [x] Auto hide `ErrorNotifier`;
* We may unnecessary to check the `SetFocus` caller thread because IME is child window with 
	game window. [# Is it legal to have a cross-process parent/child or owner/owned window relationship?](https://devblogs.microsoft.com/oldnewthing/20130412-00/?p=4683&utm_source=chatgpt.com); According to this post introduce the child/parent window will be implicitly associate message queue.