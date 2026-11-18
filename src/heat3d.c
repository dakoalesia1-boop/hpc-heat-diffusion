/* heat3d.c - shared core used by every version. See heat3d.h for the contract. */
#define _POSIX_C_SOURCE 200809L
#include "heat3d.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int heat_parse_args(int argc, char **argv, heat_params *p) {
    p->nx = p->ny = p->nz = HEAT_DEFAULT_N;
    p->steps = HEAT_DEFAULT_STEPS;
    p->alpha = HEAT_DEFAULT_ALPHA;
    p->dump = NULL;

    if (argc >= 5) {
        p->nx = atoi(argv[1]);
        p->ny = atoi(argv[2]);
        p->nz = atoi(argv[3]);
        p->steps = atoi(argv[4]);
    } else if (argc != 1) {
        goto usage;
    }
    if (argc >= 6) p->alpha = atof(argv[5]);
    if (argc >= 7) p->dump = argv[6];

    if (p->nx < 3 || p->ny < 3 || p->nz < 3 || p->steps < 1) goto usage;
    if (p->alpha <= 0.0 || p->alpha > 1.0 / 6.0) {
        fprintf(stderr, "alpha must be in (0, 1/6] for a stable explicit 3D scheme (got %g)\n", p->alpha);
        return 1;
    }
    return 0;

usage:
    fprintf(stderr, "Usage: %s [nx ny nz steps [alpha [dumpfile]]]\n", argv[0]);
    fprintf(stderr, "  nx, ny, nz >= 3, steps >= 1, 0 < alpha <= 1/6 (default %g)\n", HEAT_DEFAULT_ALPHA);
    return 1;
}

double heat_initial_value(int x, int y, int z, int nx, int ny, int nz) {
    /* Hot cube of side n/4 (at least 1 cell) centred in the domain. */
    int hx = nx / 8 > 0 ? nx / 8 : 1;
    int hy = ny / 8 > 0 ? ny / 8 : 1;
    int hz = nz / 8 > 0 ? nz / 8 : 1;
    int cx = nx / 2, cy = ny / 2, cz = nz / 2;
    if (x >= cx - hx && x < cx + hx && y >= cy - hy && y < cy + hy && z >= cz - hz && z < cz + hz)
        return HEAT_HOT_VALUE;
    return 0.0;
}

void heat_init_planes(double *u, int nx, int ny, int nz_global, int zlo, int zhi, int z_offset) {
    for (int z = zlo; z < zhi; ++z) {
        int gz = z + z_offset;
        for (int y = 0; y < ny; ++y) {
            for (int x = 0; x < nx; ++x) {
                double v = 0.0;
                /* Outer faces are fixed at 0; only interior cells get the initial condition. */
                if (gz > 0 && gz < nz_global - 1 && y > 0 && y < ny - 1 && x > 0 && x < nx - 1)
                    v = heat_initial_value(x, y, gz, nx, ny, nz_global);
                u[heat_idx(x, y, z, nx, ny)] = v;
            }
        }
    }
}

void heat_step_planes(const double *in, double *out, int nx, int ny, int zlo, int zhi, double alpha) {
    const size_t sx = 1, sy = (size_t)nx, sz = (size_t)nx * (size_t)ny;
    for (int z = zlo; z < zhi; ++z) {
        for (int y = 1; y < ny - 1; ++y) {
            size_t row = heat_idx(0, y, z, nx, ny);
            for (int x = 1; x < nx - 1; ++x) {
                size_t p = row + (size_t)x;
                double c = in[p];
                double nb = in[p - sx] + in[p + sx] + in[p - sy] + in[p + sy] + in[p - sz] + in[p + sz];
                out[p] = c + alpha * (nb - 6.0 * c);
            }
        }
    }
}

double heat_sum_planes(const double *u, int nx, int ny, int zlo, int zhi) {
    double s = 0.0;
    size_t lo = (size_t)zlo * nx * ny, hi = (size_t)zhi * nx * ny;
    for (size_t i = lo; i < hi; ++i) s += u[i];
    return s;
}

double heat_wtime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
}

int heat_slurm_nodes(void) {
    const char *s = getenv("SLURM_NNODES");
    if (!s) s = getenv("SLURM_JOB_NUM_NODES");
    int n = s ? atoi(s) : 1;
    return n > 0 ? n : 1;
}

int heat_write_field(const char *path, const double *u, int nx, int ny, int nz) {
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); return 1; }
    size_t n = (size_t)nx * ny * nz;
    if (fwrite(u, sizeof(double), n, f) != n) { perror("fwrite"); fclose(f); return 1; }
    fclose(f);

    /* Legacy VTK header so ParaView can open the raw file: open <path>.vtk. */
    char vtk[4096];
    snprintf(vtk, sizeof vtk, "%s.vtk", path);
    FILE *h = fopen(vtk, "w");
    if (!h) return 0;
    fprintf(h, "# vtk DataFile Version 3.0\nheat3d temperature\nASCII\nDATASET STRUCTURED_POINTS\n");
    fprintf(h, "DIMENSIONS %d %d %d\nORIGIN 0 0 0\nSPACING 1 1 1\nPOINT_DATA %zu\n", nx, ny, nz, n);
    fprintf(h, "SCALARS temperature double 1\nLOOKUP_TABLE default\n");
    for (size_t i = 0; i < n; ++i) fprintf(h, "%.6g\n", u[i]);
    fclose(h);
    return 0;
}

void heat_print_result(const char *version, const heat_params *p, int nodes, int ranks,
                       int threads, int gpus, double total_s, double compute_s, double comm_s,
                       double checksum, double center) {
    double cells = (double)(p->nx - 2) * (double)(p->ny - 2) * (double)(p->nz - 2);
    double mlups = total_s > 0 ? cells * p->steps / total_s / 1e6 : 0.0;
    printf("RESULT,%s,%d,%d,%d,%d,%d,%d,%d,%d,%.6f,%.6f,%.6f,%.10e,%.10e,%.2f\n",
           version, p->nx, p->ny, p->nz, p->steps, nodes, ranks, threads, gpus,
           total_s, compute_s, comm_s, checksum, center, mlups);
    fflush(stdout);
}
