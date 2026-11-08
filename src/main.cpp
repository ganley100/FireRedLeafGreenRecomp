// main.cpp — FRLG multi-variant entry point (FireRed / LeafGreen / Radical Red).
//
// One source file backs every variant; the build picks the game via
// compile-defs set in CMakeLists.txt (add_gba_variant):
//
//   GBARECOMP_BUILTIN_NAME      e.g. "Pokemon FireRed (USA)"
//   GBARECOMP_BUILTIN_SHA1      expected ROM sha1 (hash gate)
//   GBARECOMP_DEFAULT_GAME_CONFIG  variants/<name>/game.toml
//   GBARECOMP_DEFAULT_DEBUG_PORT / GBARECOMP_WINDOW_TITLE  (read by runtime)
//
// Every gbarecomp game binary takes BOTH a BIOS and a ROM at launch
// (see ../gbarecomp/PRINCIPLES.md "BIOS is sacred"). The CLI accepts:
//
//   <Variant>Recomp [--bios <path>] [--rom <path>] [game.toml]
//
// All three are optional on the command line; missing values are pulled
// from game.toml. Hashes are verified before any code runs.

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "runtime.h"
#include "mobile_platform.h"
#include "android_device.h"
#if !defined(GEN3_NO_ADAPTIVE_VIEW)
#include "field_view.h"
#endif

#ifndef GBARECOMP_BUILTIN_NAME
#define GBARECOMP_BUILTIN_NAME "GBA cartridge"
#endif
#ifndef GBARECOMP_BUILTIN_SHA1
#define GBARECOMP_BUILTIN_SHA1 ""
#endif
#ifndef GBARECOMP_WINDOW_TITLE
#define GBARECOMP_WINDOW_TITLE "gbarecomp"
#endif
#ifndef GBARECOMP_BUILTIN_CRC32
#define GBARECOMP_BUILTIN_CRC32 0
#endif
#ifndef GBARECOMP_BUILTIN_REGION
#define GBARECOMP_BUILTIN_REGION ""
#endif
#ifndef GBARECOMP_MOD_GAME_ID
#define GBARECOMP_MOD_GAME_ID ""
#endif
#ifndef GBARECOMP_BOXART
#define GBARECOMP_BOXART ""
#endif

#if defined(GBAGAME_RECOMP_UI)
#include "game_launcher_boot.h"
#endif

namespace {

void print_usage() {
    std::printf(
        "%s [--bios <path>] [--rom <path>] [game.toml]\n"
        "\n"
        "Both BIOS and ROM are required (either via flags or via the\n"
        "[bios] / [rom] sections of game.toml). The runtime refuses\n"
        "to start unless both hash-verify.\n"
        "\n"
        "Default BIOS path: ../gbarecomp/bios/gba_bios.bin\n"
        "Default game config: " GBARECOMP_DEFAULT_GAME_CONFIG " (relative to CWD)\n",
        GBARECOMP_WINDOW_TITLE);
}

}  // namespace

int frlg_main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 ||
            std::strcmp(argv[i], "-h") == 0) {
            print_usage();
            return 0;
        }
    }

    // Android / Google TV: private-storage layout, log file, staged
    // game.toml, no pre-boot launcher. No-op on desktop.
    std::vector<std::string> args(argv, argv + argc);
    gbarecomp::MobileProcessOptions mobile;
    mobile.game_config = GBARECOMP_DEFAULT_GAME_CONFIG;
    mobile.program_name = "./" GBARECOMP_WINDOW_TITLE;
    const bool on_mobile = gbarecomp::mobile_prepare_process(args, mobile);

    // Built-in defaults so a standalone <Variant>Recomp.exe ships without
    // a sibling game.toml. The asset picker still validates against these
    // values; CLI / TOML can override.
    gbarecomp::RunOptions opts;
    opts.builtin_game_name = GBARECOMP_BUILTIN_NAME;
    opts.builtin_rom_sha1  = (sizeof(GBARECOMP_BUILTIN_SHA1) > 1)
                                 ? GBARECOMP_BUILTIN_SHA1
                                 : nullptr;
    // CRC32 of the pinned ROM (same dump the SHA-1 gates on); the
    // launcher's GAME card uses it for its "ROM verified" check.
    opts.builtin_rom_crc32 = GBARECOMP_BUILTIN_CRC32;
#if !defined(GEN3_NO_ADAPTIVE_VIEW)
    // Hacks (Radical Red) build without the adaptive view: its profile
    // addresses are reviewed only for the unmodified FRLG ROMs.
    gen3::configure_view(opts, gen3::game_profile());
#endif
    opts.mod_game_id = GBARECOMP_MOD_GAME_ID;
    opts.launcher_region   = (sizeof(GBARECOMP_BUILTIN_REGION) > 1)
                                 ? GBARECOMP_BUILTIN_REGION
                                 : nullptr;
    opts.launcher_boxart = (sizeof(GBARECOMP_BOXART) > 1)
                               ? GBARECOMP_BOXART
                               : nullptr;
    opts.launcher_game_config = GBARECOMP_DEFAULT_GAME_CONFIG;  // prefill ROM/BIOS

    if (on_mobile) {
        // GBA games are landscape; resume where the OS killed a
        // backgrounded session.
        opts.orientation = gbarecomp::RunOptions::Orientation::Landscape;
        opts.resume_suspend_state_on_launch = true;
        if (frlg::android_is_tv_device()) {
            // Google TV: driven by a remote or gamepad. No on-screen touch
            // pad over the picture; menus keep the TV/desktop layout.
            opts.touch_pad_default = 0;
            opts.ui_touch_friendly = false;
        } else {
            opts.ui_touch_friendly = true;   // phones/tablets: touch pad on
        }
    }

#if defined(GBAGAME_RECOMP_UI)
    if (game_launcher_preboot(args, opts)) return 0;   // user quit the launcher
#endif
    std::vector<char*> av;
    av.reserve(args.size());
    for (auto& s : args) av.push_back(s.data());
    return gbarecomp::run_game(static_cast<int>(av.size()), av.data(), opts);
}

#if defined(__ANDROID__)
extern "C" int SDL_main(int argc, char** argv) {
    // Desktop-sized host stack for the recompiled corpus (mobile_platform.h).
    return gbarecomp::mobile_run_with_stack(frlg_main, argc, argv);
}
#else
int main(int argc, char** argv) {
    return frlg_main(argc, argv);
}
#endif
