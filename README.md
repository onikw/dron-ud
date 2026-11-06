# 🚁 Inżynieria dronowa – kurs ESP32-C6 dla Uniwersytetu Dzieci

Szkice Arduino do kursu „Inżynieria dronowa” dla uczniów 12–14 lat. Prowadzą krok po kroku od mrugania diodą do drona, który sam trzyma wysokość i wykonuje misję. Wszystko widać: w Serial Monitorze, na Serial Plotterze, na diodzie i na silnikach.

**📖 Strona kursu:** <https://onikw.github.io/dron-ud/>

```
dron-ud/
  README.md
  uczen/        wersje z lukami do uzupełnienia (// ZADANIE n:)
  prowadzacy/   te same lekcje z kompletnymi rozwiązaniami
  kurs/         strona kursu (MkDocs) – kod wczytywany prosto z plików .ino
```

Każdy szkic jest samodzielny: tylko `Wire` i biblioteka czujnika, bez wspólnych plików. Wersja ucznia zawsze się kompiluje i działa bezpiecznie, zanim cokolwiek uzupełni.

---

## 🧰 Instalacja

Pełny poradnik z obrazkami: [Krok 1: Instalacja](https://onikw.github.io/dron-ud/start/instalacja/).

1. **Arduino IDE 2.x** – <https://www.arduino.cc/en/software>
2. **Rdzeń „esp32” od Espressif w wersji 3.x** – Menedżer płytek → `esp32` (Espressif Systems). Testowane na 3.3.8.
3. **Biblioteka „VL53L1X” od Pololu** – Menedżer bibliotek. (Dla czujnika VL53L0X: biblioteka „VL53L0X” od Pololu.)
4. **Ustawienia płytki** (menu Narzędzia):
   - Płytka: **`ESP32C6 Dev Module`**
   - **USB CDC On Boot: `Enabled`** – przy natywnym porcie USB. Inaczej Serial Monitor nic nie pokaże!
   - Serial Monitor: **115200 baud**, zakończenie linii **Nowa linia**.

Z linii poleceń:

```bash
arduino-cli core install esp32:esp32@3.3.8 --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli lib install VL53L1X
arduino-cli compile --fqbn esp32:esp32:esp32c6 uczen/L01_miganie
```

---

## 🔌 Połączenia

Płytka: **ESP32-C6-DevKitC-1**. Wszystkie piny są w bloku `KONFIGURACJA` na górze każdego szkicu.

| Co | Pin | Uwagi |
|:--|:--|:--|
| I2C SDA | `6` | MPU6050 (`0x68`) i VL53L1X (`0x29`) na wspólnej magistrali |
| I2C SCL | `7` | |
| Zasilanie czujników | `3V3`, `GND` | tylko 3,3 V |
| M1 | `18` | przód-lewy, **CW** |
| M2 | `19` | przód-prawy, **CCW** |
| M3 | `20` | tył-prawy, **CW** |
| M4 | `21` | tył-lewy, **CCW** |
| Dioda stanu | `RGB_BUILTIN` (GPIO8) | czerwona = rozbrojony, zielona = uzbrojony |
| STOP awaryjny | `9` (przycisk BOOT) | czytany po starcie jako wejście |
| Dioda zewnętrzna (L02, opcja) | `22` | przez rezystor 220 Ω |
| Zasilanie silników | kabel z ziemi | wspólna masa z ESP32 |

Nie używać dla silników: `4`, `5`, `8`, `9`, `15` (piny startowe), `12`, `13` (USB), `16`, `17` (UART0).

```
            PRZÓD
   M1 (CW)         M2 (CCW)
        \         /
         [  ESP  ]
        /         \
   M4 (CCW)        M3 (CW)
```

Silniki po przekątnej kręcą się w tę samą stronę. Kolejność i kierunki sprawdza lekcja L04 (komenda `t`).

### Tryb sterowników

Jeden `#define` na górze szkicu:

| Tryb | Sygnał |
|:--|:--|
| `TRYB_MOSFET` *(domyślny)* | LEDC 20 kHz, 10 bitów, moc 0–100% = wypełnienie |
| `TRYB_ESC_RC` | impulsy 1000–2000 µs, 50 Hz (`ESC_HZ`), LEDC 14 bitów |

Dwa tryby naraz = czytelny błąd kompilacji.

---

## 🗺️ Mapa lekcji

| Zajęcia | Lekcja | Temat |
|:--|:--|:--|
| **3** – Arduino i silniki | `L01_miganie` | `setup()`/`loop()`, Serial, mruganie, SOS |
| | `L02_pwm_led` | PWM ręcznie (okres 2 s) → coraz szybciej → LEDC |
| | `L03_jeden_silnik` | moc M1 z Serial Monitora, rampa, `MAX_MOC` |
| | `L04_cztery_silniki` | tablice, `for`, test kolejności i kierunków |
| | `L05_skaner_i2c` | skaner adresów 1–126, nazwy urządzeń |
| **4** – utrzymanie wysokości | `L06_mpu6050` *(dodatkowa)* | rejestry przez `Wire`, g i °/s, kąt |
| | `L07_laser` | VL53L1X Short, 33 ms, status pomiaru |
| | `L08_filtr` | surowy / średnia z N / filtr wykładniczy |
| | `L09_trzymaj_wysokosc` | PID, komendy `h p i d z`, anti-windup |
| **5** – zawody | `L10_misja` | maszyna stanów, tablica kroków misji |

---

## 🛑 Bezpieczeństwo (w każdym szkicu z silnikami)

- Pierwsza rzecz w `setup()`: wszystkie wyjścia silników na 0.
- Start **rozbrojony**. `u` = uzbrój, `x` albo spacja = natychmiastowy STOP, przycisk BOOT = STOP.
- `MAX_MOC` = 60% – kod nigdy go nie przekracza (także w `TRYB_ESC_RC`: max 1600 µs).
- Moc zmienia się zawsze przez rampę.
- L09 i L10: STOP przy braku pomiaru > 200 ms, wysokości > `WYSOKOSC_MAX` i po `CZAS_MAX` lotu, z wypisaniem powodu.

---

## 🛠️ Najczęstsze problemy

| Problem | Co sprawdzić |
|:--|:--|
| **Brak portu** | kabel z danymi (nie tylko ładowanie), złącze `USB`; Linux: grupa `dialout`; BOOT+RST |
| **Pusty Serial Monitor** | **USB CDC On Boot: Enabled**, 115200 baud, ponowne otwarcie monitora + RST |
| **Skaner nic nie widzi** | SDA=6, SCL=7, nie zamienione; zasilanie 3,3 V i GND; krótkie kable |
| **Silnik się nie kręci** | uzbrojony (`u`, zielona dioda)? moc powyżej progu ruszenia? zasilanie silników? dobry tryb i pin? |
| **Reset przy starcie silników** | spadek napięcia: mocniejszy zasilacz, grubszy kabel, kondensator przy sterownikach, diody gaszące, mniejsza `RAMPA` |

Więcej: [Najczęstsze problemy](https://onikw.github.io/dron-ud/start/problemy/).

---

## 🌐 Strona kursu lokalnie

```bash
pip install -r requirements.txt
mkdocs serve
```

Strona buduje się sama (GitHub Actions → GitHub Pages) po każdym pushu do `main`.
