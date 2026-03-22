# ADR-0001: ToolWindow controls translator lifecycle

**Status:** Accepted
**Date:** 2026-03-10
**Context files:**

- `SimpleIME/src/ui/panels/AppearancePanel.cpp` (constructor FIXME block, now removed)
- `SimpleIME/include/i18n/translator_manager.h`
- `SimpleIME/include/ui/ToolWindow.h`
- `SimpleIME/src/ImeWnd.cpp` (`ManageToolWindowOnDemand`)

---

## Background

The `Translate()` helper requires a live `Translator` instance.
The translator is expensive to load (disk I/O, TOML parse).
The game UI layer (`ImeWindow`) never needs any translated text — only the tool UI does.

The question was: **who owns the translator, and when should it be loaded and released?**

---

## Design exploration (preserved from the inline FIXME in `AppearancePanel.cpp`)

> Which UI needs to access `TranslatorHolder`?
> 1. `SettingsWindow` — all labels
> 2. `LanguageBar` — only the "Settings" term
>
> So should `TranslatorHolder::g_translator` bind its lifecycle to `LanguageBar`?
>
> `TranslatorHolder` looks like over-design — it is only used by us!
>
> **Possible solutions:**
> 1. Just get the globally singleton translator and update it.
     > — *Which class will update it?*
     >
- `AppearancePanel` provides the language-picker UI, so it should update when the user changes language. ✓
>    - ~~`ImeWnd`: load once on create, never change~~ — rejected, user can change language at runtime.
>    - Add a new class that holds both `LanguageBar` and `SettingsWindow`, equivalent to `ToolWindow`.
       > `ImeWindow` never needs any translated term, so this boundary is clean.
>
> **ToolWindow lifecycle:**
> ```
> ToolWindow open  → translator already loaded?
>                    false → load from settings.appearance.language
>                    true  → do nothing; AppearancePanel may reload on language change
>         close    → debounce, then release translator
> ```
> The debounce delay should be generous because IO is needed on reload,
> and the user may open/close the ToolWindow in quick succession.

---

## Options considered

### Option A — `TranslatorHolder` singleton with a guarded `UpdateHandle`

A single global `Translator` instance protected by an `atomic_bool`.
Only the first caller gets an `UpdateHandle`; everyone else reads freely.

**Rejected because:**

- Ownership is unclear — the `UpdateHandle` could outlive its user
- `atomic_bool` guard only prevents double-init, not use-after-free
- Still a global; can't be reset cleanly when the tool window closes

### Option B — `AppearancePanel` holds an optional handle

`AppearancePanel` owns a `std::optional<UpdateHandle>` and calls `LoadTranslation` directly.

**Rejected because:**

- `AppearancePanel` is only alive while `SettingsWindow` is open; `LanguageBar` needs
  a live translator even when settings are closed
- Introduces implicit coupling: `AppearancePanel` must be constructed before `LanguageBar`s
  first render

### Option C (chosen) — `ToolWindow` owns the translator lifecycle

`ToolWindow` is created when the shortcut is first pressed and destroyed (via debounce)
when the user fully dismisses it. On construction it loads the translator; on destruction
it does nothing (release happens in `ImeWnd` after the debounce fires).

```
ImeWnd::Draw()
  └─ ManageToolWindowOnDemand(m_toolWindow, m_debounceTimer, settings)
       open path  → m_toolWindow = make_unique<ToolWindow>(shortcut) → (ctor)i18n::UpdateTranslator(language, "english")
       close path ← ToolWindow::Draw() returns false (m_showing == false)
                    ImeWnd pokes debounce timer
       after 10s  → m_toolWindow.reset() → (dtor) i18n::ReleaseTranslator()
                                         → sends kHide to SkSE menu
```

`AppearancePanel::DrawLanguagesCombo` calls `i18n::UpdateTranslator(language, "english")`
directly to hot-reload when the user picks a different language — no handle needed.

---

## Decision

**Implement Option C.**  Concretely:

1. Delete `TranslatorHolder` (`include/i18n/TranslatorHolder.h`, `src/i18n/ImeOverlay.cpp`)
2. Replace with `translator_manager.h/cpp`:
    - `GetTranslator() -> unique_ptr<Translator> &`
    - `UpdateTranslator(language, fallback)` (loads from disk)
    - `ReleaseTranslator()`
3. Add `ToolWindow` class (wraps `LanguageBar` + lazy `SettingsWindow`).
    - Constructor → `kShow` Skyrim menu message
    - Destructor → `kHide` Skyrim menu message
    - Owns `m_pinned`, `m_showing`, `m_shortcut` state (moved out of `LanguageBar`)
4. `ImeWnd` holds `unique_ptr<ToolWindow>` + `DebounceTimer`.
   `ManageToolWindowOnDemand` manages the create/destroy/debounce loop.
5. Debounce delay: **10 seconds** (`TRANSLATOR_DEBONCE_DELAY_SECONDS`).

---

## Consequences

**Positive:**

- Translator lifetime is bounded: loaded only while the tool UI is visible.
- `LanguageBar` becomes stateless — easier to test.
- `ImeWindow` has zero dependency on i18n.
- `TranslatorHolder`'s over-engineered `atomic_bool` guard is gone.

**Negative / watch out:**

- `ToolWindow` constructor does I/O (translator load) on the render thread — acceptable
  because it is guarded by user intent (shortcut press), not called every frame.
- The debounce timer fires on the render thread; if the user re-opens within the window,
  the `debounceTimer.IsWaiting()` guard skips the reset, but the `toolWindow` pointer
  will have been reset. A new `ToolWindow` will be created, which re-loads the translator.
  This is correct but wastes one IO cycle — acceptable.
