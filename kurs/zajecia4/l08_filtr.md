# L08: Filtr – jak wygładzić skaczący pomiar

Pomiar z lasera trochę „skacze”, nawet gdy nic się nie rusza. Gdyby dron wierzył każdemu skokowi, silniki by szarpały. Potrzebujemy **filtra**.

## 🎯 Czego się nauczysz

- Dlaczego pomiary skaczą (**szum**).
- **Średnia** z N ostatnich pomiarów.
- **Filtr wykładniczy** (z liczbą `ALFA`).
- Że gładszy wykres = większe **opóźnienie**.

## 🔌 Co podłączyć

Laser VL53L1X, tak jak w L07.

## 🧠 Jak to działa

Na Plotterze są trzy linie z **tego samego** pomiaru:

| Linia | Co to jest |
|:--|:--|
| `surowy` | prosto z czujnika, skacze |
| `srednia` | średnia z `N` ostatnich pomiarów |
| `wykladniczy` | `nowy = ALFA × pomiar + (1 − ALFA) × stary` |

**Średnia** – jak ocena na koniec roku: liczysz kilka ostatnich ocen, więc jedna pała nie zmienia wszystkiego.

**Wykładniczy** – jak plotka: każda nowa wiadomość trochę zmienia Twoje zdanie.
`ALFA = 1` – wierzysz tylko najnowszej wiadomości (brak filtra).
`ALFA = 0,1` – zmieniasz zdanie bardzo powoli (mocny filtr).

```text
 surowy:   /\/\_/\/\__/\/\/\
 filtr:    ~~~~~~~~~~~~~~~~~   gładko, ale ze spóźnieniem
```

> [!NOTE] Tablica „w kółko”
> Ostatnie pomiary trzymamy w tablicy `ostatnie`. Nowy pomiar wpisujemy w miejsce najstarszego, a `gdzie` przesuwa się 0, 1, 2 … N−1 i znowu 0. Dzięki temu nie trzeba niczego przesuwać.

## 💻 Kod

[Pobierz plik](https://github.com/onikw/dron-ud/blob/main/uczen/L08_filtr/L08_filtr.ino)

```cpp
--8<-- "uczen/L08_filtr/L08_filtr.ino"
```

## ✅ Zadania

1. Zmień `N` na `2`, `10`, `20`. Co się dzieje z linią `srednia`?
2. Zmień `ALFA` na `0.8`, `0.3`, `0.05`. Co się dzieje z linią `wykladniczy`?
3. **Szybko** przesuń dłoń nad laserem w górę. Która linia dochodzi do nowej wartości najpóźniej? Dlaczego to może być problem dla drona?

## 👀 Co powinieneś zobaczyć

Im mocniejszy filtr (większe `N`, mniejsze `ALFA`), tym **gładsza** linia – ale tym bardziej **spóźniona**. Dron z bardzo mocnym filtrem „dowiaduje się” o spadaniu za późno. Trzeba wybrać coś pośrodku.

??? success "Pokaż rozwiązanie"

    ```cpp
    --8<-- "prowadzacy/L08_filtr/L08_filtr.ino"
    ```
