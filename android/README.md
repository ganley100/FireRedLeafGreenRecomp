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

## Getting the ROM & BIOS onto a TV

The setup screen imports them with Android's file picker, but **many Google
TV devices ship without one**. Two alternatives:

1. **Private build** (personal use only — never share the APK): embed your own
   verified dumps; the build checks size + SHA-1.
   ```
   ./gradlew :firered:assembleRelease -PprivateRom=/path/firered_usa.gba -PprivateBios=/path/gba_bios.bin
   ```
2. **adb** (debug builds): enable developer options on the TV, then
   ```
   adb install firered/build/outputs/apk/debug/firered-debug.apk
   adb push firered_usa.gba gba_bios.bin /data/local/tmp/
   adb shell "run-as com.frlgrecomp.firered sh -c 'mkdir -p files/roms files/bios && cp /data/local/tmp/firered_usa.gba files/roms/ && cp /data/local/tmp/gba_bios.bin files/bios/'"
   ```

Sideload the APK onto Google TV with `adb install` or a sideloading app.
