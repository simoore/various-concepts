CC      = g++
TARGET  = geolocation
SRC	    = geolocation.cpp
INCS    = -I../../lib/json-release-3.10.2/include
LDLIBS  = -lcurl 

$(TARGET): $(SRC)
	$(CC) -std=c++17 $(INCS) -o $(TARGET) $(SRC) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(TARGET)