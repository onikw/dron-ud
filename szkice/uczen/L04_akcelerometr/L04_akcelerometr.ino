/*
 * ==============================================================
 *  LEKCJA 04: AKCELEROMETR MPU6050 – BEZ BIBLIOTEKI
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 2
 *  WERSJA UCZNIA
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Co to są REJESTRY czujnika.
 *   - Jak czytać i zapisywać rejestry przez I2C (biblioteka Wire).
 *   - Co mierzy akcelerometr, a co żyroskop.
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - MPU6050 na dronie, płasko, strzałka osi X do przodu.
 *   - Przy starcie dron stoi płasko i nieruchomo (kalibracja).
 *   - Kabel z drona (ok. 1 m) wpięty w płytkę stykową:
 *     czerwony = 3V3, czarny = GND, żółty = SDA -> pin 6,
 *     niebieski = SCL -> pin 7.
 *
 *  JAK TO DZIAŁA
 *   Czujnik ma w środku ponumerowane SZUFLADKI (rejestry).
 *   W każdej leży jedna liczba 0..255 (jeden bajt).
 *
 *     szufladka 0x75: „kim jestem” (WHO_AM_I), zwykle 0x68
 *     szufladka 0x6B: zasilanie – wpisz 0, żeby obudzić czujnik
 *     szufladki 0x3B..0x48: pomiary (14 bajtów):
 *
 *      0x3B 0x3C | 0x3D 0x3E | 0x3F 0x40 | 0x41 0x42 | 0x43 ... 0x48
 *        acc X   |   acc Y   |   acc Z   |   temp.   |  gyro X, Y, Z
 *
 *   Każdy pomiar zajmuje 2 szufladki: starszy i młodszy bajt.
 *   Sklejamy je: liczba = starszy * 256 + młodszy.
 *
 *   AKCELEROMETR mierzy przyspieszenie w „g”. Gdy dron stoi,
 *   czuje tylko grawitację: oś Z pokazuje około 1 g.
 *   ŻYROSKOP mierzy, jak szybko się obracasz (stopnie na sekundę).
 *   Gdy dron stoi, pokazuje około 0.
 *
 *   KALIBRACJA: żaden czujnik nie jest idealny – nawet gdy stoi,
 *   pokazuje trochę więcej albo mniej, niż powinien. Na starcie program
 *   przez ok. 2 s sprawdza, ile czujnik pokazuje w spoczynku, i potem
 *   to odejmuje. Dlatego przy starcie dron MUSI stać płasko i nieruchomo.
 *   Program odlicza 3 s, a gdy wykryje ruch – kalibruje jeszcze raz.
 *
 *  ZADANIA (szukaj w kodzie napisu ZADANIE)
 *   1. Przelicz surowe liczby na g i °/s.
 *   2. Przechyl drona i obserwuj osie na Plotterze.
 *   3. Policz kąt przechylenia z akcelerometru.
 */

#include <Wire.h>

// ================== KONFIGURACJA ==================
const int PIN_SDA = 6;
const int PIN_SCL = 7;
const int MPU = 0x68;                // adres czujnika na I2C
const float LSB_NA_G = 16384.0;      // zakres ±2 g:     16384 = 1 g
const float LSB_NA_STOPIEN = 131.0;  // zakres ±250 °/s: 131 = 1 °/s
const int CO_ILE_MS = 100;           // nowy punkt wykresu co 100 ms (10 na sekundę)
const int PROBKI_KALIBRACJI = 200;   // tyle pomiarów uśredniamy przy kalibracji (ok. 2 s)
const int RUCH_AKC = 1500;           // większy rozrzut akcelerometru = dron się ruszał (~0,1 g)
const int RUCH_ZYRO = 500;           // większy rozrzut żyroskopu = dron się ruszał (~4 °/s)
// ==================================================

// Zapisuje liczbę do szufladki (rejestru) czujnika.
void zapiszRejestr(byte rejestr, byte wartosc) {
  Wire.beginTransmission(MPU);
  Wire.write(rejestr);      // numer szufladki
  Wire.write(wartosc);      // co do niej wkładamy
  Wire.endTransmission();
}

// Czyta jedną szufladkę.
byte czytajRejestr(byte rejestr) {
  Wire.beginTransmission(MPU);
  Wire.write(rejestr);
  Wire.endTransmission(false);   // false = nie kończ rozmowy
  Wire.requestFrom(MPU, 1);
  return Wire.read();
}

// Skleja dwa bajty w jedną liczbę ze znakiem (może być ujemna).
int16_t sklej(byte starszy, byte mlodszy) {
  return (int16_t)((starszy << 8) | mlodszy);
}

int16_t surowe[7];   // ax, ay, az, temperatura, gx, gy, gz – surowe liczby z czujnika
float zero[7];       // poprawki z kalibracji (to, co czujnik pokazuje w spoczynku)
unsigned long czas_wykresu = 0;

// Czyta 14 szufladek naraz, zaczynając od 0x3B, i skleja je w 7 liczb.
void czytajCzujnik() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14);
  for (int i = 0; i < 7; i++) {
    byte starszy = Wire.read();
    byte mlodszy = Wire.read();
    surowe[i] = sklej(starszy, mlodszy);
  }
}

// KALIBRACJA: dron stoi płasko i nieruchomo. Liczymy średnią z wielu pomiarów.
// To, co czujnik wtedy pokazuje, to jego „błąd zera” – potem go odejmujemy.
// Wyjątek: oś Z ma pokazywać 1 g (grawitację), a nie 0.
// Jeśli w trakcie dron się ruszy, kalibracja zaczyna się od nowa.
void kalibruj() {
  Serial.println("KALIBRACJA za 3 s: połóż drona płasko i nie ruszaj go.");
  for (int s = 3; s > 0; s--) {
    Serial.println(s);
    delay(1000);
  }
  while (true) {
    float suma[7] = {0, 0, 0, 0, 0, 0, 0};
    int16_t najmniej[7], najwiecej[7];
    czytajCzujnik();
    for (int i = 0; i < 7; i++) najmniej[i] = najwiecej[i] = surowe[i];
    for (int n = 0; n < PROBKI_KALIBRACJI; n++) {
      czytajCzujnik();
      for (int i = 0; i < 7; i++) {
        suma[i] += surowe[i];
        najmniej[i] = min(najmniej[i], surowe[i]);
        najwiecej[i] = max(najwiecej[i], surowe[i]);
      }
      delay(10);
    }
    // Ruch? Akcelerometr (0..2) albo żyroskop (4..6) skakał za bardzo.
    bool ruch = false;
    for (int i = 0; i < 3; i++) if (najwiecej[i] - najmniej[i] > RUCH_AKC) ruch = true;
    for (int i = 4; i < 7; i++) if (najwiecej[i] - najmniej[i] > RUCH_ZYRO) ruch = true;
    if (ruch) {
      Serial.println("Dron się ruszał – kalibruję jeszcze raz. Nie dotykaj drona ani stołu.");
      continue;
    }
    for (int i = 0; i < 7; i++) zero[i] = suma[i] / PROBKI_KALIBRACJI;
    // Czy dron leżał płasko? Kąt między grawitacją a osią Z czujnika.
    float przechylenie = atan2(sqrt(zero[0] * zero[0] + zero[1] * zero[1]), zero[2]) * 180.0 / PI;
    if (przechylenie > 10) {
      Serial.print("Uwaga: w czasie kalibracji dron był przechylony o ok. ");
      Serial.print((int)przechylenie);
      Serial.println(" stopni. Postaw go płasko i naciśnij RST (albo czujnik jest krzywo zamocowany).");
    }
    zero[2] -= LSB_NA_G;   // oś Z: zostawiamy 1 g grawitacji
    zero[3] = 0;           // temperatury nie poprawiamy
    Serial.println("Kalibracja gotowa. Żeby ją powtórzyć: postaw drona płasko i naciśnij RST.");
    return;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);   // 100 kHz: kabel I2C ma ok. 1 m, więc wolniej i pewniej
  Wire.setTimeOut(50);     // nie czekaj w nieskończoność, gdy czujnik milczy

  byte kto = czytajRejestr(0x75);   // WHO_AM_I
  Serial.print("WHO_AM_I = 0x");
  Serial.println(kto, HEX);
  if (kto != 0x68) {
    // Klony czujnika często mają inną wartość. Nie blokujemy programu.
    Serial.println("Uwaga: oczekiwałem 0x68. To może być klon. Próbuję dalej.");
  }

  zapiszRejestr(0x6B, 0);   // budzimy czujnik (po włączeniu śpi)
  delay(100);
  kalibruj();
}

void loop() {
  if (millis() - czas_wykresu < CO_ILE_MS) return;   // jeszcze nie czas na nowy punkt
  czas_wykresu = millis();
  czytajCzujnik();

  // ZADANIE 1: teraz to liczby bez jednostek (np. 16384 zamiast 1 g).
  // Podziel ax, ay, az przez LSB_NA_G, a gx, gy, gz przez LSB_NA_STOPIEN.
  // Przykład:  float ax = (surowe[0] - zero[0]) / LSB_NA_G;
  float ax = surowe[0] - zero[0];
  float ay = surowe[1] - zero[1];
  float az = surowe[2] - zero[2];
  float gx = surowe[4] - zero[4];
  float gy = surowe[5] - zero[5];
  float gz = surowe[6] - zero[6];

  // ZADANIE 3: policz kąt przechylenia z akcelerometru.
  // Podpowiedź: atan2(ay, az) daje kąt w radianach.
  // Pomnóż go przez 180.0 / PI, żeby dostać stopnie.
  float kat = 0;

  // Kliknij nazwę w legendzie Plottera, żeby ukryć/pokazać linię.
  Serial.print("ax_g:");   Serial.print(ax);
  Serial.print(",ay_g:");  Serial.print(ay);
  Serial.print(",az_g:");  Serial.print(az);
  Serial.print(",gx:");    Serial.print(gx);
  Serial.print(",gy:");    Serial.print(gy);
  Serial.print(",gz:");    Serial.print(gz);
  Serial.print(",kat:");   Serial.println(kat);
}
