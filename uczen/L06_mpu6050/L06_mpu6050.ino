/*
 * ==============================================================
 *  LEKCJA 06 (dodatkowa): MPU6050 – CZUJNIK RUCHU, BEZ BIBLIOTEKI
 *  Kurs „Inżynieria dronowa” – Uniwersytet Dzieci
 *  WERSJA UCZNIA
 * ==============================================================
 *
 *  CZEGO SIĘ NAUCZYSZ
 *   - Co to są REJESTRY czujnika.
 *   - Jak czytać i zapisywać rejestry przez I2C (biblioteka Wire).
 *   - Co mierzy akcelerometr, a co żyroskop.
 *
 *  CO MUSI BYĆ PODŁĄCZONE
 *   - MPU6050: 3,3 V, GND, SDA -> pin 6, SCL -> pin 7.
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

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(PIN_SDA, PIN_SCL);

  byte kto = czytajRejestr(0x75);   // WHO_AM_I
  Serial.print("WHO_AM_I = 0x");
  Serial.println(kto, HEX);
  if (kto != 0x68) {
    // Klony czujnika często mają inną wartość. Nie blokujemy programu.
    Serial.println("Uwaga: oczekiwałem 0x68. To może być klon. Próbuję dalej.");
  }

  zapiszRejestr(0x6B, 0);   // budzimy czujnik (po włączeniu śpi)
  delay(100);
}

void loop() {
  // Czytamy 14 szufladek naraz, zaczynając od 0x3B.
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14);
  byte b[14];
  for (int i = 0; i < 14; i++) b[i] = Wire.read();

  // ZADANIE 1: teraz to surowe liczby (np. 16384 zamiast 1 g).
  // Podziel ax, ay, az przez LSB_NA_G, a gx, gy, gz przez LSB_NA_STOPIEN.
  // Przykład:  float ax = sklej(b[0], b[1]) / LSB_NA_G;
  float ax = sklej(b[0], b[1]);
  float ay = sklej(b[2], b[3]);
  float az = sklej(b[4], b[5]);
  float gx = sklej(b[8], b[9]);
  float gy = sklej(b[10], b[11]);
  float gz = sklej(b[12], b[13]);

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
  delay(20);   // 50 pomiarów na sekundę
}
