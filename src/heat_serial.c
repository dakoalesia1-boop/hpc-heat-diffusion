/* heat_serial.c - reference version: one core, no parallelism. Every other version must match it. */
#include "heat3d.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    heat_params p;
    if (heat_parse_args(argc, argv, &p)) return EXIT_FAILURE;

    size_t n = (size_t)p.nx * p.ny * p.nz;
    double *a = malloc(n * sizeof(double));
    double *b = malloc(n * sizeof(double));
    if (!a || !b) { fprintf(stderr, "Cannot allocate %zu cells\n", n); return EXIT_FAILURE; }

    heat_init_planes(a, p.nx, p.ny, p.nz, 0, p.nz, 0);
    heat_init_planes(b, p.nx, p.ny, p.nz, 0, p.nz, 0);

    double t0 = heat_wtime();
    for (int s = 0; s < p.steps; ++s) {
        heat_step_planes(a, b, p.nx, p.ny, 1, p.nz - 1, p.alpha);
        double *t = a; a = b; b = t;
    }
    double t1 = heat_wtime();

    double sum = heat_sum_planes(a, p.nx, p.ny, 0, p.nz);
    double center = a[heat_idx(p.nx / 2, p.ny / 2, p.nz / 2, p.nx, p.ny)];
    heat_print_result("serial", &p, heat_slurm_nodes(), 1, 1, 0, t1 - t0, t1 - t0, 0.0, sum, center);

    if (p.dump && heat_write_field(p.dump, a, p.nx, p.ny, p.nz)) return EXIT_FAILURE;
    free(a);
    free(b);
    return EXIT_SUCCESS;
}
