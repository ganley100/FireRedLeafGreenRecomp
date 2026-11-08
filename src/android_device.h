// android_device.h — host-device facts the FRLG Android shell needs before
// the runtime starts (game-side; no engine changes).
#pragma once

namespace frlg {

// True on Google TV / Android TV (leanback) or any device with no
// touchscreen. Such devices are driven by a remote or gamepad, so the
// on-screen touch pad stays off and the menus keep their TV layout.
// Always false off Android.
bool android_is_tv_device();

}  // namespace frlg
