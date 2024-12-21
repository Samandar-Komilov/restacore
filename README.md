# 🍽 Bistrosync
BistroSync - a Restaurant Management System for desktop, written in C. Primarily intended as a university project and learning with practice.

![image](https://github.com/user-attachments/assets/f0bc8b23-548d-4e57-af09-578d77510cb0)

## Compilation and Execution Process
- Clone this repository
- Setup PostgreSQL (install if you don't, create a new database and set .env configurations)
- allow start.sh to execute (using chmod or GUI)
- Run start.sh
- You should see the following result in the end:

![image](https://github.com/user-attachments/assets/7b6bb4d8-14d8-4b18-aff7-7f2f45f3fdb8)

Congrats! You can now start using the application!


## Compilation and Execution on hand
After setting up the project, if you need, you can check Makefile so that you'll understand the following commands are used to compile client and server programs:

Client process compilation:
```
gcc `pkg-config --cflags gtk+-3.0` src/client.c src/client_functions.c -o build/client `pkg-config --libs gtk+-3.0` -lpthread
```


Server process compilation:
```
gcc src/server.c src/server_functions.c -o build/server -I/usr/include/postgresql -L/usr/lib -lpq -lpthread
```

So that's all, enjoy!!!
