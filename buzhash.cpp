#include "buzhash.h"

buzhash::buzhash(unsigned int hashWindowSize)
    :   n(hashWindowSize),
        state(0),
        buffer(hashWindowSize),
        bufferPos(0),
        bufferFilled(false)
{
    ASSERT(0 < n);
    ASSERT(4 == sizeof(unsigned int));
}

unsigned int buzhash::hashByte(const unsigned char b)
{
    //https://en.wikipedia.org/wiki/Rolling_hash#Cyclic_polynomial

    //barrel shift the hash value left
    state = (state << 1) | (state >> 31);

    //add the new byte to the hash value
    state ^= bytehash[b];

    //if the hash window has been filled, the oldest byte must be removed from the hash value
    // to keep the hash window length constant
    if (bufferFilled) {

        unsigned char byteToRemove = buffer[bufferPos];     //the oldest byte in the hash window
        unsigned int removeHash = bytehash[byteToRemove];   //the hash to be XORed out to remove the byte

        //barrel shifting left n times will line up removeHash to be XORed out of the result
        //barrel shifting left n%32 times will give the same result (for a 32-bit value)
        unsigned int barrelShiftShortcut = n%32;

        //barrel shift n times left
        removeHash = (removeHash << barrelShiftShortcut) | (removeHash >> (32 - barrelShiftShortcut));

        //remove the byte from the hash value
        state ^= removeHash;

    }

    //manage the hash window circular buffer
    buffer[bufferPos] = b;  //add the new byte to the buffer
    ++bufferPos;

    if (bufferPos == n) {
        //the end of the buffer has been reached, wrap around to the beginning
        bufferPos = 0;

        //the hash window has now been filled
        bufferFilled = true;
        //  note: this will be redundantly set to true in every nth call of this function;
        //      it will never be false again unless this buzhash instance is reset
    }

    return state;
}

void buzhash::reset() {
    state = 0;
    std::fill(buffer.begin(), buffer.end(), 0);   //reset buf to all 0s without resizing
    bufferPos = 0;
    bufferFilled = false;
}

unsigned int buzhash::getHash() {
    return state;
}

unsigned int buzhash::getHashWindowSize() {
    return n;
}

const /*static*/ unsigned int buzhash::bytehash[256] = {

    0x2ce500a9, 0xe87b7d58, 0x962a06bb, 0x2ae6cd5b,
    0xebbe526f, 0x649fb074, 0xab3ca458, 0x732d093c,
    0x984cb38d, 0x09320fd2, 0xcfdcf19e, 0x22bcec60,
    0x5fbb0a59, 0x55a8e6b2, 0x59aed289, 0x7c0aabf9,
    0x01e7e544, 0xda961641, 0x71076ddb, 0x9b04388c,
    0xe56a9a8d, 0x952f9cff, 0xce762621, 0x67fe6d8e,
    0x056de50a, 0x1e67957e, 0x9aa9c754, 0x5a629851,
    0x9095e8d8, 0x3f5f317e, 0x92377bf3, 0xc106db94,
    0x7107eac7, 0xc714c98d, 0x44bcf843, 0xae040d53,
    0xfa2f361f, 0x1b7a9471, 0x5b140e07, 0x47d7cba6,
    0xec3523d7, 0x8db15be7, 0x28be309d, 0x48dc54f4,
    0x8de61241, 0x4add1dc3, 0x92956ce4, 0x61771c9a,
    0x906c99e0, 0x19f9f585, 0xd9767e1a, 0x7a7ad68b,
    0x5f0ae888, 0xf5270a15, 0x7ad6d55d, 0xaffec476,
    0xbb507688, 0x8e514a6d, 0x47f35e87, 0xd88c916b,
    0xb24dcdf8, 0x0da7a2ac, 0xc128e78d, 0xf708f5dc,
    0x82e5d37e, 0x85bb9296, 0xe0cd290c, 0xae9579c0,
    0x0b6ef49c, 0x827bf8f4, 0x9228f233, 0xaedc02c6,
    0x7361bade, 0x461b7c03, 0xea84fa7d, 0x6b58c278,
    0x8c852dc4, 0xa24979fe, 0x827d45e1, 0x93e0a3a9,
    0x9e9563d0, 0x1feae878, 0x21074d0a, 0x53aa401f,
    0xb7bb907b, 0x176308f0, 0xc9b97562, 0x3253fadc,
    0xdb01ff0e, 0x496fc0bc, 0x2aabeec5, 0xb56c7299,
    0xa335dcab, 0x0cde0647, 0x818e8012, 0xaca82273,
    0xc157a5a4, 0xa3c591a6, 0x2b4d1304, 0x95327423,
    0xb4a3ac1f, 0x42ad1dea, 0xb7576169, 0x14360b29,
    0x98a5c002, 0x20dcd620, 0xf204d994, 0x1a00b025,
    0x1474b167, 0xf24b2a96, 0x9acdbb49, 0x0bce6da6,
    0x8959c059, 0xee4e549f, 0x64206b22, 0x832ce76c,
    0x63901128, 0x02b64403, 0x8199ac5e, 0x54f6f884,
    0xc74004cf, 0xbe8f7ba3, 0xf74bd943, 0xbefa4889,
    0x4565b40f, 0x985cca23, 0x39a370c1, 0x8ab64ad0,
    0xce9d1955, 0xc12bc20e, 0x8c1aac30, 0xb8332d69,
    0xd3aa2d49, 0xb071df5d, 0x36027c47, 0x0afd98a9,
    0xd1ed3148, 0xdfd3068e, 0x847cdbc6, 0xbd165c8e,
    0x620df09e, 0x463fa7c5, 0x56bde067, 0x75a4dc34,
    0x33a3758a, 0x7514bfc0, 0x81f7b5c1, 0x00860bea,
    0x8cdfe550, 0xe7fa1586, 0x4432fba8, 0xb9fbf5f8,
    0xab5dbbb3, 0xfc5695e3, 0xf63321b5, 0xd7addab8,
    0xe94d1045, 0x35285a59, 0x1c0c7d97, 0x558283f1,
    0x7ae094d3, 0x2c55eed5, 0x8df52667, 0xd6661582,
    0xeb16324a, 0x512ff7b8, 0x75bb193e, 0x0073b3d2,
    0x34a28dc1, 0x156f0b72, 0x76a04dff, 0xd8ee951b,
    0xefc90501, 0x3d549bcf, 0x9bbcb0ef, 0x276b02dd,
    0x2583d24f, 0x01d99492, 0x1900d354, 0xac860923,
    0x2665a45b, 0x05ff74dd, 0x7a747116, 0x07825514,
    0xfbc8f199, 0x27383cf7, 0x6b29b61c, 0xf813591a,
    0x61128fea, 0x62e2d8a2, 0xad4d162a, 0xfb80234c,
    0x0c3dacbf, 0xa46d9030, 0x70bc6434, 0x15c3e792,
    0x93587817, 0xf7cfa859, 0xc55b51e5, 0xf5701574,
    0x350859b2, 0x1907f63e, 0x937bbd16, 0xc84c9f33,
    0xe21f35c2, 0x2e5130d1, 0x944c8a99, 0xfdacb64f,
    0x3685bf89, 0x0a2061a9, 0xdc91746a, 0x28fa0a8c,
    0xa040ed1c, 0x9109b224, 0x7ad00356, 0xba206d1a,
    0xcbaf338c, 0x1817ad9b, 0x560d0621, 0x16624850,
    0x3f2fce50, 0xfb60bd41, 0x1ef2d6d9, 0x00ec6323,
    0x25344935, 0x55e8d698, 0xd656a827, 0x963295b3,
    0xcd88b6f1, 0x8fae853a, 0x824c6e31, 0x5233092c,
    0xbceb1741, 0xe26dd81c, 0xc8b00ee9, 0x15ffdbce,
    0x1cab44ca, 0x1598e9a4, 0x3e325fa6, 0xc7565fd1,
    0xafbefe00, 0xe100a0e7, 0x328f6295, 0x8c5361dd,
    0x873b16c0, 0xda205989, 0xad05f250, 0xd1dec8c4,
    0xf5bc1d9b, 0x050e29f4, 0xbc0f68b8, 0x2615a149,
    0xc796d2fb, 0x286fe4ae, 0x2860fd18, 0x961174ce };
