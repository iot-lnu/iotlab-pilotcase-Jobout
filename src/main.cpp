/*
 * HelTec Automation(TM) LoRaWAN 1.0.2 OTAA example use OTAA, CLASS A
 *
 * Function summary:
 *
 * - use internal RTC(150KHz);
 *
 * - Include stop mode and deep sleep mode;
 *
 * - 15S data send cycle;
 *
 * - Informations output via serial(115200);
 *
 * - Only ESP32 + LoRa series boards can use this library, need a license
 *   to make the code run(check you license here: http://www.heltec.cn/search/);
 *
 * You can change some definition in "Commissioning.h" and "LoRaMac-definitions.h"
 *
 * HelTec AutoMation, Chengdu, China.
 * 成都惠利特自动化科技有限公司
 * https://heltec.org
 * support@heltec.cn
 *
 *this project also release in GitHub:
 *https://github.com/HelTecAutomation/ESP32_LoRaWAN
 */

#include <ESP32_LoRaWAN.h>
#include "Arduino.h"

/*
  Deep sleep
*/
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex
RTC_DATA_ATTR int bootCount = 4;

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 1 * 60      /* Time ESP32 will go to sleep (in seconds) */

/*license for Heltec ESP32 LoRaWan, quary your ChipID relevant license: http://resource.heltec.cn/search */
uint32_t license[4] = {0xD5397DF0, 0x8573F814, 0x7A38C73D, 0x48E68607};

/* OTAA para*/
uint8_t DevEui[] = {0x70, 0xB3, 0xD5, 0x49, 0x9D, 0x7D, 0x4F, 0xB0};
uint8_t AppEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23};
uint8_t AppKey[] = {0x3B, 0x21, 0xE7, 0xDB, 0xC0, 0xE3, 0x9F, 0x0D, 0xA1, 0xA6, 0x5B, 0x3D, 0x58, 0xE6, 0xDE, 0xF3};

/* ABP para*/
uint8_t NwkSKey[] = {0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85};
uint8_t AppSKey[] = {0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67};
uint32_t DevAddr = (uint32_t)0x007e6ae1;

/*LoraWan channelsmask, default channels 0-7*/
uint16_t userChannelsMask[6] = {0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t loraWanClass = CLASS_A;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 1800000; // 1800000 ms is every 30 minutes.

/*OTAA or ABP*/
bool overTheAirActivation = true;

/*ADR enable*/
bool loraWanAdr = true;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 2;

/*!
 * Number of trials to transmit the frame, if the LoRaMAC layer did not
 * receive an acknowledgment. The MAC performs a datarate adaptation,
 * according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
 * to the following table:
 *
 * Transmission nb | Data Rate
 * ----------------|-----------
 * 1 (first)       | DR
 * 2               | DR
 * 3               | max(DR-1,0)
 * 4               | max(DR-1,0)
 * 5               | max(DR-2,0)
 * 6               | max(DR-2,0)
 * 7               | max(DR-3,0)
 * 8               | max(DR-3,0)
 *
 * Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
 * the datarate, in case the LoRaMAC layer did not receive an acknowledgment
 */
uint8_t confirmedNbTrials = 8;

/*LoraWan debug level, select in arduino IDE tools.
 * None : print basic info.
 * Freq : print Tx and Rx freq, DR info.
 * Freq && DIO : print Tx and Rx freq, DR, DIO0 interrupt and DIO1 interrupt info.
 * Freq && DIO && PW: print Tx and Rx freq, DR, DIO0 interrupt, DIO1 interrupt and MCU deepsleep info.
 */
uint8_t debugLevel = LoRaWAN_DEBUG_LEVEL;

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
esp_sleep_source_t print_wakeup_reason()
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
  return wakeup_reason;
}

// static void prepareTxFrame(uint8_t port)
// {
//   appDataSize = 4; // AppDataSize max value is 64
//   appData[0] = 0x00;
//   appData[1] = 0x01;
//   appData[2] = 0x02;
//   appData[3] = 0x03;
// }

// Add your initialization code here
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  analogSetAttenuation(ADC_11db);

  // Increment boot number and print it every reboot

  Serial.println("Boot number: " + String(bootCount));

  // Print and save the wakeup reason for ESP32
  esp_sleep_wakeup_cause_t wakeup_reason = print_wakeup_reason();

  if (bootCount > 2)
  {
    /*
      In this scenario, the person is probably sitting on the pad and continuesly triggers its,
      so we enable timer deepsleep instead of external deepsleep, e.g. wake up by pin.

      Or, it just woke up to after appTxDutyCycle time, and we can safely go back to sleep.
    */
    bootCount = 0;
    /*
    First we configure the wake up source
    We set our ESP32 to wake up every 5 seconds
    */
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
  }
  else
  {
    /*
      In this scenario, the person probably just sat on the pad or have been
      sitting on it for more than 30 minutes.
    */
    SPI.begin(SCK, MISO, MOSI, SS);
    Mcu.init(SS, RST_LoRa, DIO0, DIO1, license);
    deviceState = DEVICE_STATE_INIT;

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
    {
      // If the wakeup reason is external signal, then we know that the person
      // is sitting on the pad, so we increment the bootCount
      ++bootCount;
      appData[0] = bootCount;
    }
    else
    {
      // If the wakeup reason is not external signal, then we know that the device just
      // woke up from timer deepsleep and should be ready to be woken up by pin again.
      // We send 0xFF to indicate that.
      appData[0] = 0xFF;
    }

    /*
    First we configure the wake up source
    We set our ESP32 to wake up for an external trigger.
    There are two types for ESP32, ext0 and ext1 .
    ext0 uses RTC_IO to wakeup thus requires RTC peripherals
    to be on while ext1 uses RTC Controller so doesnt need
    peripherals to be powered on.
    Note that using internal pullups/pulldowns also requires
    RTC peripherals to be turned on.
  */

    Serial.println("Setup ESP32 to wake up by pin");
    Serial.println("Going to sleep now");
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1); // 1 = High, 0 = Low
  }
}

// The loop function is called in an endless loop
void loop()
{
  switch (deviceState)
  {
    case DEVICE_STATE_INIT:
    {
      LoRaWAN.init(loraWanClass, loraWanRegion);
      break;
    }
    case DEVICE_STATE_JOIN:
    {
      LoRaWAN.join();
      break;
    }
    case DEVICE_STATE_SEND:
    {
      // prepareTxFrame(appPort);
      LoRaWAN.send(loraWanClass);
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE:
    {
      // Schedule next packet transmission
      txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP:
    {
      LoRaWAN.sleep(loraWanClass, debugLevel);
      break;
    }
    default:
    {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}
