#include "Context.h"
#include "SimpleIni.h"

void
Context::Init()
{
    config = LoadSettings("Data\\SKSE\\Plugins\\SkyrimTransmogrify.ini");
    RenderManager::Init(config);
}

Config*
Context::LoadSettings(const char* path)
{
    CSimpleIniA ini;

    ini.SetUnicode();
    SI_Error rc = ini.LoadFile(path);
    if (rc < 0) { /* handle error */
        SKSE::stl::report_and_fail("Loading config failed.");
    };

    Config* pConfig = new Config();
    pConfig->eastAsiaFontFile = ini.GetValue("General", "EastAsia_Font_File", "C:\\Windows\\Fonts\\simsun.ttc");
    pConfig->emojiFontFile = ini.GetValue("General", "Emoji_Font_File");
    pConfig->fontSize = (float)(ini.GetDoubleValue("General", "Font_Size", 16.0));
    return pConfig;
}
