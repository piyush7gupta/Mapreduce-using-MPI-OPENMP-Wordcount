LIST = 1 2 3 4 5 6 7 8
run :	
	mpicxx wordcountmpi.cpp -o mpiwc 
	g++ wordcountopenmp.cpp -o openmpwc -fopenmp
	for i in $(LIST); do \
		./openmpwc $$i; \
	done
	for i in $(LIST); do \
		mpirun --hostfile hostfile -np $$i ./mpiwc; \
	done
	gnuplot -e "set terminal jpeg;set xlabel 'No of processes';set ylabel 'Time';set title 'Performance comparison'; plot 'resultmpi.txt' smooth csplines lc rgb 'green' title 'MPI Map reduce','resultmpi.txt' lc rgb 'blue', 'resultopenmp.dat' smooth csplines lc rgb 'yellow' title 'OpenMP Map reduce','resultopenmp.dat' lc rgb 'red' " > out.jpeg 
clean :
	rm outputmpi.txt
	rm outputopenmp.txt
	rm mpiwc
	rm openmpwc
	rm resultmpi.txt	
	rm resultopenmp.dat
	rm out.jpeg
