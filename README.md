# Gradovi Srbije
Jednostavna igra u kojoj igrač treba da pronađe lokaciju grada na mapi (selekcija) ili da na osnovu odabrane lokacije na mapi unese ime grada (unos).

![Demo of the game](demo.gif)

# Instalacija

1. [Linux](#linux)
2. [Windows](#windows-10)

## Linux

### Ubuntu Mate 18.04.1, Ubuntu Mate 20.04, Solus OS 4.1

**Obavezno** (razvojne datoteke za kompajliranje):
```bash
# ubuntu
sudo apt install build-essential # libc, gcc, make itd...
sudo apt install libgtk-3-dev # gtk3 dev, glib2 dev, pkg-config itd...
sudo apt install libjson-glib-dev # glib json dev biblioteka

# solus os
sudo eopkg install -c system.devel
sudo eopkg install libgtk-3-devel
sudo eopkg install libjson-glib-devel
```
*Opcionalno*:

```bash
# ubuntu
sudo apt install libxml2-utils # koristi se u .gresource.xml datoteci (xmllint)

# solus os
sudo eopkg install libxml2
```

```bash
# ubuntu
sudo apt install fonts-noto # Noto font

# solus os
sudo eopkg install noto-sans-ttf
```
ili

```bash
# ubuntu
sudo apt install fonts-ubuntu # Ubuntu font

# solus os
sudo eopkg install font-ubuntu-ttf
```
ili
```bash
# ubuntu
sudo apt install fonts-liberation2 # Liberation font

# solus os
sudo eopkg install liberation-fonts-ttf
```

Ukoliko ste instalirali neki od navedenih fontova, izvršite sledeću komandu:
```bash
sudo fc-cache -fv # obnovite keš fonta
```

Posle toga klonirate ovaj git repozitorijum i pokrenete skriptu za instaliranje:
```bash
git clone https://github.com/dragi-ns/gradovi_srbije.git
cd gradovi_srbije/
sudo ./install-linux.sh
```

Nakon toga, ukoliko je sve prošlo kako treba, možete da pokrenete program sledećom komandom:

```bash
gradovi-srbije
```

ili iz sistemskog menija.

Kada budete hteli da deinstalirate ovaj program, pokrenite skriptu za deinstaliranje sledećom komandom:
```bash
sudo ./uninstall-linux.sh
```

## Windows 10
Preuzmite instalaciju za Windows 10 (**PREPORUČENO**):

1. 32bit: [gradovi-srbije-32-instalacija.exe](https://github.com/dragi-ns/gradovi-srbije/releases/download/1.0.1/gradovi-srbije-32-instalacija.exe)
2. 64bit: [gradovi-srbije-64-instalacija.exe](https://github.com/dragi-ns/gradovi-srbije/releases/download/1.0.1/gradovi-srbije-64-instalacija.exe)

Ukoliko želite da sami da kompajlirate iz izvora (**NIJE PREPORUČENO**) pratite sledeće upustvo:

1. Pratite korake **1**, **2** i **5** sa [Using GTK from MSYS2 packages ](https://www.gtk.org/docs/installations/windows/#using-gtk-from-msys2-packages)
2. Kada ste uspešno instalirali MSYS2 i razvojne datoteke za kompajliranje, pratite **sve** navedene korake sa [Building and distributing your application ](https://www.gtk.org/docs/installations/windows/#building-and-distributing-your-application)
3. Kada ste uspešno preuzeli potrebne resurse i postavili ih na odgovarajuća mesta, otvorite MSYS2 terminal.
4. Sada je potrebno da preuzmete git koristeći sledeću komandu:
```bash
pacman -S git
```
5. Nakon toga možete da klonirate ovaj git repozitorijum i kompajlirate:
```bash
git clone https://github.com/dragi-ns/gradovi-srbije.git
cd gradovi-srbije
make WINDOWS=1
```
Nakon toga, ukoliko je sve prošlo kako treba, možete da pokrenete program sledećom komandom:

```bash
./gradovi-srbije.exe
```
