//
// Created by jamie on 2025/5/19.
//

#ifndef UISETTINGLOADER_H
#define UISETTINGLOADER_H

#include "common/config.h"
#include "gui/UiSettings.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Settings
{
class UiSettingsLoader
{
public:
    static void Load(__out UiSettings &uiSetting);
    static void Save(__in UiSettings &uiSetting);
};
}
}

#endif // UISETTINGLOADER_H
