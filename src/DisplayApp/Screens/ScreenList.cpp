#include <libs/lvgl/lvgl.h>
#include <DisplayApp/DisplayApp.h>
#include "ScreenList.h"
#include "../../Version.h"

using namespace Pinetime::Applications::Screens;

// TODO this class must be improved to receive the list of "sub screens" via pointer or
// move operation.
// It should accept many type of "sub screen" (it only supports Label for now).
// The number of sub screen it supports must be dynamic.
ScreenList::ScreenList(Pinetime::Applications::DisplayApp *app,
        Pinetime::Controllers::DateTime &dateTimeController,
        Pinetime::Controllers::Battery& batteryController,
        Pinetime::Controllers::BrightnessController& brightnessController,
                       Pinetime::Controllers::Ble& bleController,
        Pinetime::Drivers::WatchdogView& watchdog) :
        Screen(app),
        dateTimeController{dateTimeController}, batteryController{batteryController},
        brightnessController{brightnessController}, bleController{bleController}, watchdog{watchdog} {
  screens.reserve(3);

  // TODO all of this is far too heavy (string processing). This should be improved.
  // TODO the info (battery, time,...) should be updated in the Refresh method.


  auto batteryPercent = static_cast<int16_t>(batteryController.PercentRemaining());
  if(batteryPercent > 100) batteryPercent = 100;
  else if(batteryPercent < 0) batteryPercent = 0;

  uint8_t brightness = 0;
  switch(brightnessController.Level()) {
    case Controllers::BrightnessController::Levels::Low: brightness = 1; break;
    case Controllers::BrightnessController::Levels::Medium: brightness = 2; break;
    case Controllers::BrightnessController::Levels::High: brightness = 3; break;
  }
  auto resetReason = [&watchdog]() {
    switch (watchdog.ResetReason()) {
      case Drivers::Watchdog::ResetReasons::Watchdog: return "wtdg";
      case Drivers::Watchdog::ResetReasons::HardReset: return "hardr";
      case Drivers::Watchdog::ResetReasons::NFC: return "nfc";
      case Drivers::Watchdog::ResetReasons::SoftReset: return "softr";
      case Drivers::Watchdog::ResetReasons::CpuLockup: return "cpulock";
      case Drivers::Watchdog::ResetReasons::SystemOff: return "off";
      case Drivers::Watchdog::ResetReasons::LpComp: return "lpcomp";
      case Drivers::Watchdog::ResetReasons::DebugInterface: return "dbg";
      case Drivers::Watchdog::ResetReasons::ResetPin: return "rst";
      default: return "?";
    }
  }();

  // uptime
  static constexpr uint32_t secondsInADay = 60*60*24;
  static constexpr uint32_t secondsInAnHour = 60*60;
  static constexpr uint32_t secondsInAMinute = 60;
  uint32_t uptimeSeconds = dateTimeController.Uptime().count();
  uint32_t uptimeDays = (uptimeSeconds / secondsInADay);
  uptimeSeconds = uptimeSeconds % secondsInADay;
  uint32_t uptimeHours = uptimeSeconds / secondsInAnHour;
  uptimeSeconds = uptimeSeconds % secondsInAnHour;
  uint32_t uptimeMinutes = uptimeSeconds / secondsInAMinute;
  uptimeSeconds = uptimeSeconds % secondsInAMinute;
  // TODO handle more than 100 days of uptime

  sprintf(t1, "Pinetime\n"
              "Version:%d.%d.%d\n"
              "Build: %s\n"
              "       %s\n"
              "Date: %02d/%02d/%04d\n"
              "Time: %02d:%02d:%02d\n"
              "Uptime: %02lud %02lu:%02lu:%02lu\n"
              "Battery: %d%%\n"
              "Backlight: %d/3\n"
              "Last reset: %s\n",
              Version::Major(), Version::Minor(), Version::Patch(),
              __DATE__, __TIME__,
              dateTimeController.Day(), dateTimeController.Month(), dateTimeController.Year(),
              dateTimeController.Hours(), dateTimeController.Minutes(), dateTimeController.Seconds(),
              uptimeDays, uptimeHours, uptimeMinutes, uptimeSeconds,
              batteryPercent, brightness, resetReason);

  screens.emplace_back(t1);

  auto& bleAddr = bleController.Address();
  sprintf(t2, "BLE MAC: \n  %2x:%2x:%2x:%2x:%2x:%2x",
          bleAddr[5], bleAddr[4], bleAddr[3], bleAddr[2], bleAddr[1], bleAddr[0]);
  screens.emplace_back(t2);

  strncpy(t3, "Hello from\nthe developper!", 27);

  screens.emplace_back(t3);

  auto &screen = screens[screenIndex];
  screen.Show();
}

ScreenList::~ScreenList() {
  lv_obj_clean(lv_scr_act());
}

bool ScreenList::Refresh() {
  auto &screen = screens[screenIndex];
  screen.Refresh();

  return running;
}

bool ScreenList::OnButtonPushed() {
  running = false;
  return true;
}

bool ScreenList::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeDown:
      if (screenIndex > 0) {
        app->SetFullRefresh(DisplayApp::FullRefreshDirections::Down);
        auto &oldScreen = screens[screenIndex];
        oldScreen.Hide();
        screenIndex--;
        auto &newScreen = screens[screenIndex];
        newScreen.Show();
      }
      return true;
    case TouchEvents::SwipeUp:
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Up);
      if (screenIndex < screens.size() - 1) {
        auto &oldScreen = screens[screenIndex];
        oldScreen.Hide();
        screenIndex++;
        auto &newScreen = screens[screenIndex];
        newScreen.Show();
      }
      return true;
    default:
      return false;
  }
  return false;
}
