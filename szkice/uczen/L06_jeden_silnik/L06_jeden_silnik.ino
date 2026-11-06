/*
 * ==============================================================
 *  LEKCJA 06: JEDEN SILNIK
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 3
 *  WERSJA UCZNIA
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Jak PWM steruje mocą silnika.
 *   - Jak wysyłać komendy z Serial Monitora do płytki.
 *   - Po co jest RAMPA (moc rośnie powoli) i limit MAX_MOC.
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Dron na prowadnicy, zasilanie z kabla, USB do komputera.
 *   - Silnik M1 na pinie 18 (przez moduł ESC).
 *
 *  JAK TO DZIAŁA
 *   Silnik to „duża dioda”: im większe wypełnienie PWM,
 *   tym szybciej się kręci. Moduł ESC (w środku tranzystor MOSFET)
 *   to szybki włącznik, który przełącza duży prąd silnika.
 *
 *   ESP32 --PWM--> [ESC] ==prąd==> (M1)
 *
 *   RAMPA: jak pedał gazu w samochodzie – wciskasz powoli,
 *   a nie od razu do podłogi. Na wykresie widać „schodek”
 *   zamieniony na pochylnię:
 *
 *    zadana:  ______          moc:      ______
 *            |                         /
 *       _____|               _______ /
 *
 *  BEZPIECZEŃSTWO
 *   - Silniki startują ROZBROJONE (dioda czerwona).
 *   - "u" + Enter = uzbrój (dioda zielona).
 *   - "x" albo spacja = STOP. Przycisk BOOT na płytce = STOP.
 *   - Program nigdy nie da więcej niż MAX_MOC.
 *
 *  KOMENDY (Serial Monitor, 115200, „Nowa linia”)
 *   u      – uzbrój
 *   x      – STOP
 *   0..100 – moc silnika M1 w procentach, np. 25
 *
 *  ZADANIA (szukaj w kodzie napisu ZADANIE)
 *   1. Znajdź moc, przy której śmigło rusza. Zapisz ją w MOC_STARTU.
 *   2. Ustaw PWM_HZ = 200 i posłuchaj silnika. Potem wróć do 20000.
 *   3. Zmień RAMPA i zobacz na Plotterze, jak zmienia się pochylnia.
 */

// ================== KONFIGURACJA ==================
#define TRYB_MOSFET          // moduły ESC sterowane PWM 20 kHz (domyślnie)
// #define TRYB_ESC_RC       // albo regulatory ESC (impulsy 1000-2000 us)
                             // Zostaw odkomentowany TYLKO JEDEN tryb!

const int PINY_SILNIKOW[4] = {18, 19, 20, 21};   // M1, M2, M3, M4
const int PIN_STOP = 9;          // przycisk BOOT = STOP awaryjny
const float MAX_MOC = 60;        // [%] tej mocy program nigdy nie przekroczy

// ZADANIE 3: jak szybko może rosnąć moc [% na sekundę].
// Spróbuj 5, potem 40. Obserwuj na Plotterze linie zadana i moc.
const float RAMPA = 20;

// ZADANIE 2: zmień 20000 na 200, wgraj i posłuchaj silnika.
// Potem KONIECZNIE wróć do 20000.
const int PWM_HZ = 20000;        // TRYB_MOSFET: częstotliwość PWM
const int PWM_BITY = 10;         // 10 bitów = liczby 0..1023
const int ESC_HZ = 50;           // TRYB_ESC_RC: 50 impulsów na sekundę
const int ESC_BITY = 14;

// ZADANIE 1: wpisuj moc 5, 6, 7... aż śmigło ruszy.
// Wpisz tu tę moc [%]. Na Plotterze pojawi się jako linia „start”.
const int MOC_STARTU = 0;
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
unsigned long czas_wykresu = 0;

void obsluzKomende() {
  if (komenda == 'x') {
    stop("komenda x");
  } else if (komenda == 'u') {
    uzbroj();
  } else if (komenda == 'm') {
    if (!uzbrojony) {
      Serial.println("Najpierw uzbrój: wpisz u");
      return;
    }
    moc_zadana[0] = constrain(komenda_liczba, 0, MAX_MOC);
    Serial.print("Nowa moc M1 [%]: ");
    Serial.println(moc_zadana[0]);
  } else {
    Serial.println("Znam komendy: u, x, liczba 0-100");
  }
}

void setup() {
  silnikiNaZero();           // NAJPIERW silniki na 0!
  Serial.begin(115200);
  delay(1000);
  silnikiStart();
  Serial.println("L06: jeden silnik. Wpisz u, potem moc, np. 20.");
}

void loop() {
  if (czytajKomende()) obsluzKomende();
  sprawdzPrzycisk();
  rampa();

  if (millis() - czas_wykresu >= 50) {   // wykres 20 razy na sekundę
    czas_wykresu = millis();
    Serial.print("zadana:");
    Serial.print(moc_zadana[0]);
    Serial.print(",moc:");
    Serial.print(moc_teraz[0]);
    Serial.print(",start:");
    Serial.println(MOC_STARTU);
  }
}
