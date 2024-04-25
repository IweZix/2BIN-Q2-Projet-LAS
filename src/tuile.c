#include <stdio.h>
#include <stdlib.h>

#include "utils_v1.h"

int* genTuile() {
    int tuilesBase[31] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    };

    int tuilesRestantes[31] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
    };

    int *tuiles = malloc(20 * sizeof(int));

    for (int i = 0; i < 20; i++) {
        int random = randomIntBetween(0, 30);
        while (tuilesRestantes[random] == 0) {
            random = randomIntBetween(0, 30);
        }
        tuiles[i] = tuilesBase[random];
        tuilesRestantes[random]--;    
    }

    return tuiles;
}