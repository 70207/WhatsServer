HttpDir=http
WebSocketDir=websocket
OtherDir=conf conn core cycle event event/modules log mem net os tools stat interface common
Dir=$(OtherDir) $(HttpDir) $(WebSocketDir)
Src=$(foreach dir, $(Dir), $(wildcard $(dir)/*.c)) tse1.c
FileSrc=$(notdir $(Src))


WhatsServerObjDir=obj
WhatsServerObj=$(patsubst %.c, $(WhatsServerObjDir)/%.o, $(FileSrc))


Inc=$(foreach dir, $(Dir), -I $(dir)/ )  -I../include -DKO_PRINT_LOG
Lib=-L/usr/lib64 -lcrypto -lpthread


CC=gcc


ctrl=./ctrl
whatsserver=./build/WhatsServer



all:$(ctrl) copysrc  $(whatsserver) 
	@echo make all ok! now you can install it 

$(ctrl):$(MAIN)
	@echo make ctrl
	mkdir -p  $(WhatsServerObjDir)
	mkdir -p ./build


WhatsServerFlags= -g -w -O0 $(Inc) 
$(whatsserver):$(WhatsServerObj) 
	@echo make whatsserver
	$(CC) -g -w  -o $@  $^ $(Lib) 
	

$(WhatsServerObjDir)/%.o:$(WhatsServerObjDir)/%.c
	@echo make whatsserver src
	$(CC) $(WhatsServerFlags) -c $< -o $@

copysrc:
	rsync -avzP $(Src) $(WhatsServerObjDir)
	
 
cleanall:
	rm -f $(WhatsServerObjDir)/*
	rm -f $(whatsserver) 
	rm -rf ./build 
	@echo clean ok!

clean:
	rm -f $(WhatsServerObjDir)/*
	rm -f $(whatsserver) 
	@echo clean ok! 

cleanobj:
	rm -f $(WhatsServerObjDir)/*
	@echo clean ok!

