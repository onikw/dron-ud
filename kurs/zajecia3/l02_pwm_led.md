# L02: PWM – jak „ściszyć” diodę

Pin płytki umie tylko dwie rzeczy: **włączony** albo **wyłączony**. A przecież chcemy, żeby silnik kręcił się raz wolniej, raz szybciej. Jak to zrobić? Trzeba go bardzo szybko włączać i wyłączać!

## 🎯 Czego się nauczysz

- Co to jest **PWM** i **wypełnienie**.
- Dlaczego szybkie mruganie oko widzi jako słabsze światło.
- Jak zrobić PWM sprzętem (moduł **LEDC** w ESP32).
- Jak oglądać dane na wykresie (**Serial Plotter**).

## 🔌 Co podłączyć

- Kabel USB. Dioda RGB jest na płytce.
- *(opcjonalnie)* zewnętrzna dioda: pin **22** → rezystor **220 Ω** → dłuższa nóżka diody, krótsza nóżka → **GND**.

## 🧠 Jak to działa

**PWM** (*Pulse Width Modulation*, modulacja szerokości impulsu) to bardzo szybkie włączanie i wyłączanie. **Wypełnienie** mówi, przez jaką część czasu pin jest włączony.

```text
 25%:  _       _       _
      | |_____| |_____| |_____
 50%:  ___     ___     ___
      |   |___|   |___|   |___
 75%:  _____   _____   _____
      |     |_|     |_|     |_
      |<----->|
        okres
```

Analogia: wentylator, który włączasz na chwilę co chwilę. Im dłużej jest włączony, tym mocniej „średnio” dmucha.

Program ma trzy części:

1. **PWM ręcznie i bardzo wolno** – okres 2 s. Widać, jak dioda się włącza i wyłącza, a na Plotterze rysuje się prostokąt 0/1.
2. **Coraz szybciej** – okres 1 s, 0,5 s, 0,2 s … aż 5 ms. W pewnym momencie oko przestaje widzieć mruganie i widzi tylko **słabsze światło**.
3. **PWM sprzętowy (LEDC)** – płytka mruga sama, 5000 razy na sekundę, a procesor może w tym czasie robić coś innego.

> [!NOTE] Gdzie jest Serial Plotter?
> **Narzędzia → Serial Plotter**. Rysuje wykres z liczb, które płytka wysyła w formacie `nazwa:wartość,nazwa2:wartość`. Serial Monitor i Plotter nie mogą być otwarte naraz.

## 💻 Kod

[Pobierz plik](https://github.com/onikw/dron-ud/blob/main/uczen/L02_pwm_led/L02_pwm_led.ino)

```cpp
--8<-- "uczen/L02_pwm_led/L02_pwm_led.ino"
```

## ✅ Zadania

1. Zmień `WYPELNIENIE` na `10`, `50`, `90`. Jak zmienia się prostokąt na wykresie? A jasność diody na końcu?
2. Zmień pierwszy (najwolniejszy) okres w tablicy `OKRESY_MS`, np. na `4000`.
3. Zrób **„oddychającą” diodę**: w `loop()` jasność płynnie rośnie od 0 do 100 i spada z powrotem.

## 👀 Co powinieneś zobaczyć

- Na początku prostokąt `stan` i pozioma linia `srednia` – to jasność, którą widzi oko.
- Przy szybkich etapach wykres robi się „poszarpany”: Plotter rysuje punkt co 20 ms i **nie nadąża** – tak samo jak Twoje oko.
- Na końcu linia `jasnosc` – przy „oddychaniu” wygląda jak piła.

> [!TIP] Ciekawostka
> Dioda RGB na płytce ma w środku własny mały sterownik PWM. Płytka wysyła jej tylko trzy liczby: ile czerwonego, zielonego i niebieskiego.

??? success "Pokaż rozwiązanie"

    ```cpp
    --8<-- "prowadzacy/L02_pwm_led/L02_pwm_led.ino"
    ```
