all: genData wrt_bmData

genData: genData.c ../../common/helper.h 
	gcc -o genData -pedantic -std=gnu99 -Wall genData.c ../../common/helper.h  -lm

wrt_bmData: wrt_bmData.c ../../common/helper.h 
	gcc -o wrt_bmData -pedantic -std=gnu99 -Wall wrt_bmData.c ../../common/helper.h  -lm

clean:
	rm genData wrt_bmData
