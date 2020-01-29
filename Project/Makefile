all: example_main my_main ht_main_names

example_main: Hash.o Record.o PrimaryRecord.o SecondaryHashTable.o SecondaryRecord.o ht_main_test.o Statistics.o
	gcc Hash.o Record.o PrimaryRecord.o SecondaryHashTable.o SecondaryRecord.o ht_main_test.o Statistics.o BF/BF_64.a -g -g3 -no-pie -o example_main -Wall -std=c99

my_main: main.o
	gcc Hash.o Record.o PrimaryRecord.o SecondaryHashTable.o SecondaryRecord.o main.o Statistics.o BF/BF_64.a -g -g3 -no-pie -o my_main -Wall -std=c99

ht_main_names: ht_main_names_test.o
	gcc Hash.o Record.o PrimaryRecord.o SecondaryHashTable.o SecondaryRecord.o ht_main_names_test.o Statistics.o BF/BF_64.a -g -g3 -no-pie -o ht_main_names -Wall -std=c99

Hash.o: Main\ Files/Common/Hash.c
	gcc -c 'Main Files/Common/Hash.c'

Record.o: Main\ Files/Common/Record.c
	gcc -c 'Main Files/Common/Record.c'

PrimaryRecord.o: Main\ Files/PrimaryHashTable/PrimaryRecord.c
	gcc -c 'Main Files/PrimaryHashTable/PrimaryRecord.c'

SecondaryHashTable.o: Main\ Files/SecondaryHashTable/SecondaryHashTable.c
	gcc -c 'Main Files/SecondaryHashTable/SecondaryHashTable.c'

SecondaryRecord.o: Main\ Files/SecondaryHashTable/SecondaryRecord.c
	gcc -c 'Main Files/SecondaryHashTable/SecondaryRecord.c'

Statistics.o: Main\ Files/Common/Statistics.c
	gcc -c Main\ Files/Common/Statistics.c


ht_main_test.o: Main\ Files/ht_main_test.c
	gcc -c 'Main Files/ht_main_test.c'

main.o: Main\ Files/main.c
	gcc -c 'Main Files/main.c'

ht_main_names_test.o: Main\ Files/ht_main_names_test.c
	gcc -c 'Main Files/ht_main_names_test.c'

.PHONY: clean

clean:
	rm -rf Hash.o Record.o PrimaryRecord.o Statistics.o SecondaryHashTable.o SecondaryRecord.o ht_main_test.o main.o ht_main_names_test.o my_main example_main ht_main_names
