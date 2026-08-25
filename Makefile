CXX = g++
CXXFLAGS = -Wall -std=c++17 -O2

loadbalancer: main.o Request.o WebServer.o LoadBalancer.o
	$(CXX) $(CXXFLAGS) -o loadbalancer main.o Request.o WebServer.o LoadBalancer.o

clean:
	rm -f loadbalancer *.o