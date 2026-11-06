/*
 * ==============================================================
 *  LEKCJA 01: PIERWSZY PROGRAM – SERIAL I DIODA
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 2
 *  WERSJA PROWADZĄCEGO (z rozwiązaniami)
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Każdy program Arduino ma dwie części: setup() i loop().
 *   - Jak włączyć i wyłączyć diodę.
 *   - Jak płytka „pisze” do komputera (Serial Monitor).
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Tylko kabel USB. Dioda jest już na płytce.
 *
 *  JAK TO DZIAŁA
 *   setup() wykonuje się RAZ, zaraz po włączeniu.
 *   loop() wykonuje się w kółko, bez końca.
 *
 *      start
 *        |
 *     setup()        <- raz
 *        |
 *     loop()  <--+   <- w kółko
 *        |       |
 *        +-------+
 *
 *   SERIAL (UART) to rozmowa płytki z komputerem po kablu.
 *   Płytka wysyła bity (zera i jedynki) jeden po drugim,
 *   z prędkością 115200 bitów na sekundę. Komputer pokazuje
 *   je jako tekst w Serial Monitorze (lupa w prawym górnym rogu).
 *   Obie strony muszą „mówić” w tym samym tempie: 115200.
 *
 *  ZADANIA
 *   1. Zmień tempo mrugania (TEMPO_MS).
 *   2. Zrób sygnał SOS:  ... --- ...  (3 krótkie, 3 długie, 3 krótkie).
 *   3. Wypisuj licznik mrugnięć w Serial Monitorze.
 */

// ================== KONFIGURACJA ==================
#ifdef RGB_BUILTIN
const int PIN_DIODY = RGB_BUILTIN;   // kolorowa dioda na płytce
#else
const int PIN_DIODY = LED_BUILTIN;   // zwykła dioda na płytce
#endif

// ROZWIĄZANIE 1: tempo mrugania w milisekundach (1000 ms = 1 sekunda)
const int TEMPO_MS = 300;

// ROZWIĄZANIE 2: true = nadajemy SOS, false = zwykłe mruganie
const bool TRYB_SOS = true;
// ==================================================

int licznik = 0;   // zmienna: pudełko na liczbę, która może się zmieniać

void setup() {
  Serial.begin(115200);   // otwieramy „rozmowę” z komputerem
  delay(1000);            // chwila, żeby komputer zdążył się połączyć
  pinMode(PIN_DIODY, OUTPUT);
  Serial.println("Cześć! Tu dron. Zaczynam mrugać.");
}

// Jedno mrugnięcie: włącz, poczekaj, wyłącz, poczekaj.
void mrugnij(int czas_ms) {
  digitalWrite(PIN_DIODY, HIGH);   // HIGH = włączona
  delay(czas_ms);                  // delay = czekaj (w ms)
  digitalWrite(PIN_DIODY, LOW);    // LOW = wyłączona
  delay(czas_ms);
}

// ROZWIĄZANIE 2: SOS = 3 krótkie, 3 długie, 3 krótkie
void sygnalSOS() {
  for (int i = 0; i < 3; i++) mrugnij(200);   // S: kropki
  for (int i = 0; i < 3; i++) mrugnij(600);   // O: kreski
  for (int i = 0; i < 3; i++) mrugnij(200);   // S: kropki
  delay(1500);                                // przerwa przed kolejnym SOS
}

void loop() {
  // ROZWIĄZANIE 3: licznik rośnie o 1 przy każdym obrocie pętli
  licznik = licznik + 1;
  Serial.print("Mrugnięcie numer: ");
  Serial.println(licznik);

  if (TRYB_SOS) {
    sygnalSOS();
  } else {
    mrugnij(TEMPO_MS);
  }
}
