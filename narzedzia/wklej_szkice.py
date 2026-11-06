#!/usr/bin/env python3
"""
Wkleja kod szkiców (szkice/uczen/*/*.ino) do pliku js/szkice.js,
z którego strona pokazuje kod. Dzięki temu strona działa po otwarciu z dysku
(bez fetch()), a kod na stronie jest identyczny z plikami .ino.

Użycie (z katalogu głównego repozytorium):
    python3 narzedzia/wklej_szkice.py            # zapisz js/szkice.js
    python3 narzedzia/wklej_szkice.py --sprawdz  # tylko sprawdź, nic nie zapisuj

--sprawdz kończy się błędem, gdy:
  - js/szkice.js jest nieaktualny (ktoś zmienił .ino i nie uruchomił skryptu),
  - strona odwołuje się do szkicu, którego nie ma,
  - piny w szkicach różnią się od js/config.js.
"""
import glob
import json
import os
import re
import sys

KATALOG = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WYJSCIE = os.path.join(KATALOG, "js", "szkice.js")


def czytaj(sciezka):
    with open(sciezka, encoding="utf-8", newline="") as f:
        return f.read()


def zbierz_szkice():
    szkice = {}
    for plik in sorted(glob.glob(os.path.join(KATALOG, "szkice", "uczen", "*", "*.ino"))):
        folder = os.path.basename(os.path.dirname(plik))
        szkice["uczen/" + folder] = czytaj(plik)
    return szkice


def tresc_js(szkice):
    return (
        "// PLIK GENEROWANY AUTOMATYCZNIE – nie edytuj ręcznie.\n"
        "// Źródło: szkice/uczen/*/*.ino. Po zmianie szkicu uruchom:\n"
        "//   python3 narzedzia/wklej_szkice.py\n"
        "window.SZKICE = " + json.dumps(szkice, ensure_ascii=False, indent=1) + ";\n"
    )


def piny_z_configu():
    cfg = czytaj(os.path.join(KATALOG, "js", "config.js"))
    sda = int(re.search(r"\bsda:\s*(\d+)", cfg).group(1))
    scl = int(re.search(r"\bscl:\s*(\d+)", cfg).group(1))
    silniki = [int(x) for x in re.search(r"\bsilniki:\s*\[([\d,\s]+)\]", cfg).group(1).split(",")]
    return sda, scl, silniki


def sprawdz_piny(bledy):
    sda, scl, silniki = piny_z_configu()
    for plik in sorted(glob.glob(os.path.join(KATALOG, "szkice", "*", "*", "*.ino"))):
        kod = czytaj(plik)
        nazwa = os.path.relpath(plik, KATALOG)
        m = re.search(r"const int PIN_SDA = (\d+);", kod)
        if m and int(m.group(1)) != sda:
            bledy.append(f"{nazwa}: PIN_SDA = {m.group(1)}, a w config.js sda = {sda}")
        m = re.search(r"const int PIN_SCL = (\d+);", kod)
        if m and int(m.group(1)) != scl:
            bledy.append(f"{nazwa}: PIN_SCL = {m.group(1)}, a w config.js scl = {scl}")
        m = re.search(r"const int PINY_SILNIKOW\[4\] = \{([\d,\s]+)\};", kod)
        if m and [int(x) for x in m.group(1).split(",")] != silniki:
            bledy.append(f"{nazwa}: PINY_SILNIKOW = {{{m.group(1)}}}, a w config.js silniki = {silniki}")


def sprawdz_odwolania(szkice, bledy):
    for html in glob.glob(os.path.join(KATALOG, "**", "*.html"), recursive=True):
        for nazwa in re.findall(r'data-szkic="([^"]+)"', czytaj(html)):
            if nazwa not in szkice:
                bledy.append(f"{os.path.relpath(html, KATALOG)}: brak szkicu {nazwa}")


def main():
    szkice = zbierz_szkice()
    nowy = tresc_js(szkice)
    bledy = []
    if "--sprawdz" in sys.argv:
        stary = czytaj(WYJSCIE) if os.path.exists(WYJSCIE) else ""
        if stary != nowy:
            bledy.append("js/szkice.js jest nieaktualny – uruchom: python3 narzedzia/wklej_szkice.py")
    else:
        with open(WYJSCIE, "w", encoding="utf-8", newline="") as f:
            f.write(nowy)
        print(f"Zapisano js/szkice.js ({len(szkice)} szkiców).")
    sprawdz_piny(bledy)
    sprawdz_odwolania(szkice, bledy)
    if bledy:
        print("BŁĘDY:")
        for b in bledy:
            print("  - " + b)
        sys.exit(1)
    print("OK: kod na stronie zgodny z plikami .ino, piny zgodne z config.js.")


if __name__ == "__main__":
    main()
