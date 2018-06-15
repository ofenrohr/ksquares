#include <cstdio>
#include <chrono>
#include <gsl/gsl_randist.h>
#include <vector>
#include <iostream>
#include <fstream>

#include "GSLTest.h"

void GSLTest::testGSL001() {
    size_t K = 20;
    auto alpha = new double[K]();
    auto theta = new double[K]();
    for (int i = 0; i < K; i++) {
        alpha[i] = 0.1;
        //theta[i] = 0.0;
    }
    auto rng = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    gsl_ran_dirichlet(rng, K, alpha, theta);
    for (int i = 0; i < K; i++) {
        printf("%.5f ", theta[i]);
    }
    printf("\n");
    gsl_rng_free(rng);
}

void GSLTest::testGSL002() {
    auto rng = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    int binsCnt = 10;
    std::vector<int> bins;
    bins.reserve(binsCnt);
    for (int i = 0; i < binsCnt; i++) {
        bins[i] = 0;
    }
    for (int i = 0; i < 10000; i++) {
        int movesLeft = gsl_ran_gaussian(rng, ((double)binsCnt)/8.0) + binsCnt/2;
        if (movesLeft < 0) {
            printf("lower cutoff\n");
            movesLeft = 0;
        }
        if (movesLeft >= binsCnt) {
            printf("upper cutoff\n");
            movesLeft = binsCnt-1;
        }
        bins[movesLeft]++;
    }
    printf("distribution:\n");
    for (int i = 0; i < binsCnt; i++) {
        printf("%5d ", bins[i]);
    }
    printf("\n");
    gsl_rng_free(rng);
}

void GSLTest::testGSL003() {
    auto rng = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    int binsCnt = 24; // 3x3 boxes
    std::vector<int> bins;
    bins.reserve(binsCnt);
    for (int i = 0; i < binsCnt; i++) {
        bins[i] = 0;
    }
    for (int i = 0; i < 1000000; i++) {
        int movesLeft = gsl_ran_gaussian(rng, ((double)binsCnt)/8.0) + binsCnt/2;
        if (movesLeft < 0) {
            printf("lower cutoff\n");
            movesLeft = 0;
        }
        if (movesLeft >= binsCnt) {
            printf("upper cutoff\n");
            movesLeft = binsCnt-1;
        }
        bins[movesLeft]++;
    }
    for (int i = 0; i < binsCnt; i++) {
        printf("%d %d\n", i, bins[i]);
    }
    printf("\n");
    gsl_rng_free(rng);
}


void GSLTest::testGSL004() {
    auto rng = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    int binsCnt = 24; // 3x3 boxes
    std::vector<int> bins;
    bins.reserve(binsCnt);
    for (int i = 0; i < binsCnt; i++) {
        bins[i] = 0;
    }
    double sigmaScale = 1.0/5.0;
    double meanScale = 0.6;
    for (int i = 0; i < 1000000; i++) {
        bool redo;
        int movesLeft;
        do {
            redo = false;
            movesLeft = gsl_ran_gaussian(rng, (double) binsCnt * sigmaScale) + (double) binsCnt * meanScale;
            if (movesLeft <= 0) {
                redo = true;
            }
            if (movesLeft >= binsCnt) {
                redo = true;
            }
        } while (redo);
        bins[movesLeft]++;
    }
    for (int i = 0; i < binsCnt; i++) {
        printf("%d %d\n", i, bins[i]);
    }
    printf("\n");
    gsl_rng_free(rng);
}


void GSLTest::testGSL005() {
    auto rng = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    int binsCnt = 220; // 10x10 boxes
    std::vector<int> bins;
    bins.reserve(binsCnt);
    for (int i = 0; i < binsCnt; i++) {
        bins[i] = 0;
    }
    double sigmaScale = 1.0/5.0;
    double meanScale = 0.6;
    for (int i = 0; i < 1000000; i++) {
        bool redo;
        int movesLeft;
        do {
            redo = false;
            movesLeft = gsl_ran_gaussian(rng, (double) binsCnt * sigmaScale) + (double) binsCnt * meanScale;
            if (movesLeft <= 0) {
                redo = true;
            }
            if (movesLeft >= binsCnt) {
                redo = true;
            }
        } while (redo);
        bins[movesLeft]++;
    }
    std::ofstream file;
    file.open("results/movesLeftDist3.txt");
    for (int i = 0; i < binsCnt; i++) {
        file << i << " " << bins[i] << std::endl;
    }
    file.close();
    gsl_rng_free(rng);
}