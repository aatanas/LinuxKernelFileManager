# LinuxKernelFileManager

![Preview failed to load!](https://github.com/aatanas/LinuxKernelFileManager/blob/main/preview.png?raw=true)

Cilj ovog projekta je obezbediti dva alata za Linux - file explorer i clipboard.
Napisan je u C i assembly unutar Linux kernela. Alat funkcioniše i dok je
pokrenuta neka korisnička aplikacija. Podržava sledeće funkcionalnosti:

- U jednom trenutku je prikazan jedan od dva alata. Taster F2 bira koji alat je prikazan.
Alat se prikazuje u gornjem desnom uglu ekrana.
- Explorer alat omogućava obilazak fajlova na disku i kopiranje putanje do odabranog
fajla u terminal.
- Clipboard alat omogućava zabeležavanje stringova, i njihovo kopiranje u terminal.

Snimak sa primerom korišćenja svih funkcionalnosti je dat na materijalima.


#### Prikaz alata
Oba alata će biti prikazana u gornjem desnom uglu ekrana i nalaze se u
pravougaoniku sačinjenom od ‘#’ karaktera. Dimenzija ovog “prozora” je 20x10
karaktera korisnog prostora, tj. 22x12 računajući i okvir.
Oba alata prikazuju tekst poravnat centralno.
Kod explorer alata koriste se različite boje za različite tipove datoteka na disku:

- Obične datoteke - standardna siva boja
- Direktorijumi - svetlo plava
- Izvršne datoteke - zelena
- Karakter uređaji - žuta

Kod explorer alata se prikazuje prvih 10 stavki unutar direktorijuma. 
U oba alata trenutno selektovana stavka je markirana izmenjenom bojom
pozadine.
Vrh “prozora” prikazuje trenutnu putanju kod explorer alata, i tekst “clipboard” kod
clipboard alata.
Na početku rada sistema nije prikazan nijedan alat. Po prvom pritisku na F2 taster se
prikazuje explorer alat, postavljen u koreni (“/”) direktorijum. Promena alata se obavlja
pritiskom na F2 taster.

#### Rad sa explorerom
Komande za explorer se unose preko tastature, i to su:
|R. br.| Taster| Ponašanje|
|--- |---|---|
|1. |F1 |Aktivira strelice i space|
|2. |F2 |Menja trenutno aktivni alat sa explorer na clipboard|
|3. |Strelica gore |Prethodna datoteka (radi samo uz pritisnuto F1)|
|4. |Strelica dole |Naredna datoteka (radi samo uz pritisnuto F1)|
|5. |Strelica levo |Parent direktorijum (radi samo uz pritisnuto F1)|
|6. |Strelica desno |Uđi u direktorijum (radi samo uz pritisnuto F1)|
|7. |Space |Kopira putanju do selektovane stavke u terminal (radi samo uz pritisnuto F1)|

Strelice se koriste za obilaženje fajl sistema, a space se koristi za kopiranje putanje do
selektovane datoteke u terminal, s time da funkcionišu na ovaj način samo dok se
drži F1. U slučaju da je F1 pritisnuto, strelice i space obavljaju samo ovu funkciju, ne
i svoju normalnu funkciju. U slučaju da F1 nije pritisnuto, strelice i space se
ponašaju normalno. Ako smo na poslednjoj stavci, strelica dole nas vodi na prvu stavku, i
ako smo na prvoj stavci, strelica gore nas vodi na poslednju stavku.

F tasteri ne obavljaju svoju normalnu operaciju ispisivanja slova A / B / C na
terminalu.

Strelica desno radi samo ako je selektovan direktorijum, i u tom slučaju, to postaje
novi aktivni direktorijum, i njegova sadržina se ispisuje u exploreru. Ako je selektovana neka
datoteka, ispisuje se greška.

Strelica levo nas vraća u roditeljski direktorijum (“..”). Ako se nalazimo u korenom
direktorijumu, strelica levo nema nikakvog efekta. Kod obe ove strelice se čisti explorer blok 
pre ispisivanja sadržaja novog direktorijuma.

Space kopira punu putanju do trenutno selektovane datoteke / direktorijuma u
terminal. Funkcioniše i kada unosimo komande za operativni sistem (shell) i
kada se nalazimo u korisničkoj aplikaciji.

#### Rad sa clipboardom
Komande za clipboard se unose preko tastature, i to su:
|R. br.| Taster| Ponašanje|
|---|---|---|
|1. |F1 |Aktivira strelice i space|
|2. |F2 |Menja trenutno aktivni alat sa clipboard na explorer|
|3. |F3 |Započinje / završava upis nove stavke u clipboard|
|4. |Strelica gore |Prethodna stavka (radi samo uz pritisnuto F1)|
|5. |Strelica dole |Naredna stavka (radi samo uz pritisnuto F1)|
|6. |Space |Kopira trenutnu stavku u terminal (radi samo uz pritisnuto F1)|

Strelice se koriste za selektovanje stavke unutar clipboarda, i space se koristi za kopiranje
te stavke u terminal, s time da treba da funkcionišu na ovaj način samo dok se drži F1. U
slučaju da je F1 pritisnuto, strelice i space obavljaju samo ovu funkciju, ne i svoju
normalnu funkciju. U slučaju da F1 nije pritisnuto, strelice i space se ponašaju
normalno. Ako smo na poslednjoj stavci, strelica dole nas vodi na prvu stavku, i ako smo na
prvoj stavci, strelica gore nas vodi na poslednju stavku.

F tasteri ne obavljaju svoju normalnu operaciju ispisivanja slova A / B / C na
terminalu.

Space kopira trenutno selektovanu stavku u terminal. Ovo funkcioniše i
kada unosimo komande za operativni sistem (shell) i kada se nalazimo u korisničkoj
aplikaciji.

F3 taster započinje upis stavke u clipboard. Trenutno selektovana stavka se ne briše na
početku unosa, već se nastavlja rad od kraja stavke. Backspace taster omogućava
brisanje teksta unutar clipboarda, ako on postoji. Ako nema teksta, backspace nema efekta.
Ponovni pritisak na taster F3 prekida unos u clipboard. Dok se unose podaci u clipboard
nije moguće interagovati sa terminalom.
