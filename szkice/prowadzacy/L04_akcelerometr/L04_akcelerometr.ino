/*
 * ==============================================================
 *  LEKCJA 04: AKCELEROMETR MPU6050 – BEZ BIBLIOTEKI
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci, zajęcia 2
 *  WERSJA PROWADZĄCEGO (z rozwiązaniami)
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
 *
 *  ZADANIA
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
void kalibruj() {
  Serial.println("KALIBRACJA: dron ma stać płasko i nieruchomo (ok. 2 s)...");
  float suma[7] = {0, 0, 0, 0, 0, 0, 0};
  for (int n = 0; n < PROBKI_KALIBRACJI; n++) {
    czytajCzujnik();
    for (int i = 0; i < 7; i++) suma[i] += surowe[i];
    delay(10);
  }
  for (int i = 0; i < 7; i++) zero[i] = suma[i] / PROBKI_KALIBRACJI;
  zero[2] -= LSB_NA_G;   // oś Z: zostawiamy 1 g grawitacji
  zero[3] = 0;           // temperatury nie poprawiamy
  Serial.println("Kalibracja gotowa. Żeby ją powtórzyć: postaw drona płasko i naciśnij RST.");
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

  // ROZWIĄZANIE 1: odejmujemy poprawkę z kalibracji i dzielimy przez „ile na jednostkę”
  float ax = (surowe[0] - zero[0]) / LSB_NA_G;
  float ay = (surowe[1] - zero[1]) / LSB_NA_G;
  float az = (surowe[2] - zero[2]) / LSB_NA_G;
  float gx = (surowe[4] - zero[4]) / LSB_NA_STOPIEN;
  float gy = (surowe[5] - zero[5]) / LSB_NA_STOPIEN;
  float gz = (surowe[6] - zero[6]) / LSB_NA_STOPIEN;

  // ROZWIĄZANIE 3: kąt przechylenia z kierunku grawitacji.
  // atan2 zamienia dwie długości na kąt; wynik w radianach -> stopnie.
  float kat = atan2(ay, az) * 180.0 / PI;

  // Kliknij nazwę w legendzie Plottera, żeby ukryć/pokazać linię.
  Serial.print("ax_g:");   Serial.print(ax);
  Serial.print(",ay_g:");  Serial.print(ay);
  Serial.print(",az_g:");  Serial.print(az);
  Serial.print(",gx:");    Serial.print(gx);
  Serial.print(",gy:");    Serial.print(gy);
  Serial.print(",gz:");    Serial.print(gz);
  Serial.print(",kat:");   Serial.println(kat);
}
