SCHEDULER=hw6.c
BINARIES=hw6
CCOPT=-g --std=gnu99
LIBS=-lpthread

all: $(BINARIES)

hw6: main.c hw6.h $(SCHEDULER)
	gcc $(CCOPT) main.c $(SCHEDULER) -o hw6 $(LIBS)

burn-in: clean hw6.c
	gcc $(CCOPT) -DDELAY=1000 -DLOG_LEVEL=5 -DPASSENGERS=100 -DTRIPS_PER_PASSENGER=100 -DELEVATORS=10 -DFLOORS=40 main.c $(SCHEDULER) -o hw6 $(LIBS)
	./hw6

test: clean $(SCHEDULER)
	@echo "\n\nNOTE: Logs from stderr stored in ./logs.tmp\n\n"
	@sleep 1
	gcc -DDELAY=10000 -DLOG_LEVEL=9 -DPASSENGERS=10 -DELEVATORS=1 -DFLOORS=5 -g --std=c99 main.c $(SCHEDULER) -o hw6 $(LIBS) $(CCOPT)
	@./hw6 2> ./logs.tmp
	@cat ./logs.tmp | egrep -v Passenger > ./testresults
	@cat ./logs.tmp | awk '/trip duration/{sum+=$$5;count++;if(max<$$5){max=$$5}} END {printf "Mean time: %.2f max: %.2f\n",sum/count,max}' >> ./testresults

	@gcc -DDELAY=10000 -DLOG_LEVEL=6 -DFLOORS=2 -g --std=c99 main.c $(SCHEDULER) -o hw6 $(LIBS) $(CCOPT)
	@./hw6 2> ./logs.tmp
	@cat ./logs.tmp | egrep -v Passenger >> ./testresults
	@cat ./logs.tmp | awk '/trip duration/{sum+=$$5;count++;if(max<$$5){max=$$5}} END {printf "Mean time: %.2f max: %.2f\n",sum/count,max}' >> ./testresults

	@gcc -DDELAY=10000 -DELEVATORS=10 -DLOG_LEVEL=6 -g --std=c99 main.c $(SCHEDULER) -o hw6 $(LIBS) $(CCOPT)
	@./hw6 2> ./logs.tmp
	@cat ./logs.tmp | egrep -v Passenger >> ./testresults
	@cat ./logs.tmp | awk '/trip duration/{sum+=$$5;count++;if(max<$$5){max=$$5}} END {printf "Mean time: %.2f max: %.2f\n",sum/count,max}' >> ./testresults
	cat ./testresults


clean:
	@rm -rf *~ *.dSYM $(BINARIES)

