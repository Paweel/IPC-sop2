all:
	gcc -g -Wall src/inf122499_p.c src/inf122499_queue.c -o patient
	gcc -g -Wall src/inf122499_r.c src/inf122499_queue.c -o reception
	gcc -g -Wall src/inf122499_l.c src/inf122499_queue.c -o doctor
recept:
	gcc -g -Wall src/inf122499_r.c src/inf122499_queue.c -o reception
patien:
	gcc -g -Wall src/inf122499_p.c src/inf122499_queue.c -o patient


