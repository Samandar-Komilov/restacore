# bistrosync
BistroSync - a Restaurant Management System for desktop, written in C. Primarily intended as a university project and learning with practice.


Client process compilation:
```
gcc `pkg-config --cflags gtk+-3.0` src/client.c src/client_functions.c -o build/client `pkg-config --libs gtk+-3.0` -lpthread
```


Server process compilation:
```
gcc src/server.c src/server_functions.c -o build/server -I/usr/include/postgresql -L/usr/lib -lpq -lpthread
```


Soon, i'll rewrite the README.