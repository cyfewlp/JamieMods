## What?

A SKSE plugin that make SkyrimSE/AE supports IME(Input Method Editor). 
Mainly for Chinese players. Use ImGui + Win32 + IMM/TSF to do this.

In fact, this mod just unlock os IME and recieve, forward their events, message to game.
Therefore, this mod alos should support other language(Japanese, Korean, etc.). But, I'm 
not clear whether IME works with the same logic in other languages, so the composition and
candidate windows of other languages ​​may not work properly.

SKSE插件，使 SkyrimSE/AE支持输入法(IME)，主要针对中国用户，使用 ImGui + Win32 + IMM/TSF 实现。
实际上mod只是解锁了系统的 IME，并将其事件，消息转发到游戏中，因此，应该支持韩语，日语等输入法，但是我
并不清楚其他语言的输入法如何工作，因此其他语言的 composition 和 candidate 窗口可能不会正常工作.

## Why?

IME for SkyrimSE game is unnecessary. But, if you installed SkyUI or other mods based
on SkyUi/UIExtension, there are may be need to support Unicode character.

In one words, by ime you can:

Search armor/spell/item/npc with unicode character in SkyUI(or other mods based on it).

基础游戏并不需要输入法，但是如果安装了 SKyUI 或者其他任何基于游戏UI 的 mod，可能会需要用来进行搜索。
简单来说：
使用此mod可以使得在 SkyUI以及任何基于游戏UI的mod支持unicode 字符输入。

## Note

This mod must be create and set non-exclusive mode keyboard after Game's keyboard device.
So, if you launch game and switch to other windows, the default behavior may be fail. 
If you can't input. Please pressed F2 key(default) check IME state or fix: **Reset Keyboard**.

1. Use your os shortcut(in my case is win + space) keys to switch different IME.
2. Open toolwindow(defualt key `F2`) to check/reset mod state, switch to different IME.

此模组必须在游戏的键盘设备之后创建并设置非独占模式键盘。 因此，如果您启动游戏后切换到了其他窗口，默认行为可能会失败。
如果您无法输入。请按 F2 键（默认）检查 IME 状态或修复：**Reset Keyboard**。

1. 使用系统快捷键切换不同的输入法(win + 空格)
2. 打开 ToolWindow (默认为 F2) 用以查看，重置mod状态，也可以用于切换输入法.

## How to implement?

1. By default, game will lock the player keyboard input event. All other window can't
receive any keyboard input event;
2. Game window is a ANSI window. For some unicode character may can't receive(like emoji);

1. 默认情况下，游戏会锁定键盘，导致其他所有程序都无法读取键盘输入（也包括IME）；
2. 游戏窗口是 ANSI 窗口，无法接受部分 unicode 字符（emoji)

This mod works:

1. create a **invisible** child window override Game window;
   Because wo need a Unicode window.
2. Always focused the child window;
     Snap grab input focus and handle it in child window message loop; This logic will not effect(at least in my test case) game input event 
3. Hook `GetMsgProc` and convert `WM_IME_COMPOSITION` message to our custom message WM_CUSTOM_IME_COMPOSITION.
Because we need avoid to Skyrim default message loop to destroy `WM_IME_COMPOSITION` message;
4. Override os IME's composition & candidate window and use ImGui render it.
5. Create another keyboard device by non-exclusive mode to unlock IME function(must 
be after game keyboard device created). Also unlock `win` key.

1. 创建一个**不可见**子窗口覆盖游戏窗口；因为我们需要一个 Unicode 窗口。
2. 始终聚焦子窗口； 捕捉输入焦点并在子窗口消息循环中处理它；此逻辑不会影响（至少在我的测试用例中）游戏输入事件
3. Hook `GetMsgProc` 并将 `WM_IME_COMPOSITION` 消息转换为我们的自定义消息 `WM_CUSTOM_IME_COMPOSITION`。
因为需要避免 Skyrim 默认消息循环破坏 `WM_IME_COMPOSITION` 消息；
4. 覆盖操作系统 IME 的 composition 和 candidate 窗口并使用 ImGui 渲染它。
5. 通过非独占模式创建另一个键盘设备以解锁 IME 功能（必须在游戏键盘设备创建之后）。 windows 键也会解锁

## compatibility/Supports

1. Should no compatibility issue. only hooks some functions 
that usually also hooked by other UI mod.
2. Supports SkyUi & all other mods based on it(like mod `WhereAreYou`) ;
3. not support other custom UI mod(dmenu, Modex). Because these mod has their self
render & message handle logic, only if another mod support and load unicode font and 
handle `WM_CHAR` message.

1. 应该不存在兼容性问题。仅挂接一些通常也由其他 UI 模组挂接的函数。
2. 支持 SkyUI 和所有其他基于它的模组（如 `WhereAreYou` 模组）;
3. 不支持其他自定义 UI 模组（dmenu、Modex）。因为这些模块有自己的
渲染和消息处理逻辑，只有当另一个模块支持并加载 unicode 字体并
处理 `WM_CHAR` 消息时才支持。

## Other

I'm a C++ & mod newcomer. So this mod may have some I not found bugs.
If you meet a bug, report it, I will fix it quick soon.

By the way, the mod supports emoji input. However, the game font renderer is 
implemented by Scaleform GFX. In my Google results, 
GFX SDK does not support emoji font. I found a GTA modder article about this: 
[Our work on supporting emoji in Scaleform GFx](https://cookbook.fivem.net/2019/06/09/our-work-on-supporting-emoji-in-scaleform-gfx/)
But, it's too complex......🥲﻿

我是 C++ 和 mod 新手。因此这个 mod 可能有一些我没发现的错误。
如果您发现错误，请报告，我会尽快修复。