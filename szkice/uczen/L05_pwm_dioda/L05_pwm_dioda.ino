/*
 * ==============================================================
 *  LEKCJA 05: PWM – JAK „ŚCISZYĆ” DIODĘ
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 3
 *  WERSJA UCZNIA
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Co to jest PWM i wypełnienie.
 *   - Dlaczego szybkie mruganie oko widzi jako słabsze światło.
 *   - Jak zrobić PWM sprzętem (moduł LEDC w ESP32).
 *   - Jak oglądać dane na wykresie (Serial Plotter).
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Kabel USB. Dioda RGB jest na płytce.
 *   - (opcjonalnie) zewnętrzna dioda: pin LED_ZEW -> rezystor 220 Ω
 *     -> dłuższa nóżka diody, krótsza nóżka -> GND.
 *
 *  JAK TO DZIAŁA
 *   Pin umie tylko dwie rzeczy: włączony (1) albo wyłączony (0).
 *   Żeby dioda świeciła „w połowie”, szybko ją włączamy i wyłączamy.
 *   WYPEŁNIENIE mówi, przez jaką część czasu jest włączona.
 *   Analogia: wentylator, który włączasz na chwilę co chwilę –
 *   im dłużej włączony, tym mocniej dmucha „średnio”.
 *
 *    25%:  _       _       _
 *         | |_____| |_____| |_____
 *    50%:  ___     ___     ___
 *         |   |___|   |___|   |___
 *    75%:  _____   _____   _____
 *         |     |_|     |_|     |_
 *         |<----->|
 *           okres
 *
 *   Program ma 3 części:
 *    1) PWM „ręcznie” i bardzo wolno (okres 2 s) – widać włącz/wyłącz,
 *       a na Plotterze prostokąt 0/1.
 *    2) Okres coraz krótszy – aż oko widzi tylko jasność.
 *    3) PWM sprzętowy (LEDC) – płytka mruga sama, 5000 razy na sekundę.
 *   Otwórz: Narzędzia -> Serial Plotter.
 *
 *  ZADANIA (szukaj w kodzie napisu ZADANIE)
 *   1. Zmień WYPELNIENIE (np. 10, 50, 90) i obserwuj wykres.
 *   2. Zmień okres najwolniejszego etapu.
 *   3. Zrób „oddychającą” diodę: jasność płynnie rośnie i maleje.
 */

// ================== KONFIGURACJA ==================
#ifdef RGB_BUILTIN
const int PIN_DIODY = RGB_BUILTIN;   // kolorowa dioda na płytce
#else
const int PIN_DIODY = LED_BUILTIN;
#endif
const int LED_ZEW = 22;          // zewnętrzna dioda (opcjonalna), wolny pin
const int JASNOSC_RGB = 40;      // 0..255, dioda RGB jest bardzo jasna!

// ZADANIE 1: wypełnienie w procentach (0..100). Spróbuj 10, 50, 90.
const int WYPELNIENIE = 50;

// ZADANIE 2: okresy kolejnych etapów w milisekundach.
// Zmień pierwszy (najwolniejszy) etap, np. na 4000.
const int OKRESY_MS[] = {2000, 1000, 500, 200, 100, 50, 20, 10, 5};
const int ILE_ETAPOW = 9;

const int LEDC_HZ = 5000;        // PWM sprzętowy: 5000 okresów na sekundę
const int LEDC_BITY = 8;         // 8 bitów = liczby 0..255
// ==================================================

bool stan_diody = false;

// Włącza albo wyłącza obie diody naraz.
void ustawDiode(bool wlaczona) {
  if (wlaczona == stan_diody) return;   // nic się nie zmienia
  stan_diody = wlaczona;
  digitalWrite(LED_ZEW, wlaczona ? HIGH : LOW);
#ifdef RGB_BUILTIN
  int j = wlaczona ? JASNOSC_RGB : 0;
  rgbLedWrite(PIN_DIODY, j, j, j);
#else
  digitalWrite(PIN_DIODY, wlaczona ? HIGH : LOW);
#endif
}

// Jeden etap ręcznego PWM. micros() to zegar w mikrosekundach.
// Reszta z dzielenia (%) mówi, w którym miejscu okresu jesteśmy.
void etapReczny(long okres_ms, long czas_etapu_ms) {
  long okres_us = okres_ms * 1000;
  long wlaczona_us = okres_us * WYPELNIENIE / 100;
  unsigned long start = millis();
  unsigned long ostatni_wykres = 0;
  while (millis() - start < czas_etapu_ms) {
    long miejsce = micros() % okres_us;
    ustawDiode(miejsce < wlaczona_us);
    if (millis() - ostatni_wykres >= 20) {   // punkt wykresu co 20 ms
      ostatni_wykres = millis();
      Serial.print("stan:");
      Serial.print(stan_diody ? 1 : 0);
      Serial.print(",srednia:");
      Serial.println(WYPELNIENIE / 100.0);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_ZEW, OUTPUT);
  pinMode(PIN_DIODY, OUTPUT);
  ustawDiode(true);
  ustawDiode(false);

  // Części 1 i 2: PWM ręcznie, coraz szybciej
  for (int i = 0; i < ILE_ETAPOW; i++) {
    long czas = max(2L * OKRESY_MS[i], 4000L);   // co najmniej 4 s
    etapReczny(OKRESY_MS[i], czas);
  }
  ustawDiode(false);

  // Część 3: PWM sprzętowy. LEDC sam mruga pinem, a procesor jest wolny.
  ledcAttach(LED_ZEW, LEDC_HZ, LEDC_BITY);
}

// Ustawia jasność 0..100% na obu diodach.
// Dioda RGB ma w środku własny mały sterownik PWM.
void jasnosc(int procent) {
  ledcWrite(LED_ZEW, procent * 255 / 100);
#ifdef RGB_BUILTIN
  int j = procent * JASNOSC_RGB / 100;
  rgbLedWrite(PIN_DIODY, j, j, j);
#endif
  Serial.print("jasnosc:");
  Serial.println(procent);
}

void loop() {
  // ZADANIE 3: „oddychająca” dioda.
  // Teraz jasność stoi w miejscu. Zrób dwie pętle for:
  // jedną od 0 do 100, drugą od 100 do 0, z delay(15) w środku.
  jasnosc(WYPELNIENIE);
  delay(15);
}
