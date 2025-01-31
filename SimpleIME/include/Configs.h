//
// Created by jamie on 25-1-22.
//

#ifndef HELLOWORLD_CONFIGS_H
#define HELLOWORLD_CONFIGS_H

namespace SimpleIME
{
    struct FontConfig
    {
        std::string eastAsiaFontFile;
        std::string emojiFontFile;
        float       fontSize              = 14.0f;
        uint32_t    toolWindowShortcutKey = 0x3C; // DIK_F2;
        bool        debug                 = false;
    };
}

#endif // HELLOWORLD_CONFIGS_H
