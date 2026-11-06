/*
 * ==============================================================
 *  LEKCJA 07: LASEROWY CZUJNIK ODLEGŁOŚCI
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci
 *  WERSJA UCZNIA
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Jak laser mierzy odległość („czas lotu światła”).
 *   - Jak używać gotowej biblioteki czujnika.
 *   - Kiedy pomiar jest dobry, a kiedy nie.
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Laser VL53L1X: 3,3 V, GND, SDA -> pin 6, SCL -> pin 7.
 *   - Biblioteka „VL53L1X” od Pololu (Menedżer bibliotek).
 *
 *  JAK TO DZIAŁA
 *   Czujnik wysyła błysk niewidzialnego światła i mierzy,
 *   po jakim czasie światło wróci. Jak echo w górach –
 *   tylko światło jest milion razy szybsze od dźwięku!
 *
 *    [laser] ----błysk---->  |
 *            <---odbicie---  |  podłoga / dłoń
 *            |<-- odległość ->|
 *
 *   odległość = prędkość światła × czas / 2   (tam i z powrotem)
 *   Światło pokonuje 1 mm w ok. 3 bilionowe części sekundy (3 ps)!
 *
 *   Tryb Short: do ok. 1,3 m, mniej czuły na słońce.
 *   Budżet czasu 33 ms: tyle trwa jeden pomiar (ok. 30 na sekundę).
 *   STATUS 0 = dobry pomiar. Inna liczba = coś nie tak
 *   (za blisko, za daleko, za słabe odbicie).
 *
 *  ZADANIA (szukaj w kodzie napisu ZADANIE)
 *   1. Zmierz odległość do dłoni. Zapisz ją w DLON_MM.
 *   2. Zbliżaj dłoń powoli. Znajdź najmniejszą odległość, przy której
 *      status jest jeszcze 0. Zapisz ją w NAJBLIZEJ_MM.
 *   3. Porównaj jasną i ciemną kartkę: wypisz siłę sygnału.
 */

#include <Wire.h>
#include <VL53L1X.h>

// ================== KONFIGURACJA ==================
const int PIN_SDA = 6;
const int PIN_SCL = 7;

// ZADANIE 1: wpisz odległość do dłoni [mm]. Pojawi się na wykresie.
const int DLON_MM = 0;
// ZADANIE 2: wpisz najmniejszą odległość, przy której status = 0.
const int NAJBLIZEJ_MM = 0;
// ==================================================

VL53L1X laser;   // nasz czujnik

// Uruchamia laser. Zwraca false, gdy się nie udało.
bool laserStart() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);                     // I2C szybko: 400 kHz
  laser.setTimeout(500);
  if (!laser.init()) return false;
  laser.setDistanceMode(VL53L1X::Short);     // tryb krótki
  laser.setMeasurementTimingBudget(33000);   // 33 ms (w mikrosekundach)
  laser.startContinuous(33);                 // mierz bez przerwy
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  while (!laserStart()) {
    Serial.println("Nie widzę lasera! Sprawdź kable i uruchom skaner L05.");
    delay(2000);
  }
  Serial.println("L07: laser działa.");
}

void loop() {
  if (laser.dataReady()) {           // nowy pomiar gotowy?
    laser.read(false);               // odczytaj go (bez czekania)
    Serial.print("odleglosc_mm:");
    Serial.print(laser.ranging_data.range_mm);
    Serial.print(",status:");
    Serial.print(laser.ranging_data.range_status);
    Serial.print(",dlon:");
    Serial.print(DLON_MM);
    Serial.print(",najblizej:");
    Serial.print(NAJBLIZEJ_MM);
    // ZADANIE 3: usuń // z dwóch linii niżej. Pokażą siłę odbitego
    // światła. Porównaj jasną i ciemną kartkę.
    // Serial.print(",sygnal:");
    // Serial.print(laser.ranging_data.peak_signal_count_rate_MCPS);
    Serial.println();
  }
}
