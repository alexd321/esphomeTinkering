/*
 * ESP32 Sketch shows that you can program the ulp co-processor using
 * Arduino using the ulp assembly MACRO's. This sketch just blinks the
 * on board LED on for x microseconds and OFF x microseconds using the ulp. 
 * This sketch was inspired from https://github.com/espressif/esp-idf/blob/98e5c475b31ce704e08bd1a48f5d2de3f0b216b0/components/ulp/test/test_ulp.c 
 * and https://www.esp32.com/viewtopic.php?t=4899#p21215
 */
#include "esp32/ulp.h"
#include "driver/rtc_io.h"
#include "esphome.h"

void ULP_BLINK_RUN(uint32_t us);

class ulptest : public Component {
  public:
    
    
    void setup() override {
      // microseconds to delay between halt and wake states
      ULP_BLINK_RUN(100000);
    }
    
    void loop() override {
      // put your main code here, to run repeatedly:
    
    }
    // ------------------------------------------------------------------------
    void ULP_BLINK_RUN(uint32_t us) {
      RTC_SLOW_MEM[12] = 0;
      ulp_set_wakeup_period(0, us);
      const ulp_insn_t  ulp_blink[] = {
        I_MOVI(R3, 12),                         // #12 -> R3
        I_LD(R0, R3, 0),                        // R0 = RTC_SLOW_MEM[R3(#12)] 
        M_BL(1, 1),                             // GOTO M_LABEL(1) IF R0 < 1
        I_WR_REG(RTC_GPIO_OUT_REG, 26, 27, 1),  // RTC_GPIO12 = 1
        I_SUBI(R0, R0, 1),                      // R0 = R0 - 1, R0 = 1, R0 = 0
        I_ST(R0, R3, 0),                        // RTC_SLOW_MEM[R3(#12)] = R0
        M_BX(2),                                // GOTO M_LABEL(2)
        M_LABEL(1),                             // M_LABEL(1)
          I_WR_REG(RTC_GPIO_OUT_REG, 26, 27, 0),// RTC_GPIO12 = 0
          I_ADDI(R0, R0, 1),                    // R0 = R0 + 1, R0 = 0, R0 = 1
          I_ST(R0, R3, 0),                      // RTC_SLOW_MEM[R3(#12)] = R0
        M_LABEL(2),                             // M_LABEL(2)
        I_HALT()                                // HALT COPROCESSOR
      };
      const gpio_num_t led_gpios[] = {
        GPIO_NUM_2,
        GPIO_NUM_0,
        GPIO_NUM_4
      };
      for (size_t i = 0; i < sizeof(led_gpios) / sizeof(led_gpios[0]); ++i) {
        rtc_gpio_init(led_gpios[i]);
        rtc_gpio_set_direction(led_gpios[i], RTC_GPIO_MODE_OUTPUT_ONLY);
        rtc_gpio_set_level(led_gpios[i], 0);
      }
      size_t size = sizeof(ulp_blink) / sizeof(ulp_insn_t);
      ulp_process_macros_and_load(0, ulp_blink, &size);
      ulp_run(0);
    }
};
