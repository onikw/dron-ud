# 🛠️ Najczęstsze problemy

## Brak portu w menu Narzędzia → Port

- **Kabel tylko do ładowania.** Niektóre kable USB nie przesyłają danych. Weź inny.
- **Złe złącze.** Używamy złącza oznaczonego **`USB`**.
- **Linux:** brak uprawnień. Wpisz `sudo usermod -a -G dialout $USER`, wyloguj się i zaloguj.
- **Płytka „zawiesiła” USB.** Przytrzymaj **BOOT**, naciśnij **RST**, puść **BOOT**. Port powinien wrócić, wgraj program jeszcze raz.

## Serial Monitor jest pusty

- **USB CDC On Boot** musi być **`Enabled`** (menu Narzędzia). Po zmianie wgraj program jeszcze raz.
- Prędkość w prawym dolnym rogu: **`115200 baud`**.
- Po wgraniu monitor mógł się rozłączyć: zamknij go i otwórz ponownie, potem naciśnij **RST** na płytce.
- Komendy nie działają? Ustaw zakończenie linii na **`Nowa linia`** (*New Line*).

## Skaner I2C nic nie widzi

- Sprawdź kable: **SDA → 6**, **SCL → 7**, czujniki do **3,3 V** i **GND**.
- SDA i SCL nie mogą być zamienione miejscami.
- Kable I2C powinny być krótkie (do ok. 20–30 cm).
- Widać tylko jedno urządzenie? Drugie ma luźny kabel albo jest uszkodzone. Sprawdź je osobno.
- MPU6050 widać pod `0x69` zamiast `0x68`? Pin AD0 czujnika jest podłączony do 3,3 V. To też działa, ale trzeba zmienić adres w L06.

## Silnik się nie kręci

- Czy uzbroiłeś silniki komendą **`u`**? Dioda musi być **zielona**.
- Moc za mała – silnik 816 rusza dopiero od kilkunastu procent (L03, zadanie 1).
- Czy jest zasilanie silników (kabel z ziemi)? Samo USB nie wystarczy.
- Czy wybrany jest dobry tryb (`TRYB_MOSFET` albo `TRYB_ESC_RC`)?
- Zły pin? Sprawdź tabelę w [Nasz dron](dron.md) i blok KONFIGURACJA.
- Silnik buczy, ale się nie kręci: pewnie zła częstotliwość – wróć do `PWM_HZ = 20000`.

## Płytka resetuje się przy starcie silników

To prawie zawsze **zasilanie**: silniki biorą duży prąd, napięcie na chwilę spada i ESP32 się restartuje.

- Zasilacz za słaby albo kabel zasilający za cienki/za długi.
- Brak kondensatora przy zasilaniu silników (np. 470 µF lub więcej, blisko sterowników).
- Brak diody gaszącej przy silniku (dioda równolegle do silnika, katodą do plusa).
- Masa (GND) ESP32 i silników musi być wspólna, ale prąd silników nie powinien płynąć przez płytkę ESP32.
- Zmniejsz `RAMPA`, żeby prąd rósł wolniej.

W Serial Monitorze widać wtedy znowu napis powitalny z `setup()` – to znak, że płytka się zrestartowała.

## Dron w L09 sam się zatrzymuje

Przeczytaj powód w Serial Monitorze:

- **„brak pomiaru z lasera”** – laser nie widzi podłogi (za blisko, za daleko, coś go zasłania) albo luźny kabel I2C.
- **„za wysoko”** – dron przekroczył `WYSOKOSC_MAX`. Zmniejsz Kp albo moc zawisu.
- **„minął CZAS_MAX lotu”** – lot trwał dłużej niż 60 s. Uzbrój ponownie (`u`).
