/*
 * heat_mpi.c - distributed-memory version (many processes, one or more nodes).
 *
 * Decomposition: the grid is cut into slabs along z. Rank r owns a contiguous
 * block of z-planes. Its local array has one extra "ghost" plane below and above.
 *
 *   local plane 0          ghost (copy of the last plane of rank r-1)
 *   local planes 1..nloc   planes owned by this rank
 *   local plane nloc+1     ghost (copy of the first plane of rank r+1)
 *
 * Every step: (1) halo exchange with MPI_Sendrecv to the ranks above and below,
 * (2) stencil update of the owned planes. Each z-plane is contiguous in memory,
 * so no derived MPI datatypes are needed.
 *
 * Timing: compute and communication are timed separately; the RESULT line
 * reports the maximum over all ranks.
 */
#include "heat3d.h"

#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    heat_params p;
    if (heat_parse_args(argc, argv, &p)) { MPI_Abort(MPI_COMM_WORLD, 1); }
    if (size > p.nz) {
        if (rank == 0) fprintf(stderr, "Need at least one z-plane per rank (nz=%d, ranks=%d)\n", p.nz, size);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* Slab decomposition along z: the first (nz % size) ranks get one extra plane. */
    int base = p.nz / size, rem = p.nz % size;
    int nloc = base + (rank < rem ? 1 : 0);
    int gz0 = rank * base + (rank < rem ? rank : rem); /* first owned global plane */

    const size_t plane = (size_t)p.nx * p.ny;
    double *a = malloc((size_t)(nloc + 2) * plane * sizeof(double));
    double *b = malloc((size_t)(nloc + 2) * plane * sizeof(double));
    if (!a || !b) { fprintf(stderr, "rank %d: allocation failed\n", rank); MPI_Abort(MPI_COMM_WORLD, 1); }

    /* Local plane l holds global plane l + gz0 - 1 (ghosts included, they start at 0 and are overwritten). */
    heat_init_planes(a, p.nx, p.ny, p.nz, 0, nloc + 2, gz0 - 1);
    heat_init_planes(b, p.nx, p.ny, p.nz, 0, nloc + 2, gz0 - 1);

    /* Owned planes that are not on the global z boundary get updated. */
    int zlo = (2 - gz0) > 1 ? (2 - gz0) : 1;
    int zhi = (p.nz - gz0) < (nloc + 1) ? (p.nz - gz0) : (nloc + 1);

    int down = rank > 0 ? rank - 1 : MPI_PROC_NULL;
    int up = rank < size - 1 ? rank + 1 : MPI_PROC_NULL;
    int count = (int)plane;

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime(), tcomm = 0.0;
    for (int s = 0; s < p.steps; ++s) {
        double c0 = MPI_Wtime();
        /* Send my top owned plane up, receive the lower neighbour's top plane into my bottom ghost. */
        MPI_Sendrecv(a + (size_t)nloc * plane, count, MPI_DOUBLE, up, 0,
                     a, count, MPI_DOUBLE, down, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        /* Send my bottom owned plane down, receive the upper neighbour's bottom plane into my top ghost. */
        MPI_Sendrecv(a + plane, count, MPI_DOUBLE, down, 1,
                     a + (size_t)(nloc + 1) * plane, count, MPI_DOUBLE, up, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        tcomm += MPI_Wtime() - c0;

        heat_step_planes(a, b, p.nx, p.ny, zlo, zhi, p.alpha);
        double *t = a; a = b; b = t;
    }
    double ttotal = MPI_Wtime() - t0;
    double tcompute = ttotal - tcomm;

    /* Checksum over owned planes, and the centre value from whichever rank owns it. */
    double lsum = heat_sum_planes(a, p.nx, p.ny, 1, nloc + 1), gsum = 0.0;
    MPI_Reduce(&lsum, &gsum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    int cz = p.nz / 2;
    double lcen = (cz >= gz0 && cz < gz0 + nloc) ? a[heat_idx(p.nx / 2, p.ny / 2, cz - gz0 + 1, p.nx, p.ny)] : 0.0;
    double gcen = 0.0;
    MPI_Reduce(&lcen, &gcen, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double times[3] = {ttotal, tcompute, tcomm}, tmax[3];
    MPI_Reduce(times, tmax, 3, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0)
        heat_print_result("mpi", &p, heat_slurm_nodes(), size, 1, 0, tmax[0], tmax[1], tmax[2], gsum, gcen);

    /* Optional dump: gather all owned planes on rank 0 (small grids only). */
    if (p.dump) {
        size_t ntot = plane * (size_t)p.nz;
        if (ntot > (size_t)INT_MAX) {
            if (rank == 0) fprintf(stderr, "Grid too large to dump with MPI_Gatherv; skipping dump\n");
        } else {
            int *counts = NULL, *displs = NULL;
            double *full = NULL;
            if (rank == 0) {
                counts = malloc(size * sizeof(int));
                displs = malloc(size * sizeof(int));
                full = malloc(ntot * sizeof(double));
                for (int r = 0; r < size; ++r) {
                    int nl = base + (r < rem ? 1 : 0);
                    int g0 = r * base + (r < rem ? r : rem);
                    counts[r] = nl * count;
                    displs[r] = g0 * count;
                }
            }
            MPI_Gatherv(a + plane, nloc * count, MPI_DOUBLE, full, counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            if (rank == 0) {
                heat_write_field(p.dump, full, p.nx, p.ny, p.nz);
                free(full); free(counts); free(displs);
            }
        }
    }

    free(a);
    free(b);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
