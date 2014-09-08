#ifndef THERMOCOUPLE_H
#define THERMOCOUPLE_H

double tc_temp(double cold_junction_temp, double thermocouple_voltage);

double type_k_direct(double temp_c);
double type_k_inverse(double te_voltage);

#endif
