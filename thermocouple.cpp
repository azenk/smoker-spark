#include "math.h"
#include "thermocouple.h"

double tc_temp(double cold_junction_temp, double thermocouple_voltage){
    return type_k_inverse(type_k_direct(cold_junction_temp) + thermocouple_voltage);
}

double type_k_direct(double temp_c){
    double c[10];
    c[0] = -1.7600413686e1;
    c[1] = 3.8921204975e1;
    c[2] = 1.8558770032e-2;
    c[3] = -9.9457592874e-5;
    c[4] = 3.1840945719e-7;
    c[5] = -5.6072844889e-10;
    c[6] = 5.6075059059e-13;
    c[7] = -3.2020720003e-16;
    c[8] = 9.7151147152e-20;
    c[9] = -1.2104721275e-23;
    double a0 = 1.185976e2;
    double a1 = -1.183432e-4;
    
    double te_voltage = 0;
    
    for (int n=0; n<10; n++){
        te_voltage = te_voltage + c[n]*pow(temp_c,n);
    }
    te_voltage = te_voltage + a0*exp(a1*pow(temp_c - 126.9686,2));
    
    return te_voltage * 1e-6;
}

double type_k_inverse(double te_voltage){
    double temp_c = 0.0;
    double c[10]; // Inverse constants
    
    // Convert volts to microvolts
    te_voltage = te_voltage * 1.0e6;
         
    if (te_voltage < 0.0){
        /* -200 to 0C
         * -5891 to 0uV
         */
        c[0] = 0.0;
        c[1] = 2.5173462e-2;
        c[2] = -1.1662878e-6;
        c[3] = -1.0833638e-9;
        c[4] = -8.9773540e-13;
        c[5] = -3.7342377e-16;
        c[6] = -8.6632643e-20;
        c[7] = -1.0450598e-23;
        c[8] = -5.1920577e-28;
        c[9] = 0.00;
    } else if (te_voltage >= 0.0 and te_voltage < 20644.0){
        /* 0 - 500C
         * 0 - 20644 uV
         */
        c[0] = 0.0;
        c[1] = 2.508355e-2;
        c[2] = 7.860106e-8;
        c[3] = -2.503131e-10;
        c[4] = 8.315270e-14;
        c[5] = -1.228034e-17;
        c[6] = 9.804036e-22;
        c[7] = -4.413030e-26;
        c[8] = 1.057734e-30;
        c[9] = -1.052755e-35;
    } else if (te_voltage >= 20644.0 and te_voltage < 54886.0){
        /* 500 - 1372C
         * 20644 - 54886 uV
         */
        c[0] = -1.318058e2;
        c[1] = 4.830222e-2;
        c[2] = -1.646031e-6;
        c[3] = 5.464731e-11;
        c[4] = -9.650715e-16;
        c[5] = 8.802193e-21;
        c[6] = -3.110810e-26;
        c[7] = 0;
        c[8] = 0;
        c[9] = 0;
    }    

    
    for (int n = 0; n<10; n++){
        temp_c = temp_c + c[n]*pow(te_voltage,n);
    }
    
    return temp_c;
}
