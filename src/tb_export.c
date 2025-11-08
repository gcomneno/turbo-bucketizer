#include <string.h>
#include <strings.h>
#include <errno.h>

#include "tb_export.h"

#if defined(_WIN32) || defined(_WIN64)
#define strcasecmp _stricmp
#endif

static uint64_t fnv1a64_init(void){ return 0xcbf29ce484222325ULL; }
static uint64_t fnv1a64_update(uint64_t h, const void *p, size_t n){
    const unsigned char *s = (const unsigned char*)p;
    for(size_t i=0;i<n;i++){ h ^= (uint64_t)s[i]; h *= 0x100000001b3ULL; }
    return h;
}
/* aggiorna con un uint32_t (bucket) in maniera portabile */
static uint64_t fnv1a64_u32(uint64_t h, uint32_t v){
    unsigned char b[4];
    b[0]=(unsigned char)(v & 0xFF);
    b[1]=(unsigned char)((v>>8) & 0xFF);
    b[2]=(unsigned char)((v>>16)& 0xFF);
    b[3]=(unsigned char)((v>>24)& 0xFF);
    return fnv1a64_update(h,b,4);
}

static tb_fmt_t infer_fmt(const char *path, tb_fmt_t wanted){
    if(wanted!=TB_FMT_NONE) return wanted;
    if(!path) return TB_FMT_TXT;
    const char *dot = strrchr(path,'.');
    if(dot && (strcasecmp(dot,".csv")==0)) return TB_FMT_CSV;
    return TB_FMT_TXT;
}

int tb_export_open(tb_exporter_t *ex, const tb_export_opts_t *opts){
    if(!ex || !opts || !opts->path) return -1;
    memset(ex,0,sizeof(*ex));

    ex->fmt   = infer_fmt(opts->path, opts->fmt);
    ex->limit = opts->limit;
    ex->first_txt_token = true;
    ex->fnv64 = fnv1a64_init();

    if(strcmp(opts->path,"-")==0){
        ex->fp = stdout;
        ex->to_stdout = true;
    } else {
        ex->fp = fopen(opts->path, "wb");
        if(!ex->fp) return -2;
    }

    if(ex->fmt==TB_FMT_CSV && !opts->no_header){
        if(fprintf(ex->fp, "index,bucket\n")<0) return -3;
        ex->csv_header_done = true;
    }
    return 0;
}

int tb_export_write(tb_exporter_t *ex, uint64_t index, uint32_t bucket){
    if(!ex || !ex->fp) return -1;
    if(ex->limit && ex->written >= ex->limit) return 0;

    int rc;
    if(ex->fmt==TB_FMT_TXT){
        if(!ex->first_txt_token){
            rc = fputc(' ', ex->fp);
            if(rc==EOF) return -2;
        }
        ex->first_txt_token = false;
        rc = fprintf(ex->fp, "%u", bucket);
        if(rc<0) return -2;
    } else {
        rc = fprintf(ex->fp, "%llu,%u\n",
                     (unsigned long long)index, bucket);
        if(rc<0) return -2;
    }
    ex->fnv64 = fnv1a64_u32(ex->fnv64, bucket);
    ex->written++;
    return 1;
}

int tb_export_close(tb_exporter_t *ex, uint64_t *out_checksum){
    if(!ex || !ex->fp) return -1;
    if(ex->fmt==TB_FMT_TXT){
        if(fputc('\n', ex->fp)==EOF) return -2;
    }
    if(out_checksum) *out_checksum = ex->fnv64;

    if(!ex->to_stdout){
        if(fclose(ex->fp)!=0) return -3;
    } else {
        fflush(stdout);
    }
    ex->fp = NULL;
    return 0;
}
