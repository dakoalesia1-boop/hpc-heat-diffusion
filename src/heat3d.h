/*
 * heat3d.h - shared core for every version of the 3D heat diffusion solver.
 *
 * All versions (serial, OpenMP, MPI, hybrid, GPU) use:
 *   - the same grid layout (x fastest, then y, then z),
 *   - the same initial condition (hot cube in the centre, 0 everywhere else),
 *   - the same fixed boundaries (outer faces stay at 0),
 *   - the same 7-point stencil update,
 *   - the same command line:  ./heat_<version> nx ny nz steps [alpha] [dumpfile]
 *   - the same RESULT line on stdout (read by scripts/collect.py).
 */
#ifndef HEAT3D_H
#define HEAT3D_H

#include <stddef.h>

/* Default problem: 128^3 cells, 100 steps, alpha = 0.1 (stable while alpha <= 1/6). */
#define HEAT_DEFAULT_N     128
#define HEAT_DEFAULT_STEPS 100
#define HEAT_DEFAULT_ALPHA 0.1
#define HEAT_HOT_VALUE     100.0

typedef struct {
    int nx, ny, nz;      /* global grid size, including the fixed boundary layer */
    int steps;           /* number of time steps */
    double alpha;        /* diffusion number  D*dt/dx^2, must be <= 1/6 */
    const char *dump;    /* optional file for the final field (NULL = no dump) */
} heat_params;

/* Linear index of cell (x, y, z) in an array with row length nx and plane size nx*ny. */
static inline size_t heat_idx(int x, int y, int z, int nx, int ny) {
    return ((size_t)z * (size_t)ny + (size_t)y) * (size_t)nx + (size_t)x;
}

/* Parse "nx ny nz steps [alpha] [dumpfile]". Returns 0 on success, prints usage and returns 1 on error. */
int heat_parse_args(int argc, char **argv, heat_params *p);

/* Initial value of global cell (x, y, z): HEAT_HOT_VALUE inside a centred cube of side n/4, else 0. */
double heat_initial_value(int x, int y, int z, int nx, int ny, int nz);

/*
 * Fill planes [zlo, zhi) of a local array with the initial condition.
 * The local array holds planes whose global z index is  z_offset + local_z.
 * Serial/OpenMP: z_offset = 0. MPI: z_offset = first owned global plane - 1 (because of the ghost plane).
 */
void heat_init_planes(double *u, int nx, int ny, int nz_global, int zlo, int zhi, int z_offset);

/*
 * One stencil update for local planes [zlo, zhi): out = in + alpha * (sum of 6 neighbours - 6*in).
 * Only interior x and y cells are written, so the x/y boundary stays at 0.
 * The caller chooses zlo/zhi so that the global z boundary planes are never written.
 */
void heat_step_planes(const double *in, double *out, int nx, int ny, int zlo, int zhi, double alpha);

/* Sum of all values in planes [zlo, zhi). */
double heat_sum_planes(const double *u, int nx, int ny, int zlo, int zhi);

/* Wall-clock time in seconds (monotonic clock). */
double heat_wtime(void);

/* Number of Slurm nodes in this job (SLURM_NNODES), or 1 outside Slurm. */
int heat_slurm_nodes(void);

/* Write nx*ny*nz doubles to a raw binary file plus a small .vtk header next to it for ParaView. */
int heat_write_field(const char *path, const double *u, int nx, int ny, int nz);

/*
 * Print the one machine-readable line every version must print:
 * RESULT,version,nx,ny,nz,steps,nodes,ranks,threads,gpus,total_s,compute_s,comm_s,checksum,center,mlups
 * mlups = million cell updates per second (interior cells x steps / total_s / 1e6).
 */
void heat_print_result(const char *version, const heat_params *p, int nodes, int ranks,
                       int threads, int gpus, double total_s, double compute_s, double comm_s,
                       double checksum, double center);

#endif /* HEAT3D_H */
