// =====================================================================
//  TEST_MODE 1 - Dem xung encoder bang PCNT phan cung
//  Quay banh bang tay DUNG 5 VONG, doc L_rev / R_rev
//  Ky vong: 5.00 hoac -5.00
//
//  KHONG cap nguon motor o bai test nay.
//  Day Do/Trang de ho. TB6612 chua can noi.
// =====================================================================

#include <Arduino.h>
#include "driver/pulse_cnt.h"

// ---------------------- CHAN ENCODER ----------------------
#define ENC_L_A   18
#define ENC_L_B   19
#define ENC_R_A   23
#define ENC_R_B   17

// Dao chieu dem. Chua can dung o bai nay, de false het.
#define ENC_L_INVERT   false
#define ENC_R_INVERT   false

// ---------------------- THONG SO CO KHI ----------------------
// GA25-370: 11 PPR / kenh / vong truc motor, dem x4, giam toc 21.3:1
const float ENC_PPR    = 11.0f;
const float ENC_QUAD   = 4.0f;
const float GEAR_RATIO = 21.3f;
const float CPR_OUTPUT = ENC_PPR * ENC_QUAD * GEAR_RATIO;   // ~937.2

// =====================================================================
//  ENCODER
// =====================================================================
static const int PCNT_LIMIT = 30000;   // PCNT chi 16-bit, phai bat tran

struct EncoderUnit {
  pcnt_unit_handle_t unit = nullptr;
  volatile int32_t   overflow = 0;
  bool               invert = false;
};

EncoderUnit encL, encR;

// Callback khi counter cham nguong. Phan cung tu ve 0, ta cong don lai.
bool IRAM_ATTR onPcntReach(pcnt_unit_handle_t unit,
                           const pcnt_watch_event_data_t *evt,
                           void *arg) {
  EncoderUnit *e = (EncoderUnit *)arg;
  e->overflow += evt->watch_point_value;
  return false;
}

void initUnit(EncoderUnit &e, int pinA, int pinB, bool invert) {
  e.invert   = invert;
  e.overflow = 0;

  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);

  pcnt_unit_config_t ucfg = {};
  ucfg.low_limit  = -PCNT_LIMIT;
  ucfg.high_limit =  PCNT_LIMIT;
  pcnt_new_unit(&ucfg, &e.unit);

  // Loc xung ngan hon 1us
  pcnt_glitch_filter_config_t filt = {};
  filt.max_glitch_ns = 1000;
  pcnt_unit_set_glitch_filter(e.unit, &filt);

  // Kenh 1: dem CA HAI canh cua A, muc B quyet dinh chieu
  pcnt_chan_config_t cfgA = {};
  cfgA.edge_gpio_num  = pinA;
  cfgA.level_gpio_num = pinB;
  pcnt_channel_handle_t chA;
  pcnt_new_channel(e.unit, &cfgA, &chA);
  pcnt_channel_set_edge_action(chA,
      PCNT_CHANNEL_EDGE_ACTION_DECREASE,
      PCNT_CHANNEL_EDGE_ACTION_INCREASE);
  pcnt_channel_set_level_action(chA,
      PCNT_CHANNEL_LEVEL_ACTION_KEEP,
      PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

  // Kenh 2: dem CA HAI canh cua B -> tong cong x4
  pcnt_chan_config_t cfgB = {};
  cfgB.edge_gpio_num  = pinB;
  cfgB.level_gpio_num = pinA;
  pcnt_channel_handle_t chB;
  pcnt_new_channel(e.unit, &cfgB, &chB);
  pcnt_channel_set_edge_action(chB,
      PCNT_CHANNEL_EDGE_ACTION_INCREASE,
      PCNT_CHANNEL_EDGE_ACTION_DECREASE);
  pcnt_channel_set_level_action(chB,
      PCNT_CHANNEL_LEVEL_ACTION_KEEP,
      PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

  pcnt_unit_add_watch_point(e.unit,  PCNT_LIMIT);
  pcnt_unit_add_watch_point(e.unit, -PCNT_LIMIT);

  pcnt_event_callbacks_t cbs = {};
  cbs.on_reach = onPcntReach;
  pcnt_unit_register_event_callbacks(e.unit, &cbs, &e);

  pcnt_unit_enable(e.unit);
  pcnt_unit_clear_count(e.unit);
  pcnt_unit_start(e.unit);
}

// Doc an toan: callback tran co the chen vao giua, doc lai de kiem tra
int32_t readUnit(EncoderUnit &e) {
  int32_t ovf1, ovf2;
  int raw;
  do {
    ovf1 = e.overflow;
    pcnt_unit_get_count(e.unit, &raw);
    ovf2 = e.overflow;
  } while (ovf1 != ovf2);

  int32_t total = ovf1 + raw;
  return e.invert ? -total : total;
}

int32_t encoderCountL() { return readUnit(encL); }
int32_t encoderCountR() { return readUnit(encR); }

void encoderReset() {
  pcnt_unit_clear_count(encL.unit);
  pcnt_unit_clear_count(encR.unit);
  encL.overflow = 0;
  encR.overflow = 0;
}

// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  initUnit(encL, ENC_L_A, ENC_L_B, ENC_L_INVERT);
  initUnit(encR, ENC_R_A, ENC_R_B, ENC_R_INVERT);
  encoderReset();

  Serial.println();
  Serial.println("=== TEST_MODE 1 : dem xung encoder ===");
  Serial.print("CPR output (count / vong banh) = ");
  Serial.println(CPR_OUTPUT);
  Serial.println();
  Serial.println("Quay banh DUNG 5 vong -> rev phai ra 5.00 hoac -5.00");
  Serial.println("Quay nguoc ve cho cu   -> count phai ve gan 0");
  Serial.println();
}

void loop() {
  int32_t cL = encoderCountL();
  int32_t cR = encoderCountR();

  Serial.print("L=");      Serial.print(cL);
  Serial.print("\tR=");    Serial.print(cR);
  Serial.print("\tL_rev="); Serial.print(cL / CPR_OUTPUT, 3);
  Serial.print("\tR_rev="); Serial.println(cR / CPR_OUTPUT, 3);

  delay(200);
}
