all: nspr testharness

sleep: nspr testharness-sleep

nspr:
	./scripts/build-nspr.sh 

testharness:
	./scripts/build-testharness.sh default
	./scripts/build-testharness-with-DBDS.sh
	cp ./src/runme.sh ./bin/	

testharness-sleep:
	./scripts/build-testharness.sh sleep
	cp ./src/runme.sh ./bin/	

clean:
	./scripts/clean.sh
