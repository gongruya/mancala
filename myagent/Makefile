CC = g++
CFLAGS = -O3 -std=c++11 -Wall
OBJS = myagent.cpp
EXEC = myagent
agent: $(OBJS)
	$(CC) $(CFLAGS) myagent.cpp -o $(EXEC)
run: agent
	./$(EXEC)
clean:
	rm -f $(EXEC)