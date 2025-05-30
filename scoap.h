#ifndef SCOAP_H
#define SCOAP_H

#include "circuit.h"

void scoap();
void compute_cc(NSTRUC *np);
void compute_co();
int min_cc0(NSTRUC *np);
int min_cc1(NSTRUC *np);
int sum_cc0(NSTRUC *np);
int sum_cc1(NSTRUC *np);

#endif
