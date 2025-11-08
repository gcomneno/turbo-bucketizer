#ifndef TB_EXPORT_H
#define TB_EXPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TB_FMT_NONE = 0,
    TB_FMT_TXT,   /* una riga: "b0 b1 b2 ...\n" */
    TB_FMT_CSV    /* due colonne: index,bucket (+header) */
} tb_fmt_t;

typedef struct {
    const char *path;     /* file path; "-" = stdout */
    tb_fmt_t    fmt;      /* se NONE: auto da estensione */
    uint64_t    limit;    /* 0 = nessun limite */
    bool        no_header;/* CSV: sopprime header */
} tb_export_opts_t;

typedef struct {
    FILE      *fp;
    tb_fmt_t   fmt;
    uint64_t   written;
    uint64_t   limit;
    bool       first_txt_token;
    uint64_t   fnv64;           /* checksum progressivo */
    bool       csv_header_done;
    bool       to_stdout;
} tb_exporter_t;

/* Ritorna 0 su OK, <0 su errore */
int tb_export_open(tb_exporter_t *ex, const tb_export_opts_t *opts);
/* Scrive un elemento (index, bucket). Ignora se superato limit. Ritorna 1 se scritto, 0 se saltato, <0 errore. */
int tb_export_write(tb_exporter_t *ex, uint64_t index, uint32_t bucket);
/* Chiude; stampa newline se TXT; ritorna il checksum via out_checksum se non NULL. */
int tb_export_close(tb_exporter_t *ex, uint64_t *out_checksum);

#ifdef __cplusplus
}
#endif

#endif /* TB_EXPORT_H */
