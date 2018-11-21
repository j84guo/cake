cake.out:
	g++ -o $@ -std=c++17 -Wall cake.cpp

clean:
	rm -f *.out target* dependency*

.PHONY: clean
