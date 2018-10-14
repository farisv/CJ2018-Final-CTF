CXX      := g++
SOURCES  := AES256.cpp AES256_PRNG.cpp AES256_Base.cpp S_Box.cpp
HEADERS  := AES256.hpp AES256_PRNG.hpp AES256_Base.hpp Byte_Block.hpp \
	        S_Box.hpp Padding_Type.hpp
CXXFLAGS := -O3 -ggdb -Wall -Wextra -march=native -Wno-unused-result

#-DCALC_RIJNDAEL_SBOX
#-DON_THE_FLY_RIJNDAEL_SBOX

all: aes256_string aes256_stream aes256_file rnd1 rnd2 aes256_base fips \
     nist_test nist_test_mct crypto

aes256_string: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -DTEST_AES256_STRING -o $@ $(SOURCES)

aes256_stream: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -DTEST_AES256_STREAM -o $@ $(SOURCES)

aes256_file: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -DTEST_AES256_FILE -o $@ $(SOURCES)

rnd1: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -DTEST_AES256_RAND1 -o $@ $(SOURCES)

rnd2: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -DTEST_AES256_RAND2 -o $@ $(SOURCES)

aes256_base: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -DTEST_AES256_BASE -o $@ $(SOURCES)

fips: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -DTEST_AES256_BASE_FIPS -o $@ $(SOURCES)

nist_test: nist_test.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ nist_test.cpp $(SOURCES)

nist_test_mct: nist_test_mct.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ nist_test_mct.cpp $(SOURCES)

crypto: crypto.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ crypto.cpp $(SOURCES)

clean:
	rm -f *.o *~ aes256_string aes256_stream aes256_file rnd1 rnd2 \
	      aes256_base fips nist_test nist_test_mct random.dat README.enc \
	      README.dec crypto

