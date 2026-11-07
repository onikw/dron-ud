# Inżynieria dronowa – poradnik i szkice Arduino

Strona-poradnik i komplet szkiców Arduino do kursu „Inżynieria dronowa” (Uniwersytet Dzieci, AGH Drone Engineering). Uczniowie 12–14 lat, zespoły 3-osobowe, dron na pionowej prowadnicy, ESP32-C6.

- **Strona:** czysty HTML, CSS i JS, bez frameworków, CDN i fontów z internetu. Działa po otwarciu `index.html` z dysku i na GitHub Pages.
- **Szkice:** `szkice/uczen/` – wersje z lukami `// ZADANIE n:` (zawsze się kompilują i działają bezpiecznie), `szkice/prowadzacy/` – pełne rozwiązania.
- **Strona prowadzącego:** `prowadzacy.html` (bez linku w menu ucznia) – plan zajęć 2 minuta po minucie, odpowiedzi do „Sprawdź się”, test biegunowości, checklista przygotowań.

## Uruchomienie strony

- **Z dysku:** otwórz `index.html` w przeglądarce. Internet nie jest potrzebny.
- **Online:** po pushu do `main` GitHub Actions (`.github/workflows/strona.yml`) sprawdza zgodność kodu i publikuje stronę na GitHub Pages.

Postęp (odhaczone kroki) zapisuje się w przeglądarce (localStorage). Bez localStorage strona też działa, tylko nie pamięta postępu.

## Struktura

```
index.html                strona startowa z planem 5 zajęć
zajecia-1.html            podsumowanie zajęć 1
zajecia-2/01..10-*.html   zajęcia 2 (rozdział na plik)
zajecia-3/, zajecia-4/, zajecia-5/
slowniczek.html, problemy.html
prowadzacy.html           dla prowadzącego (bez linku w menu)
css/style.css             wygląd, telefon, druk
js/config.js              << WSZYSTKIE WARTOŚCI SPRZĘTOWE STRONY >>
js/app.js                 menu, postęp, karty, kod, dymki słowniczka
js/rysunki.js             interaktywne rysunki SVG
js/quizy.js               pytania „Sprawdź się” (+ odpowiedzi na stronie prowadzącego)
js/slowniczek.js          hasła słowniczka
js/szkice.js              GENEROWANY – kod szkiców ucznia dla strony
narzedzia/wklej_szkice.py wkleja .ino do js/szkice.js i sprawdza zgodność
szkice/uczen/Lxx_*/       wersje z lukami
szkice/prowadzacy/Lxx_*/  rozwiązania
img/                      logo-ud.png
```

## Lekcje

| Zajęcia | Szkic | Temat |
|:--|:--|:--|
| 2 (7.11) | `L01_pierwszy_program` | Serial (UART), mruganie diodą RGB_BUILTIN |
| 2 | `L02_skaner_i2c` | skan adresów 1–126 z nazwami urządzeń |
| 2 (dla chętnych) | `L03_wysokosc` | czujnik odległości VL53L0X, 33 ms, Plotter |
| 2 (dla chętnych) | `L04_akcelerometr` | MPU6050 przez Wire: WHO_AM_I, rejestry, g i °/s |
| 3 (21.11) | `L05_pwm_dioda` | PWM ręcznie i wolno, potem LEDC |
| 3 | `L06_jeden_silnik` | moc z Serial Monitora, rampa, MAX_MOC |
| 3 | `L07_cztery_silniki` | tablica pinów, test kolejności i kierunków |
| 4 (5.12) | `L08_filtr` | surowy / średnia z N / filtr wykładniczy |
| 4 | `L09_trzymaj_wysokosc` | moc zawisu + PID, komendy, anti-windup |
| 5 (19.12) | `L10_misja` | maszyna stanów, tablica kroków, start „s” |

Każdy szkic jest samodzielny (tylko `Wire` i biblioteka czujnika). Czujnik odległości to **VL53L0X** (biblioteka Pololu „VL53L0X”). Do rozpoznania modelu służy `narzedzia/rozpoznaj_laser/`. I2C: `Wire.setClock(100000)` i `Wire.setTimeOut(50)` (kabel ok. 1 m). Szkice z silnikami: najpierw wyjścia ESC na 0, start rozbrojony, `u` uzbraja, `x`/spacja i BOOT (GPIO9) to STOP, `MAX_MOC` 60%, zawsze rampa. L09 i L10: STOP przy braku pomiaru > 200 ms, wysokości > `WYSOKOSC_MAX` i po `CZAS_MAX`, z wypisaniem powodu. Tryb ESC wybiera jeden `#define`: `TRYB_MOSFET` (domyślny, LEDC 20 kHz, 10 bitów) albo `TRYB_ESC_RC` (1000–2000 µs, 50 Hz).

## Jak zmienić piny i układ silników

1. **Strona:** zmień wartości w `js/config.js` – piny (`piny`), układ silników (`silniki`), typy i biegunowość (`typySilnikow`), czujnik (`czujnikOdleglosci`), kolory przewodów (`przewodyI2C`), opis mocowania (`mocowanieCzujnikow`). Schematy, tabele i ćwiczenie z orientacją przerysują się same.
2. **Szkice:** zmień blok `KONFIGURACJA` na górze każdego szkicu (`PIN_SDA`, `PIN_SCL`, `PINY_SILNIKOW`), w obu folderach.
3. Uruchom:
   ```bash
   python3 narzedzia/wklej_szkice.py
   ```
   Skrypt wkleja kod do strony i **sprawdza, czy piny w szkicach zgadzają się z `config.js`** (`--sprawdz` tylko sprawdza – to samo robi GitHub Actions).

Po każdej zmianie pliku `.ino` uruchom ten skrypt – inaczej kod na stronie będzie nieaktualny.

## Wartości do potwierdzenia

Te wartości pochodzą z planu kursu i **nie zostały sprawdzone na sprzęcie** (lista jest też w `config.js` → `doPotwierdzenia` i na stronie prowadzącego):

- płytka: ESP32-C6-DevKitC-1
- piny: SDA 6, SCL 7, ESC M1–M4: 18, 19, 20, 21
- układ silników: M1 przód-lewy CW, M2 przód-prawy CCW, M3 tył-prawy CW, M4 tył-lewy CCW
- biegunowość: typ A czerwony +, niebieski −; typ B biały +, czarny − (test przed lutowaniem – instrukcja na `prowadzacy.html`)
- kolory przewodów I2C: czerwony 3V3, czarny GND, żółty SDA, niebieski SCL
- mocowanie czujników: puste – uzupełnij `mocowanieCzujnikow`
- pinout płytki w `config.js` (`pinout`) – używany tylko w rysunkach, sprawdź z nadrukiem na płytce

Na stronach są też ramki „Do uzupełnienia przez prowadzącego”: kolory sygnałów PWM i wpięcie masy (zajęcia 3), zasady zawodów (zajęcia 5).


## Instalacja Arduino IDE i rdzenia esp32 (Windows 11)

1. Pobierz **Arduino IDE 2.x** ze strony <https://www.arduino.cc/en/software> (Windows, „Win 10 and newer, 64 bits”) i zainstaluj. Przy pytaniu o sterowniki USB zgódź się na instalację.
2. Uruchom Arduino IDE. **Plik → Preferencje** → w polu „Dodatkowe adresy URL do menedżera płytek” wklej:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   (nowsze wersje IDE mają ten pakiet już na liście – wtedy ten krok można pominąć).
3. **Menedżer płytek** (ikona płytki po lewej) → wpisz `esp32` → pakiet **„esp32” od Espressif Systems** → wybierz wersję **3.x** → **Instaluj** (kilkaset MB, kilka minut).
4. **Menedżer bibliotek** (ikona książek) → wpisz `VL53L0X` → zainstaluj **„VL53L0X” od Pololu**.
5. Podłącz ESP32-C6 kablem USB do złącza oznaczonego **USB**. Windows 11 rozpozna płytkę bez dodatkowych sterowników (port pojawi się jako `COMx`).
6. **Narzędzia → Płytka → esp32 → ESP32C6 Dev Module**, **Narzędzia → Port → COMx**, **Narzędzia → USB CDC On Boot → Enabled** (bez tego Serial Monitor jest pusty).
7. Test: otwórz `szkice/uczen/L01_pierwszy_program/L01_pierwszy_program.ino`, kliknij **Wgraj**, otwórz Serial Monitor (115200).

Jeśli używasz złącza **UART** zamiast USB, może być potrzebny sterownik CP210x (Silicon Labs) – wtedy „USB CDC On Boot” zostaw wyłączone.

### Z linii poleceń (dla przygotowania wielu komputerów)

```bash
arduino-cli core install esp32:esp32@3.3.8 --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli lib install VL53L0X
arduino-cli compile --fqbn esp32:esp32:esp32c6:CDCOnBoot=cdc szkice/uczen/L01_pierwszy_program
```

Wersję podaj wprost – bez niej `arduino-cli` potrafi wybrać starą wersję 2.x, w której nie ma `ledcAttach()`. Wszystkie szkice z obu folderów były kompilowane z rdzeniem esp32 3.3.8 i biblioteką Pololu VL53L0X 1.3.1.
