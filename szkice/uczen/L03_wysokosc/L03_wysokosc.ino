/*
 * ==============================================================
 *  LEKCJA 03: POMIAR WYSOKOŚCI LASEREM
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 2
 *  WERSJA UCZNIA
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Jak laser mierzy odległość („czas lotu światła”).
 *   - Jak używać gotowej biblioteki czujnika.
 *   - Kiedy pomiar jest dobry, a kiedy nie.
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Czujnik VL53L0X na dronie, patrzy w dół na podłogę.
 *   - Kabel z drona (ok. 1 m) wpięty w płytkę stykową:
 *     czerwony = 3V3, czarny = GND, żółty = SDA -> pin 6,
 *     niebieski = SCL -> pin 7.
 *   - Biblioteka „VL53L0X” od Pololu (Menedżer bibliotek).
 *
 *  JAK TO DZIAŁA
 *   Czujnik wysyła błysk niewidzialnego światła i mierzy,
 *   po jakim czasie światło wróci. Jak echo w górach –
 *   tylko światło jest prawie milion razy szybsze od dźwięku!
 *
 *    [laser] ----błysk---->  |
 *            <---odbicie---  |  podłoga / dłoń
 *            |<-- odległość ->|
 *
 *   odległość = prędkość światła × czas / 2   (tam i z powrotem)
 *   Światło pokonuje 1 mm w ok. 3 bilionowe części sekundy (3 ps)!
 *
 *   Zasięg: do ok. 1,2 m. Budżet czasu 33 ms: tyle trwa jeden pomiar
 *   (ok. 30 na sekundę). Gdy czujnik nic nie widzi (za daleko),
 *   podaje ok. 8190 mm. Taki pomiar pokazujemy na wykresie jako 0.
 *
 *  ZADANIA (szukaj w kodzie napisu ZADANIE)
 *   1. Zmierz odległość do dłoni. Zapisz ją w DLON_MM.
 *   2. Zbliżaj dłoń powoli. Przy jakiej najmniejszej odległości wynik
 *      jeszcze zgadza się z linijką? Zapisz ją w NAJBLIZEJ_MM.
 *   3. Z jakiej największej odległości czujnik widzi jasną kartkę?
 *      Zapisz w NAJDALEJ_MM. Potem sprawdź ciemną – widzi z daleka?
 */

#include <Wire.h>
#include <VL53L0X.h>

// ================== KONFIGURACJA ==================
const int PIN_SDA = 6;
const int PIN_SCL = 7;

// ZADANIE 1: wpisz odległość do dłoni [mm]. Pojawi się na wykresie.
const int DLON_MM = 0;
// ZADANIE 2: najmniejsza odległość, przy której wynik zgadza się z linijką.
const int NAJBLIZEJ_MM = 0;
// ZADANIE 3: największa odległość, z której czujnik widzi jasną kartkę.
const int NAJDALEJ_MM = 0;
// ==================================================

const int NIC_NIE_WIDZI_MM = 8000;   // VL53L0X daje ok. 8190, gdy nic nie widzi

VL53L0X laser;   // nasz czujnik

// Uruchamia laser. Zwraca false, gdy się nie udało.
bool laserStart() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);   // 100 kHz: kabel I2C ma ok. 1 m, więc wolniej i pewniej
  Wire.setTimeOut(50);     // nie czekaj w nieskończoność, gdy czujnik milczy
  laser.setTimeout(500);
  if (!laser.init()) return false;
  laser.setMeasurementTimingBudget(33000);   // 33 ms na jeden pomiar (w mikrosekundach)
  laser.startContinuous();                   // mierz bez przerwy
  return true;
}

// Czy czujnik ma już nowy pomiar? Pytamy rejestr przerwania w czujniku,
// więc program nie stoi i nie czeka na pomiar.
bool pomiarGotowy() {
  return (laser.readReg(VL53L0X::RESULT_INTERRUPT_STATUS) & 0x07) != 0;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  while (!laserStart()) {
    Serial.println("Nie widzę lasera! Uruchom skaner L02. Widzi 0x29? Zawołaj prowadzącego.");
    delay(2000);
  }
  Serial.println("L03: laser działa.");
}

void loop() {
  if (pomiarGotowy()) {
    int mm = laser.readRangeContinuousMillimeters();
    bool dobry = !laser.timeoutOccurred() && mm < NIC_NIE_WIDZI_MM;
    Serial.print("odleglosc_mm:");
    Serial.print(dobry ? mm : 0);      // 0 = nic nie widzi
    Serial.print(",dlon:");
    Serial.print(DLON_MM);
    Serial.print(",najblizej:");
    Serial.print(NAJBLIZEJ_MM);
    Serial.print(",najdalej:");
    Serial.println(NAJDALEJ_MM);
  }
}
