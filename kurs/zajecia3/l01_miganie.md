# L01: Miganie diodą – pierwszy program

Każdy programista zaczyna od tego samego: **mrugającej diody**. To jak „Hello World” w świecie elektroniki. Przy okazji płytka pierwszy raz „napisze” coś do komputera.

## 🎯 Czego się nauczysz

- Każdy program Arduino ma dwie części: `setup()` i `loop()`.
- Jak włączyć i wyłączyć diodę.
- Jak płytka wysyła tekst do komputera (**Serial Monitor**).

## 🔌 Co podłączyć

Tylko kabel USB. Dioda jest już na płytce.

## 🧠 Jak to działa

`setup()` wykonuje się **raz**, zaraz po włączeniu. `loop()` wykonuje się **w kółko**, bez końca.

```text
   start
     |
  setup()        <- raz
     |
  loop()  <--+   <- w kółko
     |       |
     +-------+
```

**Serial (UART)** to rozmowa płytki z komputerem po kablu. Płytka wysyła bity – zera i jedynki – jeden po drugim, z prędkością **115200 bitów na sekundę**. Komputer składa je w litery i pokazuje w Serial Monitorze. Obie strony muszą „mówić” w tym samym tempie, dlatego w programie piszemy `Serial.begin(115200)` i tę samą liczbę wybieramy w Serial Monitorze.

Nowe słowa w kodzie:

| Kod | Co znaczy |
|:--|:--|
| `pinMode(pin, OUTPUT)` | ten pin będzie **wyjściem** (będzie coś włączał) |
| `digitalWrite(pin, HIGH)` | włącz (`LOW` – wyłącz) |
| `delay(500)` | czekaj 500 milisekund (pół sekundy) |
| `Serial.println("…")` | wyślij tekst do komputera i przejdź do nowej linii |
| `int licznik = 0;` | **zmienna** – pudełko na liczbę, która może się zmieniać |

## 💻 Kod

Otwórz `uczen/L01_miganie/L01_miganie.ino` albo skopiuj kod poniżej. [Pobierz plik](https://github.com/onikw/dron-ud/blob/main/uczen/L01_miganie/L01_miganie.ino)

```cpp
--8<-- "uczen/L01_miganie/L01_miganie.ino"
```

## ✅ Zadania

1. **Tempo.** Zmień `TEMPO_MS` na `100`, potem na `2000`. Wgraj i patrz na diodę.
2. **SOS.** Dokończ funkcję `sygnalSOS()`: 3 krótkie, 3 długie, 3 krótkie mrugnięcia. Potem zmień `TRYB_SOS` na `true`.
3. **Licznik.** W `loop()` zwiększaj `licznik` o 1 i wypisuj go w Serial Monitorze.

## 👀 Co powinieneś zobaczyć

- Dioda mruga. Po zadaniu 2 nadaje `... --- ...`.
- W Serial Monitorze: *Mrugnięcie numer: 1, 2, 3…*

> [!TIP] Ciekawostka
> SOS to sygnał ratunkowy w alfabecie Morse’a. Wybrano go, bo jest prosty i trudno go z czymś pomylić – nawet gdy słychać go bardzo słabo.

??? success "Pokaż rozwiązanie"

    ```cpp
    --8<-- "prowadzacy/L01_miganie/L01_miganie.ino"
    ```
