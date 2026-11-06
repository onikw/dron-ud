/*
 * ==============================================================
 *  LEKCJA 09: TRZYMAJ WYSOKOŚĆ – REGULATOR PID
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 4
 *  WERSJA UCZNIA
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Co to jest REGULATOR i SPRZĘŻENIE ZWROTNE.
 *   - Co robią liczby Kp, Ki i Kd.
 *   - Jak dobrać je krok po kroku, patrząc na wykres.
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Dron na prowadnicy, zasilanie z kabla, USB.
 *   - Silniki M1..M4 (piny 18..21), laser patrzy w dół (SDA 6, SCL 7).
 *
 *  JAK TO DZIAŁA
 *   Analogia: PRYSZNIC. Za zimno? Kręcisz kurkiem na ciepło.
 *   Im zimniej, tym mocniej kręcisz. Tak samo dron:
 *
 *     BŁĄD = wysokość zadana − wysokość zmierzona
 *     (dron za nisko -> błąd dodatni -> więcej mocy)
 *
 *     moc = moc_zawisu + Kp·błąd + Ki·suma_błędów + Kd·zmiana_błędu
 *
 *     Kp – „im dalej, tym mocniej”. Główny kurek.
 *     Ki – „ciągle za nisko? dodaję po trochu”. Usuwa stały błąd.
 *     Kd – „zbliżam się szybko? hamuję”. Gasi bujanie.
 *
 *      zadana ---> (+) --błąd--> [ PID ] --moc--> [silniki] --> dron
 *                   ^ (−)                                        |
 *                   +--------------- [laser] <---wysokość--------+
 *
 *   Regulator liczy nową moc przy każdym pomiarze lasera (co 33 ms).
 *   ANTI-WINDUP: suma błędów ma limit SUMA_MAX, żeby się nie „nakręcić”.
 *
 *  BEZPIECZEŃSTWO
 *   - Start ROZBROJONY (czerwona). "u" = uzbrój (zielona).
 *   - "x" albo spacja = STOP. Przycisk BOOT = STOP.
 *   - Automatyczny STOP: brak pomiaru > 200 ms, wysokość > WYSOKOSC_MAX,
 *     lot dłuższy niż CZAS_MAX. Powód pojawia się w Serial Monitorze.
 *   - "h 0" = lądowanie: silniki płynnie zwalniają do zera.
 *
 *  KOMENDY (Serial Monitor, 115200, „Nowa linia”)
 *   u – uzbrój        x – STOP         ? – pokaż nastawy
 *   h 300  – zadana wysokość w mm
 *   p 0.05 – Kp     i 0.002 – Ki     d 0.5 – Kd
 *   z 35   – moc zawisu w %
 *
 *  ZADANIA (szukaj w kodzie napisu ZADANIE)
 *   1. Ki = 0, Kd = 0. Zacznij od Kp = 0.02 i zwiększaj.
 *      Kiedy dron zaczyna się bujać? Wybierz trochę mniej.
 *   2. Dodaj Ki do wzoru. Ustaw np. „i 0.002”. Co się zmienia?
 *   3. Dodaj Kd do wzoru. Ustaw np. „d 0.5”. Co się zmienia?
 *   4. Opisz własnymi słowami, co robi Kp, Ki i Kd.
 *
 *  ZADANIE 4: wasze obserwacje
 *   Kp: ...
 *   Ki: ...
 *   Kd: ...
 */

#include <Wire.h>
#include <VL53L1X.h>

// ================== KONFIGURACJA ==================
#define TRYB_MOSFET          // moduły ESC sterowane PWM 20 kHz (domyślnie)
// #define TRYB_ESC_RC       // albo regulatory ESC (impulsy 1000-2000 us)
                             // Zostaw odkomentowany TYLKO JEDEN tryb!

const int PINY_SILNIKOW[4] = {18, 19, 20, 21};   // M1, M2, M3, M4
const int PIN_STOP = 9;          // przycisk BOOT = STOP awaryjny
const int PIN_SDA = 6;
const int PIN_SCL = 7;
const float MAX_MOC = 60;        // [%] tej mocy program nigdy nie przekroczy
const float RAMPA = 100;         // [% na sekundę] w locie trzeba szybciej

const int PWM_HZ = 20000;        // TRYB_MOSFET: częstotliwość PWM
const int PWM_BITY = 10;
const int ESC_HZ = 50;           // TRYB_ESC_RC: 50 impulsów na sekundę
const int ESC_BITY = 14;

// Zabezpieczenia
const int WYSOKOSC_MAX = 800;            // [mm] wyżej = STOP
const unsigned long CZAS_MAX = 60000;    // [ms] najdłuższy lot (60 s)
const unsigned long BRAK_POMIARU_MS = 200;
const int WYSOKOSC_ZIEMI = 60;           // [mm] zadana niżej = lądujemy
const float SUMA_MAX = 10000;            // limit sumy błędów (anti-windup)

// Nastawy na start (zmieniasz je komendami)
float zadana_mm = 300;
float moc_zawisu = 35;           // [%] moc, przy której dron prawie wisi
// ZADANIE 1: gdy znajdziesz dobre Kp (komenda p), wpisz je tu,
// żeby po restarcie nie szukać od nowa.
float Kp = 0.02;
float Ki = 0;
float Kd = 0;
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

// ================== LASER ==================
VL53L1X laser;
float wysokosc_mm = 0;
unsigned long czas_pomiaru = 0;          // kiedy był ostatni dobry pomiar

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

// Czy ostatni pomiar jest dobry? (statusy z „RangeValid” w nazwie)
bool dobryPomiar() {
  int s = laser.ranging_data.range_status;
  return s == VL53L1X::RangeValid || s == VL53L1X::RangeValidMinRangeClipped ||
         s == VL53L1X::RangeValidNoWrapCheckFail;
}

// ================== REGULATOR ==================
float suma_bledow = 0;
float poprzedni_blad = 0;
bool pierwszy_krok = true;
unsigned long czas_uzbrojenia = 0;

void regulator() {
  if (!uzbrojony) return;
  if (zadana_mm < WYSOKOSC_ZIEMI) {      // lądowanie: silniki płynnie do 0
    ustawWszystkie(0);
    suma_bledow = 0;
    pierwszy_krok = true;
    return;
  }
  float blad = zadana_mm - wysokosc_mm;
  if (pierwszy_krok) {
    poprzedni_blad = blad;
    pierwszy_krok = false;
  }
  suma_bledow = constrain(suma_bledow + blad, -SUMA_MAX, SUMA_MAX);
  float zmiana_bledu = blad - poprzedni_blad;
  poprzedni_blad = blad;

  // ZADANIE 2: dopisz do wzoru  + Ki * suma_bledow
  // ZADANIE 3: dopisz do wzoru  + Kd * zmiana_bledu
  float moc = moc_zawisu + Kp * blad;
  ustawWszystkie(moc);   // ustawWszystkie pilnuje zakresu 0..MAX_MOC
}

// Automatyczny STOP, gdy coś jest nie tak.
void bezpieczenstwo() {
  if (!uzbrojony) return;
  if (millis() - czas_pomiaru > BRAK_POMIARU_MS) stop("brak pomiaru z lasera dłużej niż 200 ms");
  else if (wysokosc_mm > WYSOKOSC_MAX) stop("za wysoko (powyżej WYSOKOSC_MAX)");
  else if (millis() - czas_uzbrojenia > CZAS_MAX) stop("minął CZAS_MAX lotu");
}

void wypiszNastawy() {
  Serial.println("zadana_mm=" + String(zadana_mm) + "  moc_zawisu=" + String(moc_zawisu) +
                 "  Kp=" + String(Kp, 4) + "  Ki=" + String(Ki, 4) + "  Kd=" + String(Kd, 4));
}

// ================== PROGRAM LEKCJI ==================
void obsluzKomende() {
  if (komenda == 'x') {
    stop("komenda x");
  } else if (komenda == 'u') {
    if (millis() - czas_pomiaru > BRAK_POMIARU_MS) {
      Serial.println("Nie uzbrajam: brak pomiaru z lasera.");
      return;
    }
    suma_bledow = 0;
    pierwszy_krok = true;
    czas_uzbrojenia = millis();
    uzbroj();
  } else if (komenda == 'h') {
    float nowa = constrain(komenda_liczba, 0, WYSOKOSC_MAX - 100);
    poprzedni_blad += nowa - zadana_mm;  // żeby Kd nie „szarpnął” przy zmianie
    zadana_mm = nowa;
  } else if (komenda == 'p') {
    Kp = komenda_liczba;
  } else if (komenda == 'i') {
    Ki = komenda_liczba;
  } else if (komenda == 'd') {
    Kd = komenda_liczba;
  } else if (komenda == 'z') {
    moc_zawisu = constrain(komenda_liczba, 0, MAX_MOC);
  } else if (komenda != '?') {
    Serial.println("Znam komendy: u, x, h, p, i, d, z, ?");
  }
  wypiszNastawy();
}

void setup() {
  silnikiNaZero();           // NAJPIERW silniki na 0!
  Serial.begin(115200);
  delay(1000);
  silnikiStart();
  while (!laserStart()) {
    Serial.println("Nie widzę lasera! Uruchom skaner L02. Widzi 0x29? To może być inny model czujnika – zawołaj prowadzącego.");
    delay(2000);
  }
  Serial.println("L09: trzymaj wysokość. Wpisz u, potem h 300.");
  wypiszNastawy();
}

void loop() {
  if (czytajKomende()) obsluzKomende();
  sprawdzPrzycisk();

  if (laser.dataReady()) {               // nowy pomiar = jeden krok regulatora
    laser.read(false);
    if (dobryPomiar()) {
      wysokosc_mm = laser.ranging_data.range_mm;
      czas_pomiaru = millis();
      regulator();
      Serial.print("zadana:");
      Serial.print(zadana_mm);
      Serial.print(",wysokosc:");
      Serial.print(wysokosc_mm);
      Serial.print(",moc_x10:");         // moc × 10, żeby było ją widać obok mm
      Serial.println(moc_teraz[0] * 10);
    }
  }

  bezpieczenstwo();
  rampa();
}
