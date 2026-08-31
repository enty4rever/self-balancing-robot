

#include <Arduino.h>
#include "driver/pulse_cnt.h"

// ---------------------- CHAN ENCODER ----------------------
#define ENC_L_A   18
#define ENC_L_B   19
#define ENC_R_A   23
#define ENC_R_B   17

// SUA hai co nay sau khi doc ket qua test
#define ENC_L_INVERT   true
#define ENC_R_INVERT   false

// ---------------------- CHAN TB6612 ----------------------
#define STBY   16
#define PWM_A  25
#define IN1_A  26
#define IN2_A  27
#define PWM_B  33
#define IN1_B  14
#define IN2_B  13

// ---------------------- PWM (LEDC) ----------------------
const int PWM_FREQ = 20000;   // 20 kHz, ngoai nguong nghe
const int PWM_RES  = 8;       // 8-bit -> 0..255
const int DUTY_MAX = 204;     // tran cung 80%
const int TEST_DUTY = 130;    // ~51%, du de banh quay ro

// ---------------------- THONG SO CO KHI ----------------------
const float ENC_PPR    = 11.0f;
const float ENC_QUAD   = 4.0f;
const float GEAR_RATIO = 24.8f;   // DO THUC TE, khong phai 21.3
const float CPR_OUTPUT = ENC_PPR * ENC_QUAD * GEAR_RATIO;   // ~1091

// =====================================================================
//  ENCODER
// =====================================================================
static const int PCNT_LIMIT = 30000;

struct EncoderUnit {
  pcnt_unit_handle_t unit = nullptr;
  volatile int32_t   overflow = 0;
  bool               invert = false;
};

EncoderUnit encL, encR;

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

  pcnt_glitch_filter_config_t filt = {};
  filt.max_glitch_ns = 1000;
  pcnt_unit_set_glitch_filter(e.unit, &filt);

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
//  MOTOR - LEDC 20 kHz
// =====================================================================
void motorSetup() {
  pinMode(STBY,  OUTPUT);
  pinMode(IN1_A, OUTPUT);
  pinMode(IN2_A, OUTPUT);
  pinMode(IN1_B, OUTPUT);
  pinMode(IN2_B, OUTPUT);

  ledcAttach(PWM_A, PWM_FREQ, PWM_RES);
  ledcAttach(PWM_B, PWM_FREQ, PWM_RES);

  digitalWrite(STBY, LOW);   // giu tat cho den khi san sang
}

// speed: -255..255. 0 = BRAKE (ca hai chan HIGH).
void driveMotor(int in1, int in2, int pwmPin, int speed) {
  speed = constrain(speed, -DUTY_MAX, DUTY_MAX);

  if (speed == 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, HIGH);
    ledcWrite(pwmPin, 0);
  } else {
    digitalWrite(in1, speed > 0 ? HIGH : LOW);
    digitalWrite(in2, speed > 0 ? LOW  : HIGH);
    ledcWrite(pwmPin, abs(speed));
  }
}

void driveBoth(int s) {
  driveMotor(IN1_A, IN2_A, PWM_A, s);
  driveMotor(IN1_B, IN2_B, PWM_B, s);
}

void motorStop() {
  driveBoth(0);
  digitalWrite(STBY, LOW);   // ngat hoan toan driver
}

// =====================================================================
//  MOT LAN CHAY
// =====================================================================
void runOneDirection(const char *label, int duty, const char *expect) {
  Serial.println();
  Serial.print(">> ");
  Serial.print(label);
  Serial.println("  (3 giay)");

  encoderReset();
  digitalWrite(STBY, HIGH);
  driveBoth(duty);

  // in count trong luc chay de thay xu huong
  for (int i = 0; i < 6; i++) {
    delay(500);
    Serial.print("     t=");
    Serial.print((i + 1) * 0.5, 1);
    Serial.print("s   L=");
    Serial.print(encoderCountL());
    Serial.print("   R=");
    Serial.println(encoderCountR());
  }

  driveBoth(0);
  delay(400);                 // cho banh dung han
  digitalWrite(STBY, LOW);

  int32_t cL = encoderCountL();
  int32_t cR = encoderCountR();

  Serial.print("   KET QUA:  L=");
  Serial.print(cL);
  Serial.print("   R=");
  Serial.print(cR);
  Serial.print("   |   L_rev=");
  Serial.print(cL / CPR_OUTPUT, 2);
  Serial.print("   R_rev=");
  Serial.println(cR / CPR_OUTPUT, 2);
  Serial.print("   Ky vong: ca hai ");
  Serial.println(expect);
}

// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(800);

  motorSetup();
  initUnit(encL, ENC_L_A, ENC_L_B, ENC_L_INVERT);
  initUnit(encR, ENC_R_A, ENC_R_B, ENC_R_INVERT);
  encoderReset();

  Serial.println();
  Serial.println("========================================");
  Serial.println("  TEST_MODE 2 : kiem tra DAU encoder");
  Serial.println("========================================");
  Serial.print("CPR_OUTPUT = ");
  Serial.print(CPR_OUTPUT);
  Serial.println(" count / vong banh");
  Serial.print("PWM = ");
  Serial.print(TEST_DUTY);
  Serial.print("/255 @ ");
  Serial.print(PWM_FREQ);
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Ke xe len hop, banh khong cham dat.");
  Serial.println("Bat dau sau 3 giay...");
  delay(3000);

  runOneDirection("TIEN", TEST_DUTY, "DUONG (+)");
  delay(2000);
  runOneDirection("LUI", -TEST_DUTY, "AM (-)");

  motorStop();

  Serial.println();
  Serial.println("========================================");
  Serial.println("  BANG DOI CHIEU");
  Serial.println("========================================");
  Serial.println(" Banh quay dung chieu, count AM");
  Serial.println("   -> bat INVERT cho ben do trong code");
  Serial.println();
  Serial.println(" Banh quay SAI chieu vat ly");
  Serial.println("   -> dao day Do/Trang tai TB6612");
  Serial.println();
  Serial.println(" Mot ben count = 0");
  Serial.println("   -> day encoder ben do long");
  Serial.println();
  Serial.println("TEST XONG - motor da dung. Rut pin.");
  Serial.println("Nhan nut EN de chay lai.");
}

void loop() {
  // khong lam gi - test chi chay mot lan trong setup()
}
