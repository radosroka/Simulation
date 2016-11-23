# Simulation
Modelling and Simulation Project

## Zadanie
### 6. Model svozu odpadů
Zvolte si část území ČR (část města, vesnice) obsahující obydlené a průmyslové budovy. Modelujte síť ulic a domů. Modelujte proces produkce odpadu v jednotlivých domech. V systému modelujte vozové depo se svozovými vozy a místo pro zpracování svezeného odpadu. Modelujte proces svozu odpadu s respektováním sítě ulic. Modelujte náklady spojené se svozem odpadu a zpracováním odpadu. Experimenty určete optimální počet svozových vozů pro modelovanou oblast (s respektováním nákladů na provoz svozu).

## Návrh

### Obslužná sieť pomocou stromu
Obslužná sieť predstavujúca model mestskej časti može byť reprezentovaná buď zjednodušene a pomocou stromov, kde listy budu reprezentovať zjednodušené množiny ulíc kde bude zber prebiehať na sume domov, počet domov bude udávať počet zastavení pri nádobách. V rámci jednej sekcie by boli pri každom dome rovnaké stredné hodnoty generovania odpadu a presunu zberacieho auta medzi domami. Prechod grafom po vážených hranách by simloval presuny medzi časťami. Nevýhodou je nepresnosť tohoto modelu. Cesty na úrovni ulíc nebudú modelované. To by sa dalo zlepšiť zvýšením granulatiry z časťí na ulice.

### Obslužná siet pomocou grafu orientovaného/neorientovaného, hranového/uzlového 
Realnejší model by sa dal vytvoriť pomocou uzlov - križovatiek a hrán - ciest. V rámci uzlov by boli značené cesty kt. možno navšťíviť a v rámci hrán uzly kam sa je možné dostať, dĺžka cesty a množina domov kt. treba obslúžiť. Tento presnejší model je algoritmicky náročnejší. V prípade orientovaného grafu je treba dávať pozor na uviaznutie auta v slepej uličke, alebo skor zabezpečiť ze k takému prípadu nedôjde. 

Počas presunov alebo aj zberov može dojsť k poruche, každé vozidlo ma vlasný generátor porúch. Kt. bude vyžadovat prioritnú obsluhu => vrátenie odpadu k domu uvolnenie zariadenia a následne spustí prioritný proces opravy.

### Entity vystupujúce v modely
Nasledúje výčet možných entit a ich vlastností figurujúcich v modele
#### Auto - proces
Kapacita
Naložený odpad
Stredná hodnota medzi poruchami
Doba nákladky pre rôžne max kapacity eg 30 pre < 70 litrov odpadu etc.
Plán zberu
Pozícia (uzol)
#### Zberný dvor - generátor
Pozícia
Počeť aut
Šichta deň/noc ? Pracovná doba/2. smena/ noc

#### Opravovňa - facility
Pozícia
Stredná doba opravy
Poč simultanných opráv
    
#### Skládka - facility
Pozícia
Kapacita
Zaplnenie
Doba vykládky
-Možno pridat spracovavanie
-Spaľovať odpad etc...

#### Domov - proces/facility
Príslušnosť na hranu
Kapacita
Zaplnenie - može byť väčšie, pre povolenie hromadenia a získavanie štatiskík
Produkcia per day

#### Fabrika - proces/facility
Prislušnosť na uzol / hranu - predpokladá sa vyššia produkcia, možné speciálne plány zberu pre fabriky
Kapacita
Zaplnenie
Produkcia per day 

Je treba neajako zabezpečiť optimálnu obsluhu. Buď pomocou zberových plánov, s vyhľadávaním optimálnych ciest alebo nejako, to ešte neviem ako spravit. Napada ma tiez vytvoriť kostru na začiatku a prechádzat len po nej. 
