#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// convert to binary
char *binary(const char *input) {

    size_t capacity = strlen(input) * 8 + 1;

    char *binary = malloc(capacity);

    if (binary == NULL) {
        printf("memory allocation (conversion to binary)...\n");
        return NULL;
    }

    for (size_t index = 0; index < strlen(input); index++) {

        int power = 128;

        unsigned char x = (unsigned char)input[index];

        for (int bit = 0; bit < 8; bit++) {

            if (x >= power) {
                binary[index * 8 + bit] = '1';
                x -= power;
            }
            else {
                binary[index * 8 + bit] = '0';
            }

            power /= 2;
        }
    }

    binary[capacity - 1] = '\0';

    return binary;
}


// turn length to 64 bits
char *len64(unsigned len) {

    char *len64 = malloc(65);

    if (len64 == NULL) {
        return NULL;
    }

    uint64_t length = len;

    for (int i = 0; i < 64; i++) {

        if ((length >> (63 - i)) & 1) {
            len64[i] = '1';
        }
        else {
            len64[i] = '0';
        }
    }

    len64[64] = '\0';

    return len64;
}


// padding function
char *padding(char *b_input, char *len64) {

    size_t len = strlen(b_input);

    size_t padded_len =
        ((len + 1 + 64 + 511) / 512) * 512;

    char *padded = malloc(padded_len + 1);

    if (padded == NULL) {
        printf("memory allocation failed (padding process)...\n");
        return NULL;
    }

    // original message
    memcpy(padded, b_input, len);

    // append 1
    padded[len] = '1';

    // append zeros
    for (size_t i = len + 1; i < padded_len - 64; i++) {
        padded[i] = '0';
    }

    // append original length
    for (int i = 0; i < 64; i++) {
        padded[padded_len - 64 + i] = len64[i];
    }

    padded[padded_len] = '\0';

    return padded;
}


// 32 bits to uint32_t
uint32_t int32(char *bits_32) {

    uint32_t int_32 = 0;

    for (int i = 0; i < 32; i++) {

        if (bits_32[i] == '1') {
            int_32 += (uint32_t)1 << (31 - i);
        }
    }

    return int_32;
}


// extract words
uint32_t *words(char *padded) {

    size_t multiplier = strlen(padded) / 512;

    if (strlen(padded) % 512 != 0) {
        return NULL;
    }

    uint32_t *words =
        malloc(multiplier * 16 * sizeof(uint32_t));

    if (words == NULL) {
        printf("memory allocation failed (words creation)...\n");
        return NULL;
    }

    size_t block = 0;

    while (block < multiplier) {

        for (int w = 0; w < 16; w++) {

            char word[32];

            for (int i = 0; i < 32; i++) {

                word[i] =
                    padded[block * 512 + w * 32 + i];
            }

            words[block * 16 + w] = int32(word);
        }

        block++;
    }

    return words;
}


// ROTR
uint32_t ROTR(uint32_t x, int pos) {

    return (x >> pos) | (x << (32 - pos));
}


// lower sigma 0
uint32_t lsg0(uint32_t x) {

    return ROTR(x, 7)
         ^ ROTR(x, 18)
         ^ (x >> 3);
}


// lower sigma 1
uint32_t lsg1(uint32_t x) {

    return ROTR(x, 17)
         ^ ROTR(x, 19)
         ^ (x >> 10);
}


// extend the words list
uint32_t *final_words(uint32_t **words, char **padded) {

    size_t blocks = strlen(*padded) / 512;

    if (strlen(*padded) % 512 != 0) {
        printf("error occurred ...\n");
        return NULL;
    }

    uint32_t *W =
        malloc(64 * blocks * sizeof(uint32_t));

    if (W == NULL) {
        printf("memory allocation failed (wording)...\n");
        return NULL;
    }

    uint32_t *WORDS_temp = *words;

    size_t block_done = 0;

    while (block_done < blocks) {

        // W[0..15]
        for (int j = 0; j < 16; j++) {

            W[block_done * 64 + j] =
                WORDS_temp[block_done * 16 + j];
        }

        // W[16..63]
        for (int j = 16; j < 64; j++) {

            size_t offset = block_done * 64;

            W[j + offset] =
                  lsg1(W[j - 2 + offset])
                + W[j - 7 + offset]
                + lsg0(W[j - 15 + offset])
                + W[j - 16 + offset];
        }

        block_done++;
    }

    return W;
}


// Choice
uint32_t Ch(uint32_t a, uint32_t b, uint32_t c) {

    return (a & b) ^ (~a & c);
}


// Majority
uint32_t Maj(uint32_t a, uint32_t b, uint32_t c) {

    return (a & b)
         ^ (a & c)
         ^ (b & c);
}


// big sigma 0
uint32_t bsg0(uint32_t x) {

    return ROTR(x, 2)
         ^ ROTR(x, 13)
         ^ ROTR(x, 22);
}


// big sigma 1
uint32_t bsg1(uint32_t x) {

    return ROTR(x, 6)
         ^ ROTR(x, 11)
         ^ ROTR(x, 25);
}


// compression
void compression(char *hash, uint32_t *W, int rounds_num) {

    uint32_t K[64] = {

        0x428a2f98, 0x71374491,
        0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1,
        0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01,
        0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe,
        0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786,
        0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa,
        0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d,
        0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147,
        0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138,
        0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb,
        0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624,
        0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08,
        0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a,
        0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f,
        0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb,
        0xbef9a3f7, 0xc67178f2
    };


    uint32_t H0 = 0x6a09e667;
    uint32_t H1 = 0xbb67ae85;
    uint32_t H2 = 0x3c6ef372;
    uint32_t H3 = 0xa54ff53a;
    uint32_t H4 = 0x510e527f;
    uint32_t H5 = 0x9b05688c;
    uint32_t H6 = 0x1f83d9ab;
    uint32_t H7 = 0x5be0cd19;


    for (int r = 0; r < rounds_num; r++) {

        uint32_t a = H0;
        uint32_t b = H1;
        uint32_t c = H2;
        uint32_t d = H3;
        uint32_t e = H4;
        uint32_t f = H5;
        uint32_t g = H6;
        uint32_t h = H7;


        for (int i = 0; i < 64; i++) {

            uint32_t T1 =
                  h
                + bsg1(e)
                + Ch(e, f, g)
                + K[i]
                + W[64 * r + i];

            uint32_t T2 =
                  bsg0(a)
                + Maj(a, b, c);


            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }


        H0 += a;
        H1 += b;
        H2 += c;
        H3 += d;
        H4 += e;
        H5 += f;
        H6 += g;
        H7 += h;
    }


    sprintf(
        hash,
        "%08x%08x%08x%08x%08x%08x%08x%08x",
        H0, H1, H2, H3,
        H4, H5, H6, H7
    );
}


// main
int main(int argc, char **argv) {

    if (argc < 2) {

        printf("USAGE : sha256.exe STRING\n");

        return 1;
    }


    char *input = argv[1];


    char *b_input =
        binary(input);

    if (b_input == NULL) {
        return 1;
    }


    char *length64 =
        len64((unsigned)strlen(b_input));

    if (length64 == NULL) {

        free(b_input);

        return 1;
    }


    char *padded =
        padding(b_input, length64);

    if (padded == NULL) {

        free(b_input);
        free(length64);

        return 1;
    }


    uint32_t *word_list =
        words(padded);

    if (word_list == NULL) {

        free(b_input);
        free(length64);
        free(padded);

        return 1;
    }


    uint32_t *W =
        final_words(&word_list, &padded);

    if (W == NULL) {

        free(b_input);
        free(length64);
        free(padded);
        free(word_list);

        return 1;
    }


    char hash[65];


    int X =
        (int)(strlen(padded) / 512);


    compression(hash, W, X);


    printf("SHA-256 : %s\n", hash);


    free(b_input);
    free(length64);
    free(padded);
    free(word_list);
    free(W);


    return 0;
}