# 🧰 Krok 1: Instalacja

Zanim dron poleci, musimy przygotować komputer. Zrobimy to **raz**, na początku kursu. Zajmie to około 15 minut (najdłużej trwa pobieranie).

Co zainstalujemy:

1. **Arduino IDE** – program, w którym piszemy kod i wysyłamy go na płytkę.
2. **Pakiet esp32** – żeby Arduino IDE znało naszą płytkę ESP32-C6.
3. **Bibliotekę VL53L1X** – gotowy kod do obsługi laserowego czujnika odległości.

---

## 1️⃣ Arduino IDE

1. Wejdź na stronę 🔗 [arduino.cc/en/software](https://www.arduino.cc/en/software).
2. Pobierz **Arduino IDE 2.x** dla swojego systemu (Windows, macOS albo Linux).
3. Zainstaluj go jak każdy inny program i uruchom.

> [!NOTE] Co to jest „IDE”?
> IDE to edytor dla programistów. Piszesz w nim kod, a po kliknięciu jednego przycisku program zamienia go na język, który rozumie płytka, i wysyła go przez kabel USB.

---

## 2️⃣ Pakiet esp32 (obsługa płytki)

Arduino IDE na starcie zna tylko płytki firmy Arduino. Naszą płytkę zrobiła firma **Espressif**, więc trzeba doinstalować jej pakiet.

1. Kliknij ikonę **płytki** na lewym pasku (Menedżer płytek).
2. W wyszukiwarce wpisz **`esp32`**.
3. Znajdź pakiet **„esp32” od Espressif Systems** (nie „Arduino ESP32 Boards”!).
4. Wybierz wersję **3.x** (najnowszą) i kliknij **Instaluj**. To może potrwać kilka minut.

> [!WARNING] Wersja musi być 3.x
> Nasze programy używają nowych funkcji `ledcAttach()` i `ledcWrite()`. W starych wersjach 2.x ich nie ma i kompilacja skończy się błędem.

---

## 3️⃣ Biblioteka czujnika laserowego

1. Kliknij ikonę **książek** na lewym pasku (Menedżer bibliotek).
2. Wpisz **`VL53L1X`**.
3. Zainstaluj bibliotekę **„VL53L1X” od Pololu**.

> [!TIP] Inny czujnik?
> Jeśli na Waszym dronie jest starszy czujnik **VL53L0X**, zainstalujcie bibliotekę **„VL53L0X” od Pololu**. Prowadzący pomoże zmienić kilka linijek w lekcjach L07–L10.

Biblioteka `Wire` (do I2C) jest już w pakiecie esp32 – nie trzeba jej instalować.

---

## 4️⃣ Ustawienia płytki

Podłącz płytkę kablem USB do złącza oznaczonego **`USB`**. Potem w menu **Narzędzia**:

| Ustawienie | Wartość |
|:--|:--|
| **Płytka** | `esp32` → **`ESP32C6 Dev Module`** |
| **Port** | port płytki, np. `COM5` (Windows) albo `/dev/ttyACM0` (Linux) |
| **USB CDC On Boot** | **`Enabled`** |

> [!IMPORTANT] USB CDC On Boot: Enabled
> Bez tego ustawienia program wgra się dobrze, ale **Serial Monitor będzie pusty**. Płytka „mówiłaby” na innym złączu niż to, do którego jest podłączony kabel.
> (Jeśli używasz złącza oznaczonego `UART`, to ustawienie nie jest potrzebne, ale może trzeba zainstalować sterownik CP210x.)

> [!TIP] Który port to moja płytka?
> Rozwiń listę **Narzędzia → Port**. Odłącz płytkę i rozwiń listę jeszcze raz. Port, który zniknął, to Twoja płytka. Podłącz ją z powrotem.

> [!WARNING] Linux: brak dostępu do portu
> Jeśli wgrywanie kończy się błędem *Permission denied*, w terminalu wpisz:
> ```bash
> sudo usermod -a -G dialout $USER
> ```
> potem wyloguj się i zaloguj ponownie.

---

## 5️⃣ Pobranie programów kursu

1. Wejdź na 🔗 [github.com/onikw/dron-ud](https://github.com/onikw/dron-ud).
2. Kliknij zielony przycisk **Code** → **Download ZIP**.
3. Rozpakuj plik. W folderze `uczen` są Wasze lekcje: `L01_miganie`, `L02_pwm_led` …
4. W Arduino IDE: **Plik → Otwórz** i wybierz plik `.ino` z folderu lekcji.

> [!NOTE] Folder = nazwa szkicu
> Arduino wymaga, żeby plik `L01_miganie.ino` leżał w folderze `L01_miganie`. Nie zmieniajcie nazw folderów.

Kod każdej lekcji jest też na stronach tego kursu – możesz go skopiować przyciskiem w rogu ramki z kodem.

---

## 6️⃣ Test: czy wszystko działa?

1. Otwórz lekcję **`uczen/L01_miganie`**.
2. Kliknij **strzałkę w prawo** (Wgraj) albo naciśnij `Ctrl + U`.
3. Poczekaj na napis *Hard resetting via RTS pin…* na dole.
4. Otwórz **Serial Monitor** (ikona lupy w prawym górnym rogu, `Ctrl + Shift + M`).
5. Ustaw prędkość **`115200 baud`** i zakończenie linii **`Nowa linia`** (*New Line*).

Jeśli dioda mruga, a w Serial Monitorze widzisz *„Cześć! Tu dron.”* – wszystko gotowe! 🎉

> [!TIP] Nie widzę pierwszego napisu
> Płytka mogła wysłać go, zanim Serial Monitor zdążył się połączyć. Naciśnij przycisk **RST** na płytce – program zacznie się od początku.

> [!WARNING] Wgrywanie nie działa (*Failed to connect*)
> 1. Kliknij **Wgraj**.
> 2. Gdy pojawi się `Connecting...`, **przytrzymaj** przycisk **BOOT** na płytce.
> 3. Naciśnij i puść **RST**, a potem puść **BOOT**.

Więcej pomocy: [Najczęstsze problemy](problemy.md).

---

## 🧑‍🏫 Dla prowadzących: instalacja z linii poleceń

Jeśli przygotowujecie wiele komputerów, można to zrobić skryptem z `arduino-cli`:

```bash
arduino-cli core update-index --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.8 --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli lib install VL53L1X
arduino-cli compile --fqbn esp32:esp32:esp32c6 uczen/L01_miganie
```

> [!NOTE] Wersja pakietu
> Kursu użyliśmy z pakietem **esp32 3.3.8**. Podanie wersji wprost (`@3.3.8`) chroni przed sytuacją, w której `arduino-cli` wybiera starą wersję 2.x. Wgrywanie z linii poleceń: `arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32c6:CDCOnBoot=cdc uczen/L01_miganie` (wtedy też kompiluj z `:CDCOnBoot=cdc`).
