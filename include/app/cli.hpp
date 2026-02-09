#pragma once

#include "app/app_types.hpp"

namespace app {

// Parses argv and mutates cfg/toggles. Returns false if the program should exit (help/error).
bool parse_cli(int argc, char** argv, AppConfig& cfg, AppToggles& toggles);

} // namespace app
