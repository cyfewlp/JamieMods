# ADR 0002: Fix GFxCharEvent Memory Leak

- **Status:** Accepted
- **Date:** 2026-03-11

## Context

`GFxCharEvent` objects allocated with `new` via `Skyrim::SendUiString` are never freed.
`BSUIScaleformData`'s destructor does not `delete scaleformEvent`, so every character sent
to the game leaks one `GFxCharEvent` allocation from the GFx global heap.

This can be monitored via `RE::GMemory::GetGlobalHeap()` — heap usage grows continuously
and linearly with every character sent.

> **Observation:** Skyrim's built-in `GFxEvent` instances (e.g. the `MenuCursor` mouse event)
> appear to be long-lived global/static objects, which is why the engine never bothers to free
> the pointer stored in `BSUIScaleformData::scaleformEvent`.

## Considered Options

### Option 1 — Bracket the stream with custom sentinel messages

Send a custom "begin" message before the char-event stream and a "end" message after it.
`ImeMenu` would collect all `GFxCharEvent` pointers between the two sentinels and free them
once the "end" message is processed.

- **Rejected:** There is no guarantee that no other thread or third-party mod sends
  `GFxCharEvent` during the same window, which could cause a dangling-pointer free.

### Option 2 — Distinguish by custom event type and defer free to `PostDisplay` ✅ Chosen

SimpleIME already tags its char events with the custom type `kImeCharEvent`
(`GFxEventTypeEx::kImeCharEvent = 0x80000001`) to differentiate them from
vanilla `kCharEvent` events received from the game engine.

When `ImeMenu::ProcessMessage` receives a `kImeCharEvent`, the pointer is appended to
`m_imeCharEvents` (`std::vector<GFxCharEvent*>`).
In `ImeMenu::PostDisplay`, all collected pointers are freed via `RE::GMemory::Free()` and
the vector is cleared.

**Key prerequisite (verified via debugger):**
Within a given frame, `PostDisplay` is always called *after* `ProcessMessage`, and both
execute on the same game render thread. This makes the deferred-free pattern safe with zero
additional synchronization.

### Option 3 — Cache vector owned by ImeMenu, passed via final sentinel message

Allocate a `vector<GFxCharEvent>` before the stream, fill it, enqueue all events, then send
a final message carrying the vector pointer so `ImeMenu` can call `clear()`.

- **Rejected:** Ownership of the vector's memory is ambiguous across the
  `UIMessageQueue` boundary. A dropped final message (e.g. due to game shutdown mid-stream)
  would permanently leak the vector. Using GFx-allocated memory would sidestep the latter
  concern, but the ownership complexity remains.

### Option 4 — Bypass `UIMessageQueue`, call `ImeMenu::ProcessMessage` directly

Obtain an `ImeMenu` pointer and invoke `ProcessMessage` directly instead of routing through
`UIMessageQueue::AddMessage`.

- **Rejected:** `SendUiString` can be called from threads other than the game render thread.
  Direct `ProcessMessage` invocation would require an explicit synchronisation mechanism
  (e.g. posting a secondary message and recursing inside `ImeMenu`). Additionally, while
  `ImeMenu` has `depthPriority = 13` (the maximum permitted value), there is no guarantee
  it is always the *first* menu to process a given message — an unknown third-party mod could
  register at the same priority.

### Option 5 — Global singleton vector, free unconditionally in `PostDisplay`

Accumulate sent `GFxCharEvent` pointers in a process-global vector; `PostDisplay` drains and
frees it every frame regardless of message boundaries.

- **Rejected:** Simpler than Option 3, but still requires a mutex because `SendUiString` is
  called from TSF worker threads. Option 2 achieves the same result without any locking.

## Decision

**Option 2** is adopted.

`ImeMenu` collects `kImeCharEvent` pointers during `ProcessMessage` into `m_imeCharEvents`
and frees them at the end of `PostDisplay` using `RE::GMemory::Free()`.
No locks are needed because the entire lifecycle (enqueue → process → free) is bounded to
a single frame on the render thread.
