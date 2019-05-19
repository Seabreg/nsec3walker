all: dicthashes randomhashes

dicthashes: dicthashes.c
	gcc -O3 -o dicthashes dicthashes.c -lcrypto -lm

randomhashes: randomhashes.c
	gcc -O3 -o randomhashes randomhashes.c -lcrypto -lm
