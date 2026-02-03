#pragma once

namespace Ime
{
class ImeWnd;

namespace Events
{
void InstallEventSinks(ImeWnd *imeWnd, uint32_t shortcutKey);
void UnInstallEventSinks();
} // namespace Events

} // namespace Ime
