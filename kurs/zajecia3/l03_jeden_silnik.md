# L03: Jeden silnik

Pora na prawdziwy ruch! Uruchomimy silnik **M1** i będziemy sterować jego mocą z klawiatury.

## 🎯 Czego się nauczysz

- Jak PWM steruje mocą silnika.
- Jak wysyłać komendy z Serial Monitora do płytki.
- Po co jest **rampa** (moc rośnie powoli) i limit **`MAX_MOC`**.
- Jak działa **uzbrajanie** i **STOP**.

## 🔌 Co podłączyć

- Dron na prowadnicy, zasilanie silników, kabel USB.
- Silnik M1 na pinie **18** (przez sterownik).

## 🧠 Jak to działa

Silnik to taka „duża dioda”: im większe wypełnienie PWM, tym szybciej się kręci. Sam pin ESP32 dałby za mało prądu, więc między płytką a silnikiem jest **sterownik** (tranzystor MOSFET) – szybki włącznik dużego prądu.

```text
ESP32 --PWM--> [sterownik] ==prąd==> (M1)
```

**Rampa** działa jak pedał gazu: wciskasz go powoli, a nie od razu do podłogi. Gdy wpiszesz `40`, moc nie skoczy z 0 na 40%, tylko będzie rosła o `RAMPA` procent na sekundę.

```text
 zadana:   ______          moc:       ______
          |                          /
     _____|               _______  /
```

> [!CAUTION] Bezpieczeństwo
> Silnik startuje **rozbrojony** (dioda czerwona). Najpierw `u`, potem moc. **`x` albo spacja** – natychmiastowy STOP. Przycisk **BOOT** – też STOP.

### Komendy

| Komenda | Co robi |
|:--|:--|
| `u` | uzbrój |
| `x` lub spacja | STOP |
| `0` … `100` | moc M1 w procentach (powyżej `MAX_MOC` = 60 zostanie obcięta) |

> [!NOTE] Długi kod?
> Środek programu – część **SILNIKI I BEZPIECZEŃSTWO** i **KOMENDY** – jest taki sam we wszystkich lekcjach z silnikami. Nie musisz go rozumieć linijka po linijce. Ważne są **KONFIGURACJA** na górze i **PROGRAM LEKCJI** na dole.

## 💻 Kod

[Pobierz plik](https://github.com/onikw/dron-ud/blob/main/uczen/L03_jeden_silnik/L03_jeden_silnik.ino)

```cpp
--8<-- "uczen/L03_jeden_silnik/L03_jeden_silnik.ino"
```

## ✅ Zadania

1. **Moc startu.** Wpisuj `5`, `6`, `7`… aż śmigło ruszy. Wpisz tę wartość w `MOC_STARTU` – pojawi się na wykresie jako linia `start`.
2. **Częstotliwość.** Ustaw `PWM_HZ = 200`, wgraj i posłuchaj silnika. Słychać buczenie? Potem **wróć do 20000** (20 kHz – powyżej tego, co słyszy człowiek).
3. **Rampa.** Zmień `RAMPA` na `5`, potem na `40`. Jak zmienia się pochylnia na Plotterze?

## 👀 Co powinieneś zobaczyć

Na Plotterze: `zadana` skacze schodkiem, a `moc` dochodzi do niej po pochylni. Po `x` obie spadają od razu do zera.

??? success "Pokaż rozwiązanie"

    ```cpp
    --8<-- "prowadzacy/L03_jeden_silnik/L03_jeden_silnik.ino"
    ```
