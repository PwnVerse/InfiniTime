#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include <components/motion/MotionController.h>
namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingShakeThreshold : public Screen {
      public:
        SettingShakeThreshold(DisplayApp* app,
                              Pinetime::Controllers::Settings& settingsController,
                              Controllers::MotionController& motionController,
                              System::SystemTask& systemTask);

        ~SettingShakeThreshold() override;
        void Refresh() override;
        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;
        System::SystemTask& systemTask;


         

        uint8_t taskCount;
        lv_obj_t* cbOption[2];
        lv_obj_t *positionArc, *calButton, *calLabel;
        lv_task_t* refreshTask;
      };
    }
  }
}
