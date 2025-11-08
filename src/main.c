/* Turbo-Bucketizer (bucketizzatore)                */
/* Portabile, C standard, senza dipendenze esterne. */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "tb_export.h"

#if defined(_WIN32) || defined(_WIN64)
    #define strcasecmp _stricmp
#endif

/* ===================== Utilità ===================== */

static bool parse_u32_hexdec(const char *s, uint32_t *out) {
    if(!s || !*s || !out) return false;
    char *end = NULL;
    unsigned long v = 0;
    int base = 10;
    if((s[0]=='0' && (s[1]=='x' || s[1]=='X')) || strchr(s,'A') || strchr(s,'a') || strchr(s,'B') || strchr(s,'b') || strchr(s,'C') || strchr(s,'c') || strchr(s,'D') || strchr(s,'d') || strchr(s,'E') || strchr(s,'e') || strchr(s,'F') || strchr(s,'f'))
        base = 16;
    v = strtoul(s, &end, base);
    if(end==s || *end!='\0') return false;
    *out = (uint32_t)(v & 0xFFFFFFFFu);
    return true;
}

static bool parse_ipv4(const char *s, uint32_t *out) {
    unsigned int a,b,c,d;
    if(!s || !out) return false;
    if(sscanf(s, "%u.%u.%u.%u", &a,&b,&c,&d)!=4) return false;
    if(a>255 || b>255 || c>255 || d>255) return false;
    *out = ((uint32_t)a<<24) | ((uint32_t)b<<16) | ((uint32_t)c<<8) | (uint32_t)d;
    return true;
}

static void ipv4_to_str(uint32_t x, char buf[16]) {
    unsigned int a = (x>>24)&255, b=(x>>16)&255, c=(x>>8)&255, d=x&255;
    snprintf(buf,16,"%u.%u.%u.%u",a,b,c,d);
}

static bool parse_cidr(const char *s, uint32_t *start, uint32_t *end) {
    /* form: a.b.c.d/len */
    if(!s||!start||!end) return false;
    char ip[64]; memset(ip,0,sizeof(ip));
    const char *slash = strchr(s,'/');
    if(!slash) return false;
    size_t n = (size_t)(slash - s);
    if(n==0 || n>=sizeof(ip)) return false;
    memcpy(ip, s, n);
    unsigned int len = 0;
    if(sscanf(slash+1, "%u", &len)!=1) return false;
    if(len>32) return false;
    uint32_t base;
    if(!parse_ipv4(ip,&base)) return false;

    uint32_t mask = (len==0)? 0u : (0xFFFFFFFFu << (32-len));
    uint32_t st = base & mask;
    uint32_t ed = st | (~mask);
    *start = st;
    *end   = ed;
    return true;
}

static inline uint32_t bucket32(uint32_t a, uint32_t b, int k, uint32_t x) {
    uint32_t y = a * x + b;         /* overflow 32-bit voluto */
    if (k <= 0)  return 0u;         /* 2^0 bucket -> tutti 0 */
    if (k >= 32) return y;          /* nessuno shift */

    /* shift su 64 bit evita errori per k=0 e garantisce portabilità */
    return (uint32_t)(((uint64_t)y) >> (32 - k));
}

/* ===================== Preset ===================== */
/* Allineati a README/USAGE: default, wang. */
static void preset_apply(const char *name, uint32_t *a, uint32_t *b) {
    if(!name) { return; }
    if(strcasecmp(name,"default")==0) {
        *a = 0x9E3779B1u; *b = 0x85EBCA77u; /* default */ 
        return;
    }
    if(strcasecmp(name,"wang")==0) {
        *a = 0x27D4EB2Du; *b = 0x165667B1u; /* wang */
        return;
    }
    /* fallback: lascia invariato */
}

/* ===================== Statistiche Self-test ===================== */
typedef struct {
    double chi2;
    double mad_pct;
    double max_dev_pct;
    double uniformity_pct;
} stats_t;

static void compute_stats(uint64_t *counts, uint32_t B, uint64_t total, stats_t *out) {
    if(!counts || B==0 || total==0 || !out) return;
    double E = (double)total / (double)B;
    double chi2 = 0.0;
    double mad = 0.0;
    double max_dev = 0.0;
    for(uint32_t i=0;i<B;i++) {
        double obs = (double)counts[i];
        double dev = obs - E;
        chi2 += (dev*dev) / (E>0?E:1.0);
        double pct = (E>0)? (100.0 * (dev<0?-dev:dev) / E) : 0.0;
        mad += pct;
        if(pct > max_dev) max_dev = pct;
    }
    mad /= (double)B;
    double uniformity = 100.0 - mad;
    out->chi2 = chi2;
    out->mad_pct = mad;
    out->max_dev_pct = max_dev;
    out->uniformity_pct = uniformity;
}

/* ===================== Usage ===================== */
static void usage(const char *prog) {
    fprintf(stderr,
"Usage: %s [options]\n"
"\n"
"  --ip A.B.C.D               Bucket singolo\n"
"  --cidr A.B.C.D/LEN         Itera su un blocco CIDR\n"
"  --stdin                    Legge IP da stdin (uno per riga)\n"
"  --k <0..32>                Profondita' (2^k bucket)\n"
"  --preset default|wang      Applica preset (a,b)\n"
"  --a HEX|DEC                Override coeff a (32-bit)\n"
"  --b HEX|DEC                Override coeff b (32-bit)\n"
"\n"
"  --selftest                 Calcola statistiche su CIDR\n"
"  --sample N                 Campiona N indirizzi da CIDR (stride)\n"
"  --bench N                  Benchmark sintetico con N iterazioni\n"
"\n"
"  --export <path|->          Esporta sequenza bucket (txt/csv)\n"
"  --export-format txt|csv    Forza il formato (override estensione)\n"
"  --limit N                  Esporta solo i primi N bucket\n"
"  --no-header                CSV senza header\n"
"\n"
"  -h, --help                 Questo help\n"
, prog);
}

/* ===================== Main ===================== */
int main(int argc, char **argv) {
    const char *preset = NULL;
    uint32_t a = 0, b = 0;
    bool a_set=false, b_set=false;

    int k = -1;
    const char *ip_str = NULL;
    const char *cidr_str = NULL;
    bool opt_stdin = false;
    bool opt_selftest = false;
    uint64_t opt_sample = 0;
    uint64_t opt_bench = 0;

    const char *export_path = NULL;
    tb_fmt_t export_fmt = TB_FMT_NONE;
    uint64_t export_limit = 0;
    bool export_no_header = false;

    /* parse argv semplice e portabile */
    for(int i=1;i<argc;i++) {
        const char *arg = argv[i];

        if(strcmp(arg,"--ip")==0 && i+1<argc) { ip_str = argv[++i]; continue; }
        if(strcmp(arg,"--cidr")==0 && i+1<argc) { cidr_str = argv[++i]; continue; }
        if(strcmp(arg,"--stdin")==0) { opt_stdin = true; continue; }

        if((strcmp(arg,"--k")==0 || strcmp(arg,"-k")==0) && i+1<argc) {
            k = atoi(argv[++i]); continue;
        }
        if(strcmp(arg,"--preset")==0 && i+1<argc) { preset = argv[++i]; continue; }
        if(strcmp(arg,"--a")==0 && i+1<argc) { if(!parse_u32_hexdec(argv[++i], &a)) { fprintf(stderr,"[err] invalid --a\n"); return 2; } a_set=true; continue; }
        if(strcmp(arg,"--b")==0 && i+1<argc) { if(!parse_u32_hexdec(argv[++i], &b)) { fprintf(stderr,"[err] invalid --b\n"); return 2; } b_set=true; continue; }

        if(strcmp(arg,"--selftest")==0) { opt_selftest = true; continue; }
        if(strcmp(arg,"--sample")==0 && i+1<argc) { opt_sample = strtoull(argv[++i], NULL, 10); continue; }
        if(strcmp(arg,"--bench")==0 && i+1<argc) { opt_bench = strtoull(argv[++i], NULL, 10); continue; }

        if(strcmp(arg,"--export")==0 && i+1<argc) { export_path = argv[++i]; continue; }
        if(strcmp(arg,"--export-format")==0 && i+1<argc) {
            const char *v = argv[++i];
            if(strcmp(v,"txt")==0) export_fmt = TB_FMT_TXT;
            else if(strcmp(v,"csv")==0) export_fmt = TB_FMT_CSV;
            else { fprintf(stderr,"[err] invalid --export-format\n"); return 2; }
            continue;
        }
        if(strcmp(arg,"--limit")==0 && i+1<argc) { export_limit = strtoull(argv[++i], NULL, 10); continue; }
        if(strcmp(arg,"--no-header")==0) { export_no_header = true; continue; }

        if(strcmp(arg,"-h")==0 || strcmp(arg,"--help")==0) { usage(argv[0]); return 0; }

        fprintf(stderr,"[err] unknown arg: %s\n", arg);
        usage(argv[0]);
        return 2;
    }

    if(k < 0 || k > 32) {
        fprintf(stderr,"[err] k must be in [0..32]\n");
        return 2;
    }

    /* Applica preset se richiesto (a meno di override espliciti) */
    if(preset && (!a_set || !b_set)) {
        uint32_t A=a, B=b;
        preset_apply(preset, &A, &B);
        if(!a_set) a = A;
        if(!b_set) b = B;
    }
    /* default se non impostati */
    if(!a_set && !b_set && !preset) {
        a = 0x9E3779B1u; b = 0x85EBCA77u; /* default */
    }

    /* Setup exporter */
    tb_exporter_t ex = {0};
    bool do_export = (export_path != NULL);
    if(do_export) {
        tb_export_opts_t opts = {
            .path = export_path,
            .fmt  = export_fmt,
            .limit = export_limit,
            .no_header = export_no_header
        };
        int er = tb_export_open(&ex, &opts);
        if(er<0) {
            fprintf(stderr, "[err] export: cannot open '%s' (code %d)\n", export_path, er);
            return 3;
        }
        if(ex.fmt==TB_FMT_TXT && ex.to_stdout) {
            /* Nota: se esporti su stdout, evita stampe su stdout durante il loop. */
        }
    }

    uint64_t seq_index = 0; /* contatore globale per export */

    /* ====== Modalita': BENCH ====== */
    if(opt_bench > 0) {
        uint32_t x = 0u;
        uint64_t checksum = 0;
        clock_t t0 = clock();
        for(uint64_t i=0;i<opt_bench;i++) {
            uint32_t bk = bucket32(a,b,k,x);
            checksum += bk;
            if(do_export) {
                int wr = tb_export_write(&ex, seq_index++, bk);
                if(wr<0) { fprintf(stderr,"[err] export write failed at #%" PRIu64 "\n", seq_index-1); break; }
            }
            x += 0x9E3779B1u; /* cammino sintetico a basso bias */
        }
        clock_t t1 = clock();
        double secs = (double)(t1 - t0) / (double)CLOCKS_PER_SEC;
        double rate = (secs>0)? (opt_bench / secs) / 1e6 : 0.0;
        fprintf(stderr,"Benchmark Turbo-Bucketizer\n");
        fprintf(stderr,"a=0x%08X b=0x%08X k=%d\n", a,b,k);
        fprintf(stderr,"N=%" PRIu64 " time=%.3fs rate=%.3f Mops/s checksum=%" PRIu64 "\n",
                opt_bench, secs, rate, checksum);

        if(do_export) {
            uint64_t ck=0; int cr=tb_export_close(&ex,&ck);
            if(cr<0) { fprintf(stderr,"[err] export: close failed (%d)\n", cr); }
            fprintf(stderr,"[export] items=%" PRIu64 " checksum=0x%016" PRIX64 " fmt=%s dst=%s\n",
                    ex.written, ck, (ex.fmt==TB_FMT_CSV?"csv":"txt"), export_path);
        }
        return 0;
    }

    /* ====== Modalita': SELFTEST su CIDR ====== */
    if(opt_selftest) {
        if(!cidr_str) { fprintf(stderr,"[err] --selftest richiede --cidr\n"); return 2; }
        uint32_t st, ed;
        if(!parse_cidr(cidr_str,&st,&ed)) { fprintf(stderr,"[err] invalid --cidr\n"); return 2; }

        uint64_t total_range = (uint64_t)ed - (uint64_t)st + 1u;
        uint64_t samples = (opt_sample>0 && opt_sample<total_range)? opt_sample : total_range;
        uint64_t step = (samples<total_range)? (total_range / samples) : 1u;

        uint32_t B = (k>=32)? 0xFFFFFFFFu : (1u<<k);
        if(B == 0) { fprintf(stderr,"[err] k troppo grande per allocazione\n"); return 2; }

        uint64_t *counts = (uint64_t*)calloc(B, sizeof(uint64_t));
        if(!counts) { fprintf(stderr,"[err] alloc counts (%u)\n", B); return 2; }

        uint64_t written_local = 0;
        uint64_t idx = 0;
        for(uint64_t off=0; off<total_range; off+=step) {
            uint32_t x = st + (uint32_t)off;
            uint32_t bk = bucket32(a,b,k,x);
            counts[bk]++;

            if(do_export) {
                int wr = tb_export_write(&ex, seq_index++, bk);
                if(wr<0) { fprintf(stderr,"[err] export write failed at #%" PRIu64 "\n", seq_index-1); break; }
            } else {
                (void)idx; /* no-op: niente stdout durante selftest per non inquinare */
            }
            written_local++;
        }

        stats_t S = {0};
        compute_stats(counts, B, written_local, &S);

        fprintf(stderr,"Self-test Turbo-Bucketizer\n");
        fprintf(stderr,"cidr=%s  a=0x%08X b=0x%08X  k=%d  buckets=%u\n", cidr_str,a,b,k,B);
        fprintf(stderr,"total=%" PRIu64 "  samples=%" PRIu64 "  step=%" PRIu64 "\n",
                (uint64_t)total_range, (uint64_t)written_local, (uint64_t)step);
        fprintf(stderr,"chi2=%.3f  max_deviation=%.2f%%  MAD=%.2f%%  uniformity≈%.3f%%\n",
                S.chi2, S.max_dev_pct, S.mad_pct, S.uniformity_pct);

        free(counts);

        if(do_export) {
            uint64_t ck=0; int cr=tb_export_close(&ex,&ck);
            if(cr<0) { fprintf(stderr,"[err] export: close failed (%d)\n", cr); }
            fprintf(stderr,"[export] items=%" PRIu64 " checksum=0x%016" PRIX64 " fmt=%s dst=%s\n",
                    ex.written, ck, (ex.fmt==TB_FMT_CSV?"csv":"txt"), export_path);
        }
        return 0;
    }

    /* ====== Modalita': IP singolo ====== */
    if(ip_str) {
        uint32_t x;
        if(!parse_ipv4(ip_str,&x)) { fprintf(stderr,"[err] invalid --ip\n"); return 2; }
        uint32_t bk = bucket32(a,b,k,x);

        if(do_export) {
            int wr = tb_export_write(&ex, seq_index++, bk);
            if(wr<0) { fprintf(stderr,"[err] export write failed\n"); }
            uint64_t ck=0; int cr=tb_export_close(&ex,&ck);
            if(cr<0) { fprintf(stderr,"[err] export: close failed (%d)\n", cr); }
            fprintf(stderr,"[export] items=%" PRIu64 " checksum=0x%016" PRIX64 " fmt=%s dst=%s\n",
                    ex.written, ck, (ex.fmt==TB_FMT_CSV?"csv":"txt"), export_path);
        } else {
            printf("IP=%s  k=%d  bucket=%u\n", ip_str, k, bk);
        }
        return 0;
    }

    /* ====== Modalita': CIDR (mappatura semplice) ====== */
    if(cidr_str && !opt_selftest) {
        uint32_t st, ed;
        if(!parse_cidr(cidr_str,&st,&ed)) { fprintf(stderr,"[err] invalid --cidr\n"); return 2; }
        uint64_t total = (uint64_t)ed - (uint64_t)st + 1u;

        for(uint64_t i=0;i<total;i++) {
            uint32_t x = st + (uint32_t)i;
            uint32_t bk = bucket32(a,b,k,x);
            if(do_export) {
                int wr = tb_export_write(&ex, seq_index++, bk);
                if(wr<0) { fprintf(stderr,"[err] export write failed at #%" PRIu64 "\n", seq_index-1); break; }
            } else {
                char ipbuf[16]; ipv4_to_str(x, ipbuf);
                printf("%s\t%u\n", ipbuf, bk);
            }
        }
        if(do_export) {
            uint64_t ck=0; int cr=tb_export_close(&ex,&ck);
            if(cr<0) { fprintf(stderr,"[err] export: close failed (%d)\n", cr); }
            fprintf(stderr,"[export] items=%" PRIu64 " checksum=0x%016" PRIX64 " fmt=%s dst=%s\n",
                    ex.written, ck, (ex.fmt==TB_FMT_CSV?"csv":"txt"), export_path);
        }
        return 0;
    }

    /* ====== Modalita': STDIN ====== */
    if(opt_stdin) {
        char line[256];
        while(fgets(line,sizeof(line), stdin)) {
            char *nl = strchr(line,'\n'); if(nl) *nl = '\0';
            if(line[0]=='\0') continue;
            uint32_t x;
            if(!parse_ipv4(line,&x)) {
                fprintf(stderr,"[warn] skip invalid ip: %s\n", line);
                continue;
            }
            uint32_t bk = bucket32(a,b,k,x);
            if(do_export) {
                int wr = tb_export_write(&ex, seq_index++, bk);
                if(wr<0) { fprintf(stderr,"[err] export write failed at #%" PRIu64 "\n", seq_index-1); break; }
            } else {
                printf("%s\t%u\n", line, bk);
            }
        }
        if(do_export) {
            uint64_t ck=0; int cr=tb_export_close(&ex,&ck);
            if(cr<0) { fprintf(stderr,"[err] export: close failed (%d)\n", cr); }
            fprintf(stderr,"[export] items=%" PRIu64 " checksum=0x%016" PRIX64 " fmt=%s dst=%s\n",
                    ex.written, ck, (ex.fmt==TB_FMT_CSV?"csv":"txt"), export_path);
        }
        return 0;
    }

    /* Nessuna modalità scelta */
    usage(argv[0]);
    return 1;
}
