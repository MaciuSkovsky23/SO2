# Systemy operacyjne 2 - projekt

## Problem jedzących filozofów

### Cel projektu
Celem projektu było wykonanie aplikacji wykorzystującej wielowątkowość, która symuluje problem jedzących filozofów. Wymagania do programu:
* Każdy z wątków powinien raportować o stanie w jakim się znajduje w postaci wydruku w konsoli
* W symulacji nie dochodzi do trwałego zablokowania wątków (deadlock)
* Program ma otrzymywać jako argument liczbę filozofów

### Opis problemu
Problem jedzących (ucztujących) filozofów to klasyczny przykład zadania synchronizacji procesów. 
Pewna liczba filozofów (oryginalnie 5) siedzi przy okrągłym stole i na zmianę myśli lub je.
Na każdego przypada jeden widelec, ale żeby zacząć jeść potrzebuje ich dwóch.
Gdyby wszyscy filozofowie podnieśli tylko jeden widelec każdy z nich czekałby na drugi, więc nastąpiłoby zakleszczenie uniemożliwiając dalsze działanie.

### Rozwiązanie problemu
W ramach projektu do symulacji problemu zostało użyte rozwiązanie przy użyciu hierarchii zasobów. Każdy widelec jest indeksowany. Filozof podnosi najpierw widelec o niższym numerze, a dopiero potem drugi o wyższym numerze.
Po jedzeniu zwalniają widelce w odwrotnej kolejności do podnoszenia. 

### Wątki i ich funkcje
Program wykorzystuje wątki (`std::thread`), gdzie każdy wątek reprezentuje jednego filozofa. Tworzone są w `start()` gdzie dla każdego z nich uruchamiana jest funkcja `philosopherBehavior(int ID)`. Każdy filozof przechodzi cyklicznie przez następujące stany:
* THINKING - Myśli czekając losowy czas
* HUNGRY - Staje się głodny i próbuje podnieść dwa widelce
* EATING - Je trzymając dwa widelce przez losowy czas, a później je odkłada

Każdy filozof działa w osobnym wątku, ale korzysta ze współdzielonych zasobów - widelców.
```c++
for (int i = 0; i < numOfPhilosophers; i++) {
            philosopherThreads.emplace_back([this, i]()-> void {philosopherBehavior(i);});
        }
```

Podczas gdy filozof jest w stanie myślenia lub jedzenia jego wątek jest usypiany na losowy czas między 1s a 10s.
```c++
this_thread::sleep_for(chrono::milliseconds(randomDelay(1000, 10000)));
```

Wątki nie kończą się, więc program działa w nieskończonej pętli.

### Sekcje krytyczne i ich rozwiązanie
W programie występują dwie główne sekcje krytyczne:

#### Dostęp do widelców 
Widelce są współdzielonym zasobem, więc dostęp do nich musi być kontrolowany.
Każdy widelec jest chroniony przez mutex ('std::mutex'), aby tylko jeden filozof mógł go trzymać w danym momencie.

##### Rozwiązanie:
* Podnoszony jest najpierw widelec o niższym indeksie, a dopiero później o wyższym.
```c++
int firstFork = min(ID, (ID+1) % numOfPhilosophers);
int secondFork = max(ID, (ID+1) % numOfPhilosophers);
```

* Przy podniesieniu widelca jest on blokowany, aby żaden inny filozof nie mógł go podnieść
```c++
forks[firstFork]->lock();
forks[secondFork]->lock(); 
```
Jeżeli filozof podniesie pierwszy widelec, a drugi jest w tym czasie zajęty, to nie odkłada go i czeka z widelcem w ręce. Zacznie jeść dopiero drugi widelec zostanie odblokowany i go podniesie.

* Jak filozof skończy jeść mutexy są odblokowywane
```c++
forks[secondFork]->unlock();
forks[firstFork]->unlock();
```

#### Dostęp do konsoli
W programie może dojść do sytuacji gdzie kilka wątków jednocześnie próbuje wypisać coś do konsoli `std::cout`, przez co tekst mógłby się przeplatać i byłby nieczytelny.
#### Rozwiązanie:
* Użycie mutexa `consoleMutex`
* Każdy wątek blokuje mutex przed wypisaniem tekstu
```c++
void printState(int philosopherID, const string &action) {
        lock_guard<mutex> lock(consoleMutex);   //avoid mess in console output
        cout << "Philosopher " << philosopherID + 1 << " " << action << endl;
    }
```
* Mutex jest odblokowywany po zakończeniu operacji
Dzięki temu, gdy jeden wątek wypisuje stan filozofa, żaden inny wątek nie może jednocześnie wypisać stanu innego filozofa.

### Uruchomienie programu (Windows command line) 
#### Wymagania
* C++14 lub nowszy
* Kompilator g++
#### Kompilacja
```bash
g++ -std=c++14 main.cpp -o exe
```
W niektórych przypadkach konieczne może być dodanie flagi `-pthread`
#### Uruchomienie 
```bash
exe 5
```
Gdzie zamiast 5 można podać inną liczbę filozofów.
