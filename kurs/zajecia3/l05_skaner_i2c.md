# L05: Skaner I2C – kto jest na magistrali?

Na dronie są dwa czujniki. Oba są podłączone do **tych samych dwóch kabelków**. Jak płytka wie, z którym rozmawia? Każdy ma swój **adres**!

## 🎯 Czego się nauczysz

- Co to jest **I2C** (czyt. „i kwadrat ce”).
- Co to jest **adres** urządzenia.
- Jak sprawdzić, czy czujniki są dobrze podłączone.

## 🔌 Co podłączyć

Czujniki MPU6050 i VL53L1X: zasilanie **3,3 V** i **GND**, **SDA → 6**, **SCL → 7**.

## 🧠 Jak to działa

I2C to dwie wspólne linie dla wielu urządzeń:

- **SDA** – dane (tu jadą bity),
- **SCL** – zegar (mówi, **kiedy** czytać kolejny bit).

```text
  ESP32 ---SDA-----+-----------+----
        ---SCL---+-|---------+-|----
                 | |         | |
              [MPU6050]   [laser]
               adr 0x68   adr 0x29
```

Każde urządzenie ma **adres** – jak numer mieszkania w bloku. ESP „puka” pod adres i czeka na odpowiedź „jestem!” (to się nazywa **ACK**). Brak odpowiedzi = nikogo tam nie ma. Skaner puka po kolei pod wszystkie adresy od 1 do 126.

> [!NOTE] Liczby szesnastkowe
> Adresy zapisujemy szesnastkowo, z przedrostkiem `0x`. Cyfry to 0–9 i A–F.
> `0x29` = 2·16 + 9 = **41**, a `0x68` = 6·16 + 8 = **104**.

## 💻 Kod

[Pobierz plik](https://github.com/onikw/dron-ud/blob/main/uczen/L05_skaner_i2c/L05_skaner_i2c.ino)

```cpp
--8<-- "uczen/L05_skaner_i2c/L05_skaner_i2c.ino"
```

## ✅ Zadania

1. W funkcji `nazwa()` dopisz nazwę dla adresu `0x68` (MPU6050).
2. Policz znalezione urządzenia i wypisz ich liczbę na końcu skanowania.
3. **Wyłącz zasilanie**, odłącz jeden czujnik, włącz i uruchom skaner jeszcze raz. Co się zmieniło? Podłącz go z powrotem.

## 👀 Co powinieneś zobaczyć

```text
Skanuję adresy 1..126 ...
  Znalazłem 0x29  ->  laser VL53L1X (odległość)
  Znalazłem 0x68  ->  MPU6050 (żyroskop i akcelerometr)
Liczba urządzeń: 2
```

Nie widać nic? Zajrzyj do [Najczęstszych problemów](../start/problemy.md#skaner-i2c-nic-nie-widzi).

??? success "Pokaż rozwiązanie"

    ```cpp
    --8<-- "prowadzacy/L05_skaner_i2c/L05_skaner_i2c.ino"
    ```
