/*
 * ==============================================================
 *  LEKCJA 05: SKANER I2C – KTO JEST NA MAGISTRALI?
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci
 *  WERSJA UCZNIA
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Co to jest I2C (czyt. „i kwadrat ce”).
 *   - Co to jest adres urządzenia.
 *   - Jak sprawdzić, czy czujniki są dobrze podłączone.
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Czujniki MPU6050 i laser VL53L1X, oba zasilane z 3,3 V.
 *   - SDA -> pin 6, SCL -> pin 7, GND -> GND.
 *
 *  JAK TO DZIAŁA
 *   I2C to dwie wspólne linie dla wielu urządzeń:
 *     SDA – dane (tu jadą bity),
 *     SCL – zegar (mówi KIEDY czytać bit).
 *
 *     ESP32 ---SDA-----+-----------+----
 *           ---SCL---+-|---------+-|----
 *                    | |         | |
 *                 [MPU6050]   [laser]
 *                  adr 0x68   adr 0x29
 *
 *   Każde urządzenie ma ADRES – jak numer mieszkania w bloku.
 *   ESP „puka” pod adres i czeka na odpowiedź „jestem!”
 *   (to się nazywa ACK). Brak odpowiedzi = nikogo tam nie ma.
 *   Skaner puka po kolei pod adresy 1..126.
 *   Adresy piszemy szesnastkowo: 0x29 to 41, 0x68 to 104.
 *
 *  ZADANIA (szukaj w kodzie napisu ZADANIE)
 *   1. Dopisz nazwę urządzenia dla adresu 0x68.
 *   2. Policz znalezione urządzenia i wypisz ich liczbę.
 *   3. Odłącz jeden czujnik (przy wyłączonym zasilaniu!)
 *      i uruchom skaner jeszcze raz. Co się zmieniło?
 */

#include <Wire.h>

// ================== KONFIGURACJA ==================
const int PIN_SDA = 6;
const int PIN_SCL = 7;
const int CO_ILE_MS = 5000;      // skanuj co 5 sekund
// ==================================================

// Zamienia adres na nazwę urządzenia.
String nazwa(int adres) {
  if (adres == 0x29) return "laser VL53L1X (odległość)";
  // ZADANIE 1: dopisz podobną linijkę dla adresu 0x68.
  // To czujnik MPU6050 (żyroskop i akcelerometr).

  return "nieznane urządzenie";
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(PIN_SDA, PIN_SCL);   // włączamy I2C na naszych pinach
  Serial.println("L05: skaner I2C");
}

void loop() {
  Serial.println("Skanuję adresy 1..126 ...");
  int znalezione = 0;

  for (int adres = 1; adres <= 126; adres++) {
    Wire.beginTransmission(adres);        // „puk, puk” pod ten adres
    int wynik = Wire.endTransmission();   // 0 = ktoś odpowiedział (ACK)
    if (wynik == 0) {
      Serial.print("  Znalazłem 0x");
      if (adres < 16) Serial.print("0");
      Serial.print(adres, HEX);
      Serial.print("  ->  ");
      Serial.println(nazwa(adres));
      // ZADANIE 2 (część a): zwiększ tu znalezione o 1.
    }
  }

  // ZADANIE 2 (część b): wypisz liczbę znalezionych urządzeń.

  Serial.println();
  delay(CO_ILE_MS);
}
