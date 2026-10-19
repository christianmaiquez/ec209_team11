#ifndef CALCULATIONS_H
#define CALCULATIONS_H

void  measure_capture(void); // Takes a fresh set of samples of voltage and current from the ADC

// Results from the most recent capture
float measure_vrms_v(void);      // RMS voltage at the source, V
float measure_ipk_a(void);       // peak load current, A
float measure_power_w(void);     // average real power, W

#endif