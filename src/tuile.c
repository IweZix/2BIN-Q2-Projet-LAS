#include <stdio.h>
#include <stdlib.h>

#include "utils_v1.h"
#include "header.h"

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

void sortPlayersByScore(Player *players, int nbPlayers) {
    for (int i = 0; i < nbPlayers; i++) {
        for (int j = i + 1; j < nbPlayers; j++) {
            if (players[i].score < players[j].score) {
                Player temp = players[i];
                players[i] = players[j];
                players[j] = temp;
            }
        }
    }
}

void printPlateau(int *plateau) {
    for (int i = 0; i < 20; i++) {
        printf("%d ", plateau[i]);
    }
    printf("\n");
}

int calculeScore(int *plateau) {
    int nb_suites = 0; 
    int suites_par_longueur[21] = {0};

    int i = 0;
    
    while (i < 20) {

        int debut_suite = i;

        while (i < 19 && (plateau[i] <= plateau[i+1] || (plateau[i] == 31 && plateau[i+1] > plateau[i-1]) || plateau[i+1] == 31)) {
            i++;
        }

        int fin_suite = i;

        int longueur_suite = fin_suite - debut_suite + 1;

        if (longueur_suite >= 2) {
            nb_suites++;
            suites_par_longueur[longueur_suite]++;
        }
        i++;
    }


    int scores[] = {0, 1, 3, 5, 7, 9, 11, 15, 20, 25, 30, 35, 40, 50, 60, 70, 85, 100, 150, 300};


    int score_total = 0;
    for (int k = 2; k < 21; k++) {
        score_total += suites_par_longueur[k] * scores[k-1];
    }

    return score_total;
}