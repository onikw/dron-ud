/*
 * Interaktywne rysunki (SVG + czysty JS). Strona wstawia je w miejsca:
 *   <div data-rysunek="woda"></div>
 * Dane o sprzęcie (piny, kolory, układ silników) biorą się z js/config.js.
 */
(function () {
  "use strict";
  var K = window.KONFIG || {};
  var KOL = K.kolory || {};
  var ruch = !(window.matchMedia && window.matchMedia("(prefers-reduced-motion: reduce)").matches);

  // ---------- pomocnicze ----------
  function fmt(n, d) { return n.toFixed(d).replace(".", ","); }
  function kolor(nazwa) { return KOL[nazwa] || nazwa || "#888"; }
  function hex(n) { return "0x" + ("0" + n.toString(16).toUpperCase()).slice(-2); }
  function $(box, sel) { return box.querySelector(sel); }
  function html(box, tresc) { box.innerHTML = tresc; }
  function suwak(id, napis, min, max, krok, wart) {
    return '<label class="suwak"><span>' + napis + ': <b data-wart="' + id + '"></b></span>' +
      '<input type="range" data-suwak="' + id + '" min="' + min + '" max="' + max + '" step="' + krok + '" value="' + wart + '"></label>';
  }
  // Pętla animacji wspólna dla wszystkich rysunków; zatrzymuje się, gdy rysunek nie jest widoczny.
  function animuj(box, krok) {
    var widoczny = true, ost = null;
    if ("IntersectionObserver" in window) {
      new IntersectionObserver(function (w) { widoczny = w[0].isIntersecting; }).observe(box);
    }
    function klatka(t) {
      if (ost === null) ost = t;
      var dt = Math.min(0.05, (t - ost) / 1000);
      ost = t;
      if (widoczny && !document.hidden) krok(dt);
      requestAnimationFrame(klatka);
    }
    requestAnimationFrame(klatka);
  }
  function strzalkaObrotu(cx, cy, r, cw, kol) {
    // łuk 270° ze strzałką na końcu; cw = zgodnie z ruchem wskazówek zegara (na ekranie)
    var a0 = -135, a1 = 135, s = cw ? 1 : 0;
    function p(a) { var rad = a * Math.PI / 180; return [cx + r * Math.cos(rad), cy + r * Math.sin(rad)]; }
    var start = p(cw ? a0 : a1), stop = p(cw ? a1 : a0);
    var kat = (cw ? a1 : a0) * Math.PI / 180;
    var styczna = cw ? kat + Math.PI / 2 : kat - Math.PI / 2;
    var g = [stop[0] + 9 * Math.cos(styczna), stop[1] + 9 * Math.sin(styczna)];
    var b1 = [stop[0] + 7 * Math.cos(styczna + 2.4), stop[1] + 7 * Math.sin(styczna + 2.4)];
    var b2 = [stop[0] + 7 * Math.cos(styczna - 2.4), stop[1] + 7 * Math.sin(styczna - 2.4)];
    return '<path d="M' + start[0].toFixed(1) + " " + start[1].toFixed(1) + " A " + r + " " + r + " 0 1 " + s + " " +
      stop[0].toFixed(1) + " " + stop[1].toFixed(1) + '" fill="none" stroke="' + kol + '" stroke-width="3.5"/>' +
      '<polygon points="' + [g, b1, b2].map(function (q) { return q[0].toFixed(1) + "," + q[1].toFixed(1); }).join(" ") +
      '" fill="' + kol + '"/>';
  }

  var R = {};

  // =====================================================================
  // 1. Napięcie, prąd, opór – analogia z wodą
  // =====================================================================
  R.woda = function (box) {
    html(box,
      '<svg viewBox="0 0 640 300" role="img" aria-label="Zbiornik z wodą połączony rurą ze zwężeniem">' +
      '<text x="90" y="24" text-anchor="middle" font-size="14" font-weight="700" fill="#13171F">zbiornik</text>' +
      '<rect x="30" y="34" width="120" height="216" rx="6" fill="#F5F4EF" stroke="#13171F" stroke-width="3"/>' +
      '<rect class="w-woda" x="33" y="150" width="114" height="97" fill="#7FB0F0"/>' +
      '<line class="w-poziom" x1="20" x2="160" y1="150" y2="150" stroke="#1F5FC8" stroke-width="2" stroke-dasharray="5 4"/>' +
      '<text class="w-u" x="168" y="140" font-size="15" font-weight="700" fill="#1F5FC8">U</text>' +
      '<path class="w-rura" fill="#7FB0F0" stroke="#13171F" stroke-width="3"/>' +
      '<g class="w-kropki"></g>' +
      '<rect x="560" y="250" width="70" height="40" rx="5" fill="#F5F4EF" stroke="#13171F" stroke-width="3"/>' +
      '<text x="380" y="210" text-anchor="middle" font-size="14" font-weight="700" fill="#13171F">zwężenie = opór R</text>' +
      '<text x="250" y="275" text-anchor="middle" font-size="14" font-weight="700" fill="#13171F">przepływ = prąd I</text>' +
      '<text x="90" y="285" text-anchor="middle" font-size="13" fill="#4A5060">wysokość wody = napięcie U</text>' +
      '</svg>' +
      '<div class="sterowanie">' + suwak("u", "Napięcie U", 0, 12, 0.5, 6) + suwak("r", "Opór R", 10, 100, 5, 30) + '</div>' +
      '<div class="wynik" aria-live="polite"></div>');
    var su = $(box, '[data-suwak="u"]'), sr = $(box, '[data-suwak="r"]');
    var kropkiG = $(box, ".w-kropki"), kropki = [];
    for (var i = 0; i < 16; i++) {
      var c = document.createElementNS("http://www.w3.org/2000/svg", "circle");
      c.setAttribute("r", "3.6"); c.setAttribute("fill", "#FFFFFF"); c.setAttribute("stroke", "#1F5FC8");
      kropkiG.appendChild(c); kropki.push(c);
    }
    var I = 0, faza = 0, hc = 20;
    function rysuj() {
      var U = +su.value, Rr = +sr.value;
      I = U / Rr;
      $(box, '[data-wart="u"]').textContent = fmt(U, 1) + " V";
      $(box, '[data-wart="r"]').textContent = Rr + " Ω";
      var y = 247 - (U / 12) * 200;
      var w = $(box, ".w-woda");
      w.setAttribute("y", y.toFixed(1)); w.setAttribute("height", (247 - y).toFixed(1));
      $(box, ".w-poziom").setAttribute("y1", y); $(box, ".w-poziom").setAttribute("y2", y);
      $(box, ".w-u").setAttribute("y", (y - 6).toFixed(1));
      hc = Math.max(3, 20 * Math.sqrt(10 / Rr));
      var g = 235 - hc / 2, d = 235 + hc / 2;
      $(box, ".w-rura").setAttribute("d",
        "M150 225 L320 225 L340 " + g + " L420 " + g + " L440 225 L590 225 L590 252 L575 252 L575 245 L440 245 L420 " + d +
        " L340 " + d + " L320 245 L150 245 Z");
      $(box, ".wynik").innerHTML = "I = U / R = " + fmt(U, 1) + " V / " + Rr + " Ω = <b>" + fmt(I, 3) + " A</b> (" +
        Math.round(I * 1000) + " mA)";
    }
    su.oninput = sr.oninput = rysuj;
    rysuj();
    function ustawKropki() {
      for (var i = 0; i < kropki.length; i++) {
        var x = 155 + ((i * 28 + faza) % 430);
        kropki[i].setAttribute("cx", x.toFixed(1));
        kropki[i].setAttribute("cy", "235");
        kropki[i].setAttribute("r", (x > 340 && x < 420) ? Math.min(3.6, hc / 2 + 0.5).toFixed(1) : "3.6");
      }
    }
    ustawKropki();
    if (ruch) animuj(box, function (dt) { faza += I * 260 * dt; ustawKropki(); });
  };

  // =====================================================================
  // 2. Obwód zamknięty i otwarty
  // =====================================================================
  R.obwod = function (box) {
    html(box,
      '<svg viewBox="0 0 520 300" role="img" aria-label="Obwód: bateria, opornik, dioda i przełącznik">' +
      '<path class="o-droga" d="M80 120 L80 70 L440 70 L440 230 L80 230 L80 180" fill="none" stroke="#13171F" stroke-width="4"/>' +
      // bateria
      '<rect x="60" y="118" width="40" height="64" fill="#FFFFFF"/>' +
      '<line x1="58" y1="136" x2="102" y2="136" stroke="#13171F" stroke-width="4"/>' +
      '<line x1="68" y1="150" x2="92" y2="150" stroke="#13171F" stroke-width="8"/>' +
      '<line x1="58" y1="164" x2="102" y2="164" stroke="#13171F" stroke-width="4"/>' +
      '<line x1="68" y1="178" x2="92" y2="178" stroke="#13171F" stroke-width="8"/>' +
      '<line x1="80" y1="118" x2="80" y2="136" stroke="#13171F" stroke-width="4"/>' +
      '<line x1="80" y1="178" x2="80" y2="182" stroke="#13171F" stroke-width="4"/>' +
      '<text x="38" y="128" font-size="22" font-weight="800" fill="#D7352A">+</text>' +
      '<text x="40" y="196" font-size="24" font-weight="800" fill="#13171F">−</text>' +
      '<text x="112" y="158" font-size="13" fill="#4A5060">bateria 3 V</text>' +
      // masa
      '<line x1="80" y1="230" x2="80" y2="256" stroke="#13171F" stroke-width="3"/>' +
      '<line x1="62" y1="256" x2="98" y2="256" stroke="#13171F" stroke-width="3"/>' +
      '<line x1="68" y1="263" x2="92" y2="263" stroke="#13171F" stroke-width="3"/>' +
      '<line x1="74" y1="270" x2="86" y2="270" stroke="#13171F" stroke-width="3"/>' +
      '<text x="104" y="268" font-size="13" font-weight="700" fill="#13171F">GND (masa)</text>' +
      // opornik
      '<rect x="170" y="58" width="80" height="24" rx="4" fill="#FFFFFF" stroke="#13171F" stroke-width="3"/>' +
      '<text x="210" y="48" text-anchor="middle" font-size="13" fill="#4A5060">opornik</text>' +
      // dioda
      '<circle class="o-blask" cx="345" cy="70" r="34" fill="#F2A33A" opacity="0"/>' +
      '<rect x="322" y="52" width="46" height="36" fill="#FFFFFF"/>' +
      '<polygon class="o-dioda" points="330,52 330,88 362,70" fill="#E9E7DF" stroke="#13171F" stroke-width="3"/>' +
      '<line x1="364" y1="50" x2="364" y2="90" stroke="#13171F" stroke-width="4"/>' +
      '<text x="345" y="112" text-anchor="middle" font-size="13" fill="#4A5060">dioda LED</text>' +
      // przełącznik
      '<rect x="226" y="214" width="92" height="32" fill="#FFFFFF"/>' +
      '<circle cx="236" cy="230" r="6" fill="#13171F"/><circle cx="308" cy="230" r="6" fill="#13171F"/>' +
      '<line class="o-dzwignia" x1="236" y1="230" x2="300" y2="196" stroke="#13171F" stroke-width="5" stroke-linecap="round"/>' +
      '<rect class="o-klik klikalne" x="214" y="180" width="116" height="66" fill="transparent" tabindex="0" role="button" aria-label="Przełącznik – kliknij, żeby go przełączyć"/>' +
      '<text x="272" y="270" text-anchor="middle" font-size="13" fill="#4A5060">przełącznik – kliknij</text>' +
      '<g class="o-kropki"></g>' +
      '</svg>' +
      '<div class="sterowanie"><button type="button" class="przycisk o-przycisk">Zamknij obwód</button></div>' +
      '<div class="wynik" aria-live="polite"></div>');
    var droga = $(box, ".o-droga"), dl = droga.getTotalLength();
    var kG = $(box, ".o-kropki"), kropki = [], N = 20;
    for (var i = 0; i < N; i++) {
      var c = document.createElementNS("http://www.w3.org/2000/svg", "circle");
      c.setAttribute("r", "4.5"); c.setAttribute("fill", "#F2A33A"); c.setAttribute("stroke", "#13171F");
      kG.appendChild(c); kropki.push(c);
    }
    var zamkniety = false, faza = 0;
    function ustaw() {
      for (var i = 0; i < N; i++) {
        var p = droga.getPointAtLength((i * dl / N + faza) % dl);
        kropki[i].setAttribute("cx", p.x.toFixed(1)); kropki[i].setAttribute("cy", p.y.toFixed(1));
        kropki[i].style.display = zamkniety ? "" : "none";
      }
    }
    function przelacz() {
      zamkniety = !zamkniety;
      var d = $(box, ".o-dzwignia");
      d.setAttribute("x2", zamkniety ? "308" : "300"); d.setAttribute("y2", zamkniety ? "230" : "196");
      $(box, ".o-blask").setAttribute("opacity", zamkniety ? "0.45" : "0");
      $(box, ".o-dioda").setAttribute("fill", zamkniety ? "#F2A33A" : "#E9E7DF");
      $(box, ".o-przycisk").textContent = zamkniety ? "Otwórz obwód" : "Zamknij obwód";
      $(box, ".wynik").textContent = zamkniety
        ? "Obwód zamknięty: prąd płynie od plusa, przez opornik i diodę, z powrotem do minusa. Dioda świeci."
        : "Obwód otwarty: w pętli jest przerwa, prąd nie płynie. Dioda nie świeci, choć bateria ma napięcie.";
      ustaw();
    }
    var klik = $(box, ".o-klik");
    klik.addEventListener("click", przelacz);
    klik.addEventListener("keydown", function (e) { if (e.key === "Enter" || e.key === " ") { e.preventDefault(); przelacz(); } });
    $(box, ".o-przycisk").onclick = przelacz;
    zamkniety = true; przelacz();   // start: otwarty
    if (ruch) animuj(box, function (dt) { if (zamkniety) { faza += 70 * dt; ustaw(); } });
  };

  // =====================================================================
  // 3. Jakie napięcia mamy na stanowisku
  // =====================================================================
  R.napiecia = function (box) {
    var bloki = {
      zasilacz: { x: 20, y: 30, t: "Zasilacz 12 V", o: "Zasilacz z gniazdka daje 12 V. To dużo jak na naszą elektronikę – dlatego 12 V idzie tylko do przetwornicy." },
      przetw: { x: 190, y: 30, t: "Przetwornica", t2: "step-down", o: "Przetwornica step-down obniża 12 V do niższego napięcia, którym zasilamy moduły ESC i silniki. Silniki biorą duży prąd, więc mają osobne zasilanie." },
      esc: { x: 360, y: 30, t: "4 × ESC", t2: "od zajęć 3", o: "Moduły ESC dostają zasilanie z przetwornicy i sygnał PWM z ESP32. Na tej podstawie podają prąd silnikom." },
      silniki: { x: 530, y: 30, t: "Silniki 816", o: "Cztery silniki szczotkowe. Im większa moc z ESC, tym szybciej się kręcą." },
      usb: { x: 20, y: 190, t: "Komputer", t2: "USB 5 V", o: "Port USB komputera daje 5 V. Tym kablem zasilamy ESP32 i wgrywamy programy." },
      esp: { x: 190, y: 190, t: "ESP32-C6", t2: "5 V → 3,3 V", o: "Na płytce ESP32 jest mały stabilizator, który robi z 5 V napięcie 3,3 V. Mikrokontroler pracuje na 3,3 V i daje 3,3 V na pinie 3V3." },
      czujniki: { x: 360, y: 190, t: "Czujniki", t2: "MPU6050, VL53L0X", o: "Czujniki zasilamy z pinu 3V3 płytki ESP32. Ich sygnały też mają 3,3 V, więc mogą rozmawiać z ESP32 bezpośrednio." }
    };
    var s = '<svg viewBox="0 0 680 330" role="img" aria-label="Schemat napięć na stanowisku">' +
      '<defs><marker id="np-gr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto"><path d="M0 0 L10 5 L0 10 z" fill="#4A5060"/></marker></defs>';
    function strz(x1, y1, x2, y2, kol, opis) {
      return '<line x1="' + x1 + '" y1="' + y1 + '" x2="' + x2 + '" y2="' + y2 + '" stroke="' + kol + '" stroke-width="6" marker-end="url(#np-gr)"/>' +
        (opis ? '<text x="' + ((x1 + x2) / 2) + '" y="' + (y1 - 10) + '" text-anchor="middle" font-size="13" font-weight="700" fill="' + kol + '">' + opis + "</text>" : "");
    }
    s += strz(150, 70, 186, 70, "#D7352A", "12 V") + strz(320, 70, 356, 70, "#F2A33A", "") + strz(490, 70, 526, 70, "#F2A33A", "") +
      strz(150, 230, 186, 230, "#8E44AD", "5 V") + strz(320, 230, 356, 230, "#1F5FC8", "3,3 V");
    s += '<line class="np-zly" x1="85" y1="112" x2="420" y2="186" stroke="#D7352A" stroke-width="5" stroke-dasharray="9 7" style="display:none"/>';
    Object.keys(bloki).forEach(function (k) {
      var b = bloki[k];
      s += '<g class="klikalne np-blok" data-blok="' + k + '" tabindex="0" role="button" aria-label="' + b.t + '">' +
        '<rect x="' + b.x + '" y="' + b.y + '" width="130" height="80" rx="10" fill="#FFFFFF" stroke="#13171F" stroke-width="2.5"/>' +
        '<text x="' + (b.x + 65) + '" y="' + (b.y + (b.t2 ? 36 : 46)) + '" text-anchor="middle" font-size="15" font-weight="700" fill="#13171F">' + b.t + "</text>" +
        (b.t2 ? '<text x="' + (b.x + 65) + '" y="' + (b.y + 58) + '" text-anchor="middle" font-size="12.5" fill="#4A5060">' + b.t2 + "</text>" : "") +
        "</g>";
    });
    s += '<text class="np-spalony" x="425" y="300" text-anchor="middle" font-size="16" font-weight="800" fill="#D7352A" style="display:none">SPALONY!</text>';
    s += '<text x="540" y="200" font-size="12.5" fill="#4A5060">Wszystkie masy (GND)</text><text x="540" y="216" font-size="12.5" fill="#4A5060">są połączone razem.</text>';
    s += "</svg>";
    html(box, s + '<div class="sterowanie"><button type="button" class="przycisk drugi np-test">Co jeśli podłączę 12 V do czujnika?</button></div>' +
      '<div class="wynik" aria-live="polite">Kliknij dowolny prostokąt, żeby przeczytać, co robi.</div>');
    var wynik = $(box, ".wynik");
    box.querySelectorAll(".np-blok").forEach(function (g) {
      function pokaz() {
        box.querySelectorAll(".np-blok rect").forEach(function (r) { r.setAttribute("stroke", "#13171F"); r.setAttribute("stroke-width", "2.5"); });
        g.querySelector("rect").setAttribute("stroke", "#1F5FC8"); g.querySelector("rect").setAttribute("stroke-width", "4.5");
        var b = bloki[g.getAttribute("data-blok")];
        wynik.innerHTML = "<b>" + b.t + ":</b> " + b.o;
      }
      g.addEventListener("click", pokaz);
      g.addEventListener("keydown", function (e) { if (e.key === "Enter" || e.key === " ") { e.preventDefault(); pokaz(); } });
    });
    var spalony = false;
    $(box, ".np-test").onclick = function () {
      spalony = !spalony;
      $(box, ".np-zly").style.display = spalony ? "" : "none";
      $(box, ".np-spalony").style.display = spalony ? "" : "none";
      var r = $(box, '[data-blok="czujniki"] rect');
      r.setAttribute("fill", spalony ? "#3A2A28" : "#FFFFFF");
      $(box, '[data-blok="czujniki"] text').setAttribute("fill", spalony ? "#FFFFFF" : "#13171F");
      this.textContent = spalony ? "Cofnij" : "Co jeśli podłączę 12 V do czujnika?";
      wynik.innerHTML = spalony
        ? "<b>Czujnik jest zrobiony na 3,3 V.</b> 12 V to prawie 4 razy więcej. Przez układ popłynąłby za duży prąd i czujnik spaliłby się w ułamku sekundy. Dlatego przewody 12 V nigdy nie trafiają na płytkę stykową z czujnikami."
        : "Kliknij dowolny prostokąt, żeby przeczytać, co robi.";
    };
  };

  // =====================================================================
  // 4. Multimetr
  // =====================================================================
  R.multimetr = function (box) {
    var TRYBY = [
      { id: "off", nazwa: "OFF", kat: -90, ekran: "", czerwona: null,
        opis: "Wyłączony. Po pomiarze zawsze przekręć pokrętło na OFF – bateria w multimetrze wystarczy na dłużej." },
      { id: "v", nazwa: "V⎓", kat: -30, ekran: "12.03 V", czerwona: "VΩ",
        opis: "<b>Napięcie stałe.</b> Czarna sonda w COM, czerwona w VΩ. Przykładasz sondy <b>równolegle</b> – do dwóch punktów obwodu, np. plus i minus zasilacza. Niczego nie rozłączasz." },
      { id: "ma", nazwa: "mA", kat: 30, ekran: "48.2 mA", czerwona: "mA",
        opis: "<b>Mały prąd</b> (do ok. 200 mA). Czerwona sonda w gnieździe mA. Multimetr wstawiasz <b>szeregowo</b>: rozłączasz obwód i prąd musi przepłynąć przez multimetr." },
      { id: "a", nazwa: "10A", kat: 90, ekran: "0.42 A", czerwona: "10A",
        opis: "<b>Duży prąd</b>, np. prąd silnika. Czerwona sonda w gnieździe 10A, połączenie szeregowe. <b>Uwaga:</b> po pomiarze od razu przełóż czerwoną sondę z powrotem do VΩ. Jeśli zostawisz ją w 10A i zmierzysz napięcie, zrobisz zwarcie." },
      { id: "o", nazwa: "Ω / pisk", kat: 150, ekran: "000 · piip!", czerwona: "VΩ",
        opis: "<b>Ciągłość</b> – czy przewód przewodzi. Obwód musi być <b>bez zasilania</b>. Dotykasz sondami dwóch końców przewodu: pisk oznacza połączenie. Tak sprawdzamy przewody i luty." }
    ];
    var GN = { "10A": 300, "mA": 360, "COM": 420, "VΩ": 480 };
    var s = '<svg viewBox="0 0 760 380" role="img" aria-label="Multimetr z pokrętłem i gniazdami">' +
      '<rect x="250" y="10" width="290" height="365" rx="26" fill="#F2A33A" stroke="#13171F" stroke-width="3"/>' +
      '<rect x="275" y="34" width="240" height="72" rx="8" fill="#C9D6B8" stroke="#13171F" stroke-width="2"/>' +
      '<text class="mm-ekran" x="500" y="83" text-anchor="end" font-family="monospace" font-size="34" font-weight="700" fill="#13171F"></text>' +
      '<circle cx="395" cy="205" r="58" fill="#232937" stroke="#13171F" stroke-width="3"/>' +
      '<g class="mm-wskaz"><rect x="389" y="150" width="12" height="54" rx="5" fill="#F5F4EF"/></g>' +
      '<circle cx="395" cy="205" r="16" fill="#13171F"/>';
    TRYBY.forEach(function (t, i) {
      var rad = t.kat * Math.PI / 180, x = 395 + 84 * Math.cos(rad), y = 205 + 80 * Math.sin(rad) + 5;
      s += '<g class="klikalne mm-poz" data-i="' + i + '" tabindex="0" role="button" aria-label="Pokrętło: ' + t.nazwa + '">' +
        '<circle cx="' + x.toFixed(1) + '" cy="' + (y - 5).toFixed(1) + '" r="22" fill="transparent"/>' +
        '<text x="' + x.toFixed(1) + '" y="' + y.toFixed(1) + '" text-anchor="middle" font-size="15" font-weight="800" fill="#13171F">' + t.nazwa + "</text></g>";
    });
    Object.keys(GN).forEach(function (g) {
      s += '<circle class="mm-gn" data-gn="' + g + '" cx="' + GN[g] + '" cy="345" r="13" fill="#13171F" stroke="#F5F4EF" stroke-width="3"/>' +
        '<text x="' + GN[g] + '" y="324" text-anchor="middle" font-size="13" font-weight="800" fill="#13171F">' + g + "</text>";
    });
    // schemat połączenia po lewej
    s += '<g class="mm-schemat"></g></svg>';
    html(box, s + '<div class="sterowanie grupa-przyciskow">' + TRYBY.map(function (t, i) {
      return '<button type="button" class="przycisk drugi" data-tryb="' + i + '">' + t.nazwa + "</button>";
    }).join("") + '</div><div class="wynik" aria-live="polite"></div>');

    function schemat(t) {
      // mały obwód: bateria + silnik, i miejsce multimetru
      var g = '<text x="125" y="30" text-anchor="middle" font-size="14" font-weight="700" fill="#13171F">Jak podłączyć</text>';
      if (t.id === "off") return g + '<text x="125" y="190" text-anchor="middle" font-size="14" fill="#4A5060">multimetr wyłączony</text>';
      var szer = t.id === "ma" || t.id === "a";
      g += '<rect x="30" y="140" width="30" height="80" rx="4" fill="#FFFFFF" stroke="#13171F" stroke-width="2.5"/>' +
        '<text x="45" y="134" text-anchor="middle" font-size="16" font-weight="800" fill="#D7352A">+</text>' +
        '<text x="45" y="240" text-anchor="middle" font-size="18" font-weight="800" fill="#13171F">−</text>' +
        '<circle cx="200" cy="180" r="26" fill="#FFFFFF" stroke="#13171F" stroke-width="2.5"/><text x="200" y="186" text-anchor="middle" font-size="16" font-weight="800">M</text>';
      if (t.id === "o") {
        g = '<text x="125" y="30" text-anchor="middle" font-size="14" font-weight="700" fill="#13171F">Jak podłączyć</text>' +
          '<path d="M40 180 C 90 130, 160 230, 210 180" fill="none" stroke="#D7352A" stroke-width="6"/>' +
          '<text x="125" y="250" text-anchor="middle" font-size="13" fill="#4A5060">przewód bez zasilania</text>';
        g += '<line x1="40" y1="180" x2="70" y2="90" stroke="#D7352A" stroke-width="3"/><line x1="210" y1="180" x2="180" y2="90" stroke="#13171F" stroke-width="3"/>' +
          '<rect x="85" y="60" width="80" height="44" rx="8" fill="#F2A33A" stroke="#13171F" stroke-width="2"/><text x="125" y="88" text-anchor="middle" font-size="14" font-weight="800">piip</text>';
        return g;
      }
      if (szer) {
        g += '<path d="M45 140 L45 90 L95 90"  fill="none" stroke="#13171F" stroke-width="3"/>' +
          '<path d="M155 90 L200 90 L200 154" fill="none" stroke="#13171F" stroke-width="3"/>' +
          '<path d="M200 206 L200 270 L45 270 L45 220" fill="none" stroke="#13171F" stroke-width="3"/>' +
          '<rect x="95" y="68" width="60" height="44" rx="8" fill="#F2A33A" stroke="#13171F" stroke-width="2"/><text x="125" y="96" text-anchor="middle" font-size="15" font-weight="800">A</text>' +
          '<text x="125" y="300" text-anchor="middle" font-size="13" fill="#4A5060">szeregowo: prąd płynie przez multimetr</text>';
      } else {
        g += '<path d="M45 140 L45 110 L200 110 L200 154" fill="none" stroke="#13171F" stroke-width="3"/>' +
          '<path d="M200 206 L200 250 L45 250 L45 220" fill="none" stroke="#13171F" stroke-width="3"/>' +
          '<line x1="45" y1="110" x2="45" y2="70" stroke="#D7352A" stroke-width="3"/><line x1="45" y1="250" x2="20" y2="300" stroke="#13171F" stroke-width="3"/>' +
          '<rect x="75" y="48" width="70" height="44" rx="8" fill="#F2A33A" stroke="#13171F" stroke-width="2"/><text x="110" y="76" text-anchor="middle" font-size="15" font-weight="800">V</text>' +
          '<line x1="45" y1="70" x2="75" y2="70" stroke="#D7352A" stroke-width="3"/><path d="M145 70 L230 70 L230 310 L20 310 L20 300" fill="none" stroke="#13171F" stroke-width="3"/>' +
          '<text x="125" y="340" text-anchor="middle" font-size="13" fill="#4A5060">równolegle: sondy na dwóch punktach</text>';
      }
      return g;
    }
    function ustaw(i) {
      var t = TRYBY[i];
      $(box, ".mm-wskaz").setAttribute("transform", "rotate(" + (t.kat + 90) + " 395 205)");
      $(box, ".mm-ekran").textContent = t.ekran;
      box.querySelectorAll(".mm-gn").forEach(function (c) {
        var g = c.getAttribute("data-gn");
        c.setAttribute("fill", g === "COM" && t.czerwona ? "#13171F" : (g === t.czerwona ? "#D7352A" : "#6B7385"));
        c.setAttribute("r", (g === "COM" && t.czerwona) || g === t.czerwona ? "16" : "11");
      });
      $(box, ".mm-schemat").innerHTML = schemat(t);
      box.querySelectorAll("[data-tryb]").forEach(function (b) { b.classList.toggle("aktywny", +b.getAttribute("data-tryb") === i); });
      $(box, ".wynik").innerHTML = t.opis + (t.czerwona ? " <br><small>Gniazda: czarna sonda – <b>COM</b>, czerwona – <b>" + t.czerwona + "</b>.</small>" : "");
    }
    box.querySelectorAll(".mm-poz").forEach(function (g) {
      var i = +g.getAttribute("data-i");
      g.addEventListener("click", function () { ustaw(i); });
      g.addEventListener("keydown", function (e) { if (e.key === "Enter" || e.key === " ") { e.preventDefault(); ustaw(i); } });
    });
    box.querySelectorAll("[data-tryb]").forEach(function (b) { b.onclick = function () { ustaw(+b.getAttribute("data-tryb")); }; });
    ustaw(1);
  };

  // =====================================================================
  // 5. Silnik szczotkowy
  // =====================================================================
  R.silnik = function (box) {
    var T = K.typySilnikow || { A: { kierunek: "CW", plus: "czerwony", minus: "niebieski" }, B: { kierunek: "CCW", plus: "biały", minus: "czarny" } };
    html(box,
      '<svg viewBox="0 0 640 330" role="img" aria-label="Silnik szczotkowy widziany z góry, podłączony do zasilacza">' +
      '<text x="180" y="14" text-anchor="middle" font-size="13" fill="#4A5060">widok z góry, od strony śmigła</text>' +
      '<circle cx="180" cy="140" r="100" fill="#D9D7CF" stroke="#13171F" stroke-width="3"/>' +
      '<path d="M100 80 A 100 100 0 0 0 100 200 L118 186 A 78 78 0 0 1 118 94 Z" fill="#D7352A"/>' +
      '<path d="M260 80 A 100 100 0 0 1 260 200 L242 186 A 78 78 0 0 0 242 94 Z" fill="#1F5FC8"/>' +
      '<text x="96" y="146" font-size="18" font-weight="800" fill="#FFFFFF">N</text><text x="252" y="146" font-size="18" font-weight="800" fill="#FFFFFF">S</text>' +
      '<g class="s-wirnik">' +
      [0, 120, 240].map(function (a) {
        return '<g transform="rotate(' + a + ' 180 140)"><rect x="174" y="78" width="12" height="52" fill="#8A6A3C"/>' +
          '<rect x="160" y="70" width="40" height="16" rx="5" fill="#C98B3A" stroke="#13171F" stroke-width="1.5"/></g>';
      }).join("") +
      '<circle cx="180" cy="140" r="16" fill="#13171F"/><circle cx="180" cy="140" r="5" fill="#F5F4EF"/></g>' +
      '<g class="s-strzalka"></g>' +
      '<text class="s-kier" x="180" y="292" text-anchor="middle" font-size="16" font-weight="800" fill="#13171F"></text>' +
      '<path class="s-p1" fill="none" stroke-width="6" stroke-linecap="round"/>' +
      '<path class="s-p2" fill="none" stroke-width="6" stroke-linecap="round"/>' +
      '<rect x="420" y="70" width="190" height="150" rx="12" fill="#232937" stroke="#13171F" stroke-width="3"/>' +
      '<text x="515" y="96" text-anchor="middle" font-size="13" fill="#C9CEDA">zasilacz</text>' +
      '<rect x="440" y="106" width="150" height="56" rx="6" fill="#0E1015"/>' +
      '<text class="s-u" x="580" y="130" text-anchor="end" font-family="monospace" font-size="20" fill="#F2A33A"></text>' +
      '<text class="s-i" x="580" y="154" text-anchor="end" font-family="monospace" font-size="20" fill="#7FD49A"></text>' +
      '<circle cx="470" cy="192" r="11" fill="#D7352A" stroke="#FFFFFF" stroke-width="2"/><text x="470" y="198" text-anchor="middle" font-size="16" font-weight="800" fill="#FFFFFF">+</text>' +
      '<circle cx="560" cy="192" r="11" fill="#13171F" stroke="#FFFFFF" stroke-width="2"/><text x="560" y="198" text-anchor="middle" font-size="18" font-weight="800" fill="#FFFFFF">−</text>' +
      '<text class="s-opis" x="515" y="58" text-anchor="middle" font-size="13" fill="#4A5060"></text>' +
      '</svg>' +
      '<div class="sterowanie"><div class="grupa-przyciskow">' +
      '<button type="button" class="przycisk drugi" data-typ="A">Silnik typu A</button>' +
      '<button type="button" class="przycisk drugi" data-typ="B">Silnik typu B</button>' +
      '<button type="button" class="przycisk s-zamien">Zamień bieguny</button></div>' +
      suwak("u", "Napięcie z zasilacza", 0, 4.2, 0.1, 2.5) + '</div>' +
      '<div class="wynik" aria-live="polite"></div>' +
      '<p class="podpis">Liczby na zasilaczu są przykładowe. Prawdziwy silnik 816 kręci się kilkadziesiąt tysięcy razy na minutę – na rysunku jest bardzo zwolniony.</p>');
    var typ = "A", zamienione = false, kat = 0, obr = 0;
    var su = $(box, '[data-suwak="u"]');
    function rysuj() {
      var t = T[typ], U = +su.value;
      var kierunek = zamienione ? (t.kierunek === "CW" ? "CCW" : "CW") : t.kierunek;
      var doPlusa = zamienione ? t.minus : t.plus, doMinusa = zamienione ? t.plus : t.minus;
      // przewody: od silnika (dół) do zacisków zasilacza
      var p1 = $(box, ".s-p1"), p2 = $(box, ".s-p2");
      p1.setAttribute("d", "M266 190 C 340 195, 410 280, 470 205");
      p2.setAttribute("d", "M250 212 C 320 260, 540 300, 560 205");
      p1.setAttribute("stroke", kolor(doPlusa)); p2.setAttribute("stroke", kolor(doMinusa));
      [p1, p2].forEach(function (p) { p.style.filter = "drop-shadow(0 0 1px #13171F)"; });
      $(box, ".s-strzalka").innerHTML = U > 0 ? strzalkaObrotu(180, 140, 118, kierunek === "CW", "#2E8B4F") : "";
      $(box, ".s-kier").textContent = U > 0 ? (kierunek === "CW" ? "CW – zgodnie z zegarem" : "CCW – przeciwnie do zegara") : "stoi";
      var I = U > 0 ? 0.06 + 0.07 * U : 0;
      $(box, ".s-u").textContent = fmt(U, 1) + " V";
      $(box, ".s-i").textContent = fmt(I, 2) + " A";
      $(box, '[data-wart="u"]').textContent = fmt(U, 1) + " V";
      $(box, ".s-opis").textContent = "+ " + doPlusa + "  ·  − " + doMinusa;
      obr = U / 4.2 * 1.6 * (kierunek === "CW" ? 1 : -1);
      box.querySelectorAll("[data-typ]").forEach(function (b) { b.classList.toggle("aktywny", b.getAttribute("data-typ") === typ); });
      var dobrze = !zamienione;
      $(box, ".wynik").innerHTML = "Silnik typu <b>" + typ + "</b> (" + t.plus + " i " + t.minus + ") powinien kręcić się <b>" + t.kierunek + "</b>. " +
        (dobrze ? "Plus jest na przewodzie " + t.plus + " – kierunek się zgadza."
          : "Bieguny są zamienione – silnik kręci się w złą stronę (" + kierunek + ").") +
        " Większe napięcie = szybsze obroty i większy prąd.";
    }
    box.querySelectorAll("[data-typ]").forEach(function (b) { b.onclick = function () { typ = b.getAttribute("data-typ"); zamienione = false; rysuj(); }; });
    $(box, ".s-zamien").onclick = function () { zamienione = !zamienione; rysuj(); };
    su.oninput = rysuj;
    rysuj();
    if (ruch) animuj(box, function (dt) {
      kat = (kat + obr * 360 * dt) % 360;
      $(box, ".s-wirnik").setAttribute("transform", "rotate(" + kat.toFixed(1) + " 180 140)");
    });
  };

  // =====================================================================
  // 6. PWM
  // =====================================================================
  R.pwm = function (box) {
    html(box,
      '<svg viewBox="0 0 640 250" role="img" aria-label="Przebieg PWM, dioda i śmigło">' +
      '<line x1="20" y1="130" x2="430" y2="130" stroke="#BDB9AD" stroke-width="1.5"/>' +
      '<text x="14" y="44" text-anchor="end" font-size="12" fill="#4A5060">1</text><text x="14" y="134" text-anchor="end" font-size="12" fill="#4A5060">0</text>' +
      '<polyline class="p-fala" fill="none" stroke="#1F5FC8" stroke-width="4" stroke-linejoin="round"/>' +
      '<line class="p-sr" x1="20" x2="430" stroke="#F2A33A" stroke-width="3" stroke-dasharray="8 6"/>' +
      '<text class="p-sr-t" x="430" text-anchor="end" font-size="13" font-weight="700" fill="#B5711C">średnio</text>' +
      '<text x="225" y="160" text-anchor="middle" font-size="13" fill="#4A5060">czas →  (4 okresy)</text>' +
      '<circle class="p-blask" cx="520" cy="70" r="44" fill="#F2A33A"/>' +
      '<circle cx="520" cy="70" r="20" fill="#FDF0DC" stroke="#13171F" stroke-width="3"/>' +
      '<circle class="p-led" cx="520" cy="70" r="17" fill="#F2A33A"/>' +
      '<text x="520" y="128" text-anchor="middle" font-size="13" fill="#4A5060">dioda</text>' +
      '<g class="p-smiglo"><ellipse cx="520" cy="188" rx="38" ry="7" fill="#13171F"/><circle cx="520" cy="188" r="6" fill="#F2A33A"/></g>' +
      '<text x="520" y="244" text-anchor="middle" font-size="13" fill="#4A5060">śmigło</text>' +
      '</svg>' +
      '<div class="sterowanie">' + suwak("w", "Wypełnienie", 0, 100, 5, 25) + '</div><div class="wynik" aria-live="polite"></div>');
    var sw = $(box, '[data-suwak="w"]'), kat = 0, w = 0.25;
    function rysuj() {
      w = +sw.value / 100;
      var pts = [], okres = 100, x0 = 25;
      for (var i = 0; i < 4; i++) {
        var a = x0 + i * okres, b = a + okres * w;
        if (w > 0) pts.push(a + ",130", a + ",40", b + ",40", b + ",130");
        else pts.push(a + ",130");
        pts.push((a + okres) + ",130");
      }
      $(box, ".p-fala").setAttribute("points", pts.join(" "));
      var ys = 130 - 90 * w;
      $(box, ".p-sr").setAttribute("y1", ys); $(box, ".p-sr").setAttribute("y2", ys);
      $(box, ".p-sr-t").setAttribute("y", ys - 6);
      $(box, ".p-blask").setAttribute("opacity", (0.6 * w).toFixed(2));
      $(box, ".p-led").setAttribute("opacity", (0.15 + 0.85 * w).toFixed(2));
      $(box, '[data-wart="w"]').textContent = sw.value + "%";
      $(box, ".wynik").textContent = "Pin jest włączony przez " + sw.value + "% czasu i wyłączony przez " + (100 - sw.value) +
        "%. Dioda i silnik czują średnią: " + sw.value + "% mocy.";
    }
    sw.oninput = rysuj;
    rysuj();
    if (ruch) animuj(box, function (dt) {
      kat = (kat + w * 900 * dt) % 360;
      $(box, ".p-smiglo").setAttribute("transform", "rotate(" + kat.toFixed(1) + " 520 188)");
    });
  };

  // =====================================================================
  // 7. Płytka stykowa (interaktywna)
  // =====================================================================
  var PL = { x0: 46, p: 18, kol: 30 };
  function plytkaY(rzad) {
    var m = { "g+": 30, "g-": 48, a: 92, b: 110, c: 128, d: 146, e: 164, f: 208, g: 226, h: 244, i: 262, j: 280, "d+": 324, "d-": 342 };
    return m[rzad];
  }
  function plytkaX(kol) { return PL.x0 + (kol - 1) * PL.p; }

  R.plytka = function (box) {
    var podzielone = false, esp = false;
    var przew = K.przewodyI2C || {};
    var J1 = (K.pinout && K.pinout.J1) || [], J3 = (K.pinout && K.pinout.J3) || [];
    var ESP_OD = 8;   // kolumna pierwszej nóżki
    function grupa(rzad, kol) {
      if ("abcde".indexOf(rzad) >= 0) return "k" + kol + "-ae";
      if ("fghij".indexOf(rzad) >= 0) return "k" + kol + "-fj";
      return "s" + rzad + (podzielone ? (kol <= 15 ? "-L" : "-P") : "");
    }
    function zbuduj() {
      var W = PL.x0 + PL.kol * PL.p + 26;
      var s = '<svg viewBox="0 0 ' + W + ' 372" role="img" aria-label="Płytka stykowa – kliknij otwór">' +
        '<rect x="8" y="8" width="' + (W - 16) + '" height="356" rx="12" fill="#F4F3EE" stroke="#BDB9AD" stroke-width="2"/>' +
        '<rect x="20" y="182" width="' + (W - 40) + '" height="12" rx="3" fill="#E2E0D8"/>';
      // linie szyn
      [["g+", "#D7352A", 21], ["g-", "#1F5FC8", 57], ["d+", "#D7352A", 315], ["d-", "#1F5FC8", 351]].forEach(function (sz) {
        if (podzielone) {
          s += '<line x1="' + (plytkaX(1) - 8) + '" x2="' + (plytkaX(15) + 8) + '" y1="' + sz[2] + '" y2="' + sz[2] + '" stroke="' + sz[1] + '" stroke-width="2"/>';
          s += '<line x1="' + (plytkaX(16) - 8) + '" x2="' + (plytkaX(30) + 8) + '" y1="' + sz[2] + '" y2="' + sz[2] + '" stroke="' + sz[1] + '" stroke-width="2"/>';
        } else s += '<line x1="' + (plytkaX(1) - 8) + '" x2="' + (plytkaX(30) + 8) + '" y1="' + sz[2] + '" y2="' + sz[2] + '" stroke="' + sz[1] + '" stroke-width="2"/>';
      });
      s += '<text x="22" y="35" font-size="13" font-weight="800" fill="#D7352A">+</text><text x="23" y="53" font-size="15" font-weight="800" fill="#1F5FC8">−</text>';
      s += '<text x="22" y="329" font-size="13" font-weight="800" fill="#D7352A">+</text><text x="23" y="347" font-size="15" font-weight="800" fill="#1F5FC8">−</text>';
      if (podzielone) s += '<text x="' + ((plytkaX(15) + plytkaX(16)) / 2) + '" y="72" text-anchor="middle" font-size="11" font-weight="700" fill="#C7372B">przerwa</text>';
      "abcdefghij".split("").forEach(function (r) {
        s += '<text x="' + (PL.x0 - 16) + '" y="' + (plytkaY(r) + 4) + '" font-size="11" fill="#6B7385">' + r + "</text>";
      });
      for (var k = 1; k <= PL.kol; k++) {
        if (k === 1 || k % 5 === 0) s += '<text x="' + plytkaX(k) + '" y="80" text-anchor="middle" font-size="10" fill="#6B7385">' + k + "</text>";
        ["g+", "g-", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "d+", "d-"].forEach(function (r) {
          var szyna = r.length === 2;
          if (szyna && k % 6 === 0) return;   // na szynach otwory są w grupach po 5
          s += '<rect class="otwor" data-r="' + r + '" data-k="' + k + '" data-g="' + grupa(r, k) + '" x="' + (plytkaX(k) - 5) + '" y="' + (plytkaY(r) - 5) +
            '" width="10" height="10" rx="2" fill="#4A4A4A"/>';
        });
      }
      if (esp) {
        var x1 = plytkaX(ESP_OD) - 9, x2 = plytkaX(ESP_OD + 15) + 9;
        s += '<g pointer-events="none"><rect x="' + x1 + '" y="' + (plytkaY("b") - 12) + '" width="' + (x2 - x1) + '" height="' + (plytkaY("i") - plytkaY("b") + 24) +
          '" rx="6" fill="#13171F" opacity=".88"/>' +
          '<rect x="' + (x2 - 52) + '" y="' + (plytkaY("d") - 4) + '" width="40" height="54" rx="4" fill="#B8BEC9"/>' +
          '<text x="' + ((x1 + x2) / 2 - 20) + '" y="' + (plytkaY("e") + 18) + '" text-anchor="middle" font-size="15" font-weight="800" fill="#F5F4EF">ESP32-C6</text>' +
          '<rect x="' + (x1 - 2) + '" y="' + (plytkaY("e") + 4) + '" width="22" height="26" rx="3" fill="#8A91A3"/>' +
          '<text x="' + (x1 + 9) + '" y="' + (plytkaY("e") + 42) + '" text-anchor="middle" font-size="9" fill="#F5F4EF">USB</text>';
        for (var n = 0; n < 16; n++) {
          var kx = plytkaX(ESP_OD + n);
          s += '<rect x="' + (kx - 4) + '" y="' + (plytkaY("b") - 4) + '" width="8" height="8" fill="#E8B90C"/>' +
            '<rect x="' + (kx - 4) + '" y="' + (plytkaY("i") - 4) + '" width="8" height="8" fill="#E8B90C"/>' +
            '<text x="' + kx + '" y="' + (plytkaY("c") + 6) + '" text-anchor="middle" font-size="8" fill="#F5F4EF">' + (J1[n] || "") + "</text>" +
            '<text x="' + kx + '" y="' + (plytkaY("h") + 1) + '" text-anchor="middle" font-size="8" fill="#F5F4EF">' + (J3[n] || "") + "</text>";
        }
        // przewody I2C w rzędzie a
        var doPinu = { "3V3": "3V3", GND: "GND", SDA: String(K.piny ? K.piny.sda : 6), SCL: String(K.piny ? K.piny.scl : 7) };
        Object.keys(doPinu).forEach(function (sygnal) {
          var idx = J1.indexOf(doPinu[sygnal]);
          if (idx < 0) return;
          var kx = plytkaX(ESP_OD + idx), kk = kolor(przew[sygnal]);
          s += '<path d="M' + kx + " " + plytkaY("a") + " L" + kx + " " + (plytkaY("a") - 16) + " L" + (kx + 6) + ' 14" fill="none" stroke="' + kk +
            '" stroke-width="5" stroke-linecap="round" style="filter:drop-shadow(0 0 1px #13171F)"/>';
        });
        s += "</g>";
      }
      s += "</svg>";
      $(box, ".pl-svg").innerHTML = s;
      box.querySelectorAll(".otwor").forEach(function (o) { o.addEventListener("click", function () { zaznacz(o); }); });
    }
    function zaznacz(o) {
      var g = o.getAttribute("data-g"), ile = 0;
      box.querySelectorAll(".otwor").forEach(function (x) {
        var t = x.getAttribute("data-g") === g;
        x.setAttribute("fill", t ? "#1F5FC8" : "#4A4A4A");
        x.setAttribute("width", t ? "12" : "10"); x.setAttribute("height", t ? "12" : "10");
        x.setAttribute("x", plytkaX(+x.getAttribute("data-k")) - (t ? 6 : 5));
        x.setAttribute("y", plytkaY(x.getAttribute("data-r")) - (t ? 6 : 5));
        if (t) ile++;
      });
      o.setAttribute("fill", "#F2A33A");
      var r = o.getAttribute("data-r"), k = +o.getAttribute("data-k"), tekst;
      if (r.length === 2) {
        tekst = "Szyna zasilania " + (r[1] === "+" ? "„+”" : "„−”") + (r[0] === "g" ? " na górze" : " na dole") + ": połączonych jest " + ile + " otworów" +
          (podzielone ? " – tylko w tej połowie płytki." : " – wzdłuż całej płytki.");
      } else {
        var gora = "abcde".indexOf(r) >= 0;
        tekst = "Kolumna " + k + ", rzędy " + (gora ? "a–e" : "f–j") + ": te 5 otworów jest połączonych ze sobą. Z drugą stroną przerwy nie mają połączenia.";
        if (esp && k >= ESP_OD && k < ESP_OD + 16) {
          var pin = gora ? J1[k - ESP_OD] : J3[k - ESP_OD];
          tekst += " Ta kolumna jest połączona z nóżką <b>" + pin + "</b> płytki ESP32.";
        }
      }
      $(box, ".wynik").innerHTML = tekst;
    }
    html(box, '<div class="pl-svg przewin"></div><div class="sterowanie grupa-przyciskow">' +
      '<button type="button" class="przycisk drugi pl-esp">Pokaż ESP32 na płytce</button>' +
      '<button type="button" class="przycisk drugi pl-dziel">Szyny przerwane w połowie</button></div>' +
      '<div class="wynik" aria-live="polite">Kliknij dowolny otwór – podświetlą się wszystkie otwory, które są z nim połączone.</div>' +
      '<p class="podpis">Kolory przewodów I2C: ' + Object.keys(przew).map(function (s) {
        return '<span class="kolor-przewodu" style="background:' + kolor(przew[s]) + '"></span>' + s + " – " + przew[s];
      }).join(", ") + ".</p>");
    $(box, ".pl-esp").onclick = function () { esp = !esp; this.classList.toggle("aktywny", esp); this.textContent = esp ? "Ukryj ESP32" : "Pokaż ESP32 na płytce"; zbuduj(); };
    $(box, ".pl-dziel").onclick = function () { podzielone = !podzielone; this.classList.toggle("aktywny", podzielone); zbuduj(); };
    zbuduj();
  };

  // =====================================================================
  // 8. Gdzie jest błąd? (płytka stykowa)
  // =====================================================================
  R.bledy = function (box) {
    function mini(zawartosc) {
      // mała płytka: 10 kolumn, szyny + i − na górze, rzędy a–e
      var s = '<svg viewBox="0 0 230 170" role="img"><rect x="4" y="4" width="222" height="162" rx="8" fill="#F4F3EE" stroke="#BDB9AD" stroke-width="2"/>' +
        '<line x1="20" x2="212" y1="14" y2="14" stroke="#D7352A" stroke-width="2"/><line x1="20" x2="212" y1="44" y2="44" stroke="#1F5FC8" stroke-width="2"/>' +
        '<text x="8" y="26" font-size="11" font-weight="800" fill="#D7352A">+</text><text x="8" y="40" font-size="12" font-weight="800" fill="#1F5FC8">−</text>';
      for (var k = 0; k < 10; k++) {
        var x = 26 + k * 20;
        s += '<rect x="' + (x - 4) + '" y="18" width="8" height="8" rx="1.5" fill="#4A4A4A"/><rect x="' + (x - 4) + '" y="32" width="8" height="8" rx="1.5" fill="#4A4A4A"/>';
        for (var r = 0; r < 5; r++) s += '<rect x="' + (x - 4) + '" y="' + (66 + r * 20) + '" width="8" height="8" rx="1.5" fill="#4A4A4A"/>';
        s += '<text x="' + x + '" y="62" text-anchor="middle" font-size="8" fill="#6B7385">' + (k + 1) + "</text>";
      }
      return s + zawartosc + "</svg>";
    }
    function X(k) { return 26 + (k - 1) * 20; }
    function Y(r) { return 70 + r * 20; }   // r = 0 (a) … 4 (e)
    var opornik = function (k1, r1, k2, r2, blad) {
      var x1 = X(k1), y1 = Y(r1), x2 = X(k2), y2 = Y(r2), mx = (x1 + x2) / 2, my = (y1 + y2) / 2;
      var poziomo = r1 === r2;
      return '<g class="klikalne" data-blad="' + (blad ? 1 : 0) + '" tabindex="0" role="button" aria-label="opornik">' +
        '<line x1="' + x1 + '" y1="' + y1 + '" x2="' + x2 + '" y2="' + y2 + '" stroke="#8A91A3" stroke-width="2.5"/>' +
        '<rect x="' + (poziomo ? mx - 14 : mx - 6) + '" y="' + (poziomo ? my - 6 : my - 14) + '" width="' + (poziomo ? 28 : 12) + '" height="' + (poziomo ? 12 : 28) +
        '" rx="4" fill="#E3C38E" stroke="#13171F" stroke-width="1.5"/>' +
        '<rect x="' + Math.min(x1, x2) + '" y="' + (Math.min(y1, y2) - 8) + '" width="' + (Math.abs(x2 - x1) + 16) + '" height="' + (Math.abs(y2 - y1) + 16) + '" fill="transparent" transform="translate(-8 0)"/></g>';
    };
    var dioda = function (k1, r1, k2, r2, blad, etyk) {
      var x1 = X(k1), y1 = Y(r1), x2 = X(k2), y2 = Y(r2), mx = (x1 + x2) / 2;
      return '<g class="klikalne" data-blad="' + (blad ? 1 : 0) + '" tabindex="0" role="button" aria-label="' + (etyk || "dioda") + '">' +
        '<line x1="' + x1 + '" y1="' + y1 + '" x2="' + mx + '" y2="' + (y1 - 22) + '" stroke="#8A91A3" stroke-width="2.5"/>' +
        '<line x1="' + x2 + '" y1="' + y2 + '" x2="' + mx + '" y2="' + (y1 - 22) + '" stroke="#8A91A3" stroke-width="2.5"/>' +
        '<circle cx="' + mx + '" cy="' + (y1 - 26) + '" r="11" fill="#E8574A" stroke="#13171F" stroke-width="1.5"/>' +
        '<circle cx="' + mx + '" cy="' + (y1 - 14) + '" r="20" fill="transparent"/></g>';
    };
    var przewod = function (x1, y1, x2, y2, kol, blad, etyk) {
      return '<g class="klikalne" data-blad="' + (blad ? 1 : 0) + '" tabindex="0" role="button" aria-label="' + etyk + '">' +
        '<path d="M' + x1 + " " + y1 + " C " + x1 + " " + (y1 - 30) + ", " + x2 + " " + (y2 + 30) + ", " + x2 + " " + y2 + '" fill="none" stroke="' + kol +
        '" stroke-width="5" stroke-linecap="round" style="filter:drop-shadow(0 0 1px #13171F)"/>' +
        '<path d="M' + x1 + " " + y1 + " C " + x1 + " " + (y1 - 30) + ", " + x2 + " " + (y2 + 30) + ", " + x2 + " " + y2 + '" fill="none" stroke="transparent" stroke-width="16"/></g>';
    };
    var czerw = kolor((K.przewodyI2C || {})["3V3"] || "czerwony"), czarny = kolor((K.przewodyI2C || {}).GND || "czarny");
    var PRZYKLADY = [
      { t: "Przykład 1: dioda i opornik",
        svg: mini(przewod(X(2), 22, X(2), Y(0), czerw, false, "przewód plus") + opornik(2, 1, 2, 3, true) + dioda(5, 4, 6, 4, false) +
          przewod(X(6), Y(2), X(8), 36, czarny, false, "przewód minus")),
        ok: "Brawo! Oba końce opornika są w tej samej kolumnie (2). Te otwory są połączone, więc prąd omija opornik – opornik nic nie robi. Jeden koniec przełóż do innej kolumny.",
        nie: "Ten element jest wpięty dobrze. Szukaj dalej – sprawdź, w których kolumnach są końce każdego elementu." },
      { t: "Przykład 2: zasilanie czujnika",
        svg: mini(przewod(X(3), Y(0), X(3), 22, czerw, false, "przewód 3V3") + przewod(X(5), Y(0), X(6), 22, czarny, true, "przewód GND") +
          '<rect x="' + (X(3) - 8) + '" y="' + (Y(1) - 6) + '" width="' + (X(6) - X(3) + 16) + '" height="34" rx="4" fill="#2E6B4F"/><text x="' + ((X(3) + X(6)) / 2) +
          '" y="' + (Y(2) + 4) + '" text-anchor="middle" font-size="10" fill="#fff">czujnik</text>'),
        ok: "Brawo! Czarny przewód GND jest wpięty w szynę „+” zamiast w „−”. Czujnik dostałby plus na oba piny zasilania i nie zadziała. Przełóż go do szyny niebieskiej (−).",
        nie: "Ten przewód jest dobrze. Sprawdź, do której szyny idzie każdy przewód – czerwona linia to +, niebieska to −." },
      { t: "Przykład 3: przewód do diody",
        svg: mini(przewod(X(2), 22, X(2), Y(0), czerw, false, "przewód plus") + opornik(2, 2, 5, 2, false) + dioda(6, 4, 8, 4, true, "dioda – lewa nóżka") +
          przewod(X(8), Y(2), X(9), 36, czarny, false, "przewód minus")),
        ok: "Brawo! Lewa nóżka diody jest w kolumnie 6, a opornik kończy się w kolumnie 5. Przesunięcie o jeden otwór i nie ma połączenia. Wepnij nóżkę diody w kolumnę 5.",
        nie: "Ten element jest dobrze. Porównaj numery kolumn tam, gdzie elementy powinny się stykać." }
    ];
    html(box, '<div class="bledy">' + PRZYKLADY.map(function (p, i) {
      return '<div class="blad-przyklad" data-p="' + i + '"><h4>' + p.t + '</h4>' + p.svg + '<div class="komunikat neutralnie" aria-live="polite">Gdzie jest błąd? Kliknij element.</div></div>';
    }).join("") + "</div>");
    box.querySelectorAll(".blad-przyklad").forEach(function (d) {
      var p = PRZYKLADY[+d.getAttribute("data-p")], kom = d.querySelector(".komunikat");
      d.querySelectorAll(".klikalne").forEach(function (g) {
        function klik() {
          var blad = g.getAttribute("data-blad") === "1";
          kom.className = "komunikat " + (blad ? "dobrze" : "zle");
          kom.textContent = blad ? p.ok : p.nie;
        }
        g.addEventListener("click", klik);
        g.addEventListener("keydown", function (e) { if (e.key === "Enter" || e.key === " ") { e.preventDefault(); klik(); } });
      });
    });
  };

  // =====================================================================
  // 9. Orientacja silników – ćwiczenie
  // =====================================================================
  var POZ = {
    "przod-lewy": { x: 110, y: 110, n: "przód-lewy" },
    "przod-prawy": { x: 330, y: 110, n: "przód-prawy" },
    "tyl-prawy": { x: 330, y: 330, n: "tył-prawy" },
    "tyl-lewy": { x: 110, y: 330, n: "tył-lewy" }
  };
  function typDlaKierunku(kier) {
    var T = K.typySilnikow || {};
    for (var t in T) if (T[t].kierunek === kier) return t;
    return "?";
  }
  function smigloDlaKierunku(kier) {
    var S = K.smigla || {};
    for (var t in S) if (t !== "opis" && S[t] === kier) return t;
    return "?";
  }
  function dronZGory(stan, pokazPodpisy) {
    // stan: { pozycja: {nr, typ, kier, smiglo} }
    var T = K.typySilnikow || {};
    var s = '<svg viewBox="0 0 440 450" role="img" aria-label="Dron widziany z góry">' +
      '<defs><marker id="dz-gr" viewBox="0 0 10 10" refX="5" refY="5" markerWidth="6" markerHeight="6" orient="auto"><path d="M0 0 L10 5 L0 10 z" fill="#1F5FC8"/></marker></defs>' +
      '<line x1="220" y1="60" x2="220" y2="20" stroke="#1F5FC8" stroke-width="5" marker-end="url(#dz-gr)"/>' +
      '<text x="250" y="34" font-size="15" font-weight="800" fill="#1F5FC8">PRZÓD</text>' +
      '<line x1="110" y1="110" x2="330" y2="330" stroke="#13171F" stroke-width="16" stroke-linecap="round"/>' +
      '<line x1="330" y1="110" x2="110" y2="330" stroke="#13171F" stroke-width="16" stroke-linecap="round"/>' +
      '<rect x="180" y="180" width="80" height="80" rx="12" fill="#232937" stroke="#13171F" stroke-width="3"/>' +
      '<text x="220" y="226" text-anchor="middle" font-size="12" fill="#C9CEDA">rama</text>';
    Object.keys(POZ).forEach(function (k) {
      var p = POZ[k], st = stan[k] || {};
      s += '<circle cx="' + p.x + '" cy="' + p.y + '" r="56" fill="#FFFFFF" stroke="#BDB9AD" stroke-width="2" stroke-dasharray="6 5"/>';
      if (st.kier) s += strzalkaObrotu(p.x, p.y, 48, st.kier === "CW", "#2E8B4F");
      s += '<circle cx="' + p.x + '" cy="' + p.y + '" r="30" fill="#E9E7DF" stroke="#13171F" stroke-width="3"/>';
      s += '<text x="' + p.x + '" y="' + (p.y + 7) + '" text-anchor="middle" font-size="20" font-weight="800" fill="#13171F">' + (st.nr || "?") + "</text>";
      if (st.typ && T[st.typ]) {
        s += '<circle cx="' + (p.x - 10) + '" cy="' + (p.y + 42) + '" r="7" fill="' + kolor(T[st.typ].plus) + '" stroke="#13171F" stroke-width="1.5"/>' +
          '<circle cx="' + (p.x + 10) + '" cy="' + (p.y + 42) + '" r="7" fill="' + kolor(T[st.typ].minus) + '" stroke="#13171F" stroke-width="1.5"/>';
      }
      if (st.smiglo) s += '<text x="' + (p.x + 40) + '" y="' + (p.y - 38) + '" font-size="13" font-weight="800" fill="#B5711C">śm. ' + st.smiglo + "</text>";
      if (pokazPodpisy) s += '<text x="' + p.x + '" y="' + (p.y + (p.y < 200 ? -64 : 78)) + '" text-anchor="middle" font-size="12" fill="#4A5060">' + p.n + "</text>";
    });
    return s + "</svg>";
  }
  window.dronZGory = dronZGory;

  // Montaż: gdzie wcisnąć silniki typu A i B (bez numerów – numery są w ćwiczeniu z orientacji)
  R.ukladTypow = function (box) {
    var stan = {};
    (K.silniki || []).forEach(function (m) { stan[m.pozycja] = { nr: typDlaKierunku(m.kierunek), typ: typDlaKierunku(m.kierunek), kier: m.kierunek }; });
    var T = K.typySilnikow || {};
    html(box, '<div style="max-width:420px;margin:0 auto">' + dronZGory(stan, true) + "</div>" +
      '<p class="podpis">Litera w środku to typ silnika. Kropki to kolory przewodów: ' + Object.keys(T).map(function (t) {
        return "typ " + t + " – " + T[t].plus + " i " + T[t].minus + " (" + T[t].kierunek + ")";
      }).join(", ") + ". Zielona strzałka pokazuje kierunek obrotu.</p>");
  };

  R.orientacja = function (box) {
    var T = K.typySilnikow || {};
    var poprawne = {};
    (K.silniki || []).forEach(function (m) {
      poprawne[m.pozycja] = { nr: m.nr, kier: m.kierunek, typ: typDlaKierunku(m.kierunek), smiglo: smigloDlaKierunku(m.kierunek) };
    });
    function opcje(lista) { return '<option value="">—</option>' + lista.map(function (o) { return '<option value="' + o[0] + '">' + o[1] + "</option>"; }).join(""); }
    var nrOpcje = opcje((K.silniki || []).map(function (m) { return [m.nr, m.nr]; }).sort());
    var typOpcje = opcje(Object.keys(T).map(function (t) { return [t, t + " (" + T[t].plus + "-" + T[t].minus + ")"]; }));
    var kierOpcje = opcje([["CW", "CW"], ["CCW", "CCW"]]);
    var smOpcje = opcje(Object.keys(K.smigla || {}).filter(function (k) { return k !== "opis"; }).map(function (k) { return [k, k]; }));
    var wiersze = Object.keys(POZ).map(function (k) {
      return '<tr data-poz="' + k + '"><th scope="row" style="background:#F5F4EF;color:#13171F">' + POZ[k].n + "</th>" +
        '<td><select data-pole="nr" aria-label="Numer – ' + POZ[k].n + '">' + nrOpcje + "</select></td>" +
        '<td><select data-pole="typ" aria-label="Typ – ' + POZ[k].n + '">' + typOpcje + "</select></td>" +
        '<td><select data-pole="kier" aria-label="Kierunek – ' + POZ[k].n + '">' + kierOpcje + "</select></td>" +
        '<td><select data-pole="smiglo" aria-label="Śmigło – ' + POZ[k].n + '">' + smOpcje + "</select></td></tr>";
    }).join("");
    html(box, '<div class="orientacja"><div class="or-svg"></div><div><div class="tabela-kontener"><table>' +
      "<thead><tr><th>Pozycja</th><th>Numer</th><th>Typ silnika</th><th>Kierunek</th><th>Śmigło</th></tr></thead><tbody>" + wiersze +
      '</tbody></table></div><div class="grupa-przyciskow"><button type="button" class="przycisk or-sprawdz">Sprawdź</button>' +
      '<button type="button" class="przycisk drugi or-czysc">Wyczyść</button></div><div class="or-wynik" aria-live="polite"></div></div></div>');
    function stan() {
      var s = {};
      box.querySelectorAll("tr[data-poz]").forEach(function (tr) {
        var o = {};
        tr.querySelectorAll("select").forEach(function (sel) { o[sel.getAttribute("data-pole")] = sel.value; });
        s[tr.getAttribute("data-poz")] = o;
      });
      return s;
    }
    function rysuj() { $(box, ".or-svg").innerHTML = dronZGory(stan(), true); }
    box.querySelectorAll("select").forEach(function (sel) {
      sel.onchange = function () { sel.parentNode.className = ""; rysuj(); };
    });
    $(box, ".or-sprawdz").onclick = function () {
      var s = stan(), bledne = 0, puste = 0;
      box.querySelectorAll("tr[data-poz]").forEach(function (tr) {
        var p = poprawne[tr.getAttribute("data-poz")] || {}, wiersz = s[tr.getAttribute("data-poz")];
        tr.querySelectorAll("select").forEach(function (sel) {
          var pole = sel.getAttribute("data-pole");
          if (!sel.value) { puste++; sel.parentNode.className = "zle"; return; }
          var ok = wiersz[pole] === p[pole];
          sel.parentNode.className = ok ? "dobrze" : "zle";
          if (!ok) bledne++;
        });
      });
      var w = $(box, ".or-wynik");
      if (!bledne && !puste) {
        w.className = "or-wynik komunikat dobrze";
        w.textContent = "Wszystko się zgadza! Po przekątnej ten sam kierunek, typ silnika i śmigło pasują do kierunku.";
      } else {
        w.className = "or-wynik komunikat zle";
        w.textContent = (puste ? "Uzupełnij puste pola. " : "") + (bledne ? "Pola na czerwono są błędne (" + bledne + "). " : "") +
          "Podpowiedź: typ A kręci się " + (T.A ? T.A.kierunek : "CW") + ", typ B – " + (T.B ? T.B.kierunek : "CCW") + ". Po przekątnej zawsze ten sam kierunek.";
      }
    };
    $(box, ".or-czysc").onclick = function () {
      box.querySelectorAll("select").forEach(function (sel) { sel.value = ""; sel.parentNode.className = ""; });
      var w = $(box, ".or-wynik"); w.className = "or-wynik"; w.textContent = ""; rysuj();
    };
    rysuj();
  };

  // =====================================================================
  // 10. Stożek widzenia czujnika odległości
  // =====================================================================
  R.stozek = function (box) {
    var kat = ((K.czujnikOdleglosci || {}).stozekStopnie || 25) / 2;
    var tg = Math.tan(kat * Math.PI / 180);
    html(box, '<svg viewBox="0 0 600 380" role="img" aria-label="Widok z boku: stożek widzenia czujnika odległości"></svg>' +
      '<div class="sterowanie">' + suwak("h", "Wysokość drona", 100, 800, 10, 400) + suwak("d", "Odległość prowadnicy od czujnika", 0, 200, 5, 120) + '</div>' +
      '<div class="komunikat" aria-live="polite"></div>');
    var sh = $(box, '[data-suwak="h"]'), sd = $(box, '[data-suwak="d"]'), svg = $(box, "svg");
    function rysuj() {
      var h = +sh.value, d = +sd.value, sk = 300 / 800, podloga = 350;
      var ys = podloga - h * sk, xs = 260;
      var pol = h * tg * sk;
      var xp = xs + d * sk + 30;   // prowadnica po prawej
      var wStozku = d < h * tg + 1e-9;
      var glebokosc = d / tg;
      var s = '<line x1="20" y1="' + podloga + '" x2="580" y2="' + podloga + '" stroke="#13171F" stroke-width="3"/>' +
        '<text x="24" y="' + (podloga + 22) + '" font-size="13" fill="#4A5060">podłoga</text>' +
        '<polygon points="' + xs + "," + ys + " " + (xs - pol) + "," + podloga + " " + (xs + pol) + "," + podloga + '" fill="#F2A33A" opacity=".28"/>' +
        '<line x1="' + xs + '" y1="' + ys + '" x2="' + (xs - pol) + '" y2="' + podloga + '" stroke="#F2A33A" stroke-width="2"/>' +
        '<line x1="' + xs + '" y1="' + ys + '" x2="' + (xs + pol) + '" y2="' + podloga + '" stroke="#F2A33A" stroke-width="2"/>' +
        '<text x="' + (xs - pol / 2 - 8) + '" y="' + (podloga - 8) + '" text-anchor="end" font-size="12" fill="#B5711C">stożek ok. ' + (kat * 2) + "°</text>" +
        '<rect x="' + (xp - 4) + '" y="20" width="8" height="' + (podloga - 20) + '" fill="' + (wStozku ? "#C7372B" : "#8A91A3") + '"/>' +
        '<text x="' + (xp + 10) + '" y="40" font-size="13" font-weight="700" fill="' + (wStozku ? "#C7372B" : "#4A5060") + '">prowadnica</text>' +
        // dron z boku
        '<rect x="' + (xs - 90) + '" y="' + (ys - 26) + '" width="180" height="18" rx="5" fill="#232937"/>' +
        '<rect x="' + (xs - 120) + '" y="' + (ys - 36) + '" width="60" height="5" rx="2" fill="#4A5060"/><rect x="' + (xs + 60) + '" y="' + (ys - 36) + '" width="60" height="5" rx="2" fill="#4A5060"/>' +
        '<line x1="' + (xs - 70) + '" y1="' + (ys - 8) + '" x2="' + (xs - 78) + '" y2="' + (ys + 6) + '" stroke="#232937" stroke-width="5"/>' +
        '<line x1="' + (xs + 70) + '" y1="' + (ys - 8) + '" x2="' + (xs + 78) + '" y2="' + (ys + 6) + '" stroke="#232937" stroke-width="5"/>' +
        '<rect x="' + (xs - 10) + '" y="' + (ys - 8) + '" width="20" height="8" fill="#2E6B4F"/>' +
        '<text x="' + (xs - 100) + '" y="' + (ys - 44) + '" font-size="12" fill="#4A5060">dron (widok z boku)</text>';
      if (wStozku) {
        var yt = ys + glebokosc * sk;
        s += '<circle cx="' + (xp - 4) + '" cy="' + yt + '" r="8" fill="none" stroke="#C7372B" stroke-width="3"/>';
      }
      svg.innerHTML = s;
      $(box, '[data-wart="h"]').textContent = h + " mm";
      $(box, '[data-wart="d"]').textContent = d + " mm";
      var k = $(box, ".komunikat");
      if (wStozku) {
        k.className = "komunikat zle";
        k.textContent = "Prowadnica wchodzi w stożek około " + Math.round(glebokosc) + " mm pod czujnikiem. Czujnik może zmierzyć odległość do prowadnicy zamiast do podłogi.";
      } else {
        k.className = "komunikat dobrze";
        k.textContent = "W stożku jest tylko podłoga. Na tej wysokości stożek ma u dołu szerokość ok. " + Math.round(2 * h * tg) + " mm.";
      }
    }
    sh.oninput = sd.oninput = rysuj;
    rysuj();
  };

  // =====================================================================
  // 11. Schemat I2C: dron → kabel → płytka stykowa → ESP32
  // =====================================================================
  var SYGNALY = ["3V3", "GND", "SDA", "SCL"];
  R.schematI2C = function (box) {
    var P = K.przewodyI2C || {}, piny = K.piny || { sda: 6, scl: 7 };
    var naMpu = { "3V3": "VCC", GND: "GND", SDA: "SDA", SCL: "SCL" };
    var naLaser = { "3V3": "VIN", GND: "GND", SDA: "SDA", SCL: "SCL" };
    var naEsp = { "3V3": "3V3", GND: "GND", SDA: "GPIO " + piny.sda, SCL: "GPIO " + piny.scl };
    var ys = [0, 1, 2, 3].map(function (i) { return 0; });
    var s = '<svg viewBox="0 0 800 380" role="img" aria-label="Schemat połączeń I2C">' +
      '<rect x="10" y="20" width="270" height="340" rx="16" fill="#F5F4EF" stroke="#13171F" stroke-width="2" stroke-dasharray="8 6"/>' +
      '<text x="24" y="44" font-size="15" font-weight="800" fill="#13171F">DRON</text>' +
      '<rect x="560" y="20" width="230" height="340" rx="16" fill="#F5F4EF" stroke="#13171F" stroke-width="2" stroke-dasharray="8 6"/>' +
      '<text x="574" y="44" font-size="15" font-weight="800" fill="#13171F">PLATFORMA</text>';
    // czujniki
    function modul(y, nazwa, adres, mapa, kol) {
      var g = '<rect x="30" y="' + y + '" width="120" height="130" rx="8" fill="' + kol + '" stroke="#13171F" stroke-width="2"/>' +
        '<text x="90" y="' + (y + 22) + '" text-anchor="middle" font-size="14" font-weight="800" fill="#FFFFFF">' + nazwa + "</text>" +
        '<text x="90" y="' + (y + 40) + '" text-anchor="middle" font-size="12" fill="#E9E7DF">adres ' + adres + "</text>";
      SYGNALY.forEach(function (sy, i) {
        var py = y + 58 + i * 20;
        g += '<text x="142" y="' + (py + 4) + '" text-anchor="end" font-size="11.5" font-weight="700" fill="#FFFFFF">' + mapa[sy] + "</text>" +
          '<circle cx="150" cy="' + py + '" r="4" fill="#E8B90C" stroke="#13171F"/>';
      });
      return g;
    }
    s += modul(60, (K.mpu || {}).model || "MPU6050", hex((K.mpu || {}).adres || 0x68), naMpu, "#1F4F8A");
    s += modul(210, (K.czujnikOdleglosci || {}).model || "VL53L0X", hex((K.czujnikOdleglosci || {}).adres || 0x29), naLaser, "#2E6B4F");
    // magistrala: 4 pionowe linie
    SYGNALY.forEach(function (sy, i) {
      var x = 190 + i * 18, kk = kolor(P[sy]);
      var yBus = 300 + i * 14;   // wyjście kabla
      s += '<g class="sch-przewod klikalne" data-sygnal="' + sy + '" tabindex="0" role="button" aria-label="Przewód ' + sy + '">';
      s += '<line x1="' + x + '" y1="' + (118 + i * 20) + '" x2="' + x + '" y2="' + yBus + '" stroke="' + kk + '" stroke-width="4"/>';
      s += '<line x1="150" y1="' + (118 + i * 20) + '" x2="' + x + '" y2="' + (118 + i * 20) + '" stroke="' + kk + '" stroke-width="4"/>';
      s += '<line x1="150" y1="' + (268 + i * 20) + '" x2="' + x + '" y2="' + (268 + i * 20) + '" stroke="' + kk + '" stroke-width="4"/>';
      s += '<circle cx="' + x + '" cy="' + (118 + i * 20) + '" r="5" fill="' + kk + '" stroke="#13171F"/>';
      s += '<circle cx="' + x + '" cy="' + (268 + i * 20) + '" r="5" fill="' + kk + '" stroke="#13171F"/>';
      // kabel ok. 1 m (lekko skręcony)
      var yk = 120 + i * 40;
      s += '<path d="M' + x + " " + yBus + " L" + x + " " + (yBus + 4) + " C " + (x + 60) + " " + (yBus + 4) + ", 300 " + yk + ", 330 " + yk +
        " C 360 " + (yk - 8) + ", 380 " + (yk + 8) + ", 410 " + yk + " C 440 " + (yk - 8) + ", 460 " + (yk + 8) + ", 490 " + yk + " L600 " + yk + '" fill="none" stroke="' + kk +
        '" stroke-width="5" stroke-linecap="round" style="filter:drop-shadow(0 0 1px #13171F)"/>';
      s += '<text x="545" y="' + (yk - 8) + '" text-anchor="middle" font-size="12" font-weight="700" fill="#13171F">' + sy + " · " + (P[sy] || "") + "</text>";
      s += '<rect x="600" y="' + (yk - 9) + '" width="22" height="18" rx="3" fill="#E9E7DF" stroke="#13171F"/>';
      s += '<line x1="622" y1="' + yk + '" x2="680" y2="' + yk + '" stroke="' + kk + '" stroke-width="4"/>';
      s += '<text x="760" y="' + (yk + 5) + '" text-anchor="end" font-size="13" font-weight="800" fill="#FFFFFF">' + naEsp[sy] + "</text>";
      s += "</g>";
    });
    s += '<text x="410" y="96" text-anchor="middle" font-size="13" fill="#4A5060">kabel ok. 1 m</text>' +
      '<text x="611" y="300" text-anchor="middle" font-size="11" fill="#4A5060">płytka</text><text x="611" y="313" text-anchor="middle" font-size="11" fill="#4A5060">stykowa</text>' +
      '<rect x="680" y="90" width="90" height="170" rx="8" fill="#13171F"/>' +
      '<text x="725" y="80" text-anchor="middle" font-size="14" font-weight="800" fill="#13171F">' + (K.plytka ? "ESP32-C6" : "ESP32") + "</text>" +
      '<text x="22" y="353" font-size="11.5" fill="#4A5060">wspólne linie = magistrala I2C</text>';
    // napisy ESP na wierzchu
    SYGNALY.forEach(function (sy, i) {
      s += '<text x="760" y="' + (120 + i * 40 + 5) + '" text-anchor="end" font-size="13" font-weight="800" fill="#FFFFFF">' + naEsp[sy] + "</text>";
    });
    s += "</svg>";
    html(box, '<div class="przewin">' + s + '</div><div class="wynik" aria-live="polite">Kliknij przewód, żeby go wyróżnić. Oba czujniki są podłączone do tych samych czterech linii.</div>');
    box.querySelectorAll(".sch-przewod").forEach(function (g) {
      function klik() {
        var sy = g.getAttribute("data-sygnal");
        box.querySelectorAll(".sch-przewod").forEach(function (x) { x.style.opacity = x === g ? "1" : "0.25"; });
        var opis = { "3V3": "zasilanie 3,3 V dla obu czujników", GND: "masa – wspólne „zero” dla wszystkich", SDA: "dane I2C", SCL: "zegar I2C" }[sy];
        $(box, ".wynik").innerHTML = "<b>" + sy + "</b> (" + (P[sy] || "") + "): " + opis + ". Na MPU6050 pin " + naMpu[sy] + ", na czujniku odległości pin " +
          naLaser[sy] + ", na ESP32 pin " + naEsp[sy] + ".";
      }
      g.addEventListener("click", klik);
      g.addEventListener("keydown", function (e) { if (e.key === "Enter" || e.key === " ") { e.preventDefault(); klik(); } });
    });
  };

  // Tabela połączeń z config.js
  R.tabelaI2C = function (box) {
    var P = K.przewodyI2C || {}, piny = K.piny || {};
    var w = [
      ["3V3", "VCC", "VIN", "3V3"], ["GND", "GND", "GND", "GND"],
      ["SDA", "SDA", "SDA", "GPIO " + piny.sda], ["SCL", "SCL", "SCL", "GPIO " + piny.scl]
    ];
    box.classList.add("tabela-kontener");
    box.innerHTML = "<table><thead><tr><th>Przewód</th><th>Kolor</th><th>Pin na MPU6050</th><th>Pin na " +
      ((K.czujnikOdleglosci || {}).model || "VL53L0X") + "</th><th>Pin na ESP32</th></tr></thead><tbody>" +
      w.map(function (r) {
        return "<tr><td><b>" + r[0] + "</b></td><td><span class=\"kolor-przewodu\" style=\"background:" + kolor(P[r[0]]) + "\"></span>" + (P[r[0]] || "") +
          "</td><td>" + r[1] + "</td><td>" + r[2] + "</td><td><b>" + r[3] + "</b></td></tr>";
      }).join("") + "</tbody></table>";
  };

  // =====================================================================
  // 12. Schemat zajęć 3: ESC, PWM, zasilanie
  // =====================================================================
  R.schematESC = function (box) {
    var piny = (K.piny || {}).silniki || [18, 19, 20, 21], sil = K.silniki || [];
    var s = '<svg viewBox="0 0 800 440" role="img" aria-label="Schemat podłączenia ESC">' +
      '<rect x="20" y="30" width="130" height="70" rx="10" fill="#FFFFFF" stroke="#13171F" stroke-width="2.5"/>' +
      '<text x="85" y="62" text-anchor="middle" font-size="15" font-weight="800">Zasilacz</text><text x="85" y="82" text-anchor="middle" font-size="13" fill="#4A5060">12 V</text>' +
      '<rect x="190" y="30" width="140" height="70" rx="10" fill="#FFFFFF" stroke="#13171F" stroke-width="2.5"/>' +
      '<text x="260" y="62" text-anchor="middle" font-size="15" font-weight="800">Przetwornica</text><text x="260" y="82" text-anchor="middle" font-size="13" fill="#4A5060">step-down</text>' +
      '<line x1="150" y1="55" x2="190" y2="55" stroke="#D7352A" stroke-width="5"/><line x1="150" y1="78" x2="190" y2="78" stroke="#222" stroke-width="5"/>' +
      // szyny zasilania ESC
      '<path d="M330 55 L380 55 L380 380" fill="none" stroke="#D7352A" stroke-width="5"/>' +
      '<path d="M330 78 L400 78 L400 400 L120 400 L120 330" fill="none" stroke="#222" stroke-width="5"/>' +
      '<text x="372" y="48" font-size="12" font-weight="700" fill="#D7352A">+ (silniki)</text>' +
      '<text x="250" y="420" text-anchor="middle" font-size="13" font-weight="800" fill="#13171F">wspólne GND</text>' +
      // ESP32
      '<rect x="40" y="150" width="160" height="180" rx="10" fill="#13171F"/>' +
      '<text x="120" y="176" text-anchor="middle" font-size="15" font-weight="800" fill="#F5F4EF">ESP32-C6</text>' +
      '<text x="120" y="320" text-anchor="middle" font-size="12" fill="#C9CEDA">GND</text>';
    for (var i = 0; i < 4; i++) {
      var y = 140 + i * 66, m = sil[i] || { nr: "M" + (i + 1), kierunek: "?" };
      s += '<text x="190" y="' + (205 + i * 30) + '" text-anchor="end" font-size="13" font-weight="700" fill="#F5F4EF">GPIO ' + piny[i] + "</text>" +
        '<path d="M200 ' + (200 + i * 30) + " L" + (300 - i * 8) + " " + (200 + i * 30) + " L" + (300 - i * 8) + " " + (y + 28) + " L460 " + (y + 28) +
        '" fill="none" stroke="#F2A33A" stroke-width="4"/>' +
        '<line x1="380" y1="' + (y + 10) + '" x2="460" y2="' + (y + 10) + '" stroke="#D7352A" stroke-width="3"/>' +
        '<line x1="400" y1="' + (y + 46) + '" x2="460" y2="' + (y + 46) + '" stroke="#222" stroke-width="3"/>' +
        '<circle cx="380" cy="' + (y + 10) + '" r="4.5" fill="#D7352A"/><circle cx="400" cy="' + (y + 46) + '" r="4.5" fill="#222"/>' +
        '<rect x="460" y="' + y + '" width="120" height="56" rx="8" fill="#FFFFFF" stroke="#13171F" stroke-width="2.5"/>' +
        '<text x="520" y="' + (y + 24) + '" text-anchor="middle" font-size="14" font-weight="800">ESC ' + (i + 1) + "</text>" +
        '<text x="520" y="' + (y + 43) + '" text-anchor="middle" font-size="11" fill="#4A5060">PWM · + · −</text>' +
        '<line x1="580" y1="' + (y + 20) + '" x2="660" y2="' + (y + 20) + '" stroke="#D7352A" stroke-width="3"/>' +
        '<line x1="580" y1="' + (y + 36) + '" x2="660" y2="' + (y + 36) + '" stroke="#1F5FC8" stroke-width="3"/>' +
        '<circle cx="690" cy="' + (y + 28) + '" r="26" fill="#E9E7DF" stroke="#13171F" stroke-width="2.5"/>' +
        '<text x="690" y="' + (y + 33) + '" text-anchor="middle" font-size="14" font-weight="800">' + m.nr + "</text>" +
        '<text x="724" y="' + (y + 33) + '" font-size="12" fill="#4A5060">' + m.kierunek + "</text>";
    }
    s += '<path d="M120 330 L120 330" stroke="#222"/>' +
      '<text x="285" y="128" font-size="12" font-weight="700" fill="#B5711C">sygnały PWM</text>' +
      '<text x="620" y="128" font-size="12" fill="#4A5060">+ do + silnika</text></svg>';
    html(box, '<div class="przewin">' + s + "</div>");
  };

  // Tabela: silnik → pin ESP32 (z config.js)
  R.tabelaESC = function (box) {
    var piny = (K.piny || {}).silniki || [], T = K.typySilnikow || {};
    var nazwy = { "przod-lewy": "przód-lewy", "przod-prawy": "przód-prawy", "tyl-prawy": "tył-prawy", "tyl-lewy": "tył-lewy" };
    box.classList.add("tabela-kontener");
    box.innerHTML = "<table><thead><tr><th>Silnik</th><th>Pozycja</th><th>Kierunek</th><th>Typ</th><th>Sygnał PWM z pinu</th></tr></thead><tbody>" +
      (K.silniki || []).map(function (m, i) {
        return "<tr><td><b>" + m.nr + "</b></td><td>" + (nazwy[m.pozycja] || m.pozycja) + "</td><td>" + m.kierunek + "</td><td>" + typDlaKierunku(m.kierunek) +
          "</td><td><b>GPIO " + piny[i] + "</b></td></tr>";
      }).join("") + "</tbody></table>";
  };

  // ---------- uruchomienie ----------
  window.dronRysunki = function () {
    document.querySelectorAll("[data-rysunek]").forEach(function (box) {
      var f = R[box.getAttribute("data-rysunek")];
      if (!f) return;
      if (!box.classList.contains("bez-ramki") && !/^tabela/.test(box.getAttribute("data-rysunek"))) box.classList.add("rysunek");
      try { f(box); } catch (e) {
        box.textContent = "Nie udało się narysować: " + e.message;
        if (window.console) console.error(e);
      }
    });
  };
})();
