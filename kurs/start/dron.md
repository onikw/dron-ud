# 🚁 Krok 2: Nasz dron

## 🧩 Z czego się składa

| Część | Co robi |
|:--|:--|
| **ESP32-C6-DevKitC-1** | „mózg” drona – mały komputer (mikrokontroler) |
| **4 silniki 816 + śmigła 60 mm** | dają siłę ciągu w górę |
| **4 sterowniki (MOSFET)** | szybkie włączniki dużego prądu silników |
| **MPU6050** (adres `0x68`) | czuje przechylenie i obrót |
| **VL53L1X** (adres `0x29`) | laser – mierzy odległość od podłogi |
| **Kabel zasilający z ziemi** | dron nie ma baterii, prąd płynie kablem |
| **Pionowa prowadnica** | dron lata tylko w górę i w dół |

---

## 🔌 Połączenia

| Co | Pin ESP32-C6 | Uwagi |
|:--|:--|:--|
| I2C **SDA** | `6` | wspólna dla MPU6050 i lasera |
| I2C **SCL** | `7` | wspólna dla MPU6050 i lasera |
| Zasilanie czujników | `3V3` + `GND` | **tylko 3,3 V!** |
| Silnik **M1** | `18` | przód-lewy |
| Silnik **M2** | `19` | przód-prawy |
| Silnik **M3** | `20` | tył-prawy |
| Silnik **M4** | `21` | tył-lewy |
| Dioda stanu | RGB na płytce (GPIO8) | czerwona = rozbrojony, zielona = uzbrojony |
| Przycisk **BOOT** | `9` | po starcie działa jako **STOP awaryjny** |
| Zewnętrzna dioda (opcja, L02) | `22` | przez rezystor 220 Ω do GND |

> [!WARNING] Pinów, których nie używamy do silników
> `4`, `5`, `8`, `9`, `15` – piny startowe (decydują, jak płytka się uruchamia),
> `12`, `13` – USB, `16`, `17` – UART0.
> Podłączenie do nich silnika może sprawić, że płytka nie wystartuje albo zniknie port USB.

Wszystkie piny są w bloku **KONFIGURACJA** na samej górze każdego programu. Jeśli Wasz dron jest polutowany inaczej, zmieńcie je tylko tam.

---

## 🔄 Układ silników i kierunki obrotów

Widok drona **z góry**:

```
            PRZÓD
   M1 (CW)         M2 (CCW)
        \         /
         [  ESP  ]
        /         \
   M4 (CCW)        M3 (CW)
```

- **CW** – zgodnie z ruchem wskazówek zegara, **CCW** – przeciwnie.
- Silniki **po przekątnej** kręcą się w tę samą stronę (M1 i M3 – CW, M2 i M4 – CCW).
  Dzięki temu dron nie kręci się wokół własnej osi.
- Silnik kręci się w złą stronę? Zamieńcie miejscami jego dwa kabelki.
- Śmigło też ma kierunek: wypukła (zaokrąglona) krawędź łopatki idzie przodem.

Kolejność i kierunki sprawdzicie w lekcji [L04](../zajecia3/l04_cztery_silniki.md).

---

## ⚙️ Dwa rodzaje sterowników silników

Na górze każdego programu z silnikami jest **jeden** przełącznik:

```cpp
#define TRYB_MOSFET          // sterowniki MOSFET (silniki szczotkowe)
// #define TRYB_ESC_RC       // albo regulatory ESC (impulsy 1000-2000 us)
```

| Tryb | Sygnał | Kiedy |
|:--|:--|:--|
| `TRYB_MOSFET` *(domyślny)* | PWM 20 kHz, 10 bitów, moc 0–100% = wypełnienie | silniki szczotkowe 816 przez tranzystory |
| `TRYB_ESC_RC` | impuls 1000–2000 µs, 50 Hz (`ESC_HZ`) | gotowe regulatory ESC |

Zostawcie odkomentowany **tylko jeden** – inaczej program się nie skompiluje i powie dlaczego.

---

## 🛑 Zasady bezpieczeństwa

Każdy program z silnikami pilnuje tych zasad. **Nie wyłączajcie ich.**

1. Po włączeniu wszystkie silniki dostają **0**. Dopiero potem reszta programu.
2. Silniki startują **ROZBROJONE** – dioda **czerwona**. Nic się nie kręci.
3. Komenda **`u`** uzbraja silniki – dioda **zielona**.
4. **`x`** albo **spacja** = natychmiastowy **STOP**. Przycisk **BOOT** na płytce = STOP.
5. Moc nigdy nie przekracza **`MAX_MOC`** (60%).
6. Moc zmienia się zawsze **płynnie** (rampa), bez skoków.
7. W L09 i L10 dron sam się zatrzymuje, gdy:
    - laser nie dał pomiaru przez **200 ms**,
    - dron jest wyżej niż **`WYSOKOSC_MAX`**,
    - lot trwa dłużej niż **`CZAS_MAX`**.

    Powód zatrzymania pojawia się w Serial Monitorze.

> [!CAUTION] Przy dronie
> - Zanim wpiszesz `u`, upewnij się, że nikt nie trzyma rąk przy śmigłach.
> - Strażnik zespołu trzyma palec przy **spacji**.
> - Włosy związane, nic nie wisi przy śmigłach.

---

## ⌨️ Komendy w Serial Monitorze

W Serial Monitorze ustaw **115200 baud** i **Nowa linia**. Wpisz komendę i naciśnij Enter.

| Komenda | Co robi | Lekcje |
|:--|:--|:--|
| `u` | uzbrój silniki | L03, L04, L09, L10 |
| `x` lub spacja | STOP (działa od razu, nawet bez Entera) | L03, L04, L09, L10 |
| `25` | moc silnika M1 = 25% | L03 |
| `t`, `r`, `s` | test kolejności / rozpędzanie / schody | L04 |
| `h 300` | wysokość zadana 300 mm (`h 0` = lądowanie) | L09 |
| `p 0.05`, `i 0.002`, `d 0.5` | nastawy regulatora Kp, Ki, Kd | L09, L10 |
| `z 35` | moc zawisu 35% | L09, L10 |
| `s` | start misji | L10 |
| `?` | pokaż nastawy | L09, L10 |

We wszystkich lekcjach komendy czyta ta sama funkcja `czytajKomende()`.
