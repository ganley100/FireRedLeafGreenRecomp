# Android & Google TV

One APK per game, all built from this repo and the shared gbarecomp Android
shell (`gbarecomp/platform/android`). Each APK installs on **Google TV /
Android TV, phones and tablets**; the applicationIds differ, so all three can
live side by side.

| Module        | APK / applicationId          | Notes |
|---------------|------------------------------|-------|
| `:firered`    | `com.frlgrecomp.firered`     | FireRed USA v1.0 |
| `:leafgreen`  | `com.frlgrecomp.leafgreen`   | LeafGreen USA v1.0 |
| `:radicalred` | `com.frlgrecomp.radicalred`  | opt-in: `-PradicalRedSha1=<your patched ROM's sha1>` |
| `:tv`         | (library)                    | Google TV overlay merged into every game |

## What makes it work on Google TV

- **Home screen:** `:tv` adds a `LEANBACK_LAUNCHER` entry and a 320×180
  banner (`<game>/src/main/res/drawable/tv_banner.xml`), and declares
  leanback/touchscreen as *optional* so one APK serves TV and phone.
- **No touch overlay on TV:** `src/android_device.cpp` detects a leanback /
  touchless device at launch; the on-screen touch pad then stays off and the
  menus keep their TV layout.
- **Controls:**
  - **Gamepad (recommended):** any Bluetooth/USB controller works with no
    setup. A, B, Back→Select, Start, D-pad, LB/RB→L/R; Guide/Home-on-pad opens
    the settings menu.
  - **TV remote:** D-pad = D-pad, **OK = A**, **Play/Pause = B**,
    **Menu = Select**, Back = settings menu (`payload/common/keybinds.ini`).
    Many Google TV remotes have no Play/Pause or Menu key, and Start is only
    on a keyboard/gamepad, so a gamepad is needed for full play.

## Build

Requires the Android SDK (platform 35, NDK `27.1.12297006`, CMake 3.22.1) and
JDK 17+. Generate each game's C first (desktop README, step 3); the APK
compiles `variants/<game>/generated/`.

```
git submodule update --init --recursive        # engine, recomp-ui, SDL
cd android
./gradlew :firered:assembleRelease
./gradlew :leafgreen:assembleRelease
./gradlew :radicalred:assembleRelease -PradicalRedSha1=<sha1> [-PradicalRedRomSize=<bytes>]
```

APKs land in `android/<game>/build/outputs/apk/`. On Windows the engine's
`gbarecomp/platform/android/tools/build-apk.ps1 -GameAndroidDir android`
wrapper also works.

## Build in GitHub Actions

**Actions → Android APKs → Run workflow** (pick a game and CPU ABIs). The
recompiler needs your ROM and BIOS to generate the code, so add these
repository secrets first (Settings → Secrets and variables → Actions):
`GBA_BIOS_URL`, `FIRERED_ROM_URL`, `LEAFGREEN_ROM_URL`, `RADICALRED_ROM_URL`
(direct-download https links to your own files) and `RADICALRED_ROM_SHA1`.
Files are SHA-1 checked, never committed, and the uploaded APKs are checked
to contain no ROM/BIOS. Optional `ANDROID_KEYSTORE_BASE64` (+ password /
alias secrets) keeps one signing key so updates install over old versions.
Details at the top of `.github/workflows/android.yml`. Locally,
`tools/recompile_variant.sh <game> <rom>` does the same regeneration.

## Getting the ROM & BIOS onto a TV

The APKs contain **no ROM and no BIOS**; you import your own after installing.

**With a file manager on the TV (recommended):**
1. Copy your ROM (e.g. `firered_usa.gba`) and `gba_bios.bin` onto the TV
   (USB stick, network share, or your file manager's transfer feature).
2. In the file manager, select the file → **Open with** (or **Share**) →
   pick the game (e.g. *FireRedRecomp*). Some file managers let you select
   both files and share them together.
3. The app checks the size and SHA-1 and stores the file privately. Once both
   are in, choose **PLAY**. From then on the game opens straight from the
   home-screen tile.

Only the exact supported dumps are accepted (same check as the setup
screen); anything else is rejected with its size and SHA-1 shown. The setup
screen's own CHOOSE buttons use Android's system picker, which many Google TV
devices lack — use Open with / Share there instead.

**Other options:** `adb` into a debug build:
```
adb push firered_usa.gba gba_bios.bin /data/local/tmp/
adb shell "run-as com.frlgrecomp.firered sh -c 'mkdir -p files/roms files/bios && cp /data/local/tmp/firered_usa.gba files/roms/ && cp /data/local/tmp/gba_bios.bin files/bios/'"
```
or a private personal build with `-PprivateRom=... -PprivateBios=...`
(never share that APK).

Sideload the APK onto Google TV with `adb install` or your file manager.
