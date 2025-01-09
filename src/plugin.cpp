
#include <stddef.h>

#include "Context.h"
#include "Transmogrify.h"
#include "Hooks.h"

static void InitializeLogging()
{
    auto path = SKSE::log::log_directory();
    if (!path) {
        SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
    }
    *path /= SKSE::PluginDeclaration::GetSingleton()->GetName();
    *path += L".log";

    std::shared_ptr<spdlog::logger> log;

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
    log->set_level(spdlog::level::debug);
    log->flush_on(spdlog::level::trace);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    InitializeLogging();

    auto* plugin = SKSE::PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    SKSE::log::info("{} {} is loading...", plugin->GetName(), "version");

    SKSE::Init(skse);

    // This example prints "Hello, world!" to the Skyrim ~ console.
    // To view it, open the ~ console from the Skyrim Main Menu.
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kInputLoaded) {
            // create our input device
            auto* devices = Context::Singleton().GetDevices();
            devices->RegisterInputEvent();
        }
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            Transmogrify::CombineEventSinks::Install();
        }
    });

    LOG(debug, "Initializing Context.");
    auto& context = Context::Singleton();
    context.Init();
    Hooks::Install();

    SKSE::log::info("{} has finished loading.", plugin->GetName());
    return true;
}