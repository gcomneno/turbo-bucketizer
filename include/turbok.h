#ifndef TURBOK_H
#define TURBOK_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Converte "a.b.c.d" in uint32 (network order -> host order flatten)
int ip4_parse(const char *s, uint32_t *out);

// Bucketization deterministica: y = a*x + b (mod 2^32); bucket = top k bits di y
uint32_t turbok_bucketize(uint32_t ip_be, uint8_t k);

// Parametri (a,b) di default (odd 'a' ⇒ permutazione su Z_2^32)
typedef struct {
    uint32_t a;
    uint32_t b;
} turbok_params_t;

// Set/get dei parametri globali (thread-unsafe semplice MVP)
void turbok_set_params(turbok_params_t p);
turbok_params_t turbok_get_params(void);

#ifdef __cplusplus
}
#endif
#endif
