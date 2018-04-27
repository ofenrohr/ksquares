#include <cstdio>
#include <chrono>
#include <gsl/gsl_randist.h>

int main (void) {
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
    return 0;
}