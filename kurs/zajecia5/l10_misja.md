# 🏆 L10: Misja – zawody!

Wasz dron umie już trzymać wysokość. Teraz zaplanujecie mu całą **misję**: wznieś się, leć na różne wysokości, wyląduj. Wygrywa zespół, którego dron najdokładniej wykona misję!

## 🎯 Czego się nauczysz

- Co to jest **maszyna stanów**.
- Jak zaplanować lot jako listę kroków (**tablicę**).

## 🔌 Co podłączyć

Tak jak w L09: dron na prowadnicy, silniki, laser w dół.

## 🧠 Jak to działa

**Maszyna stanów:** dron jest zawsze w **jednym** stanie – jak postać w grze, która stoi, skacze albo biegnie. Gdy coś się stanie, przechodzi do innego stanu.

```text
 CZEKAJ --"s"--> START --2 s--> MISJA --kroki gotowe--> LĄDOWANIE
   ^                           (krok 1, 2, 3 ...)           |
   |                                                        v
   +------------- STOP (x, BOOT, zabezpieczenia) <------ KONIEC
```

| Stan | Co robi dron |
|:--|:--|
| **CZEKAJ** | stoi; czeka na `u` i `s` |
| **START** | wznosi się na `WYSOKOSC_STARTU` (150 mm) przez 2 s |
| **MISJA** | wykonuje po kolei kroki z tablicy `KROKI` |
| **LĄDOWANIE** | płynnie schodzi w dół, przy ziemi wyłącza silniki |
| **KONIEC** | rozbraja się i wypisuje czas misji |

Misja to tablica. Każda linijka to jeden krok: **{wysokość w mm, czas w ms}** – „leć na tę wysokość i trzymaj ją przez tyle czasu”.

```cpp
const int KROKI[][2] = {
  {200, 3000},   // 200 mm przez 3 s
  {300, 3000},   // 300 mm przez 3 s
};
```

Wysokość zadana nie skacze, tylko płynnie „jedzie” do nowej wartości (`PREDKOSC_ZADANEJ`). Wysokość trzyma **ten sam regulator PID** co w L09, z tymi samymi zabezpieczeniami.

### Komendy

| Komenda | Co robi |
|:--|:--|
| `u` | uzbrój |
| `s` | start misji |
| `x` lub spacja | STOP |
| `p`, `i`, `d`, `z`, `?` | nastawy regulatora, jak w L09 |

> [!IMPORTANT] Nastawy z L09
> Zanim zaczniecie, wpiszcie w bloku KONFIGURACJA swoje `moc_zawisu`, `Kp`, `Ki` i `Kd` z lekcji L09.

## 💻 Kod

[Pobierz plik](https://github.com/onikw/dron-ud/blob/main/uczen/L10_misja/L10_misja.ino)

```cpp
--8<-- "uczen/L10_misja/L10_misja.ino"
```

## ✅ Zadanie

1. Zaplanujcie misję: zmieńcie **tylko** tablicę `KROKI`. Wysokości od 100 do 700 mm, cała misja krótsza niż 60 s (program to sprawdzi).

> [!TIP] Pomysły na misję
> - **Schody:** 200, 300, 400, 500 mm.
> - **Huśtawka:** 200, 500, 200, 500 mm.
> - **Precyzja:** 333 mm przez 10 s – kto będzie najbliżej?

## 👀 Co powinieneś zobaczyć

Na Plotterze: `zadana` rysuje „plan” misji, `wysokosc` próbuje za nim nadążyć, `stan_x100` pokazuje stan (0 = CZEKAJ, 100 = START, 200 = MISJA, 300 = LĄDOWANIE, 400 = KONIEC). Im bliżej siebie są `zadana` i `wysokosc`, tym lepiej!

??? success "Pokaż przykładową misję prowadzącego"

    ```cpp
    --8<-- "prowadzacy/L10_misja/L10_misja.ino"
    ```
