# 🚁 Inżynieria dronowa – mapa kursu

Cześć! W tym kursie zaprogramujecie **prawdziwego drona**. Zaczniemy od mrugania diodą, a skończymy na dronie, który **sam trzyma wysokość** i wykonuje misję, którą mu zaplanujecie.

Dron jest przypięty do pionowej prowadnicy, więc lata tylko w górę i w dół. Dzięki temu możecie śmiało eksperymentować.

> [!TIP] Jak pracujemy
> Pracujecie w zespołach po 3 osoby. Najlepiej zmieniajcie się co lekcję:
>
> - **pilot** – siedzi przy klawiaturze i wpisuje kod,
> - **obserwator** – patrzy na drona i na wykres, mówi, co widzi,
> - **strażnik** – trzyma palec przy klawiszu spacji (STOP) i pilnuje bezpieczeństwa.

## 🗺️ Kolejność

```
🧰 Instalacja → 🚁 Nasz dron → 💡 Zajęcia 3 → 📏 Zajęcia 4 → 🏆 Zajęcia 5
```

Do poruszania się używaj menu u góry i po lewej. Na dole każdej strony są strzałki *wstecz* / *dalej*.

Każda lekcja ma **kod do uzupełnienia** (miejsca oznaczone `// ZADANIE`) oraz **rozwijane rozwiązanie** na samym dole. Najpierw spróbujcie sami!

---

## 🧰 Start – *jesteś tutaj*

- [Krok 1: Instalacja Arduino IDE i bibliotek](start/instalacja.md)
- [Krok 2: Nasz dron – połączenia i zasady bezpieczeństwa](start/dron.md)
- [Najczęstsze problemy](start/problemy.md)

---

## 💡 Zajęcia 3: Arduino i silniki

Pierwszy program, PWM, pierwszy silnik, cztery silniki i rozmowa z czujnikami.

| Lekcja | Czego się nauczysz |
|:--|:--|
| [L01 Miganie diodą](zajecia3/l01_miganie.md) | `setup()`, `loop()`, Serial Monitor |
| [L02 PWM i dioda](zajecia3/l02_pwm_led.md) | jak „ściszyć” diodę, Serial Plotter |
| [L03 Jeden silnik](zajecia3/l03_jeden_silnik.md) | moc silnika, komendy, rampa |
| [L04 Cztery silniki](zajecia3/l04_cztery_silniki.md) | tablice, pętla `for`, kierunki obrotów |
| [L05 Skaner I2C](zajecia3/l05_skaner_i2c.md) | magistrala I2C i adresy |

---

## 📏 Zajęcia 4: Utrzymanie wysokości

Czujniki, filtry i regulator, który sam pilnuje wysokości.

| Lekcja | Czego się nauczysz |
|:--|:--|
| [L06 MPU6050](zajecia4/l06_mpu6050.md) *(dodatkowa)* | rejestry, akcelerometr, żyroskop |
| [L07 Laser](zajecia4/l07_laser.md) | pomiar odległości światłem |
| [L08 Filtr](zajecia4/l08_filtr.md) | jak wygładzić skaczący pomiar |
| [L09 Trzymaj wysokość](zajecia4/l09_trzymaj_wysokosc.md) | regulator PID |

---

## 🏆 Zajęcia 5: Zawody

| Lekcja | Czego się nauczysz |
|:--|:--|
| [L10 Misja](zajecia5/l10_misja.md) | maszyna stanów, planowanie lotu |

---

> [!NOTE] Dla prowadzących
> Wszystkie szkice leżą w repozytorium [onikw/dron-ud](https://github.com/onikw/dron-ud):
> `uczen/` – wersje z lukami, `prowadzacy/` – pełne rozwiązania. Każdy szkic jest samodzielny.
