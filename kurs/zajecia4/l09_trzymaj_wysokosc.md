# L09: Trzymaj wysokość – regulator PID

To najważniejsza lekcja kursu. Dron sam będzie mierzył wysokość i sam ustawiał moc silników tak, żeby wisieć dokładnie tam, gdzie mu każecie.

## 🎯 Czego się nauczysz

- Co to jest **regulator** i **sprzężenie zwrotne**.
- Co robią liczby **Kp**, **Ki** i **Kd**.
- Jak dobrać je krok po kroku, patrząc na wykres.

## 🔌 Co podłączyć

Dron na prowadnicy, zasilanie silników, USB. Silniki M1–M4, laser patrzy **w dół** na podłogę.

## 🧠 Jak to działa

Analogia: **prysznic**. Za zimno? Kręcisz kurkiem na ciepło. Im zimniej, tym mocniej kręcisz. Tak samo dron:

```text
BŁĄD = wysokość zadana − wysokość zmierzona
(dron za nisko -> błąd dodatni -> więcej mocy)

moc = moc_zawisu + Kp·błąd + Ki·suma_błędów + Kd·zmiana_błędu
```

| Część | Po ludzku | Co robi |
|:--|:--|:--|
| `moc_zawisu` | „tyle mniej więcej trzeba” | punkt startowy |
| **Kp** · błąd | „im dalej, tym mocniej” | główny kurek |
| **Ki** · suma błędów | „ciągle za nisko? dodaję po trochu” | usuwa stały błąd |
| **Kd** · zmiana błędu | „zbliżam się szybko? hamuję” | gasi bujanie |

```text
 zadana ---> (+) --błąd--> [ PID ] --moc--> [silniki] --> dron
              ^ (−)                                        |
              +--------------- [laser] <---wysokość--------+
```

To jest **pętla sprzężenia zwrotnego**: wynik (wysokość) wraca na początek i zmienia to, co robimy. Regulator liczy nową moc przy każdym pomiarze lasera, czyli co **33 ms**.

**Anti-windup:** suma błędów ma limit `SUMA_MAX`. Bez niego, gdy dron stoi na ziemi, suma rosłaby bez końca, a po starcie dron wystrzeliłby w górę.

### Zabezpieczenia

Dron sam się zatrzyma, gdy laser nie da pomiaru przez **200 ms**, gdy będzie wyżej niż **`WYSOKOSC_MAX`** albo gdy lot potrwa dłużej niż **`CZAS_MAX`**. Powód zobaczysz w Serial Monitorze.

### Komendy

| Komenda | Co robi |
|:--|:--|
| `u` | uzbrój (tylko gdy laser mierzy) |
| `h 300` | wysokość zadana 300 mm; `h 0` – lądowanie |
| `p 0.05` | Kp |
| `i 0.002` | Ki |
| `d 0.5` | Kd |
| `z 35` | moc zawisu w % |
| `?` | pokaż nastawy |
| `x` lub spacja | STOP |

## 💻 Kod

[Pobierz plik](https://github.com/onikw/dron-ud/blob/main/uczen/L09_trzymaj_wysokosc/L09_trzymaj_wysokosc.ino)

```cpp
--8<-- "uczen/L09_trzymaj_wysokosc/L09_trzymaj_wysokosc.ino"
```

## ✅ Zadania

> [!TIP] Najpierw moc zawisu
> Uzbrój drona z `p 0` i zwiększaj `z` (np. `z 25`, `z 30`, `z 35`…). Gdy dron prawie się unosi – to Twoja moc zawisu. Potem `h 0` albo `x`.

1. **Kp.** Ustaw `Ki = 0`, `Kd = 0`. Zacznij od `p 0.02` i zwiększaj. Kiedy dron zaczyna się bujać w górę i w dół? Wybierz trochę mniejsze Kp i wpisz je w kodzie.
2. **Ki.** Dopisz do wzoru `+ Ki * suma_bledow`. Ustaw np. `i 0.002`. Co się dzieje z różnicą między `zadana` a `wysokosc`?
3. **Kd.** Dopisz do wzoru `+ Kd * zmiana_bledu`. Ustaw np. `d 0.5`. Zmień `h` z 200 na 400. Czy dron mniej „przestrzeliwuje”?
4. **Opisz** w komentarzu na górze programu, co robi Kp, Ki i Kd.

## 👀 Co powinieneś zobaczyć

Trzy linie na Plotterze: `zadana`, `wysokosc` i `moc_x10` (moc w procentach razy 10 – żeby było ją widać obok milimetrów).

```text
 tylko Kp, za małe:     ___________ zadana
                       /‾‾‾‾‾‾‾‾‾‾‾ wysokość – wisi trochę za nisko
 Kp za duże:           /\/\/\/\/\/  bujanie
 Kp + Ki + Kd:         /‾‾‾‾‾‾‾‾‾‾  szybko i dokładnie
```

??? success "Pokaż rozwiązanie"

    ```cpp
    --8<-- "prowadzacy/L09_trzymaj_wysokosc/L09_trzymaj_wysokosc.ino"
    ```
