/*
 * Wspólny kod wszystkich stron poradnika.
 * Strona podaje w <body>: data-strona="z2-01" oraz data-root="../" (ścieżka do katalogu głównego).
 * Działa po otwarciu z dysku (file://) – bez fetch() i bez zewnętrznych bibliotek.
 */
(function () {
  "use strict";

  // ---------- struktura poradnika ----------
  var STRUKTURA = [
    { nr: 1, tytul: "Zajęcia 1: Start", data: "17.10", strony: [
      { id: "z1", plik: "zajecia-1.html", tytul: "Podsumowanie zajęć 1" }
    ] },
    { nr: 2, tytul: "Zajęcia 2: Z czego składa się dron?", data: "7.11", strony: [
      { id: "z2-01", plik: "zajecia-2/01-elektronika.html", tytul: "Wstęp do elektroniki" },
      { id: "z2-02", plik: "zajecia-2/02-bezpieczenstwo.html", tytul: "Bezpieczeństwo" },
      { id: "z2-03", plik: "zajecia-2/03-montaz.html", tytul: "Montaż drona" },
      { id: "z2-04", plik: "zajecia-2/04-silniki.html", tytul: "Orientacja silników" },
      { id: "z2-05", plik: "zajecia-2/05-czujniki.html", tytul: "Czujniki na dronie" },
      { id: "z2-06", plik: "zajecia-2/06-podlaczenie.html", tytul: "Podłączenie do platformy" },
      { id: "z2-07", plik: "zajecia-2/07-arduino.html", tytul: "Arduino od zera" },
      { id: "z2-08", plik: "zajecia-2/08-skaner.html", tytul: "I2C i skaner" },
      { id: "z2-09", plik: "zajecia-2/09-wysokosc.html", tytul: "Pomiar wysokości", dodatkowe: true },
      { id: "z2-10", plik: "zajecia-2/10-akcelerometr.html", tytul: "Akcelerometr", dodatkowe: true }
    ] },
    { nr: 3, tytul: "Zajęcia 3: Ruch silników", data: "21.11", strony: [
      { id: "z3-01", plik: "zajecia-3/01-podlaczenie-esc.html", tytul: "Podłączenie ESC" },
      { id: "z3-02", plik: "zajecia-3/02-pwm.html", tytul: "PWM i dioda" },
      { id: "z3-03", plik: "zajecia-3/03-jeden-silnik.html", tytul: "Jeden silnik" },
      { id: "z3-04", plik: "zajecia-3/04-cztery-silniki.html", tytul: "Cztery silniki" }
    ] },
    { nr: 4, tytul: "Zajęcia 4: Dron trzyma wysokość", data: "5.12", strony: [
      { id: "z4-01", plik: "zajecia-4/01-filtr.html", tytul: "Filtrowanie pomiarów" },
      { id: "z4-02", plik: "zajecia-4/02-regulator-p.html", tytul: "Regulator P" },
      { id: "z4-03", plik: "zajecia-4/03-pi-pid.html", tytul: "PI, PID i strojenie" }
    ] },
    { nr: 5, tytul: "Zajęcia 5: Misja i zawody", data: "19.12", strony: [
      { id: "z5-01", plik: "zajecia-5/01-misja.html", tytul: "Misja" },
      { id: "z5-02", plik: "zajecia-5/02-zawody.html", tytul: "Zawody i prezentacja" }
    ] },
    { nr: 0, tytul: "Pomoc", strony: [
      { id: "slowniczek", plik: "slowniczek.html", tytul: "Słowniczek" },
      { id: "problemy", plik: "problemy.html", tytul: "Coś nie działa?" }
    ] }
  ];
  window.STRUKTURA = STRUKTURA;

  var body = document.body;
  var ROOT = body.getAttribute("data-root") || "";
  var STRONA = body.getAttribute("data-strona") || "";
  var K = window.KONFIG || {};

  // ---------- pamięć (localStorage może nie działać) ----------
  var KLUCZ = "dron-ud-postep-v1";
  var pamiec = {};
  try { pamiec = JSON.parse(window.localStorage.getItem(KLUCZ) || "{}") || {}; } catch (e) { pamiec = {}; }
  function zapisz() {
    try { window.localStorage.setItem(KLUCZ, JSON.stringify(pamiec)); } catch (e) { /* bez pamięci też działa */ }
  }
  window.dronPamiec = { dane: function () { return pamiec; }, zapisz: zapisz };

  function el(tag, klasa, tekst) {
    var e = document.createElement(tag);
    if (klasa) e.className = klasa;
    if (tekst != null) e.textContent = tekst;
    return e;
  }

  function znajdzStrone(id) {
    for (var i = 0; i < STRUKTURA.length; i++)
      for (var j = 0; j < STRUKTURA[i].strony.length; j++)
        if (STRUKTURA[i].strony[j].id === id) return { grupa: STRUKTURA[i], strona: STRUKTURA[i].strony[j] };
    return null;
  }

  function wszystkieStrony() {
    var w = [];
    STRUKTURA.forEach(function (g) { if (g.nr > 0) g.strony.forEach(function (s) { w.push(s); }); });
    return w;
  }

  function stanStrony(id) {
    var s = pamiec[id];
    if (!s || !s.wszystkie) return { zrobione: 0, wszystkie: 0 };
    var n = 0;
    for (var k in s.kroki) if (s.kroki[k]) n++;
    return { zrobione: n, wszystkie: s.wszystkie };
  }
  window.dronStanStrony = stanStrony;

  // ---------- wartości z config.js w tekście ----------
  function wartosc(sciezka) {
    var cz = sciezka.split("."), v = K;
    for (var i = 0; i < cz.length; i++) { if (v == null) return null; v = v[cz[i]]; }
    return v;
  }
  function hex(n) { return "0x" + ("0" + n.toString(16).toUpperCase()).slice(-2); }
  function wstawKonfig() {
    // Ramka „do uzupełnienia”: gdy prowadzący wpisze tekst w config.js, ramka zamienia się w zwykły opis.
    document.querySelectorAll("[data-k-ramka]").forEach(function (e) {
      var v = wartosc(e.getAttribute("data-k-ramka"));
      if (!v) return;
      e.className = "ramka info";
      e.textContent = "";
      e.appendChild(el("span", "tytul", e.getAttribute("data-tytul") || "Od prowadzącego"));
      e.appendChild(el("p", null, v));
    });
    document.querySelectorAll("[data-k]").forEach(function (e) {
      var v = wartosc(e.getAttribute("data-k"));
      if (v == null || v === "") return;
      if (e.getAttribute("data-format") === "hex") v = hex(v);
      if (Array.isArray(v)) v = v.join(", ");
      e.textContent = v;
      if (e.hasAttribute("data-kolor")) {
        var kropka = el("span", "kolor-przewodu");
        kropka.style.background = (K.kolory || {})[v] || v;
        e.insertBefore(kropka, e.firstChild);
      }
    });
  }

  // ---------- nagłówek, menu, pasek postępu ----------
  function zbudujRame() {
    var tresc = document.querySelector("main.tresc");
    if (!tresc) return;

    var nag = el("header", "naglowek");
    var logo = el("a", "logo");
    logo.href = ROOT + "index.html";
    var img = new Image();
    img.alt = "Uniwersytet Dzieci";
    img.src = ROOT + "img/logo-ud.png";
    var zaslepkaLogo = el("span", "zaslepka-logo", "logo UD");
    img.onerror = function () { if (img.parentNode) img.parentNode.replaceChild(zaslepkaLogo, img); };
    logo.appendChild(img);
    var napis = el("span", null, "Inżynieria dronowa");
    napis.appendChild(el("small", null, "Uniwersytet Dzieci · AGH Drone Engineering"));
    logo.appendChild(napis);
    nag.appendChild(logo);
    var przycisk = el("button", "przycisk-menu", "Menu");
    przycisk.type = "button";
    przycisk.setAttribute("aria-expanded", "false");
    przycisk.onclick = function () {
      var o = body.classList.toggle("menu-otwarte");
      przycisk.setAttribute("aria-expanded", o ? "true" : "false");
      przycisk.textContent = o ? "Zamknij" : "Menu";
    };
    nag.appendChild(przycisk);

    var pasek = el("div", "postep");
    pasek.setAttribute("role", "progressbar");
    pasek.setAttribute("aria-label", "Postęp na tej stronie");
    var tor = el("div", "tor");
    var wyp = el("div", "wypelnienie");
    tor.appendChild(wyp);
    var opis = el("span", "opis", "");
    pasek.appendChild(opis);
    pasek.appendChild(tor);

    var uklad = el("div", "uklad");
    var menu = el("nav", "menu-boczne");
    menu.setAttribute("aria-label", "Rozdziały");
    STRUKTURA.forEach(function (g) {
      var h = el("h4", null, g.tytul);
      if (g.data) { h.appendChild(document.createTextNode(" ")); h.appendChild(el("span", "data", "· " + g.data)); }
      menu.appendChild(h);
      var ul = el("ul");
      g.strony.forEach(function (s) {
        var li = el("li");
        var a = el("a");
        a.href = ROOT + s.plik;
        if (s.id === STRONA) { a.className = "aktywny"; a.setAttribute("aria-current", "page"); }
        var zn = el("span", "znacznik", "✓");
        zn.setAttribute("aria-hidden", "true");
        zn.setAttribute("data-znacznik", s.id);
        a.appendChild(zn);
        a.appendChild(document.createTextNode(s.tytul + (s.dodatkowe ? " (dla chętnych)" : "")));
        li.appendChild(a);
        ul.appendChild(li);
      });
      menu.appendChild(ul);
    });

    tresc.parentNode.insertBefore(nag, tresc);
    tresc.parentNode.insertBefore(pasek, tresc);
    tresc.parentNode.insertBefore(uklad, tresc);
    uklad.appendChild(menu);
    uklad.appendChild(tresc);

    // wstecz / dalej
    var lista = wszystkieStrony();
    var i = -1;
    for (var n = 0; n < lista.length; n++) if (lista[n].id === STRONA) i = n;
    if (i >= 0) {
      var dw = el("nav", "dalej-wstecz");
      dw.setAttribute("aria-label", "Poprzednia i następna strona");
      if (i > 0) dw.appendChild(linkNaw(lista[i - 1], "Wstecz", "wstecz"));
      else dw.appendChild(el("span", "pusty"));
      if (i < lista.length - 1) dw.appendChild(linkNaw(lista[i + 1], "Dalej", "dalej"));
      else dw.appendChild(el("span", "pusty"));
      tresc.appendChild(dw);
    }
    var st = el("footer", "stopka");
    st.innerHTML = 'Kurs „Inżynieria dronowa” · Uniwersytet Dzieci · AGH Drone Engineering · <a href="' + ROOT +
      'slowniczek.html">Słowniczek</a> · <a href="' + ROOT + 'problemy.html">Coś nie działa?</a>';
    tresc.appendChild(st);

    return { wyp: wyp, opis: opis, pasek: pasek };
  }

  function linkNaw(s, napis, klasa) {
    var a = el("a", klasa);
    a.href = ROOT + s.plik;
    a.appendChild(el("small", null, napis));
    a.appendChild(el("strong", null, s.tytul));
    return a;
  }

  function odswiezZnaczniki() {
    document.querySelectorAll("[data-znacznik]").forEach(function (zn) {
      var s = stanStrony(zn.getAttribute("data-znacznik"));
      zn.classList.remove("gotowe", "czesciowo");
      if (s.wszystkie && s.zrobione >= s.wszystkie) zn.classList.add("gotowe");
      else if (s.zrobione > 0) zn.classList.add("czesciowo");
    });
  }

  // ---------- karty kroków ----------
  function obsluzKroki(rama) {
    var kroki = document.querySelectorAll("main .krok");
    if (!STRONA || !kroki.length) {
      if (rama) rama.pasek.style.display = "none";
      return;
    }
    if (!pamiec[STRONA]) pamiec[STRONA] = { kroki: {}, wszystkie: 0 };
    var stan = pamiec[STRONA];
    stan.wszystkie = kroki.length;

    function odswiez() {
      var n = 0;
      kroki.forEach(function (k) { if (k.classList.contains("zrobiony")) n++; });
      if (rama) {
        var proc = Math.round(100 * n / kroki.length);
        rama.wyp.style.width = proc + "%";
        rama.opis.textContent = "Ta strona: " + n + " z " + kroki.length;
        rama.pasek.setAttribute("aria-valuenow", String(proc));
      }
      odswiezZnaczniki();
    }

    kroki.forEach(function (k, idx) {
      var id = k.id || ("krok-" + (idx + 1));
      var napis = k.getAttribute("data-odhacz") || (k.classList.contains("kontrolny") ? "Prowadzący sprawdził" : "Zrobione");
      var lab = el("label", "odhacz");
      var cb = document.createElement("input");
      cb.type = "checkbox";
      lab.appendChild(cb);
      lab.appendChild(document.createTextNode(napis));
      k.appendChild(lab);
      if (stan.kroki[id]) { cb.checked = true; k.classList.add("zrobiony"); }
      cb.addEventListener("change", function () {
        stan.kroki[id] = cb.checked;
        k.classList.toggle("zrobiony", cb.checked);
        zapisz();
        odswiez();
      });
    });
    zapisz();
    odswiez();
  }

  // ---------- listy kontrolne (zapamiętywane, ale nie liczą się do postępu) ----------
  function obsluzListy() {
    document.querySelectorAll("ul.lista-kontrolna").forEach(function (ul, u) {
      ul.querySelectorAll("li").forEach(function (li, i) {
        var id = (ul.id || "lista" + u) + "-" + i;
        var lab = el("label");
        var cb = document.createElement("input");
        cb.type = "checkbox";
        var tekst = el("span");
        while (li.firstChild) tekst.appendChild(li.firstChild);
        lab.appendChild(cb);
        lab.appendChild(tekst);
        li.appendChild(lab);
        var klucz = STRONA + ":listy";
        if (!pamiec[klucz]) pamiec[klucz] = {};
        cb.checked = !!pamiec[klucz][id];
        cb.addEventListener("change", function () { pamiec[klucz][id] = cb.checked; zapisz(); });
      });
    });
  }

  // ---------- Sprawdź się ----------
  function obsluzQuizy() {
    var Q = window.QUIZY || {};
    document.querySelectorAll("[data-quiz]").forEach(function (box) {
      var q = Q[box.getAttribute("data-quiz")];
      if (!q) return;
      box.classList.add("sprawdz");
      box.appendChild(el("div", "etykieta", "Sprawdź się"));
      box.appendChild(el("div", "pytanie", q.pytanie));
      var odp = el("div", "odpowiedzi");
      var wyj = el("div", "wyjasnienie");
      wyj.setAttribute("aria-live", "polite");
      q.odpowiedzi.forEach(function (t, i) {
        var b = el("button", null, t);
        b.type = "button";
        b.onclick = function () {
          odp.querySelectorAll("button").forEach(function (x) { x.classList.remove("dobrze", "zle"); });
          if (i === q.poprawna) {
            b.classList.add("dobrze");
            wyj.textContent = "Dobrze! " + q.wyjasnienie;
          } else {
            b.classList.add("zle");
            wyj.textContent = "Jeszcze nie. " + (q.podpowiedz || "Przeczytaj sekcję jeszcze raz i spróbuj ponownie.");
          }
          wyj.classList.add("widoczne");
        };
        odp.appendChild(b);
      });
      box.appendChild(odp);
      box.appendChild(wyj);
    });
  }

  // ---------- kod: podświetlanie składni ----------
  var SLOWA = /^(if|else|for|while|do|return|break|continue|switch|case|default|const|static|static_assert|true|false|sizeof|void|unsigned|signed)$/;
  var TYPY = /^(int|long|float|double|bool|char|byte|short|String|uint8_t|uint16_t|uint32_t|int8_t|int16_t|int32_t|size_t)$/;
  var WZOR = /(\/\*[\s\S]*?\*\/|\/\/[^\n]*)|("(?:\\.|[^"\\\n])*"|'(?:\\.|[^'\\\n])')|(^[ \t]*#[^\n]*)|(\b0x[0-9A-Fa-f]+\b|\b\d+(?:\.\d+)?[fFuUlL]*\b)|([A-Za-z_]\w*)/gm;

  function esc(t) { return t.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;"); }

  function podswietl(kod) {
    var wynik = "", ost = 0, m;
    WZOR.lastIndex = 0;
    while ((m = WZOR.exec(kod)) !== null) {
      wynik += esc(kod.slice(ost, m.index));
      var t = m[0], klasa = null;
      if (m[1]) klasa = "k-kom";
      else if (m[2]) klasa = "k-str";
      else if (m[3]) klasa = "k-pre";
      else if (m[4]) klasa = "k-num";
      else if (m[5]) {
        if (SLOWA.test(t)) klasa = "k-slowo";
        else if (TYPY.test(t)) klasa = "k-typ";
        else if (/^[A-Z][A-Z0-9_]+$/.test(t)) klasa = "k-stala";
        else if (kod.charAt(WZOR.lastIndex) === "(") klasa = "k-fun";
      }
      wynik += klasa ? '<span class="' + klasa + '">' + esc(t) + "</span>" : esc(t);
      ost = WZOR.lastIndex;
    }
    return wynik + esc(kod.slice(ost));
  }
  window.dronPodswietl = podswietl;

  function kopiuj(tekst, przycisk, pre) {
    function ok() {
      przycisk.textContent = "Skopiowano";
      przycisk.classList.add("skopiowano");
      setTimeout(function () { przycisk.textContent = "Kopiuj"; przycisk.classList.remove("skopiowano"); }, 1800);
    }
    function zapas() {
      var ta = document.createElement("textarea");
      ta.value = tekst;
      ta.setAttribute("readonly", "");
      ta.style.position = "fixed";
      ta.style.opacity = "0";
      document.body.appendChild(ta);
      ta.select();
      var udalo = false;
      try { udalo = document.execCommand("copy"); } catch (e) { udalo = false; }
      document.body.removeChild(ta);
      if (udalo) { ok(); return; }
      // Ostatnia możliwość: zaznacz kod, żeby wystarczyło nacisnąć Ctrl+C.
      if (pre && window.getSelection) {
        var r = document.createRange();
        r.selectNodeContents(pre);
        var sel = window.getSelection();
        sel.removeAllRanges();
        sel.addRange(r);
      }
      przycisk.textContent = "Zaznaczono – naciśnij Ctrl+C";
    }
    if (navigator.clipboard && navigator.clipboard.writeText) {
      navigator.clipboard.writeText(tekst).then(ok, zapas);
    } else zapas();
  }

  function obsluzKod() {
    var S = window.SZKICE || {};
    document.querySelectorAll("[data-szkic]").forEach(function (box) {
      var nazwa = box.getAttribute("data-szkic");      // np. "uczen/L01_pierwszy_program"
      var kod = S[nazwa];
      box.classList.add("kod");
      if (kod == null) {
        box.textContent = "Brak kodu " + nazwa + ". Uruchom: python3 narzedzia/wklej_szkice.py";
        return;
      }
      var folder = nazwa.split("/")[1];
      var plik = "szkice/" + nazwa + "/" + folder + ".ino";
      var pre = el("pre");
      var pasek = el("div", "kod-pasek");
      pasek.appendChild(el("span", "nazwa", folder + ".ino"));
      var b = el("button", null, "Kopiuj");
      b.type = "button";
      b.onclick = function () { kopiuj(kod, b, pre); };
      pasek.appendChild(b);
      var a = el("a", null, "Pobierz plik");
      a.href = ROOT + plik;
      a.setAttribute("download", folder + ".ino");
      pasek.appendChild(a);
      box.appendChild(pasek);
      var code = el("code");
      code.innerHTML = podswietl(kod);
      pre.appendChild(code);
      pre.tabIndex = 0;
      box.appendChild(pre);
    });
  }

  // ---------- słowniczek: dymek przy pierwszym wystąpieniu ----------
  var dymek = null;
  function pokazDymek(cel) {
    if (!dymek) { dymek = el("div", "dymek"); dymek.setAttribute("role", "tooltip"); dymek.id = "dymek"; body.appendChild(dymek); }
    dymek.innerHTML = "<strong>" + esc(cel.getAttribute("data-haslo")) + "</strong> – " + esc(cel.getAttribute("data-opis"));
    dymek.style.display = "block";
    var r = cel.getBoundingClientRect();
    var w = Math.min(300, window.innerWidth - 24);
    dymek.style.maxWidth = w + "px";
    var x = r.left + window.scrollX;
    x = Math.max(12 + window.scrollX, Math.min(x, window.scrollX + window.innerWidth - dymek.offsetWidth - 12));
    var y = r.bottom + window.scrollY + 8;
    if (r.bottom + dymek.offsetHeight + 16 > window.innerHeight) y = r.top + window.scrollY - dymek.offsetHeight - 8;
    dymek.style.left = x + "px";
    dymek.style.top = y + "px";
    cel.setAttribute("aria-describedby", "dymek");
  }
  function ukryjDymek() { if (dymek) dymek.style.display = "none"; }

  function obsluzSlowniczek() {
    var lista = window.SLOWNICZEK;
    var main = document.querySelector("main.tresc");
    if (!lista || !main || body.hasAttribute("data-bez-dymkow")) return;
    var POMIN = /^(PRE|CODE|SCRIPT|STYLE|H1|H2|A|BUTTON|SVG|TEXT|TSPAN|LABEL|SELECT|OPTION|KBD|TEXTAREA)$/;
    function pominiety(n) {
      for (var p = n.parentNode; p && p !== main; p = p.parentNode) {
        if (POMIN.test(p.nodeName.toUpperCase())) return true;
        if (p.classList && (p.classList.contains("pojecie") || p.classList.contains("sprawdz") ||
            p.classList.contains("kod") || p.classList.contains("rysunek") || p.classList.contains("bez-dymkow"))) return true;
      }
      return false;
    }
    lista.forEach(function (h) {
      var formy = h.formy.slice().sort(function (a, b) { return b.length - a.length; })
        .map(function (f) { return f.replace(/[.*+?^${}()|[\]\\]/g, "\\$&"); });
      var re;
      try { re = new RegExp("(?<![\\p{L}\\p{N}])(" + formy.join("|") + ")(?![\\p{L}\\p{N}])", h.wielkie ? "u" : "iu"); }
      catch (e) { return; }
      var w = document.createTreeWalker(main, NodeFilter.SHOW_TEXT, null);
      var n;
      while ((n = w.nextNode())) {
        if (pominiety(n)) continue;
        var m = re.exec(n.nodeValue);
        if (!m) continue;
        var reszta = n.splitText(m.index);
        reszta.splitText(m[1].length);
        var s = el("span", "pojecie");
        s.tabIndex = 0;
        s.setAttribute("data-haslo", h.haslo);
        s.setAttribute("data-opis", h.opis);
        reszta.parentNode.replaceChild(s, reszta);
        s.appendChild(reszta);
        break;
      }
    });
    main.addEventListener("mouseover", function (e) { if (e.target.classList && e.target.classList.contains("pojecie")) pokazDymek(e.target); });
    main.addEventListener("mouseout", function (e) { if (e.target.classList && e.target.classList.contains("pojecie")) ukryjDymek(); });
    main.addEventListener("focusin", function (e) { if (e.target.classList && e.target.classList.contains("pojecie")) pokazDymek(e.target); });
    main.addEventListener("focusout", ukryjDymek);
    main.addEventListener("click", function (e) { if (e.target.classList && e.target.classList.contains("pojecie")) pokazDymek(e.target); });
    window.addEventListener("scroll", ukryjDymek, { passive: true });
    document.addEventListener("keydown", function (e) { if (e.key === "Escape") ukryjDymek(); });
  }

  // ---------- zdjęcia: zaślepka, dopóki nie ma pliku ----------
  function obsluzZdjecia() {
    document.querySelectorAll("figure.zdjecie[data-plik]").forEach(function (f) {
      var plik = f.getAttribute("data-plik");
      var opis = f.getAttribute("data-opis") || "";
      var zas = el("div", "zaslepka");
      zas.appendChild(el("strong", null, "Tu będzie zdjęcie"));
      zas.appendChild(el("span", null, opis));
      zas.appendChild(el("span", "plik", plik));
      f.insertBefore(zas, f.firstChild);
      var img = new Image();
      img.alt = opis;
      img.loading = "lazy";
      img.onload = function () { f.replaceChild(img, zas); };
      img.src = ROOT + plik;
    });
  }

  // ---------- strona startowa: postęp zajęć ----------
  function obsluzPlan() {
    // Karty zajęć: data-dzien="2026-11-07". Odbyte – zielone, najbliższe – pomarańczowe.
    var dzis = new Date(); dzis.setHours(0, 0, 0, 0);
    var nastepneJest = false;
    document.querySelectorAll("[data-dzien]").forEach(function (k) {
      var d = new Date(k.getAttribute("data-dzien") + "T00:00:00");
      var et = k.querySelector(".etykieta");
      if (d < dzis) { k.classList.add("odbyte"); if (et) et.textContent = "odbyte"; }
      else if (!nastepneJest) { nastepneJest = true; k.classList.add("nastepne"); if (et) et.textContent = d.getTime() === dzis.getTime() ? "dzisiaj" : "najbliższe"; }
      else if (et) et.parentNode.removeChild(et);
    });
    document.querySelectorAll("[data-postep-zajec]").forEach(function (box) {
      var nr = +box.getAttribute("data-postep-zajec");
      var g = STRUKTURA.filter(function (x) { return x.nr === nr; })[0];
      if (!g) return;
      var z = 0, w = 0;
      g.strony.forEach(function (s) { var st = stanStrony(s.id); z += st.zrobione; w += st.wszystkie; });
      if (!w) return;
      box.querySelector("div").style.width = Math.round(100 * z / w) + "%";
      box.title = "Odhaczone kroki: " + z + " z " + w + " (na odwiedzonych stronach)";
    });
  }

  document.addEventListener("DOMContentLoaded", function () {
    wstawKonfig();
    var rama = zbudujRame();
    obsluzKroki(rama);
    obsluzListy();
    obsluzQuizy();
    obsluzKod();
    obsluzZdjecia();
    obsluzPlan();
    odswiezZnaczniki();
    if (window.dronRysunki) window.dronRysunki();   // interaktywne rysunki (js/rysunki.js)
    obsluzSlowniczek();
  });
})();
