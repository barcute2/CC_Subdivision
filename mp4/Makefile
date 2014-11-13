CXX = g++
LD = g++
CXXFLAGS = -Wall  -g -ggdb
EXEC = mp4
LDFLAGS = -o $(EXEC) -L/usr/lib -lGL -lGLU -lglut -lGLEW
SOURCES = mp4.cpp
OBJS = $(SOURCES:.cpp=.o)


$(EXEC) : $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS)  

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^

clean:
	rm *.o

