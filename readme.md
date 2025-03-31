# DistanceMeterMCU

## Temat projektu
Ultradźwiękowy miernik odległości (HC-SR04p lub RCW-0001 – zasilanie 3.3V) z alarmem:
wyświetlanie odległości w metrach, centymetrach, milimetrach lub calach (do wyboru). Regulowany próg zadziałania alarmu („za blisko”) – synteza DDS za pomocą DAC0. 

## Wprowadzenie
System mikroprocesorowy został zaprojektowany do precyzyjnego pomiaru odległości z
wykorzystaniem czujnika ultradźwiękowego. Urządzenie charakteryzuje się wysoką dokładnością
pomiarów oraz możliwością dostosowania jednostek pomiarowych do potrzeb użytkownika. Systemzostał wyposażony w wyświetlacz LCD do wyświetlania najpotrzebniejszych danych oraz interfejs
dotykowy i klawiaturę matrycową do regulacji wyświetlanych informacji, co zapewnia intuicyjną
obsługę i czytelną prezentację wyników. Dodatkowo w projekcie został wykorzystany moduł głośnika, który zapewnia nam ostrzegawcze sygnały akustyczne. 

## Parametry systemowe
- Zakres pomiarowy: 2-400 cm
- Dostępne jednostki: milimetry, centymetry, metry, cale
- Częstotliwość odświeżania: 10 Hz -
- Rozdzielczość wyświetlacza: 16x2 znaki 
- Interfejs użytkownika: 4 przyciski, panel dotykowy 
- Sygnalizacja: wyświetlacz LCD, moduł głośnika 
- Zasilanie: zgodne ze standardem mikrokontrolera

## Elementy sprzętowe
- Mikrokontroler główny: zestaw FRDMKL05Z 
- Czujnik ultradźwiękowy do pomiaru odległości: HC-SR04p
- Wyświetlacz LCD
- Panel dotykowy 
- 4 przyciski funkcyjne znajdujące się na klawiaturze matrycowej. 
- Moduł głośnika: WSR-04489

## Opis
DistanceMeterMCU to projekt miernika odległości oparty na mikrokontrolerze, który wykorzystuje czujniki ultradźwiękowe do precyzyjnego pomiaru dystansu. Projekt został zaprojektowany z myślą o łatwej integracji z systemami wbudowanymi oraz możliwością dalszej rozbudowy.

## Funkcje
- Pomiar odległości przy użyciu czujników ultradźwiękowych  
- Współpraca z mikrokontrolerami  
- Możliwość wyświetlania wyników na ekranie LCD lub przesyłania do komputera  
- Efektywne zarządzanie poborem mocy  

## Wymagania
- Mikrokontroler kompatybilny z kodem projektu  
- Czujnik ultradźwiękowy (np. HC-SR04)  
- Opcjonalnie wyświetlacz LCD do prezentacji wyników  
- Środowisko programistyczne do wgrywania kodu  

## Instalacja
1. Sklonuj repozytorium:  
   ```bash
   git clone https://github.com/kamildrozdzz/DistanceMeterMCU.git
