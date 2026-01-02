mana-patcher: main.c
	$(CC) -O3 -DNDEBUG -o $@ $^
