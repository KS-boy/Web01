CXX=g++
TARGET=webServer_Muduo
SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,%.o,$(SRC))

CXXFLAGS= -c -g 
# -Wall
#  -lmuduo_base -lmuduo_net

$(TARGET):$(OBJ)
	$(CXX) -o $@ $^    -lpthread 


%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY:clean

clean:
	rm -f *.o $(TARGET)
