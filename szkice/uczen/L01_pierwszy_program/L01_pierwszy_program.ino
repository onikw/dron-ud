/*
 * ==============================================================
 *  LEKCJA 01: PIERWSZY PROGRAM – SERIAL I DIODA
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 2
 *  WERSJA UCZNIA
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
 *  ZADANIA (szukaj w kodzie napisu ZADANIE)
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

// ZADANIE 1: zmień tempo mrugania. 1000 ms = 1 sekunda.
// Spróbuj 100, potem 2000. Co widzisz?
const int TEMPO_MS = 500;

// ZADANIE 2 (część a): gdy funkcja sygnalSOS() będzie gotowa,
// zmień false na true.
const bool TRYB_SOS = false;
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

// ZADANIE 2 (część b): SOS = 3 krótkie, 3 długie, 3 krótkie.
// Na razie są tylko 3 krótkie. Dopisz resztę.
// Podpowiedź: pętla for powtarza coś kilka razy:
//   for (int i = 0; i < 3; i++) mrugnij(200);
void sygnalSOS() {
  for (int i = 0; i < 3; i++) mrugnij(200);   // S: kropki

  delay(1500);                                // przerwa przed kolejnym SOS
}

void loop() {
  // ZADANIE 3: zwiększ licznik o 1 i wypisz go, np.:
  //   licznik = licznik + 1;
  //   Serial.print("Mrugnięcie numer: ");
  //   Serial.println(licznik);

  if (TRYB_SOS) {
    sygnalSOS();
  } else {
    mrugnij(TEMPO_MS);
  }
}
