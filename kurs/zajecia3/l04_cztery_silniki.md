# L04: Cztery silniki

Dron ma cztery silniki. Czy trzeba pisać cztery razy ten sam kod? Nie! Wystarczy **tablica** i **pętla `for`**.

## 🎯 Czego się nauczysz

- Co to jest **tablica**: jedna nazwa, kilka ponumerowanych szufladek.
- Jak pętla `for` robi to samo dla każdego silnika.
- Jak sprawdzić, czy silniki są dobrze podłączone i kręcą się w dobrą stronę.

## 🔌 Co podłączyć

Dron na prowadnicy, zasilanie silników, USB. Silniki M1–M4 na pinach 18, 19, 20, 21.

## 🧠 Jak to działa

```cpp
const int PINY_SILNIKOW[4] = {18, 19, 20, 21};
```

To 4 szufladki. Numeracja zaczyna się od **zera**: `PINY_SILNIKOW[0]` to 18 (M1), `PINY_SILNIKOW[3]` to 21 (M4).

```cpp
for (int i = 0; i < 4; i++) {
  // to się wykona dla i = 0, 1, 2, 3
}
```

Widok drona z góry:

```text
            PRZÓD
   M1 (CW)         M2 (CCW)
        \         /
         [  ESP  ]
        /         \
   M4 (CCW)        M3 (CW)
```

Silniki **po przekątnej** kręcą się w tę samą stronę. Gdyby wszystkie kręciły się w jedną, dron obracałby się wokół własnej osi (jak bąk), bo każde śmigło „odpycha” powietrze trochę w bok.

### Komendy

| Komenda | Co robi |
|:--|:--|
| `u` | uzbrój |
| `t` | **test kolejności**: każdy silnik po kolei przez 1 s na niskiej mocy |
| `r` | **rozpędzanie** wszystkich do `MOC_POKAZU` i zwalnianie |
| `s` | **schody** (zadanie 2) |
| `x` lub spacja | STOP |

Podczas testu w Serial Monitorze pojawia się, który silnik powinien się kręcić i w którą stronę. Sprawdźcie to patrząc na drona!

## 💻 Kod

[Pobierz plik](https://github.com/onikw/dron-ud/blob/main/uczen/L04_cztery_silniki/L04_cztery_silniki.ino)

```cpp
--8<-- "uczen/L04_cztery_silniki/L04_cztery_silniki.ino"
```

## ✅ Zadania

1. **Czas rozpędzania.** Zmień `CZAS_ROZPEDZANIA_MS` na `1000`, potem `8000`. Przy `1000` silniki nie zdążą – **rampa bezpieczeństwa** nie pozwala zwiększać mocy szybciej niż `RAMPA`. Zobacz to na Plotterze.
2. **Schody.** Napisz funkcję `schody()`: moc 10, 20, 30, 40%, każda przez 1,5 s. Użyj pętli `for`, `ustawWszystkie(moc)` i `czekaj(ms)`.

> [!TIP] Dlaczego `czekaj()`, a nie `delay()`?
> Podczas `delay()` płytka nic nie robi – nie usłyszałaby nawet komendy STOP! Funkcja `czekaj()` w tym czasie cały czas sprawdza klawiaturę i przycisk BOOT. Gdy był STOP, zwraca `false`, i wtedy przerywamy pokaz: `if (!czekaj(1500)) return;`

## 👀 Co powinieneś zobaczyć

- Test `t`: na Plotterze po kolei „górka” M1, M2, M3, M4. Na dronie – po kolei kręcące się śmigła.
- Rozpędzanie `r`: cztery linie rosną razem, chwilę stoją i opadają.
- Schody `s`: cztery stopnie.

??? success "Pokaż rozwiązanie"

    ```cpp
    --8<-- "prowadzacy/L04_cztery_silniki/L04_cztery_silniki.ino"
    ```
