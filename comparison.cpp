#include "comparison.h"

comparison::comparison()
{

}

void comparison::rollingHashTest()
{
    std::vector<unsigned char> data = {0,1,2,3,4,0,1,2,3,4};


    std::cout<<"rollingHashTest" << std::endl;

    uint32 n = 5;
    int L = 9;

    //GeneralHash<> hf(n,L );
    //KarpRabinHash<> hf(n,L );
    CyclicHash<> hf(n,L );

    int index=0;
    for(uint32 k = 0; k<n;++k) {
              unsigned char c = data[index++]; ; // grab some character
              hf.eat(c); // feed it to the hasher
    }

    for (int i = 0; i < 2; ++i) {

/*
        for(uint32 k = 0; k<n;++k) {
                  unsigned char c = data[index++]; ; // grab some character
                  hf.eat(c); // feed it to the hasher
        }
*/
        std::cout<< " initial hashvalue: " << hex << hf.hashvalue << std::endl;

        while(index < data.size()) { // go over your string
           hf.hashvalue; // at all times, this contains the hash value
           unsigned char c = data[index];// points to the next character
           unsigned char out = data[index-n]; // character we want to forget

           hf.update(out,c); // update hash value

           std::cout<< " hashvalue: 0x" << hex << hf.hashvalue << "  c: 0x" << hex << (uint32)c << "  out: 0x" << hex << (uint32)out << std::endl;

           index++;
        }

        std::cout << std::endl;
        index = n;
    }

}
