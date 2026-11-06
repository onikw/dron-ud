/*
 * ==============================================================
 *  LEKCJA 02: SKANER I2C – KTO JEST NA MAGISTRALI?
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 2
 *  WERSJA PROWADZĄCEGO (z rozwiązaniami)
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Co to jest I2C (czyt. „i kwadrat ce”).
 *   - Co to jest adres urządzenia.
 *   - Jak sprawdzić, czy czujniki są dobrze podłączone.
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Na dronie: MPU6050 i laser VL53L0X (oba na tych samych 4 przewodach).
 *   - Kabel z drona (ok. 1 m) wpięty w płytkę stykową:
 *     czerwony = 3V3, czarny = GND, żółty = SDA -> pin 6,
 *     niebieski = SCL -> pin 7.
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
 *  ZADANIA
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
  if (adres == 0x29) return "laser VL53L0X (odległość)";
  // ROZWIĄZANIE 1:
  if (adres == 0x68) return "MPU6050 (żyroskop i akcelerometr)";
  if (adres == 0x69) return "MPU6050 (adres zapasowy, AD0 = 1)";
  return "nieznane urządzenie";
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);   // 100 kHz: kabel I2C ma ok. 1 m, więc wolniej i pewniej
  Wire.setTimeOut(50);     // nie czekaj w nieskończoność, gdy czujnik milczy   // włączamy I2C na naszych pinach
  Serial.println("L02: skaner I2C");
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
      // ROZWIĄZANIE 2:
      znalezione++;
    }
  }

  // ROZWIĄZANIE 2:
  Serial.print("Liczba urządzeń: ");
  Serial.println(znalezione);
  if (znalezione == 0) {
    Serial.println("Nikogo nie ma! Sprawdź kable SDA, SCL, 3,3 V i GND.");
  }
  Serial.println();
  delay(CO_ILE_MS);
}
