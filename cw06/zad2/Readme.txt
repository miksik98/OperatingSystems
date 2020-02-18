Aby uruchomić przetestować działanie programu, należy:

1) Uruchomić kolejno 4 terminale. 
2) W pierwszym z nich uruchomić serwer (./server), w kolejnych dwóch klientów (./client), 
i w ostatnim uruchomić klienta korzystajacego z test.txt (./client test.txt).

W kliencie 0 powinny wypisać się message5 oraz message6
W kliencie 1 powinny wypisać się message2, message4 oraz message5
W kliencie 2 powinny wypisać się message1, ... message5, a następnie powinien zakończyć działanie
W serwerze powinni wylistować się aktywni klienci
 
3) Wysłać SIGINT (ctrl+C) do klientów 0 i 1. 
W ich terminalach powinien wypisać się komunikat "SIGINT signal", natomiast w serwerze "No active clients", wszystkie programy powinny zakończyć swoje działanie. 
