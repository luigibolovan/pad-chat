# Camera de discutii - BSD Sockets(UNIX)

### Componente
- server concurent
- client

## Cerinte
**Server**

*Gestioneaza o camera de chat acceptand mai multe conexiuni simultane de la
utilizatorii care vor sa discute. Utilizatorii trebuie sa se autentifice
printr-un nume si o parola. Numele trebuie sa fie unice.*

*Serverul va primi mesaje de la utilizatorii conectati si va trimite imediat
tuturor clientilor mesajele primite.*

**Client**

*Se conecteaza la server si ofera utilizatorului o interfata text prin care
poate sa trimita mesaje spre camera de discutii. Un mesaj trimis de
un utilizator va aparea simultan in interfata oferita tuturor 
utilizatorilor, inclusiv a celui care a scris initial mesajul.*


## Instalare

**_make_**        - compileaza clientul si serverul

**_make client_** - compileaza clientul

**_make server_** - compileaza serverul

**_make clean_**  - sterge executabilele generate dupa instalare

## Rulare

Pentru a rula clientul, este nevoie de 2 argumente:
- ip-ul server-ului
- portul

Dupa introducerea ip-ului si server-ului ca argumente, clientului i se va cere un username si o parola care ulterior vor fi verificate de server. Imediat ce serverul valideaza userul, acesta poate introduce mesaje ce vor fi vazute de ceilalti clienti conectati la server.
Pentru iesirea din *camera de discutii*, clientul poate tasta fie **_/exit_**, fie poate inchide pur si simplu programul prin **_CTRL+C_**


