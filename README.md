# Simulation
Modelling and Simulation Project

## Zadanie
### 6. Model svozu odpadů
Zvolte si část území ČR (část města, vesnice) obsahující obydlené a průmyslové budovy. Modelujte síť ulic a domů. Modelujte proces produkce odpadu v jednotlivých domech. V systému modelujte vozové depo se svozovými vozy a místo pro zpracování svezeného odpadu. Modelujte proces svozu odpadu s respektováním sítě ulic. Modelujte náklady spojené se svozem odpadu a zpracováním odpadu. Experimenty určete optimální počet svozových vozů pro modelovanou oblast (s respektováním nákladů na provoz svozu).

## Návrh

### Obslužná sieť pomocou stromu
Obslužná sieť predstavujúca model mestskej časti može byť reprezentovaná buď zjednodušene a pomocou stromov, kde listy budu reprezentovať zjednodušené množiny ulíc v ktorých bude zber prebiehať na sume domov, počet domov bude udávať počet zastavení pri nádobách. V rámci jednej sekcie by boli objemy produkcie a doby presunu zberacieho auta medzi domami rovnaké. Prechod grafom po vážených hranách by simloval presuny medzi časťami. Nevýhodou je nepresnosť tohoto modelu. Presné polohy ulíc nebudú modelované. To by sa dalo zlepšiť zvýšením granulatiry z časťí na ulice. Výhodou by bolo možnosť zistiť v kt častiach mesta dochádza k hormadeniu

### Obslužná siet pomocou grafu orientovaného/neorientovaného, hranového/uzlového 
Realnejší model by sa dal vytvoriť pomocou uzlov - križovatiek a hrán - ciest. V rámci uzlov by boli značené cesty kt. možno navšťíviť a v rámci hrán uzly kam sa je možné dostať, dĺžka cesty a množina domov kt. treba obslúžiť. Tento presnejší model je algoritmicky náročnejší. V prípade orientovaného grafu je treba dávať pozor na uviaznutie auta v slepej uličke, alebo skor zabezpečiť ze k takému prípadu nedôjde. Zbytočne to komplikuje simuláciu, ale pokiaľ by bolo treba modelovať logistiku bola by to zaujimavá varianta.

### Obslužná siet pomocou parametrizácie lokality
Zadaný je počet domov a celková dľžka ulíc, z toho sa spočita stredná doba presunu medzi zbermi a počty domov sa rozdelia medzi auta. Odhadne sa cas pre presun z depa do mesta, z mesta na skládku, doba nakladania odpadu, doba vykladania odpadu. V prípade ze by auta nestihli zviezt odpad z x domov, v nasledujucom tyzdni odnásaju dvojnásobok odpadu z x domov. Pre túto variantu je treba získať štatistiky aby bol model realistický.

#### Možné zahrnúť
Počas presunov alebo aj zberov može dojsť k poruche, každé vozidlo ma vlasný generátor porúch. Kt. bude vyžadovat prioritnú obsluhu => vrátenie odpadu k domu uvolnenie zariadenia a následne spustí prioritný proces opravy.

#### UPDATE see code
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

### Český štatistický úrad

Pre Brno-mesto

Rozloha: 230.22 km^2  
Počet obyvateľov 377 028
Počet domácností 174 162

Statistika odpadov
https://www.czso.cz/documents/11256/49147777/odpad+2015.pdf/a3fabc5e-3fe1-4805-8849-f5ea143a1d46?redirect=https%3A%2F%2Fwww.czso.cz%2Fweb%2Fczso%2F404-%3Fp_p_id%3D3%26p_p_lifecycle%3D0%26p_p_state%3Dmaximized%26p_p_mode%3Dview%26_3_groupId%3D0%26_3_keywords%3Dodpady%26_3_struts_action%3D%252Fsearch%252Fsearch%26_3_redirect%3D%252Fweb%252Fczso%252F404-

Počty domácností
https://vdb.czso.cz/vdbvo2/faces/cs/index.jsf?page=vystup-objekt&z=T&f=TABULKA&pvoch=3115&pvo=SLDB-ZAKL-KRAJE-kraje&pvokc=100&katalog=30261&str=v98&rouska=true&clsp=null

Počty ľudí
https://vdb.czso.cz/vdbvo2/faces/cs/index.jsf?page=vystup-objekt&pvo=SLDB-ZAKL-KRAJE-kraje&z=T&f=TABULKA&katalog=30261&pvokc=100&pvoch=3115