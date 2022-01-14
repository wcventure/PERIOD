all: js testharness

js:
	./scripts/build-js.sh

testharness:
	./scripts/build-testharness.sh
	./scripts/build-testharness-with-DBDS.sh
	cp ./src/runme.sh ./bin/runme.sh

clean:
	./scripts/clean.sh
