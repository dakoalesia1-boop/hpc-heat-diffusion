#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static inline size_t idx(int x, int y, int z, int nx, int ny) {
    return (size_t)z * (size_t)ny * (size_t)nx + (size_t)y * (size_t)nx + (size_t)x;
}

static double wall_time_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static void initialize(double *grid, int nx, int ny, int nz) {
    memset(grid, 0, (size_t)nx * (size_t)ny * (size_t)nz * sizeof(double));

    int cx = nx / 2;
    int cy = ny / 2;
    int cz = nz / 2;

    grid[idx(cx, cy, cz, nx, ny)] = 100.0;

    if (cx > 0) grid[idx(cx - 1, cy, cz, nx, ny)] = 50.0;
    if (cx + 1 < nx) grid[idx(cx + 1, cy, cz, nx, ny)] = 50.0;
    if (cy > 0) grid[idx(cx, cy - 1, cz, nx, ny)] = 50.0;
    if (cy + 1 < ny) grid[idx(cx, cy + 1, cz, nx, ny)] = 50.0;
    if (cz > 0) grid[idx(cx, cy, cz - 1, nx, ny)] = 50.0;
    if (cz + 1 < nz) grid[idx(cx, cy, cz + 1, nx, ny)] = 50.0;
}

static void step_heat(const double *in, double *out, int nx, int ny, int nz, double alpha) {
    for (int z = 1; z < nz - 1; ++z) {
        for (int y = 1; y < ny - 1; ++y) {
            for (int x = 1; x < nx - 1; ++x) {
                size_t p = idx(x, y, z, nx, ny);
                double center = in[p];
                double sum_neighbors =
                    in[idx(x - 1, y, z, nx, ny)] + in[idx(x + 1, y, z, nx, ny)] +
                    in[idx(x, y - 1, z, nx, ny)] + in[idx(x, y + 1, z, nx, ny)] +
                    in[idx(x, y, z - 1, nx, ny)] + in[idx(x, y, z + 1, nx, ny)];

                out[p] = center + alpha * (sum_neighbors - 6.0 * center);
            }
        }
    }

    for (int z = 0; z < nz; ++z) {
        for (int y = 0; y < ny; ++y) {
            out[idx(0, y, z, nx, ny)] = 0.0;
            out[idx(nx - 1, y, z, nx, ny)] = 0.0;
        }
    }

    for (int z = 0; z < nz; ++z) {
        for (int x = 0; x < nx; ++x) {
            out[idx(x, 0, z, nx, ny)] = 0.0;
            out[idx(x, ny - 1, z, nx, ny)] = 0.0;
        }
    }

    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            out[idx(x, y, 0, nx, ny)] = 0.0;
            out[idx(x, y, nz - 1, nx, ny)] = 0.0;
        }
    }
}

static double checksum(const double *grid, int nx, int ny, int nz) {
    double sum = 0.0;
    size_t total = (size_t)nx * (size_t)ny * (size_t)nz;
    for (size_t i = 0; i < total; ++i) {
        sum += grid[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int nx = 128, ny = 128, nz = 128;
    int steps = 100;
    double alpha = 0.1;

    if (argc >= 5) {
        nx = atoi(argv[1]);
        ny = atoi(argv[2]);
        nz = atoi(argv[3]);
        steps = atoi(argv[4]);
    }
    if (argc >= 6) {
        alpha = atof(argv[5]);
    }

    if (nx < 3 || ny < 3 || nz < 3 || steps < 1) {
        fprintf(stderr, "Usage: %s [nx ny nz steps [alpha]]\n", argv[0]);
        fprintf(stderr, "All dimensions must be at least 3, steps must be >= 1.\n");
        return EXIT_FAILURE;
    }

    size_t total_cells = (size_t)nx * (size_t)ny * (size_t)nz;
    double *a = (double *)malloc(total_cells * sizeof(double));
    double *b = (double *)malloc(total_cells * sizeof(double));

    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed for %zu cells.\n", total_cells);
        free(a);
        free(b);
        return EXIT_FAILURE;
    }

    initialize(a, nx, ny, nz);
    initialize(b, nx, ny, nz);

    double t0 = wall_time_seconds();

    double *in = a;
    double *out = b;

    for (int s = 0; s < steps; ++s) {
        step_heat(in, out, nx, ny, nz, alpha);
        double *tmp = in;
        in = out;
        out = tmp;
    }

    double t1 = wall_time_seconds();

    int cx = nx / 2;
    int cy = ny / 2;
    int cz = nz / 2;

    printf("3D heat diffusion finished\n");
    printf("Grid: %d x %d x %d\n", nx, ny, nz);
    printf("Steps: %d\n", steps);
    printf("Alpha: %.6f\n", alpha);
    printf("Final center temperature: %.6f\n", in[idx(cx, cy, cz, nx, ny)]);
    printf("Checksum: %.6f\n", checksum(in, nx, ny, nz));
    printf("Runtime: %.6f seconds\n", t1 - t0);

    free(a);
    free(b);
    return EXIT_SUCCESS;
}