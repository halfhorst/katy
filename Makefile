CFLAGS = -std=c99 -g -Wall -Werror -pedantic
CXXFLAGS = -g -Wall -Werror -pedantic
LDFLAGS = -lm

SRC_DIR = src
TEST_DIR = src/tests
OBJ_DIR = obj
BUILD_DIR = build

default: test

# Test related things

test: $(BUILD_DIR)/test_tree $(BUILD_DIR)/test_heap

# This always builds because test doesn't match where t	he executable is placed
$(BUILD_DIR)/test_tree: $(OBJ_DIR)/katy.o $(OBJ_DIR)/test_tree.o $(OBJ_DIR)/heap.o
	$(CXX) $(CFLAGS) $^ -L/mnt/c/Users/chors/Documents/Programming/googletest/lib/ -lgtest -lgtest_main -lpthread -o $@

# This always builds because test doesn't match where the executable is placed
$(BUILD_DIR)/test_heap: $(OBJ_DIR)/test_heap.o $(OBJ_DIR)/heap.o
	$(CXX) $(CFLAGS) $^ -L/mnt/c/Users/chors/Documents/Programming/googletest/lib/ -lgtest -lgtest_main -lpthread -o $@

$(OBJ_DIR)/test_tree.o: $(TEST_DIR)/test_tree.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $^ -o $@ -I/mnt/c/Users/chors/Documents/Programming/googletest/googletest/include

$(OBJ_DIR)/test_heap.o: $(TEST_DIR)/test_heap.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $^ -o $@ -I/mnt/c/Users/chors/Documents/Programming/googletest/googletest/include

###

$(OBJ_DIR)/katy.o: $(SRC_DIR)/katy.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJ_DIR)/heap.o: $(SRC_DIR)/heap.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^ -o $@


.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/* $(BUILD_DIR)/*
