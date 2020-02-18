Możliwe wywołania programu:
./wynik (test) funkcjonalnosc nazwa_pliku (nazwa_pliku_do_kopiowania) liczba_rekordów długość_rekordów sys/lib

np.
./wynik generate plik 100 512 - generuje 100 rekordów o długości 512 do pliku plik
./wynik sort plik 100 512 sys - sortowanie rekordów w pliku plik, użycie f. systemowych
./wynik sort plik 100 512 lib - -\\- bibliotecznych
./wynik copy plik1 plik2 100 512 sys - kopiowanie rekordów z plik1 do plik2, użycie f. systemowych
./wynik copy plik1 plik2 100 512 lib - -\\- bibliotecznych

Dodanie jako drugiego argumentu "test" uruchamia testy funkcjonalności (pomiary czasu), np:
./wynik test sort plik 100 512 sys
./wynik test copy plik1 plik2 100 512 lib
