/*
 * ==============================================================
 *  LEKCJA 10: MISJA – ZAWODY!
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 5
 *  WERSJA PROWADZĄCEGO (z rozwiązaniami)
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Co to jest MASZYNA STANÓW.
 *   - Jak zaplanować lot jako listę kroków (tablicę).
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - Tak jak w L09: dron na prowadnicy, silniki, laser w dół.
 *
 *  JAK TO DZIAŁA
 *   Maszyna stanów: dron jest zawsze w JEDNYM stanie, jak postać
 *   w grze (stoi / skacze / biegnie). Coś się dzieje -> zmiana stanu.
 *
 *    CZEKAJ --"s"--> START --2 s--> MISJA --kroki gotowe--> LĄDOWANIE
 *      ^                           (krok 1, 2, 3 ...)           |
 *      |                                                        v
 *      +------------- STOP (x, BOOT, zabezpieczenia) <------ KONIEC
 *
 *   MISJA bierze kolejne kroki z tablicy KROKI:
 *     {wysokość w mm, czas w ms}  – „leć na tę wysokość i trzymaj”.
 *   Wysokość zadana nie skacze, tylko płynnie „jedzie” do celu.
 *   Wysokość trzyma ten sam regulator PID co w L09.
 *
 *  BEZPIECZEŃSTWO (jak w L09)
 *   - Start ROZBROJONY (czerwona). "u" = uzbrój (zielona).
 *   - "x" albo spacja = STOP. Przycisk BOOT = STOP.
 *   - Automatyczny STOP: brak pomiaru > 200 ms, wysokość > WYSOKOSC_MAX,
 *     lot dłuższy niż CZAS_MAX.
 *
 *  KOMENDY
 *   u – uzbrój     s – start misji     x – STOP     ? – nastawy
 *   p, i, d, z – nastawy regulatora (jak w L09)
 *
 *  ZADANIE
 *   1. Zaplanuj misję: zmień TYLKO tablicę KROKI.
 */

#include <Wire.h>
#include <VL53L1X.h>

// ================== MISJA ==================
// ROZWIĄZANIE: przykładowa misja „schodki w górę i w dół”
// {wysokość w mm, czas w ms}
const int KROKI[][2] = {
  {250, 3000},
  {400, 3000},
  {550, 3000},
  {300, 4000},
  {450, 2000},
};
const int ILE_KROKOW = sizeof(KROKI) / sizeof(KROKI[0]);   // liczy się samo

// ================== KONFIGURACJA ==================
#define TRYB_MOSFET          // moduły ESC sterowane PWM 20 kHz (domyślnie)
// #define TRYB_ESC_RC       // albo regulatory ESC (impulsy 1000-2000 us)
                             // Zostaw odkomentowany TYLKO JEDEN tryb!

const int PINY_SILNIKOW[4] = {18, 19, 20, 21};   // M1, M2, M3, M4
const int PIN_STOP = 9;          // przycisk BOOT = STOP awaryjny
const int PIN_SDA = 6;
const int PIN_SCL = 7;
const float MAX_MOC = 60;        // [%] tej mocy program nigdy nie przekroczy
const float RAMPA = 100;         // [% na sekundę]

const int PWM_HZ = 20000;        // TRYB_MOSFET: częstotliwość PWM
const int PWM_BITY = 10;
const int ESC_HZ = 50;           // TRYB_ESC_RC: 50 impulsów na sekundę
const int ESC_BITY = 14;

// Zabezpieczenia
const int WYSOKOSC_MAX = 800;            // [mm] wyżej = STOP
const unsigned long CZAS_MAX = 60000;    // [ms] najdłuższy lot (60 s)
const unsigned long BRAK_POMIARU_MS = 200;
const int WYSOKOSC_ZIEMI = 60;           // [mm] zadana niżej = silniki stop
const float SUMA_MAX = 10000;            // limit sumy błędów (anti-windup)

// Start i lądowanie
const int WYSOKOSC_STARTU = 150;         // [mm] na tę wysokość wzlatujemy
const unsigned long CZAS_STARTU_MS = 2000;
const float PREDKOSC_ZADANEJ = 150;      // [mm/s] jak szybko „jedzie” zadana

// Nastawy regulatora – wpiszcie te z L09
float moc_zawisu = 35;
float Kp = 0.05;
float Ki = 0.002;
float Kd = 0.5;
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
unsigned long czas_pomiaru = 0;

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

bool dobryPomiar() {
  int s = laser.ranging_data.range_status;
  return s == VL53L1X::RangeValid || s == VL53L1X::RangeValidMinRangeClipped ||
         s == VL53L1X::RangeValidNoWrapCheckFail;
}

// ================== REGULATOR (jak w L09) ==================
float zadana_mm = 0;
float suma_bledow = 0;
float poprzedni_blad = 0;
bool pierwszy_krok = true;
unsigned long czas_uzbrojenia = 0;

void regulator() {
  if (!uzbrojony) return;
  if (zadana_mm < WYSOKOSC_ZIEMI) {      // przy ziemi: silniki płynnie do 0
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
  float moc = moc_zawisu + Kp * blad + Ki * suma_bledow + Kd * zmiana_bledu;
  ustawWszystkie(moc);
}

void bezpieczenstwo() {
  if (!uzbrojony) return;
  if (millis() - czas_pomiaru > BRAK_POMIARU_MS) stop("brak pomiaru z lasera dłużej niż 200 ms");
  else if (wysokosc_mm > WYSOKOSC_MAX) stop("za wysoko (powyżej WYSOKOSC_MAX)");
  else if (millis() - czas_uzbrojenia > CZAS_MAX) stop("minął CZAS_MAX lotu");
}

void wypiszNastawy() {
  Serial.println("moc_zawisu=" + String(moc_zawisu) + "  Kp=" + String(Kp, 4) +
                 "  Ki=" + String(Ki, 4) + "  Kd=" + String(Kd, 4));
}

// ================== MASZYNA STANÓW ==================
const int CZEKAJ = 0;
const int START = 1;
const int MISJA = 2;
const int LADOWANIE = 3;
const int KONIEC = 4;
const String NAZWY_STANOW[5] = {"CZEKAJ", "START", "MISJA", "LĄDOWANIE", "KONIEC"};

int stan = CZEKAJ;
unsigned long czas_stanu = 0;    // kiedy weszliśmy w obecny stan
unsigned long czas_startu = 0;   // kiedy zaczęła się misja
int krok = 0;                    // który krok misji (od 0)
float cel_mm = 0;                // dokąd „jedzie” zadana

void zmienStan(int nowy) {
  stan = nowy;
  czas_stanu = millis();
  Serial.println("STAN -> " + NAZWY_STANOW[nowy]);
}

// Ile ms potrwa cała misja (start + kroki + zapas na lądowanie).
unsigned long czasMisji() {
  unsigned long suma = CZAS_STARTU_MS + 5000;
  for (int i = 0; i < ILE_KROKOW; i++) suma += KROKI[i][1];
  return suma;
}

// Wołana przy każdym pomiarze: decyduje, co robić dalej.
void maszynaStanow() {
  unsigned long w_stanie = millis() - czas_stanu;

  if (stan == START) {
    cel_mm = WYSOKOSC_STARTU;
    if (w_stanie > CZAS_STARTU_MS) {
      krok = 0;
      zmienStan(MISJA);
    }
  } else if (stan == MISJA) {
    cel_mm = constrain(KROKI[krok][0], 0, WYSOKOSC_MAX - 100);
    if (w_stanie > (unsigned long)KROKI[krok][1]) {
      krok++;
      czas_stanu = millis();
      if (krok >= ILE_KROKOW) zmienStan(LADOWANIE);
      else Serial.println("KROK " + String(krok + 1) + " z " + String(ILE_KROKOW));
    }
  } else if (stan == LADOWANIE) {
    cel_mm = 0;
    if (zadana_mm < WYSOKOSC_ZIEMI) zmienStan(KONIEC);
  } else if (stan == KONIEC) {
    ustawWszystkie(0);
    if (uzbrojony && moc_teraz[0] == 0) {   // silniki już stanęły
      uzbrojony = false;
      pokazStan();
      Serial.println("MISJA ZAKOŃCZONA! Czas: " + String((millis() - czas_startu) / 1000.0) + " s");
    }
  }

  // Zadana płynnie „jedzie” do celu, najwyżej PREDKOSC_ZADANEJ mm/s.
  float krok_mm = PREDKOSC_ZADANEJ * 0.033;
  if (zadana_mm < cel_mm) zadana_mm = min(zadana_mm + krok_mm, cel_mm);
  else zadana_mm = max(zadana_mm - krok_mm, cel_mm);

  if (stan != KONIEC) regulator();
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
    czas_uzbrojenia = millis();
    zmienStan(CZEKAJ);
    uzbroj();
  } else if (komenda == 's') {
    if (!uzbrojony || stan != CZEKAJ) {
      Serial.println("Start tylko w stanie CZEKAJ i po uzbrojeniu (u).");
    } else if (czasMisji() > CZAS_MAX) {
      Serial.println("Misja za długa! Skróć czasy w tablicy KROKI.");
    } else {
      zadana_mm = wysokosc_mm;          // zaczynamy od tego, gdzie jesteśmy
      suma_bledow = 0;
      pierwszy_krok = true;
      czas_uzbrojenia = millis();
      czas_startu = millis();
      zmienStan(START);
    }
  } else if (komenda == 'p') {
    Kp = komenda_liczba;
  } else if (komenda == 'i') {
    Ki = komenda_liczba;
  } else if (komenda == 'd') {
    Kd = komenda_liczba;
  } else if (komenda == 'z') {
    moc_zawisu = constrain(komenda_liczba, 0, MAX_MOC);
  } else if (komenda != '?') {
    Serial.println("Znam komendy: u, s, x, p, i, d, z, ?");
  }
  if (komenda == 'p' || komenda == 'i' || komenda == 'd' || komenda == 'z' || komenda == '?') {
    wypiszNastawy();
  }
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
  Serial.println("L10: misja. Kroków: " + String(ILE_KROKOW) +
                 ", potrwa ok. " + String(czasMisji() / 1000) + " s. Wpisz u, potem s.");
  wypiszNastawy();
}

void loop() {
  if (czytajKomende()) obsluzKomende();
  sprawdzPrzycisk();

  if (laser.dataReady()) {
    laser.read(false);
    if (dobryPomiar()) {
      wysokosc_mm = laser.ranging_data.range_mm;
      czas_pomiaru = millis();
      if (stan != CZEKAJ) maszynaStanow();
      Serial.print("zadana:");
      Serial.print(zadana_mm);
      Serial.print(",wysokosc:");
      Serial.print(wysokosc_mm);
      Serial.print(",moc_x10:");
      Serial.print(moc_teraz[0] * 10);
      Serial.print(",stan_x100:");
      Serial.println(stan * 100);
    }
  }

  bezpieczenstwo();
  // Po STOP (x, BOOT albo zabezpieczenie) wracamy do CZEKAJ.
  if (!uzbrojony && stan != CZEKAJ && stan != KONIEC) zmienStan(CZEKAJ);
  rampa();
}
