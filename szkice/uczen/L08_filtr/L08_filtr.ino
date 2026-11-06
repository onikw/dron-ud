/*
 * ==============================================================
 *  LEKCJA 08: FILTR – JAK WYGŁADZIĆ SKACZĄCY POMIAR
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 4
 *  WERSJA UCZNIA
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Dlaczego pomiary „skaczą” (szum).
 *   - Średnia z N ostatnich pomiarów.
 *   - Filtr wykładniczy (z liczbą alfa).
 *   - Że gładszy wykres = większe opóźnienie.
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Laser VL53L1X: 3,3 V, GND, SDA -> pin 6, SCL -> pin 7.
 *
 *  JAK TO DZIAŁA
 *   Na Plotterze są 3 linie z TEGO SAMEGO pomiaru:
 *    surowy      – prosto z czujnika, skacze,
 *    srednia     – średnia z N ostatnich pomiarów,
 *    wykladniczy – nowy = alfa × pomiar + (1 − alfa) × stary.
 *
 *   ŚREDNIA: jak ocena na koniec roku – liczysz kilka ostatnich
 *   ocen, więc jedna pała nie zmienia wszystkiego.
 *
 *   WYKŁADNICZY: jak plotka – każda nowa wiadomość trochę
 *   zmienia zdanie. alfa = 1: wierzysz tylko nowej (bez filtra).
 *   alfa = 0,1: zmieniasz zdanie powoli (mocny filtr).
 *
 *    surowy:   /\/\_/\/\__/\/\/\
 *    filtr:    ~~~~~~~~~~~~~~~~~   gładko, ale ze spóźnieniem
 *
 *  ZADANIA (szukaj w kodzie napisu ZADANIE)
 *   1. Zmień N (np. 2, 10, 20). Co się dzieje z linią srednia?
 *   2. Zmień ALFA (np. 0.8, 0.3, 0.05).
 *   3. Szybko ruszaj dłonią. Która linia spóźnia się najbardziej?
 */

#include <Wire.h>
#include <VL53L1X.h>

// ================== KONFIGURACJA ==================
const int PIN_SDA = 6;
const int PIN_SCL = 7;

// ZADANIE 1: z ilu ostatnich pomiarów liczymy średnią (1..20)
const int N = 5;
// ZADANIE 2: siła filtra wykładniczego (0..1). Mniej = gładziej.
const float ALFA = 0.5;
static_assert(N >= 1 && N <= 20, "N musi być od 1 do 20");
// ==================================================

VL53L1X laser;

int ostatnie[20];      // tablica na ostatnie pomiary
int gdzie = 0;         // do której szufladki wpisać następny
float wykladniczy = 0;
bool pierwszy = true;

bool laserStart() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);   // 100 kHz: kabel I2C ma ok. 1 m, więc wolniej i pewniej
  Wire.setTimeOut(50);     // nie czekaj w nieskończoność, gdy czujnik milczy
  laser.setTimeout(500);
  if (!laser.init()) return false;
  laser.setDistanceMode(VL53L1X::Short);
  laser.setMeasurementTimingBudget(33000);
  laser.startContinuous(33);
  return true;
}

// Średnia z N ostatnich pomiarów.
float srednia(int pomiar) {
  ostatnie[gdzie] = pomiar;          // wkładamy nowy pomiar
  gdzie = (gdzie + 1) % N;           // następna szufladka (w kółko)
  long suma = 0;
  for (int i = 0; i < N; i++) suma += ostatnie[i];
  return suma / (float)N;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  while (!laserStart()) {
    Serial.println("Nie widzę lasera! Uruchom skaner L02. Widzi 0x29? To może być inny model czujnika – zawołaj prowadzącego.");
    delay(2000);
  }
}

void loop() {
  if (laser.dataReady()) {
    int surowy = laser.read(false);
    if (pierwszy) {                  // na start: wszystko = pierwszy pomiar
      for (int i = 0; i < 20; i++) ostatnie[i] = surowy;
      wykladniczy = surowy;
      pierwszy = false;
    }

    float sr = srednia(surowy);
    // Filtr wykładniczy: trochę nowego pomiaru + dużo starej wartości
    wykladniczy = ALFA * surowy + (1 - ALFA) * wykladniczy;

    Serial.print("surowy:");
    Serial.print(surowy);
    Serial.print(",srednia:");
    Serial.print(sr);
    Serial.print(",wykladniczy:");
    Serial.println(wykladniczy);
  }
}
