/*
 * ==============================================================
 *  LEKCJA 07: CZTERY SILNIKI
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 3
 *  WERSJA UCZNIA
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Co to jest TABLICA: jedna nazwa, kilka szufladek z numerami.
 *   - Jak pętla for robi to samo dla każdego silnika.
 *   - Jak sprawdzić, czy silniki są dobrze podłączone.
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Dron na prowadnicy, zasilanie z kabla, USB do komputera.
 *   - Silniki M1..M4 na pinach 18, 19, 20, 21.
 *
 *  JAK TO DZIAŁA
 *   Tablica PINY_SILNIKOW to 4 szufladki. Numeracja od ZERA:
 *     PINY_SILNIKOW[0] = 18  (M1)
 *     PINY_SILNIKOW[1] = 19  (M2) ...
 *   Pętla for (int i = 0; i < 4; i++) { ... } robi coś
 *   dla i = 0, 1, 2, 3 – czyli dla każdego silnika.
 *
 *   Widok drona z góry:            CW  = jak wskazówki zegara
 *            PRZÓD                 CCW = przeciwnie do zegara
 *      M1 (CW)     M2 (CCW)
 *          \       /
 *           [ ESP ]
 *          /       \
 *      M4 (CCW)    M3 (CW)
 *   Silniki po przekątnej kręcą się w tę samą stronę.
 *   Dzięki temu dron nie obraca się wokół własnej osi.
 *   Zły kierunek? Zamień miejscami dwa kable tego silnika.
 *
 *  BEZPIECZEŃSTWO
 *   - Start ROZBROJONY (czerwona). "u" = uzbrój (zielona).
 *   - "x" albo spacja = STOP. Przycisk BOOT = STOP.
 *   - Program nigdy nie da więcej niż MAX_MOC.
 *
 *  KOMENDY
 *   u – uzbrój        x – STOP
 *   t – test kolejności silników
 *   r – rozpędzanie i zwalnianie wszystkich
 *   s – schody
 *
 *  ZADANIA (szukaj w kodzie napisu ZADANIE)
 *   1. Zmień czas rozpędzania (CZAS_ROZPEDZANIA_MS).
 *   2. Zrób „schody”: moc 10, 20, 30, 40%, każda przez 1,5 s.
 */

// ================== KONFIGURACJA ==================
#define TRYB_MOSFET          // moduły ESC sterowane PWM 20 kHz (domyślnie)
// #define TRYB_ESC_RC       // albo regulatory ESC (impulsy 1000-2000 us)
                             // Zostaw odkomentowany TYLKO JEDEN tryb!

const int PINY_SILNIKOW[4] = {18, 19, 20, 21};   // M1, M2, M3, M4
const int PIN_STOP = 9;          // przycisk BOOT = STOP awaryjny
const float MAX_MOC = 60;        // [%] tej mocy program nigdy nie przekroczy
const float RAMPA = 30;          // [% na sekundę] najszybsza zmiana mocy

const int PWM_HZ = 20000;        // TRYB_MOSFET: częstotliwość PWM
const int PWM_BITY = 10;         // 10 bitów = liczby 0..1023
const int ESC_HZ = 50;           // TRYB_ESC_RC: 50 impulsów na sekundę
const int ESC_BITY = 14;

const float MOC_TESTU = 15;      // [%] niska moc do testu kolejności
const float MOC_POKAZU = 40;     // [%] najwyższa moc przy rozpędzaniu

// ZADANIE 1: czas rozpędzania w ms. Spróbuj 1000 i 8000.
// Przy 1000 RAMPA nie pozwoli rozpędzić się tak szybko – sprawdź na Plotterze!
const int CZAS_ROZPEDZANIA_MS = 3000;

const String GDZIE[4] = {"przód-lewy", "przód-prawy", "tył-prawy", "tył-lewy"};
const String KIERUNEK[4] = {"CW (jak zegar)", "CCW (przeciwnie do zegara)",
                            "CW (jak zegar)", "CCW (przeciwnie do zegara)"};
// ==================================================

#if defined(TRYB_MOSFET) && defined(TRYB_ESC_RC)
#error "Wybierz tylko jeden tryb: TRYB_MOSFET albo TRYB_ESC_RC"
#endif

// ================== SILNIKI I BEZPIECZEŃSTWO ==================
// Ta część jest taka sama we wszystkich lekcjach z silnikami.

bool uzbrojony = false;                 // false = silniki zablokowane
float moc_zadana[4] = {0, 0, 0, 0};     // [%] do tej mocy dążymy
float moc_teraz[4] = {0, 0, 0, 0};      // [%] tyle jest teraz (po rampie)
unsigned long czas_rampy = 0;

// Dioda: czerwona = rozbrojony, zielona = uzbrojony.
void pokazStan() {
#ifdef RGB_BUILTIN
  if (uzbrojony) rgbLedWrite(RGB_BUILTIN, 0, 30, 0);
  else rgbLedWrite(RGB_BUILTIN, 30, 0, 0);
#else
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, uzbrojony ? HIGH : LOW);
#endif
}

// Pierwsza rzecz w setup(): wszystkie piny silników na 0.
void silnikiNaZero() {
  for (int i = 0; i < 4; i++) {
    pinMode(PINY_SILNIKOW[i], OUTPUT);
    digitalWrite(PINY_SILNIKOW[i], LOW);
  }
}

// Wysyła moc [%] na silnik nr 0..3 (to M1..M4).
void wyslijMoc(int nr, float moc) {
  if (!uzbrojony) moc = 0;
  moc = constrain(moc, 0, MAX_MOC);
#ifdef TRYB_ESC_RC
  float impuls_us = 1000 + moc * 10;    // 0% = 1000 us, 100% = 2000 us
  uint32_t wypelnienie = impuls_us * ESC_HZ * (1 << ESC_BITY) / 1000000.0;
#else
  uint32_t wypelnienie = moc / 100.0 * ((1 << PWM_BITY) - 1);
#endif
  ledcWrite(PINY_SILNIKOW[nr], wypelnienie);
}

// Włącza PWM sprzętowy (LEDC) na pinach silników, z mocą 0.
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
  czas_rampy = millis();
  pokazStan();
}

// RAMPA: moc zmienia się najwyżej o RAMPA % na sekundę.
void rampa() {
  float dt = (millis() - czas_rampy) / 1000.0;
  czas_rampy = millis();
  float krok = RAMPA * dt;
  for (int i = 0; i < 4; i++) {
    float cel = uzbrojony ? constrain(moc_zadana[i], 0, MAX_MOC) : 0;
    if (moc_teraz[i] < cel) moc_teraz[i] = min(moc_teraz[i] + krok, cel);
    else moc_teraz[i] = max(moc_teraz[i] - krok, cel);
    wyslijMoc(i, moc_teraz[i]);
  }
}

void ustawWszystkie(float moc) {
  for (int i = 0; i < 4; i++) moc_zadana[i] = constrain(moc, 0, MAX_MOC);
}

void uzbroj() {
  uzbrojony = true;
  Serial.println("UZBROJONY! Uwaga na śmigła.");
  pokazStan();
}

// STOP: od razu wyłącza wszystkie silniki i mówi dlaczego.
void stop(String powod) {
  uzbrojony = false;
  for (int i = 0; i < 4; i++) {
    moc_zadana[i] = 0;
    moc_teraz[i] = 0;
    wyslijMoc(i, 0);
  }
  Serial.println("STOP! Powód: " + powod);
  pokazStan();
}

void sprawdzPrzycisk() {
  if (uzbrojony && digitalRead(PIN_STOP) == LOW) stop("przycisk BOOT");
}

// ================== KOMENDY Z SERIAL MONITORA ==================
// Ta sama funkcja we wszystkich lekcjach. Czyta linię, np. "p 0.05":
// litera trafia do komenda, liczba do komenda_liczba.
// Sama liczba (np. "40") to komenda 'm'. "x" i spacja działają od razu.
char komenda = 0;
float komenda_liczba = 0;
String linia = "";

bool czytajKomende() {
  while (Serial.available() > 0) {
    char znak = Serial.read();
    if (linia.length() == 0 && (znak == 'x' || znak == 'X' || znak == ' ')) {
      komenda = 'x';
      return true;
    }
    if (znak == '\n' || znak == '\r') {
      if (linia.length() == 0) continue;
      komenda = linia.charAt(0);
      if (isDigit(komenda)) {
        komenda = 'm';
        komenda_liczba = linia.toFloat();
      } else {
        komenda_liczba = linia.substring(1).toFloat();
      }
      linia = "";
      return true;
    }
    linia += znak;
  }
  return false;
}

// ================== PROGRAM LEKCJI ==================
char do_zrobienia = 0;           // co mamy zaraz zrobić: 't', 'r' albo 's'
unsigned long czas_wykresu = 0;

void obsluzKomende() {
  if (komenda == 'x') {
    stop("komenda x");
  } else if (komenda == 'u') {
    uzbroj();
  } else if (komenda == 't' || komenda == 'r' || komenda == 's') {
    if (uzbrojony) do_zrobienia = komenda;
    else Serial.println("Najpierw uzbrój: wpisz u");
  } else {
    Serial.println("Znam komendy: u, x, t, r, s");
  }
}

// Pilnuje bezpieczeństwa i rysuje wykres. Wołamy ją cały czas.
void pilnuj() {
  if (czytajKomende()) obsluzKomende();
  sprawdzPrzycisk();
  rampa();
  if (millis() - czas_wykresu >= 50) {
    czas_wykresu = millis();
    for (int i = 0; i < 4; i++) {
      Serial.print("M");
      Serial.print(i + 1);
      Serial.print(":");
      Serial.print(moc_teraz[i]);
      Serial.print(i < 3 ? "," : "\n");
    }
  }
}

// Czeka ms milisekund, ale cały czas pilnuje bezpieczeństwa.
// Zwraca false, gdy w tym czasie był STOP.
bool czekaj(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    pilnuj();
    if (!uzbrojony) return false;
    delay(5);
  }
  return true;
}

void testKolejnosci() {
  Serial.println("TEST KOLEJNOŚCI. Patrz, które śmigło się kręci!");
  for (int i = 0; i < 4; i++) {
    Serial.println("M" + String(i + 1) + " (" + GDZIE[i] + ") powinien kręcić się " + KIERUNEK[i]);
    moc_zadana[i] = MOC_TESTU;
    bool dalej = czekaj(1000);
    moc_zadana[i] = 0;
    if (!dalej || !czekaj(800)) return;
  }
  Serial.println("Test skończony.");
}

void rozpedzanie() {
  Serial.println("ROZPĘDZANIE wszystkich silników.");
  int kroki = 50;
  for (int k = 0; k <= kroki; k++) {
    ustawWszystkie(MOC_POKAZU * k / kroki);
    if (!czekaj(CZAS_ROZPEDZANIA_MS / kroki)) return;
  }
  if (!czekaj(1000)) return;
  Serial.println("ZWALNIANIE.");
  for (int k = kroki; k >= 0; k--) {
    ustawWszystkie(MOC_POKAZU * k / kroki);
    if (!czekaj(CZAS_ROZPEDZANIA_MS / kroki)) return;
  }
}

// ZADANIE 2: „schody” – moc 10, 20, 30, 40%, każda przez 1,5 s.
// Użyj pętli for, funkcji ustawWszystkie(moc) i czekaj(ms).
// Ważne: pisz  if (!czekaj(1500)) return;
// – wtedy schody zatrzymają się po STOP.
void schody() {
  Serial.println("Schody jeszcze nie gotowe – to Twoje ZADANIE 2!");
}

void setup() {
  silnikiNaZero();           // NAJPIERW silniki na 0!
  Serial.begin(115200);
  delay(1000);
  silnikiStart();
  Serial.println("L07: cztery silniki. Wpisz u, potem t, r albo s.");
}

void loop() {
  pilnuj();
  char co = do_zrobienia;
  do_zrobienia = 0;
  if (co == 't') testKolejnosci();
  if (co == 'r') rozpedzanie();
  if (co == 's') schody();
  ustawWszystkie(0);         // po każdym pokazie wracamy do zera
}
