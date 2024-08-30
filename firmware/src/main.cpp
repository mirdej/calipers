#include <Arduino.h>
#include <BleKeyboard.h>
#include "Calipers.h"
#include "SPI.h"

const int NUM_BTNS = 2;
const int PIN_CALIP_CLK = 1;
const int PIN_CALIP_DATA = 0;
const int PIN_BTN[NUM_BTNS] = {3, 10};

BleKeyboard bleKeyboard("Calipers", "[ a n y m a ]", 90);
Calipers caliper;

uint64_t button_pin_mask()
{
  uint64_t mask = 0;
  for (int i = 0; i < NUM_BTNS; i++)
  {
    mask |= (1 << PIN_BTN[i]);
  }
  return mask;
}

//----------------------------------------------------------------------------------------
//																				BUTTONS

void btn_task(void *)
{
  int state[NUM_BTNS];
  int old_state[NUM_BTNS];

  for (int i = 0; i < NUM_BTNS; i++)
  {
    pinMode(PIN_BTN[i], INPUT_PULLUP);
    old_state[i] = digitalRead(PIN_BTN[i]);
    log_v("Button %d initialised", i);
  }

  while (1)
  {
    for (int i = 0; i < NUM_BTNS; i++)
    {
      state[i] = digitalRead(PIN_BTN[i]);
      if (state[i] != old_state[i])
      {
        old_state[i] = state[i];
        if (!state[i])
        {
          log_v("Button %d pressed", i);
          char buf[10];
          float f = caliper.get_mm();

          if (bleKeyboard.isConnected())
          {

            // Weird Bug? prints ' instead of - on negative numbers (BLE only)
            if (f < 0.)
            {
              f = 0. - f;
              bleKeyboard.print('/'); // ASCII slash character turns into minus sign
            }

            switch (i)
            {
            case 0:

              sprintf(buf, "%.2f\n", f);
              Serial.print(buf);
              bleKeyboard.print(buf);
              break;

              // radius from diameter (for sketchup...)
            case 1:
              sprintf(buf, "%.2f\n", f / 2.);
              Serial.print(buf);
              bleKeyboard.print(buf);
              break;
            }
          }
        }
        else
        {
          log_v("Button %d released", i);
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

//========================================================================================
//----------------------------------------------------------------------------------------
//																				Loop

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println(PROJECT_PATH);
  Serial.println(FIRMWARE_VERSION);

  print_wakeup_reason();

  log_v("Starting BLE");
  bleKeyboard.begin();

  caliper.begin(PIN_CALIP_DATA, PIN_CALIP_CLK);
 // esp_deep_sleep_enable_gpio_wakeup(button_pin_mask(), ESP_GPIO_WAKEUP_GPIO_LOW);
 esp_deep_sleep_enable_gpio_wakeup(1 << PIN_CALIP_CLK, ESP_GPIO_WAKEUP_GPIO_LOW);
  xTaskCreate(
      btn_task,  // Function that implements the task.
      "Buttons", // Text name for the task.
      8192,      // Stack size in words, not bytes.
      NULL,      // Parameter passed into the task.
      0,         // Priority at which the task is created.
      NULL);
}

//========================================================================================
//----------------------------------------------------------------------------------------
//																				Loop

void loop()
{
  if (!caliper.is_on())
  {
    Serial.print("Caliper is off");
    esp_deep_sleep_start();
    delay(1000);
    Serial.print("Caliper is still off");
  }

  if (caliper.available())
  {
    //  caliper.print(); // print debug info

    delay(2);
    // Serial.printf("Distance: %.2f\n",caliper.get_mm());
  }
  delay(4);
}
