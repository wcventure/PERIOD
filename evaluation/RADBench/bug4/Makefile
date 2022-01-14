all: nspr-bug testharness

sleep: nspr-sleep testharness

patch: nspr-patch testharness

nspr-bug:
	./scripts/build-nspr.sh default

nspr-sleep:
	./scripts/build-nspr.sh sleep

nspr-patch:
	./scripts/build-nspr.sh patch

testharness:
	./scripts/build-testharness.sh
	./scripts/build-testharness-with-DBDS.sh
	cp ./src/runme.sh ./bin/

clean:
	./scripts/clean.sh
