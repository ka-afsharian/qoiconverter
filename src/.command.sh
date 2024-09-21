g++ -c -I../include/graphth -fPIC graphth.cpp -o graphth.o
#g++ -c -fPIC graphth.cpp -o graphth.o
g++ -shared graphth.o -o libgraphth.so
#g++ -I../include/graphth example.cpp -L. -lgraphth -o main
#export LD_LIBRARY_PATH=.:.
#./main


