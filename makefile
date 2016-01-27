All: prog1 prog2 prog3 prog4 prog5
prog1: client.c
	@echo "Compiling Client Program"
	gcc client.c -o client -lsocket -lnsl -lresolv

prog2: serverA.c
	@echo "Compiling ServerA program"
	gcc serverA.c -o serverA -lsocket -lnsl -lresolv

prog3: serverB.c
	@echo "Compiling ServerB program"
	gcc serverB.c -o serverB -lsocket -lnsl -lresolv

prog4: serverC.c
	@echo "Compiling ServerC program"
	gcc serverC.c -o serverC -lsocket -lnsl -lresolv

prog5: serverD.c
	@echo "Compiling ServerD program"
	gcc serverD.c -o serverD -lsocket -lnsl -lresolv
