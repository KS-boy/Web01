src=$(wildcard *.cpp)
obj=$(patsubst %.cpp,%.o,$(src))


ALL:a.out

a.out:$(obj)
	g++  $^ -o $@



%.o:%.cpp
	g++ -c $< -o $@


clean:
	-rm -rf $(obj) a.out


.PHONY: clean ALL
