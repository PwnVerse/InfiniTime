#include "DisplayApp.h"
#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include <boards.h>
#include <nrf_font.h>
#include <hal/nrf_rtc.h>
#include "Components/Gfx/Gfx.h"
#include <queue.h>
#include <Components/DateTime/DateTimeController.h>
#include <drivers/Cst816s.h>
#include <chrono>
#include <string>
#include <lvgl/lvgl.h>
#include <DisplayApp/Screens/Tile.h>
#include <DisplayApp/Screens/Tab.h>

using namespace Pinetime::Applications;

DisplayApp::DisplayApp(Pinetime::Drivers::St7789& lcd,
                       Pinetime::Components::Gfx& gfx,
                       Pinetime::Components::LittleVgl& lvgl,
                       Pinetime::Drivers::Cst816S& touchPanel,
                       Controllers::Battery &batteryController,
                       Controllers::Ble &bleController,
                       Controllers::DateTime &dateTimeController) :
        lcd{lcd},
        gfx{gfx},
        lvgl{lvgl},
        touchPanel{touchPanel},
        batteryController{batteryController},
        bleController{bleController},
        dateTimeController{dateTimeController},
        currentScreen{new Screens::Tile(this, gfx) } {
  msgQueue = xQueueCreate(queueSize, itemSize);
}

void DisplayApp::Start() {
  if (pdPASS != xTaskCreate(DisplayApp::Process, "DisplayApp", 512, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void DisplayApp::Process(void *instance) {
  auto *app = static_cast<DisplayApp *>(instance);
  NRF_LOG_INFO("DisplayApp task started!");
  app->InitHw();

  while (1) {

    app->Refresh();
    lv_task_handler();
  }
}

void DisplayApp::InitHw() {
  nrf_gpio_cfg_output(pinLcdBacklight1);
  nrf_gpio_cfg_output(pinLcdBacklight2);
  nrf_gpio_cfg_output(pinLcdBacklight3);
  nrf_gpio_pin_clear(pinLcdBacklight1);
  nrf_gpio_pin_clear(pinLcdBacklight2);
  nrf_gpio_pin_clear(pinLcdBacklight3);
}

uint32_t acc = 0;
uint32_t count = 0;
bool toggle = true;
void DisplayApp::Refresh() {
  TickType_t queueTimeout;
  switch (state) {
    case States::Idle:
      IdleState();
      queueTimeout = portMAX_DELAY;
      break;
    case States::Running:
      RunningState();
      queueTimeout = 20;
      break;
  }

  Messages msg;
  if (xQueueReceive(msgQueue, &msg, queueTimeout)) {
    switch (msg) {
      case Messages::GoToSleep:
        nrf_gpio_pin_set(pinLcdBacklight3);
        vTaskDelay(100);
        nrf_gpio_pin_set(pinLcdBacklight2);
        vTaskDelay(100);
        nrf_gpio_pin_set(pinLcdBacklight1);
        lcd.DisplayOff();
        lcd.Sleep();
        touchPanel.Sleep();
        state = States::Idle;
        break;
      case Messages::GoToRunning:
        lcd.Wakeup();
        touchPanel.Wakeup();

        lcd.DisplayOn();
        nrf_gpio_pin_clear(pinLcdBacklight3);
        nrf_gpio_pin_clear(pinLcdBacklight2);
        nrf_gpio_pin_clear(pinLcdBacklight1);
        state = States::Running;
        break;
      case Messages::UpdateDateTime:
        break;
      case Messages::UpdateBleConnection:
//        clockScreen.SetBleConnectionState(bleController.IsConnected() ? Screens::Clock::BleConnectionStates::Connected : Screens::Clock::BleConnectionStates::NotConnected);
        break;
      case Messages::UpdateBatteryLevel:
//        clockScreen.SetBatteryPercentRemaining(batteryController.PercentRemaining());
        break;
      case Messages::TouchEvent:
        if(state != States::Running) break;
        OnTouchEvent();
        break;
      case Messages::ButtonPushed:
        currentScreen->OnButtonPushed();
    }
  }
}

bool first = true;

void DisplayApp::RunningState() {
//  clockScreen.SetCurrentDateTime(dateTimeController.CurrentDateTime());

  if(currentScreen != nullptr) {
    currentScreen->Refresh(first);
    if(currentScreen->GetNextScreen() != Screens::Screen::NextScreen::None) {
      switch(currentScreen->GetNextScreen()) {
        case Screens::Screen::NextScreen::Clock:
          currentScreen.reset(nullptr);
          currentScreen.reset(new Screens::Clock(this, gfx, dateTimeController));
          break;
        case Screens::Screen::NextScreen::Menu:
          currentScreen.reset(nullptr);
          currentScreen.reset(new Screens::Tile(this, gfx));
          break;
        case Screens::Screen::NextScreen::App:
          currentScreen.reset(nullptr);
          currentScreen.reset(new Screens::Message(this, gfx));
          break;
      }
    }
    first = false;
  }
}

void DisplayApp::IdleState() {

}

void DisplayApp::PushMessage(DisplayApp::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(msgQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}

static uint16_t pointColor = 0x07e0;
void DisplayApp::OnTouchEvent() {
//  auto info = touchPanel.GetTouchInfo();
//
//  if(info.isTouch) {
//    gfx.FillRectangle(info.x-10, info.y-10, 20,20, pointColor);
//    pointColor+=10;
//  }
}
