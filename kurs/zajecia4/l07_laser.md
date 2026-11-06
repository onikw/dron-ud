# L07: Laserowy czujnik odległości

Żeby dron mógł trzymać wysokość, musi wiedzieć, **jak wysoko jest**. Zmierzy to światłem!

## 🎯 Czego się nauczysz

- Jak laser mierzy odległość – **czas lotu światła** (*Time of Flight*).
- Jak używać gotowej biblioteki czujnika.
- Kiedy pomiar jest dobry, a kiedy nie (**status**).

## 🔌 Co podłączyć

- Laser VL53L1X: **3,3 V**, **GND**, **SDA → 6**, **SCL → 7**.
- Biblioteka **„VL53L1X” od Pololu** ([instalacja](../start/instalacja.md#3-biblioteka-czujnika-laserowego)).

## 🧠 Jak to działa

Czujnik wysyła błysk niewidzialnego światła i mierzy, po jakim czasie wróci. Jak **echo** w górach – tylko że światło jest prawie milion razy szybsze od dźwięku!

```text
 [laser] ----błysk---->  |
         <---odbicie---  |  podłoga / dłoń
         |<-- odległość ->|
```

`odległość = prędkość światła × czas / 2` (dzielimy przez 2, bo światło leci tam i z powrotem).

Światło pokonuje 1 mm w około **3 bilionowe części sekundy**. Czujnik musi mierzyć naprawdę maleńkie czasy!

| Ustawienie | Wartość | Co znaczy |
|:--|:--|:--|
| Tryb | **Short** | do ok. 1,3 m, mniej czuły na światło słońca |
| Budżet czasu | **33 ms** | tyle trwa jeden pomiar – ok. 30 pomiarów na sekundę |
| Status | **0** | dobry pomiar; inne liczby = coś nie tak |

## 💻 Kod

[Pobierz plik](https://github.com/onikw/dron-ud/blob/main/uczen/L07_laser/L07_laser.ino)

```cpp
--8<-- "uczen/L07_laser/L07_laser.ino"
```

## ✅ Zadania

1. Trzymaj dłoń nad laserem. Zmierz odległość i wpisz ją w `DLON_MM`. Sprawdź linijką!
2. Zbliżaj dłoń powoli. Przy jakiej najmniejszej odległości `status` jest jeszcze `0`? Wpisz ją w `NAJBLIZEJ_MM`.
3. Usuń `//` przed liniami z `sygnal`. Porównaj **jasną** i **ciemną** kartkę na tej samej wysokości. Która odbija więcej światła? Czy odległość jest taka sama?

## 👀 Co powinieneś zobaczyć

Linia `odleglosc_mm` idzie w górę, gdy odsuwasz dłoń, i w dół, gdy ją zbliżasz. Lekko „drży” – to **szum**. Zajmiemy się nim w następnej lekcji.

??? success "Pokaż rozwiązanie"

    ```cpp
    --8<-- "prowadzacy/L07_laser/L07_laser.ino"
    ```
