# 📏 Zajęcia 4: Utrzymanie wysokości

Dziś dron nauczy się **sam** trzymać wysokość. Potrzebujemy do tego trzech rzeczy: czujnika, który mierzy wysokość, filtra, który uspokaja pomiar, i **regulatora**, który na tej podstawie ustawia moc silników.

```text
  laser --> filtr --> regulator --> silniki --> dron leci
    ^                                              |
    +----------------------------------------------+
```

## ⚡ Czego się nauczysz

1. **Rejestry czujnika** – jak rozmawiać z czujnikiem bez gotowej biblioteki *(L06, dla szybszych)*.
2. **Pomiar odległości laserem** – czas lotu światła.
3. **Filtrowanie** – średnia i filtr wykładniczy.
4. **Regulator PID** – serce każdego drona, robota i termostatu.

## 🗺️ Lekcje

1. [L06: MPU6050](l06_mpu6050.md) *(dodatkowa – dla zespołów, które skończą wcześniej)*
2. [L07: Laser](l07_laser.md)
3. [L08: Filtr](l08_filtr.md)
4. [L09: Trzymaj wysokość](l09_trzymaj_wysokosc.md)

> [!TIP] Kolejność
> L06 można pominąć – L07, L08 i L09 z niej nie korzystają.
