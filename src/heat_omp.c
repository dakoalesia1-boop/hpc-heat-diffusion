/*
 * heat_omp.c - shared-memory version (one node, many threads).
 *
 * Parallelization: the z and y loops are split across threads (collapse(2));
 * the x loop stays contiguous so the compiler can vectorize it.
 * Arrays are initialized in parallel with the same schedule ("first touch"),
 * so each thread's part of memory sits on the socket that later uses it.
 *
 * Thread count: OMP_NUM_THREADS. Pinning: OMP_PLACES / OMP_PROC_BIND (set in the Slurm script).
 */
#include "heat3d.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

static void init_parallel(double *u, const heat_params *p) {
    #pragma omp parallel for collapse(2) schedule(static)
    for (int z = 0; z < p->nz; ++z)
        for (int y = 0; y < p->ny; ++y)
            for (int x = 0; x < p->nx; ++x) {
                double v = 0.0;
                if (z > 0 && z < p->nz - 1 && y > 0 && y < p->ny - 1 && x > 0 && x < p->nx - 1)
                    v = heat_initial_value(x, y, z, p->nx, p->ny, p->nz);
                u[heat_idx(x, y, z, p->nx, p->ny)] = v;
            }
}

static void step_parallel(const double *restrict in, double *restrict out, int nx, int ny, int nz, double alpha) {
    const size_t sy = (size_t)nx, sz = (size_t)nx * (size_t)ny;
    #pragma omp parallel for collapse(2) schedule(static)
    for (int z = 1; z < nz - 1; ++z)
        for (int y = 1; y < ny - 1; ++y) {
            size_t row = heat_idx(0, y, z, nx, ny);
            #pragma omp simd
            for (int x = 1; x < nx - 1; ++x) {
                size_t q = row + (size_t)x;
                double c = in[q];
                double nb = in[q - 1] + in[q + 1] + in[q - sy] + in[q + sy] + in[q - sz] + in[q + sz];
                out[q] = c + alpha * (nb - 6.0 * c);
            }
        }
}

int main(int argc, char **argv) {
    heat_params p;
    if (heat_parse_args(argc, argv, &p)) return EXIT_FAILURE;

    size_t n = (size_t)p.nx * p.ny * p.nz;
    double *a = malloc(n * sizeof(double));
    double *b = malloc(n * sizeof(double));
    if (!a || !b) { fprintf(stderr, "Cannot allocate %zu cells\n", n); return EXIT_FAILURE; }

    init_parallel(a, &p);
    init_parallel(b, &p);

    double t0 = omp_get_wtime();
    for (int s = 0; s < p.steps; ++s) {
        step_parallel(a, b, p.nx, p.ny, p.nz, p.alpha);
        double *t = a; a = b; b = t;
    }
    double t1 = omp_get_wtime();

    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (size_t i = 0; i < n; ++i) sum += a[i];

    double center = a[heat_idx(p.nx / 2, p.ny / 2, p.nz / 2, p.nx, p.ny)];
    heat_print_result("openmp", &p, heat_slurm_nodes(), 1, omp_get_max_threads(), 0,
                      t1 - t0, t1 - t0, 0.0, sum, center);

    if (p.dump && heat_write_field(p.dump, a, p.nx, p.ny, p.nz)) return EXIT_FAILURE;
    free(a);
    free(b);
    return EXIT_SUCCESS;
}
