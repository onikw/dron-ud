# 💡 Zajęcia 3: Arduino i silniki

Na tych zajęciach napiszecie pierwszy program, poznacie PWM, uruchomicie jeden silnik, potem wszystkie cztery, a na koniec sprawdzicie, czy czujniki „odpowiadają”.

## ⚡ Czego się nauczysz

1. **Struktura programu** – `setup()` i `loop()`, wypisywanie tekstu do komputera.
2. **PWM** – jak pinem, który umie tylko „włącz/wyłącz”, ustawić połowę jasności albo mocy.
3. **Sterowanie silnikiem** – komendy z klawiatury, rampa, limit mocy, uzbrajanie.
4. **Tablice i pętle** – jeden kod dla czterech silników.
5. **I2C** – jak płytka rozmawia z czujnikami.

## 🛠️ Co jest potrzebne

| Lekcja | Sprzęt |
|:--|:--|
| L01, L02 | sama płytka i kabel USB (w L02 opcjonalnie dioda + rezystor 220 Ω) |
| L03, L04 | dron na prowadnicy, zasilanie silników |
| L05 | dron z czujnikami MPU6050 i VL53L1X |

> [!CAUTION] Od lekcji L03 kręcą się śmigła
> Przeczytajcie [zasady bezpieczeństwa](../start/dron.md#zasady-bezpieczenstwa) zanim pierwszy raz wpiszecie `u`.

## 🗺️ Lekcje

1. [L01: Miganie diodą](l01_miganie.md)
2. [L02: PWM i dioda](l02_pwm_led.md)
3. [L03: Jeden silnik](l03_jeden_silnik.md)
4. [L04: Cztery silniki](l04_cztery_silniki.md)
5. [L05: Skaner I2C](l05_skaner_i2c.md)
