#include "turbok_core.h"
#include "turbok.h"
#include <arpa/inet.h>
#include <string.h>

static turbok_params_t G = {
    // a odd ⇒ invertibile mod 2^32. Scelgo 2654435761 (Knuth).
    .a = 0x9E3779B1u,
    // b può essere qualunque; uso costante dorata “cugina”.
    .b = 0x85EBCA77u
};

void turbok_set_params(turbok_params_t p) { G = p; }
turbok_params_t turbok_get_params(void) { return G; }

// Parsing IPv4 "a.b.c.d" -> uint32 host-order. Ritorna 0 ok, -1 errore.
int ip4_parse(const char *s, uint32_t *out) {
    struct in_addr addr;
    if (inet_pton(AF_INET, s, &addr) != 1) return -1;
    // addr.s_addr è in network byte order; per aritmetica su 32 bit va bene così
    *out = ntohl(addr.s_addr); // uso host order per leggibilità del calcolo
    return 0;
}

uint32_t turbok_bucketize(uint32_t ip_host, uint8_t k) {
    if (k > 32) k = 32;
    // y = a*x + b mod 2^32
    uint32_t y = (uint32_t)(G.a * (uint64_t)ip_host + G.b);
    if (k == 32) return y; // bucket=full range
    // Prendo i k bit ALTI: più stabili e "mescolati"
    return y >> (32 - k);
}
