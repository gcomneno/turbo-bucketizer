// src/main.c
#define _POSIX_C_SOURCE 200809L  // abilita clock_gettime in modo portabile

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "turbok.h"

// ---------- UTIL ----------
static void usage(const char *prog) {
    fprintf(stderr,
        "Turbo-Bucketizer — MVP\n"
        "Uso:\n"
        "  %s --ip <a.b.c.d> --k <0..32> [--a HEX] [--b HEX] [--preset NAME]\n"
        "  %s --selftest [--cidr A.B.C.D/NN] [--k <0..32>] [--sample N] [--a HEX] [--b HEX] [--preset NAME]\n"
        "  %s --bench [N] [--k <0..32>] [--preset NAME]\n"
        "\nPresets: default | wang\n"
        "\nEsempi:\n"
        "  %s --ip 192.168.0.1 --k 12 --preset default\n"
        "  %s --selftest --cidr 10.0.0.0/16 --k 12 --preset wang\n"
        "  %s --bench 20000000 --k 12 --preset wang\n",
        prog, prog, prog, prog, prog, prog);
}

static bool parse_hex32(const char *s, uint32_t *out) {
    char *end = NULL;
    unsigned long v = strtoul(s, &end, 16);
    if (!s[0] || *end != '\0' || v > 0xFFFFFFFFul) return false;
    *out = (uint32_t)v;
    return true;
}

static inline double dabs(double x) { return x < 0.0 ? -x : x; }

// ---------- PRESETS ----------
static const char *apply_preset(const char *name) {
    turbok_params_t p = turbok_get_params();
    if (!name) return "custom";

    if (strcmp(name, "default")==0) {
        p.a = 0x9E3779B1u; // 2654435761 (Knuth-ish)
        p.b = 0x85EBCA77u; // murmur-ish offset (dispari)
        turbok_set_params(p);
        return "default";
    } else if (strcmp(name, "wang")==0) {
        p.a = 0x27D4EB2Du; // Thomas Wang mix
        p.b = 0x165667B1u;
        turbok_set_params(p);
        return "wang";
    }
    return "custom";
}

// ---------- CIDR & SELFTEST ----------
static int parse_cidr(const char *s, uint32_t *base_host, uint8_t *prefix) {
    // formato: A.B.C.D/NN
    char buf[64];
    size_t n = strlen(s);
    if (n >= sizeof(buf)) return -1;
    memcpy(buf, s, n + 1);

    char *slash = strchr(buf, '/');
    if (!slash) return -1;
    *slash = '\0';
    const char *pfx = slash + 1;

    uint32_t ip_h;
    if (ip4_parse(buf, &ip_h) != 0) return -1;

    char *end = NULL;
    long pr = strtol(pfx, &end, 10);
    if (*end!='\0' || pr<0 || pr>32) return -1;

    uint32_t mask = (pr==0) ? 0u : (~0u << (32 - pr));
    *base_host = ip_h & mask;
    *prefix = (uint8_t)pr;
    return 0;
}

static int selftest_run(const char *cidr_opt, int k, const char *preset_name, uint64_t sample_N) {
    uint32_t base_h; uint8_t prefix;
    if (!cidr_opt) cidr_opt = "10.0.0.0/16";
    if (k < 0) k = 12;

    const char *used = apply_preset(preset_name);

    if (parse_cidr(cidr_opt, &base_h, &prefix) != 0) {
        fprintf(stderr, "CIDR non valido: %s\n", cidr_opt);
        return 2;
    }

    uint64_t total = (prefix==32) ? 1ull : (1ull << (32 - prefix));

    // selezione campioni
    const uint64_t HARD_CAP = (1ull<<20); // 1.048.576
    uint64_t samples = total;
    if (sample_N > 0 && sample_N < total) samples = sample_N;
    else if (total > HARD_CAP) samples = HARD_CAP;

    uint32_t B = (k <= 0) ? 1u : (1u << k);
    uint64_t *hist = (uint64_t*)calloc(B, sizeof(uint64_t));
    if (!hist) { fprintf(stderr, "alloc fallita\n"); return 4; }

    // stride uniforme
    uint64_t step = (samples == 0) ? 1 : ( (total + samples - 1) / samples );
    if (step == 0) step = 1;

    uint64_t taken = 0;
    for (uint64_t i=0; i<total && taken<samples; i += step, ++taken) {
        uint32_t ip = (uint32_t)(base_h + (i & 0xFFFFFFFFull));
        uint32_t bkt = turbok_bucketize(ip, (uint8_t)k);
        if (k>0) hist[bkt]++; else hist[0]++;
    }

    double exp = (double)taken / (double)B;
    double chi2 = 0.0, max_dev_pct = 0.0, mad_sum = 0.0;

    for (uint32_t b=0; b<B; ++b) {
        double obs = (double)hist[b];
        double diff = obs - exp;
        if (exp > 0.0) chi2 += (diff*diff)/exp;
        double dev_pct = (exp > 0.0) ? (dabs(diff) * 100.0 / exp) : 0.0;
        if (dev_pct > max_dev_pct) max_dev_pct = dev_pct;
        mad_sum += dabs(diff);
    }
    free(hist);

    double mad_pct = (exp > 0.0) ? ((mad_sum / B) * 100.0 / exp) : 0.0;
    double uniformity = 100.0 - mad_pct;

    turbok_params_t P = turbok_get_params();
    printf("Self-test Turbo-Bucketizer\n");
    printf("preset=%s  a=0x%08X  b=0x%08X\n", used, P.a, P.b);
    printf("CIDR=%s  k=%d  buckets=%u  total=%llu  samples=%llu  step=%llu\n",
           cidr_opt, k, B,
           (unsigned long long)total,
           (unsigned long long)taken,
           (unsigned long long)step);
    printf("expected_per_bucket=%.2f\n", exp);
    printf("chi2=%.3f  max_deviation=%.2f%%  MAD=%.2f%%  uniformity≈%.3f%%\n",
           chi2, max_dev_pct, mad_pct, uniformity);

    return 0;
}

// ---------- BENCH ----------
static inline uint64_t now_ns(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec*1000000000ull + (uint64_t)ts.tv_nsec;
}

static int bench_run(uint64_t N, int k, const char *preset_name) {
    if (k < 0) k = 12;
    const char *used = apply_preset(preset_name);

    uint32_t acc = 0;
    uint32_t base = 0x0A000000u; // 10.0.0.0
    uint64_t t0 = now_ns();
    for (uint64_t i=0; i<N; ++i) {
        uint32_t ip = base + (uint32_t)i;
        acc ^= turbok_bucketize(ip, (uint8_t)k);
    }
    uint64_t t1 = now_ns();
    double sec = (t1 - t0) / 1e9;

    turbok_params_t P = turbok_get_params();
    printf("Benchmark Turbo-Bucketizer\n");
    printf("preset=%s  a=0x%08X  b=0x%08X  k=%d\n", used, P.a, P.b, k);
    printf("N=%llu  time=%.3fs  rate=%.3f Mops/s  checksum=%u\n",
           (unsigned long long)N, sec, (N/1e6)/sec, acc);
    return 0;
}

// ---------- MAIN ----------
int main(int argc, char **argv) {
    const char *ip_str = NULL;
    int k = -1;

    uint32_t a=0, b=0;
    bool have_a=false, have_b=false;

    bool do_selftest = false;
    const char *cidr_str = NULL;
    uint64_t sample_N = 0;

    bool do_bench = false;
    uint64_t bench_N = 20000000ull; // default 20M

    const char *preset = NULL;

    for (int i=1; i<argc; ++i) {
        if (!strcmp(argv[i], "--ip") && i+1<argc) ip_str = argv[++i];
        else if (!strcmp(argv[i], "--k") && i+1<argc) k = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--a") && i+1<argc) { have_a = parse_hex32(argv[++i], &a); }
        else if (!strcmp(argv[i], "--b") && i+1<argc) { have_b = parse_hex32(argv[++i], &b); }
        else if (!strcmp(argv[i], "--preset") && i+1<argc) { preset = argv[++i]; }
        else if (!strcmp(argv[i], "--selftest")) { do_selftest = true; }
        else if (!strcmp(argv[i], "--cidr") && i+1<argc) { cidr_str = argv[++i]; }
        else if (!strcmp(argv[i], "--sample") && i+1<argc) { sample_N = strtoull(argv[++i], NULL, 10); }
        else if (!strcmp(argv[i], "--bench")) {
            do_bench = true;
            if (i+1<argc && argv[i+1][0] != '-') {
                bench_N = strtoull(argv[++i], NULL, 10);
                if (bench_N == 0) bench_N = 20000000ull;
            }
        }
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) { usage(argv[0]); return 0; }
        else { fprintf(stderr, "Argomento sconosciuto: %s\n", argv[i]); usage(argv[0]); return 1; }
    }

    // preset e (a,b): (a,b) espliciti battono preset
    if (preset) (void)apply_preset(preset);
    if (have_a || have_b) {
        turbok_params_t p = turbok_get_params();
        if (have_a) p.a = a;
        if (have_b) p.b = b;
        if ((p.a & 1u) == 0u) {
            fprintf(stderr, "[warn] 'a' non è dispari: la permutazione su 2^32 potrebbe non essere invertibile.\n");
        }
        turbok_set_params(p);
    }

    if (do_selftest) {
        return selftest_run(cidr_str, k, preset, sample_N);
    }
    if (do_bench) {
        return bench_run(bench_N, k, preset);
    }

    // Modalità normale
    if (!ip_str || k<0 || k>32) {
        usage(argv[0]);
        return 1;
    }

    uint32_t ip_host;
    if (ip4_parse(ip_str, &ip_host) != 0) {
        fprintf(stderr, "IP non valido: %s\n", ip_str);
        return 2;
    }

    uint32_t bucket = turbok_bucketize(ip_host, (uint8_t)k);
    printf("IP=%s  k=%d  bucket=%u\n", ip_str, k, bucket);
    return 0;
}
