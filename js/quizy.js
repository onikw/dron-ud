/*
 * Pytania „Sprawdź się”. Strona wstawia je w miejsca <div data-quiz="klucz"></div>.
 * poprawna = numer poprawnej odpowiedzi, liczony od 0.
 * Strona prowadzącego (prowadzacy.html) wypisuje z tego pliku wszystkie odpowiedzi.
 */
window.QUIZY = {
  // ---------- Zajęcia 2: elektronika ----------
  woda: {
    gdzie: "Zajęcia 2 · Elektronika · Napięcie, prąd, opór",
    pytanie: "Napięcie zostaje takie samo, a opór rośnie dwa razy. Co dzieje się z prądem?",
    odpowiedzi: ["Rośnie dwa razy", "Maleje dwa razy", "Nie zmienia się"],
    poprawna: 1,
    wyjasnienie: "I = U / R. Większy opór to węższa rura, więc przepływa mniej wody – prąd maleje tyle razy, ile razy wzrósł opór."
  },
  obwod: {
    gdzie: "Zajęcia 2 · Elektronika · Obwód",
    pytanie: "Dlaczego dioda gaśnie, gdy otworzysz przełącznik?",
    odpowiedzi: [
      "Bo bateria przestaje mieć napięcie",
      "Bo obwód jest przerwany i prąd nie ma którędy wrócić do minusa",
      "Bo dioda się przepala"
    ],
    poprawna: 1,
    wyjasnienie: "Prąd płynie tylko w zamkniętej pętli: od plusa, przez elementy, z powrotem do minusa (masy). Bateria cały czas ma napięcie."
  },
  napiecia: {
    gdzie: "Zajęcia 2 · Elektronika · Napięcia na stanowisku",
    pytanie: "Jakim napięciem zasilamy czujniki MPU6050 i VL53L0X?",
    odpowiedzi: ["12 V", "5 V", "3,3 V"],
    poprawna: 2,
    wyjasnienie: "Czujniki i ESP32 pracują na 3,3 V. 12 V z zasilacza idzie tylko do przetwornicy, która zasila silniki. 12 V na czujniku by go zniszczyło."
  },
  multimetr: {
    gdzie: "Zajęcia 2 · Elektronika · Multimetr",
    pytanie: "Chcesz zmierzyć napięcie baterii. Jak podłączasz multimetr?",
    odpowiedzi: [
      "Równolegle: czerwona sonda na plus, czarna na minus, pokrętło na V⎓",
      "Szeregowo: rozcinasz obwód i wstawiasz multimetr, gniazdo 10 A",
      "Dowolnie, byle pokrętło było na Ω"
    ],
    poprawna: 0,
    wyjasnienie: "Napięcie mierzymy równolegle, między dwoma punktami. Szeregowo (w przerwie obwodu) mierzy się prąd."
  },
  silnik: {
    gdzie: "Zajęcia 2 · Elektronika · Silnik szczotkowy",
    pytanie: "Co się stanie, gdy zamienisz miejscami dwa przewody silnika szczotkowego?",
    odpowiedzi: ["Silnik się zatrzyma", "Silnik będzie kręcił się w drugą stronę", "Silnik zacznie kręcić się szybciej"],
    poprawna: 1,
    wyjasnienie: "Zamiana biegunów odwraca kierunek prądu w uzwojeniach, więc wirnik kręci się w przeciwną stronę. Dlatego biegunowość jest tak ważna."
  },
  pwm: {
    gdzie: "Zajęcia 2 · Elektronika · PWM",
    pytanie: "Wypełnienie PWM wynosi 25%. Co to znaczy?",
    odpowiedzi: [
      "Pin jest włączony przez 1/4 czasu, a wyłączony przez 3/4",
      "Pin daje 25 V",
      "Silnik kręci się przez 25 sekund"
    ],
    poprawna: 0,
    wyjasnienie: "Wypełnienie to część okresu, w której pin jest włączony. Średnio silnik dostaje wtedy około 1/4 mocy."
  },
  plytka: {
    gdzie: "Zajęcia 2 · Elektronika · Płytka stykowa",
    pytanie: "Które otwory płytki stykowej są ze sobą połączone?",
    odpowiedzi: [
      "Wszystkie otwory w tym samym rzędzie poziomym przez całą płytkę",
      "5 otworów w jednej kolumnie po jednej stronie przerwy (np. a–e) oraz otwory wzdłuż jednej szyny zasilania",
      "Żadne – każdy otwór jest osobny"
    ],
    poprawna: 1,
    wyjasnienie: "Każda kolumna ma dwie grupy po 5 otworów (a–e i f–j), rozdzielone przerwą. Szyny zasilania biegną wzdłuż krawędzi – na niektórych płytkach są przerwane w połowie."
  },

  // ---------- Zajęcia 2: dalsze rozdziały ----------
  bezpieczenstwo: {
    gdzie: "Zajęcia 2 · Bezpieczeństwo",
    pytanie: "Śmigło zaczęło się kręcić, choć nikt tego nie planował. Co robisz najpierw?",
    odpowiedzi: [
      "Łapię śmigło ręką",
      "Naciskam spację (STOP) albo przycisk BOOT, a potem odłączam zasilanie silników",
      "Czekam, aż samo się zatrzyma"
    ],
    poprawna: 1,
    wyjasnienie: "Najpierw STOP z klawiatury albo przycisk BOOT, potem odłączenie zasilania. Nigdy nie zatrzymujemy śmigła ręką."
  },
  montaz: {
    gdzie: "Zajęcia 2 · Montaż drona",
    pytanie: "Silnik siedzi w tulejce krzywo. Co robisz?",
    odpowiedzi: ["Zostawiam – i tak poleci", "Wyjmuję i wciskam jeszcze raz, prosto, do oporu", "Przyklejam taśmą"],
    poprawna: 1,
    wyjasnienie: "Krzywy silnik pcha powietrze w bok i trzęsie dronem. Silnik ma stać pionowo i siedzieć w tulejce do oporu."
  },
  orientacja: {
    gdzie: "Zajęcia 2 · Orientacja silników",
    pytanie: "Dlaczego silniki po przekątnej kręcą się w tę samą stronę?",
    odpowiedzi: [
      "Żeby było ładniej",
      "Żeby skręcanie (moment obrotowy) dwóch par silników się zrównoważyło i dron nie obracał się wokół własnej osi",
      "Bo tak jest łatwiej lutować"
    ],
    poprawna: 1,
    wyjasnienie: "Każde śmigło próbuje obrócić ramę w przeciwną stronę. Dwa CW i dwa CCW, ustawione po przekątnych, znoszą się nawzajem."
  },
  czujniki: {
    gdzie: "Zajęcia 2 · Czujniki na dronie",
    pytanie: "Nóżka drona wchodzi w stożek widzenia czujnika odległości. Co pokaże czujnik?",
    odpowiedzi: [
      "Dokładną wysokość nad podłogą",
      "Może pokazać odległość do nóżki zamiast do podłogi – pomiar będzie zły",
      "Nic się nie zmieni, bo laser jest bardzo cienki"
    ],
    poprawna: 1,
    wyjasnienie: "Czujnik widzi cały stożek (około 25°), a nie cienką kreskę. Wszystko, co jest w stożku, może zepsuć pomiar."
  },
  podlaczenie: {
    gdzie: "Zajęcia 2 · Podłączenie do platformy",
    pytanie: "Ile przewodów sygnałowych potrzeba, żeby ESP32 rozmawiało z OBOMA czujnikami?",
    odpowiedzi: ["Dwa (SDA i SCL) – wspólne dla obu czujników", "Cztery – po dwa na czujnik", "Osiem"],
    poprawna: 0,
    wyjasnienie: "To magistrala I2C: oba czujniki siedzą na tych samych liniach SDA i SCL. Do tego wspólne zasilanie 3,3 V i GND – razem 4 przewody."
  },
  arduino: {
    gdzie: "Zajęcia 2 · Arduino od zera",
    pytanie: "Który fragment programu wykonuje się w kółko, bez końca?",
    odpowiedzi: ["setup()", "loop()", "Serial.begin()"],
    poprawna: 1,
    wyjasnienie: "setup() wykonuje się raz po włączeniu, a loop() powtarza się w nieskończoność."
  },
  i2c: {
    gdzie: "Zajęcia 2 · I2C i skaner",
    pytanie: "Skaner pokazał tylko 0x68. Co to znaczy?",
    odpowiedzi: [
      "Wszystko w porządku",
      "MPU6050 odpowiada, ale czujnik odległości (0x29) nie – trzeba sprawdzić jego podłączenie",
      "Czujnik odległości ma teraz adres 0x68"
    ],
    poprawna: 1,
    wyjasnienie: "Każdy adres to jedno urządzenie. Brak 0x29 oznacza, że czujnik odległości nie odpowiedział na „pukanie”."
  },
  laser: {
    gdzie: "Zajęcia 2 · Pomiar wysokości",
    pytanie: "Czujnik odległości mierzy…",
    odpowiedzi: [
      "czas, po którym wraca odbite światło",
      "jasność podłogi",
      "temperaturę powietrza"
    ],
    poprawna: 0,
    wyjasnienie: "To pomiar czasu lotu światła (ToF). Odległość = prędkość światła × czas / 2."
  },
  akcelerometr: {
    gdzie: "Zajęcia 2 · Akcelerometr",
    pytanie: "Dron stoi płasko na stole. Ile pokazuje oś Z akcelerometru?",
    odpowiedzi: ["0 g", "około 1 g", "około 10 g"],
    poprawna: 1,
    wyjasnienie: "Akcelerometr czuje grawitację. Gdy leży płasko, cała grawitacja przypada na oś Z – około 1 g."
  },

  // ---------- Zajęcia 3 ----------
  esc: {
    gdzie: "Zajęcia 3 · Podłączenie ESC",
    pytanie: "Dlaczego ESP32 i moduły ESC muszą mieć wspólną masę (GND)?",
    odpowiedzi: [
      "Bo inaczej sygnał PWM nie ma punktu odniesienia i ESC go nie zrozumie",
      "Żeby silniki kręciły się szybciej",
      "Nie muszą"
    ],
    poprawna: 0,
    wyjasnienie: "Napięcie to zawsze różnica między dwoma punktami. Sygnał z ESP32 ma sens tylko względem wspólnej masy."
  },
  rampa: {
    gdzie: "Zajęcia 3 · Jeden silnik",
    pytanie: "Wpisujesz moc 40, a silnik rozpędza się przez chwilę, zamiast od razu. Dlaczego?",
    odpowiedzi: ["Bo silnik jest zepsuty", "Bo program zmienia moc przez rampę – powoli, bez skoków", "Bo Serial jest za wolny"],
    poprawna: 1,
    wyjasnienie: "Rampa to bezpiecznik: moc rośnie najwyżej o RAMPA procent na sekundę. Mniejsze szarpnięcia i mniejsze skoki prądu."
  },
  uzbrojenie: {
    gdzie: "Zajęcia 3 · Cztery silniki",
    pytanie: "Dioda na płytce świeci na czerwono. Co to znaczy?",
    odpowiedzi: ["Silniki są uzbrojone", "Silniki są rozbrojone – nic się nie zakręci", "Płytka jest zepsuta"],
    poprawna: 1,
    wyjasnienie: "Czerwona = rozbrojony, zielona = uzbrojony. Uzbrajamy komendą u, dopiero gdy wszyscy są z dala od śmigieł."
  },

  // ---------- Zajęcia 4 ----------
  filtr: {
    gdzie: "Zajęcia 4 · Filtrowanie",
    pytanie: "Zwiększasz N w średniej z N pomiarów. Co się dzieje?",
    odpowiedzi: [
      "Wykres jest gładszy, ale spóźnia się za prawdziwą wysokością",
      "Wykres jest bardziej poszarpany",
      "Nic się nie zmienia"
    ],
    poprawna: 0,
    wyjasnienie: "Mocniejszy filtr = gładszy wykres, ale większe opóźnienie. Dla drona trzeba wybrać coś pośrodku."
  },
  regulatorP: {
    gdzie: "Zajęcia 4 · Regulator P",
    pytanie: "Dron jest za nisko. Jaki jest znak błędu i co robi człon P?",
    odpowiedzi: [
      "Błąd dodatni, więc P dodaje mocy",
      "Błąd ujemny, więc P zmniejsza moc",
      "Błąd zero, więc P nic nie robi"
    ],
    poprawna: 0,
    wyjasnienie: "błąd = zadana − zmierzona. Za nisko: zadana większa od zmierzonej, błąd dodatni, więc Kp·błąd dodaje mocy."
  },
  pid: {
    gdzie: "Zajęcia 4 · PI i PID",
    pytanie: "Który człon regulatora gasi bujanie, hamując, gdy dron szybko zbliża się do celu?",
    odpowiedzi: ["P", "I", "D"],
    poprawna: 2,
    wyjasnienie: "D patrzy na zmianę błędu – na to, jak szybko dron się porusza – i hamuje, zanim przestrzeli."
  },

  // ---------- Zajęcia 5 ----------
  misja: {
    gdzie: "Zajęcia 5 · Misja",
    pytanie: "W jakim stanie jest dron zaraz po komendzie s?",
    odpowiedzi: ["CZEKAJ", "START", "LĄDOWANIE"],
    poprawna: 1,
    wyjasnienie: "Komenda s przełącza maszynę stanów z CZEKAJ do START. Po 2 sekundach zaczyna się MISJA."
  }
};
