/*
 * ==============================================================
 *  TEST DRONA – narzędzie dla prowadzącego
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci
 * ==============================================================
 *
 *  CO ROBI
 *   1. Skanuje magistralę I2C i uruchamia czujniki:
 *      VL53L0X (0x29) i MPU6050 (0x68). Wypisuje OK albo BŁĄD.
 *   2. Cały czas wypisuje pomiary (Serial Monitor albo Plotter):
 *      laser_mm, ax/ay/az [g], gx/gy/gz [°/s] oraz moc M1..M4 [%].
 *   3. Po komendzie "t" odpala silniki PO KOLEI:
 *      M1: 0% -> 100% -> 0%, przerwa, M2: ..., M3: ..., M4: ...
 *
 *  BEZPIECZEŃSTWO
 *   - Test idzie do MOC_TESTU (domyślnie 100%)! Pierwszy raz
 *     zrób go BEZ ŚMIGIEŁ albo zmniejsz MOC_TESTU.
 *   - Start ROZBROJONY (czerwona dioda). "u" = uzbrój (zielona).
 *   - "x" albo spacja = STOP. Przycisk BOOT = STOP.
 *
 *  KOMENDY (Serial Monitor, 115200, „Nowa linia”)
 *   u – uzbrój         t – test silników po kolei
 *   x – STOP           s – ponowny skan I2C i start czujników
 *
 *  PODŁĄCZENIE
 *   I2C: SDA pin 6, SCL pin 7 (kabel z drona).
 *   ESC: M1..M4 na pinach 18, 19, 20, 21.
 *   Biblioteka „VL53L0X” od Pololu.
 */

#include <Wire.h>
#include <VL53L0X.h>

// ================== KONFIGURACJA ==================
#define TRYB_MOSFET          // moduły ESC sterowane PWM 20 kHz (domyślnie)
// #define TRYB_ESC_RC       // albo regulatory ESC (impulsy 1000-2000 us)
                             // Zostaw odkomentowany TYLKO JEDEN tryb!

const int PIN_SDA = 6;
const int PIN_SCL = 7;
const int PINY_SILNIKOW[4] = {18, 19, 20, 21};   // M1, M2, M3, M4
const int PIN_STOP = 9;          // przycisk BOOT = STOP awaryjny

const float MOC_TESTU = 100;     // [%] do tej mocy rozpędza się każdy silnik
const int CZAS_W_GORE_MS = 4000; // rozpędzanie 0 -> MOC_TESTU
const int CZAS_NA_GORZE_MS = 500;// ile trzyma najwyższą moc
const int CZAS_W_DOL_MS = 4000;  // zwalnianie MOC_TESTU -> 0
const int PRZERWA_MS = 1500;     // przerwa przed kolejnym silnikiem
const int CO_ILE_MS = 50;        // jak często wypisywać pomiary

const int PWM_HZ = 20000;        // TRYB_MOSFET: częstotliwość PWM
const int PWM_BITY = 10;
const int ESC_HZ = 50;           // TRYB_ESC_RC: 50 impulsów na sekundę
const int ESC_BITY = 14;
// ==================================================

#if defined(TRYB_MOSFET) && defined(TRYB_ESC_RC)
#error "Wybierz tylko jeden tryb: TRYB_MOSFET albo TRYB_ESC_RC"
#endif

const int ADRES_MPU = 0x68;
const float LSB_NA_G = 16384.0;   // zakres ±2 g
const float LSB_NA_ST = 131.0;    // zakres ±250 °/s

VL53L0X laser;
bool laser_ok = false;
bool mpu_ok = false;
int laser_mm = 0;                 // 0 = brak dobrego pomiaru
float akc[3] = {0, 0, 0};         // [g]
float zyro[3] = {0, 0, 0};        // [°/s]

bool uzbrojony = false;
float moc[4] = {0, 0, 0, 0};      // [%]
unsigned long czas_wypisu = 0;

// ================== SILNIKI ==================
void silnikiNaZero() {
  for (int i = 0; i < 4; i++) {
    pinMode(PINY_SILNIKOW[i], OUTPUT);
    digitalWrite(PINY_SILNIKOW[i], LOW);
  }
}

void pokazStan() {
#ifdef RGB_BUILTIN
  if (uzbrojony) rgbLedWrite(RGB_BUILTIN, 0, 30, 0);
  else rgbLedWrite(RGB_BUILTIN, 30, 0, 0);
#endif
}

void wyslijMoc(int nr, float m) {
  if (!uzbrojony) m = 0;
  m = constrain(m, 0, MOC_TESTU);
  moc[nr] = m;
#ifdef TRYB_ESC_RC
  float impuls_us = 1000 + m * 10;      // 0% = 1000 us, 100% = 2000 us
  uint32_t wypelnienie = impuls_us * ESC_HZ * (1 << ESC_BITY) / 1000000.0;
#else
  uint32_t wypelnienie = m / 100.0 * ((1 << PWM_BITY) - 1);
#endif
  ledcWrite(PINY_SILNIKOW[nr], wypelnienie);
}

void silnikiStart() {
  for (int i = 0; i < 4; i++) {
#ifdef TRYB_ESC_RC
    ledcAttach(PINY_SILNIKOW[i], ESC_HZ, ESC_BITY);
#else
    ledcAttach(PINY_SILNIKOW[i], PWM_HZ, PWM_BITY);
#endif
    wyslijMoc(i, 0);
  }
  pinMode(PIN_STOP, INPUT_PULLUP);
  pokazStan();
}

void stop(String powod) {
  uzbrojony = false;
  for (int i = 0; i < 4; i++) wyslijMoc(i, 0);
  Serial.println("STOP! Powód: " + powod);
  pokazStan();
}

// ================== CZUJNIKI ==================
void skanujI2C() {
  Serial.println("Skan I2C:");
  int ile = 0;
  for (int adres = 1; adres < 127; adres++) {
    Wire.beginTransmission(adres);
    if (Wire.endTransmission() == 0) {
      Serial.print("  0x");
      if (adres < 16) Serial.print("0");
      Serial.print(adres, HEX);
      if (adres == 0x29) Serial.print("  VL53L0X (laser)");
      if (adres == 0x68 || adres == 0x69) Serial.print("  MPU6050");
      Serial.println();
      ile++;
    }
  }
  if (ile == 0) Serial.println("  nic! Sprawdź kabel: SDA pin 6, SCL pin 7, 3V3, GND.");
}

void zapiszMPU(byte rejestr, byte wartosc) {
  Wire.beginTransmission(ADRES_MPU);
  Wire.write(rejestr);
  Wire.write(wartosc);
  Wire.endTransmission();
}

void startCzujnikow() {
  skanujI2C();

  laser.setTimeout(500);
  laser_ok = laser.init();
  if (laser_ok) {
    laser.setMeasurementTimingBudget(33000);
    laser.startContinuous();
  }
  Serial.println(laser_ok ? "VL53L0X: OK" : "VL53L0X: BŁĄD (brak odpowiedzi)");

  Wire.beginTransmission(ADRES_MPU);
  Wire.write(0x75);                          // WHO_AM_I
  mpu_ok = Wire.endTransmission(false) == 0 && Wire.requestFrom(ADRES_MPU, 1) == 1;
  byte kto = mpu_ok ? Wire.read() : 0;
  if (mpu_ok) zapiszMPU(0x6B, 0);            // obudź czujnik
  Serial.print(mpu_ok ? "MPU6050: OK, WHO_AM_I = 0x" : "MPU6050: BŁĄD (brak odpowiedzi)");
  if (mpu_ok) Serial.print(kto, HEX);
  Serial.println();
}

void czytajLaser() {
  if (!laser_ok) return;
  if ((laser.readReg(VL53L0X::RESULT_INTERRUPT_STATUS) & 0x07) == 0) return;
  int mm = laser.readRangeContinuousMillimeters();
  laser_mm = (!laser.timeoutOccurred() && mm < 8000) ? mm : 0;
}

void czytajMPU() {
  if (!mpu_ok) return;
  Wire.beginTransmission(ADRES_MPU);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom(ADRES_MPU, 14) != 14) return;
  int16_t s[7];
  for (int i = 0; i < 7; i++) s[i] = (Wire.read() << 8) | Wire.read();
  for (int i = 0; i < 3; i++) {
    akc[i] = s[i] / LSB_NA_G;
    zyro[i] = s[i + 4] / LSB_NA_ST;          // s[3] to temperatura
  }
}

void wypisz() {
  Serial.print("laser_mm:"); Serial.print(laser_mm);
  Serial.print(",ax:"); Serial.print(akc[0], 2);
  Serial.print(",ay:"); Serial.print(akc[1], 2);
  Serial.print(",az:"); Serial.print(akc[2], 2);
  Serial.print(",gx:"); Serial.print(zyro[0], 1);
  Serial.print(",gy:"); Serial.print(zyro[1], 1);
  Serial.print(",gz:"); Serial.print(zyro[2], 1);
  for (int i = 0; i < 4; i++) {
    Serial.print(",M"); Serial.print(i + 1); Serial.print(":");
    Serial.print(moc[i], 0);
  }
  Serial.println();
}

// ================== KOMENDY ==================
bool chce_test = false;

void obsluzSerial() {
  while (Serial.available() > 0) {
    char z = Serial.read();
    if (z == 'x' || z == 'X' || z == ' ') stop("komenda x");
    else if (z == 'u' || z == 'U') {
      uzbrojony = true;
      Serial.println("UZBROJONY! Uwaga na śmigła. Wpisz t, żeby zacząć test.");
      pokazStan();
    } else if (z == 't' || z == 'T') {
      if (uzbrojony) chce_test = true;
      else Serial.println("Najpierw uzbrój: wpisz u");
    } else if (z == 's' || z == 'S') {
      if (!uzbrojony) startCzujnikow();
    }
  }
}

// Wszystko, co dzieje się „w tle”: komendy, BOOT, czujniki, wypisywanie.
void pilnuj() {
  obsluzSerial();
  if (uzbrojony && digitalRead(PIN_STOP) == LOW) stop("przycisk BOOT");
  czytajLaser();
  if (millis() - czas_wypisu >= CO_ILE_MS) {
    czas_wypisu = millis();
    czytajMPU();
    wypisz();
  }
}

// Płynnie zmienia moc silnika nr od..do w czasie ms. False = był STOP.
bool plynnie(int nr, float od, float do_mocy, unsigned long ms) {
  unsigned long start = millis();
  while (true) {
    pilnuj();
    if (!uzbrojony) return false;
    unsigned long minelo = millis() - start;
    if (minelo >= ms) break;
    wyslijMoc(nr, od + (do_mocy - od) * minelo / ms);
    delay(2);
  }
  wyslijMoc(nr, do_mocy);
  return true;
}

void testSilnikow() {
  Serial.println("TEST SILNIKÓW po kolei (0 -> " + String(MOC_TESTU, 0) + "% -> 0).");
  for (int i = 0; i < 4; i++) {
    Serial.println("M" + String(i + 1) + " ...");
    if (!plynnie(i, 0, MOC_TESTU, CZAS_W_GORE_MS)) return;
    if (!plynnie(i, MOC_TESTU, MOC_TESTU, CZAS_NA_GORZE_MS)) return;
    if (!plynnie(i, MOC_TESTU, 0, CZAS_W_DOL_MS)) return;
    if (i < 3 && !plynnie(i, 0, 0, PRZERWA_MS)) return;
  }
  Serial.println("Test skończony. Wpisz t, żeby powtórzyć, albo x.");
}

void setup() {
  silnikiNaZero();           // NAJPIERW silniki na 0!
  Serial.begin(115200);
  delay(1000);
  silnikiStart();
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);     // kabel I2C ok. 1 m
  Wire.setTimeOut(50);
  Serial.println("TEST DRONA");
  startCzujnikow();
  Serial.println("Komendy: u = uzbrój, t = test silników, x = STOP, s = skan I2C.");
}

void loop() {
  pilnuj();
  if (chce_test) {
    chce_test = false;
    testSilnikow();
    for (int i = 0; i < 4; i++) wyslijMoc(i, 0);
  }
}
