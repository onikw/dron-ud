/*
 * ==============================================================
 *  NARZĘDZIE: ROZPOZNAJ CZUJNIK ODLEGŁOŚCI (dla prowadzącego)
 * ==============================================================
 *
 *  Skaner I2C widzi 0x29, ale L03 pisze „Nie widzę lasera!”?
 *  VL53L0X i VL53L1X mają ten sam adres 0x29, więc skaner ich nie odróżni.
 *  Biblioteka Pololu „VL53L1X” na starcie czyta numer modelu i gdy to nie
 *  jest VL53L1X, init() zwraca false.
 *
 *  Ten szkic czyta numer modelu na oba sposoby i wypisuje, jaki to czujnik.
 *  Nie potrzebuje żadnej biblioteki – tylko Wire.
 *
 *    VL53L0X: adres rejestru 1-bajtowy, rejestr 0xC0 = 0xEE (0xC1 = 0xAA)
 *    VL53L1X: adres rejestru 2-bajtowy, rejestr 0x010F = 0xEA, 0x0110 = 0xCC
 *
 *  Wgraj, otwórz Serial Monitor (115200) i przeczytaj wynik.
 */

#include <Wire.h>

// ================== KONFIGURACJA ==================
const int PIN_SDA = 6;
const int PIN_SCL = 7;
const uint8_t ADRES = 0x29;
// ==================================================

// Odczyt rejestru z 1-bajtowym adresem (tak jak w VL53L0X).
int czytaj8(uint8_t rejestr) {
  Wire.beginTransmission(ADRES);
  Wire.write(rejestr);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom(ADRES, (uint8_t)1) != 1) return -1;
  return Wire.read();
}

// Odczyt rejestru z 2-bajtowym adresem (tak jak w VL53L1X).
int czytaj16(uint16_t rejestr) {
  Wire.beginTransmission(ADRES);
  Wire.write((uint8_t)(rejestr >> 8));
  Wire.write((uint8_t)rejestr);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom(ADRES, (uint8_t)1) != 1) return -1;
  return Wire.read();
}

void wypiszHex(const char* opis, int wartosc) {
  Serial.print(opis);
  if (wartosc < 0) { Serial.println("brak odpowiedzi"); return; }
  Serial.print("0x");
  if (wartosc < 16) Serial.print("0");
  Serial.println(wartosc, HEX);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);
  Wire.setTimeOut(50);
}

void loop() {
  Serial.println("---------------------------------------");
  Wire.beginTransmission(ADRES);
  if (Wire.endTransmission() != 0) {
    Serial.println("Nikt nie odpowiada pod 0x29. Sprawdź kable (skaner L02).");
    delay(3000);
    return;
  }
  Serial.println("Pod 0x29 ktoś odpowiada. Sprawdzam, kto to.");

  // Najpierw test VL53L0X: sam odczyt, nic nie zapisujemy.
  int c0 = czytaj8(0xC0), c1 = czytaj8(0xC1);
  wypiszHex("  rejestr 0xC0 (VL53L0X ma 0xEE): ", c0);
  wypiszHex("  rejestr 0xC1 (VL53L0X ma 0xAA): ", c1);
  if (c0 == 0xEE && c1 == 0xAA) {
    Serial.println("WYNIK: to jest VL53L0X.");
    Serial.println("  Potrzebna biblioteka Pololu \"VL53L0X\" i szkice dla VL53L0X.");
    delay(5000);
    return;
  }

  int m1 = czytaj16(0x010F), m2 = czytaj16(0x0110);
  wypiszHex("  rejestr 0x010F (VL53L1X ma 0xEA): ", m1);
  wypiszHex("  rejestr 0x0110 (VL53L1X ma 0xCC): ", m2);
  if (m1 == 0xEA && m2 == 0xCC) {
    Serial.println("WYNIK: to jest VL53L1X – biblioteka i szkice pasują.");
    Serial.println("  Jeśli L03 dalej nie startuje: czujnik nie kończy startu.");
    Serial.println("  Sprawdź zasilanie 3V3, pin XSHUT (nie może być na GND) i odłącz/podłącz USB.");
  } else {
    Serial.println("WYNIK: nie rozpoznaję czujnika (ani VL53L0X, ani VL53L1X).");
    Serial.println("  Sprawdź napis na płytce czujnika i przewody SDA/SCL/GND.");
  }
  delay(5000);
}
