# Systemy operacyjne 2 - projekt

## Wielowątkowy serwer czatu

### Cel projektu
Celem projektu było stworzenie aplikacji wielowątkowej umożliwiającej komunikację wielu klientów z jednym serwerem czatu. Kluczowe wymagania:
* Serwer tworzy osobny wątek dla każdego połączenia od klienta
* Serwer dba o synchronizację wiadomości od klientów
* Klient widzi wiadomości na czacie
* Klient może wysyłać wiadomości

### Opis projektu
Program został podzielony na dwie części:
* Serwer - zarządza połączeniami i dystrybuuje wiadomości do wszystkich klientów
* Klient - umożliwia użytkownikowi wysyłanie i odbieranie wiadomości w czasie rzeczywistym

Komunikacja odbywa się za pomocą gniazd TCP. Serwer uruchamia dwa typy wątków:
* wątek obsługujący klienta (odbiór wiadomości)
* wątek nadzorujący dystrybucję wiadomości do wszystkich klientów

Wiadomości są przechowywane w wielowątkowej kolejce, która synchronizuje dostęp między wątkami

### Wątki i ich funkcje
* `handleClient()` - osobny wątek dla każdego klienta, odbiera wiadomości, przypisuje nadawcę i dodaje je do kolejki
* `messageBroadcaster()` - odbiera wiadomości z kolejki i wysyła je do wszystkich aktualnie połączonych klientów
* Klient posiada dwa wątki:
  - wysyłanie wiadomości
  - odbieranie wiadomości od serwera
 
### Sekcje krytyczne i synchronizacja
Wielowątkowa klasa `MessageQueue` synchronizuje dostęp:
* `mutex` i `condition_variable`
* `push()` blokuje inne wątki podczas dodawania wiadomości
* `pop()` czeka aż pojawi się wiadomość i ją odbiera

#### Lista klientów
* Współdzielona lista `clientSockets`
* Zabezpieczona przez `clientMutex` przy każdym odczycie/zapisie

### Format wiadomości
`[YYYY-MM-DD HH:MM:SS] client: message`

### Uruchomienie programu

#### Wymagania
* System Windows
* Kompilator c++ z obsługą WinSock
* Biblioteki winsock2.h i Ws2_32.lib

#### Kompilacja 
W katalogu projektu

##### Kompilacja serwera:
```bash
g++ server.cpp -o server.exe -lws2_32
```
##### Uruchomienie
```bash
server.exe
```
##### Kompilacja klienta:
```bash
g++ client.cpp -o client.exe -lws2_32
```
##### Uruchomienie
```bash
client.exe
```

Przy rozpoczęciu połączenia klient zostanie zapytany o nazwę użytkownika, która będzie wyświetlana przy wysyłanej wiadomości.
