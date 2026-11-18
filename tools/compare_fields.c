/*
 * compare_fields.c - check that two dumped temperature fields are the same.
 * Usage: ./bin/compare_fields reference.raw other.raw [tolerance]
 * Prints the maximum absolute difference; exit code 0 = PASS, 1 = FAIL.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "Usage: %s reference.raw other.raw [tolerance]\n", argv[0]); return 2; }
    double tol = argc > 3 ? atof(argv[3]) : 1e-10;
    FILE *fa = fopen(argv[1], "rb"), *fb = fopen(argv[2], "rb");
    if (!fa || !fb) { perror("open"); return 2; }

    double a, b, maxdiff = 0.0, maxref = 0.0;
    size_t n = 0;
    while (fread(&a, sizeof a, 1, fa) == 1) {
        if (fread(&b, sizeof b, 1, fb) != 1) { fprintf(stderr, "FAIL: %s is shorter\n", argv[2]); return 1; }
        double d = fabs(a - b);
        if (d > maxdiff) maxdiff = d;
        if (fabs(a) > maxref) maxref = fabs(a);
        ++n;
    }
    if (fread(&b, sizeof b, 1, fb) == 1) { fprintf(stderr, "FAIL: %s is longer\n", argv[2]); return 1; }
    fclose(fa); fclose(fb);

    int ok = maxdiff <= tol;
    printf("%s  cells=%zu  max_abs_diff=%.3e  (tolerance %.1e, max value %.3f)  %s vs %s\n",
           ok ? "PASS" : "FAIL", n, maxdiff, tol, maxref, argv[1], argv[2]);
    return ok ? 0 : 1;
}
