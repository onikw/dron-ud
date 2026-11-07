/*
 * KONFIGURACJA STRONY
 * -------------------
 * Jedyne miejsce, w którym prowadzący zmienia wartości opisujące sprzęt.
 * Z tych danych rysują się schematy, ćwiczenie z orientacją silników i tabele.
 *
 * Te same piny muszą być w bloku KONFIGURACJA na górze każdego szkicu
 * (szkice/.../*.ino). Skrypt narzedzia/wklej_szkice.py sprawdza, czy się zgadzają.
 *
 * Pola z listy "doPotwierdzenia" to wartości domyślne z planu kursu,
 * których nikt jeszcze nie sprawdził na sprzęcie. Po sprawdzeniu usuń je z listy.
 */
window.KONFIG = {
  plytka: "ESP32-C6-DevKitC-1",

  piny: {
    sda: 6,
    scl: 7,
    silniki: [18, 19, 20, 21]   // sygnały ESC dla M1, M2, M3, M4
  },

  // Układ silników: widok z góry, przód u góry.
  // pozycja: "przod-lewy" | "przod-prawy" | "tyl-prawy" | "tyl-lewy"
  silniki: [
    { nr: "M1", pozycja: "przod-lewy",  kierunek: "CW" },
    { nr: "M2", pozycja: "przod-prawy", kierunek: "CCW" },
    { nr: "M3", pozycja: "tyl-prawy",   kierunek: "CW" },
    { nr: "M4", pozycja: "tyl-lewy",    kierunek: "CCW" }
  ],

  // Typy silników. Kierunek liczymy, patrząc z góry, od strony śmigła.
  typySilnikow: {
    A: { kierunek: "CW",  plus: "czerwony", minus: "niebieski" },
    B: { kierunek: "CCW", plus: "biały",    minus: "czarny" }
  },

  czujnikOdleglosci: {
    model: "VL53L0X",        // sprawdzone szkicem narzedzia/rozpoznaj_laser
    biblioteka: "VL53L0X (Pololu)",
    adres: 0x29,
    zasiegMm: 1200,          // tryb domyślny
    stozekStopnie: 25        // pole widzenia czujnika (karta katalogowa VL53L0X)
  },

  mpu: { model: "MPU6050", adres: 0x68 },

  // Kolory przewodów I2C z drona.
  przewodyI2C: { "3V3": "czerwony", GND: "czarny", SDA: "żółty", SCL: "niebieski" },

  // Opis mocowania czujników na dronie (prowadzący uzupełni).
  // Pusty tekst = na stronie pojawi się ramka "do uzupełnienia".
  mocowanieCzujnikow: "",

  // Wyprowadzenia płytki ESP32-C6-DevKitC-1 (od góry, złącze J1 po lewej, J3 po prawej,
  // port USB na dole). Używane tylko do rysunków. Sprawdź z nadrukiem na płytce.
  pinout: {
    J1: ["3V3", "RST", "4", "5", "6", "7", "0", "1", "8", "10", "11", "2", "3", "5V", "GND", "NC"],
    J3: ["GND", "TX", "RX", "15", "23", "22", "21", "20", "19", "18", "9", "GND", "13", "12", "GND", "NC"]
  },

  // Kolory do rysowania przewodów.
  kolory: {
    "czerwony": "#D7352A",
    "czarny": "#222222",
    "żółty": "#E8B90C",
    "niebieski": "#1F5FC8",
    "biały": "#F2F2F2",
    "zielony": "#2E9B4F",
    "pomarańczowy": "#F2A33A"
  },

  doPotwierdzenia: [
    "plytka", "piny", "silniki", "typySilnikow (biegunowość)",
    "mocowanieCzujnikow", "przewodyI2C", "pinout"
  ]
};
