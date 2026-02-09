#include "app/cli.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace app {
namespace {

static void print_help(const char* exe) {
    std::printf("Usage: %s [options]\n", exe ? exe : "renderer");
    std::printf("\n");
    std::printf("Options:\n");
    std::printf("  --render-w N        Internal render width (default: 720)\n");
    std::printf("  --render-h N        Internal render height (default: 480)\n");
    std::printf("  --window-w N        Window width (default: 1280)\n");
    std::printf("  --window-h N        Window height (default: 720)\n");
    std::printf("  --no-fps            Disable FPS overlay\n");
    std::printf("  -h, --help          Show this help\n");
}

static bool parse_int(const char* s, int& out) {
    if (!s || !*s)
        return false;
    char* end = nullptr;
    long v = std::strtol(s, &end, 10);
    if (!end || *end != '\0')
        return false;
    if (v < 1 || v > 16384)
        return false;
    out = int(v);
    return true;
}

} // namespace

bool parse_cli(int argc, char** argv, AppConfig& cfg, AppToggles& toggles) {
    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];
        if (!a)
            continue;

        if (std::strcmp(a, "-h") == 0 || std::strcmp(a, "--help") == 0) {
            print_help(argv[0]);
            return false;
        }

        if (std::strcmp(a, "--no-fps") == 0) {
            toggles.show_fps = false;
            continue;
        }

        auto take_int = [&](int& dst) -> bool {
            if (i + 1 >= argc)
                return false;
            return parse_int(argv[++i], dst);
        };

        if (std::strcmp(a, "--render-w") == 0) {
            if (!take_int(cfg.render_w)) {
                std::fprintf(stderr, "Invalid --render-w\n");
                return false;
            }
            continue;
        }
        if (std::strcmp(a, "--render-h") == 0) {
            if (!take_int(cfg.render_h)) {
                std::fprintf(stderr, "Invalid --render-h\n");
                return false;
            }
            continue;
        }
        if (std::strcmp(a, "--window-w") == 0) {
            if (!take_int(cfg.window_w)) {
                std::fprintf(stderr, "Invalid --window-w\n");
                return false;
            }
            continue;
        }
        if (std::strcmp(a, "--window-h") == 0) {
            if (!take_int(cfg.window_h)) {
                std::fprintf(stderr, "Invalid --window-h\n");
                return false;
            }
            continue;
        }

        std::fprintf(stderr, "Unknown option: %s\n", a);
        print_help(argv[0]);
        return false;
    }

    return true;
}

} // namespace app
