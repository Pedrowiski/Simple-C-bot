all:
	gcc -Wall bot.c -pthread -ldiscord -lcurl -lcrypto -lm -lnxjson -o bot
