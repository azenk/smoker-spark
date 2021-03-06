#include "pid-control.h"
#include <stdlib.h>

PID::PID(int mode_in,double setpoint_in,double Kp_in, double Ki_in = 0.0, double Kd_in = 0.0){
    mode = mode_in;
    setpoint = setpoint_in;
    Kp = Kp_in;
    Ki = Ki_in;
    Kd = Kd_in;
    err_i = 0.0;

    for (int i = 0; i < PID_HISTORY_SIZE; i++){
        err_hist[i] = 0.0;
    }
}

double PID::get_kp(){
    return Kp;
}

double PID::get_ki(){
    return Ki;
}

double PID::get_kd(){
    return Kd;
}

void PID::reset_setpoint(double new_setpoint){
    err_i = 0.0;
    setpoint = new_setpoint;
}

double PID::update(double process_value){
    double err = setpoint - process_value;
    current_sample = (current_sample + 1) % PID_HISTORY_SIZE;
    err_hist[current_sample] = process_value;
    double rate =  calculate_rate();
    if (abs(mode * (Kp * err + Ki * err_i + Kd * rate)) < 100) {
			err_i = err_i + err;
    } 
    return mode * (Kp * err + Ki * err_i + Kd * rate);
}

double PID::calculate_rate(){
    double rate = 0;
    double sum_x = 0;
    double sum_xy = 0;
    double sum_y = 0;
    double sum_xs = 0;
    for (int i = 0; i < d_samples; i++){
        int idx1 = (current_sample + PID_HISTORY_SIZE - i) % PID_HISTORY_SIZE;
        sum_x = sum_x + i;
        sum_xs = sum_xs + i * i;
        sum_y = sum_y + err_hist[idx1];
        sum_xy = sum_xy + i * err_hist[idx1];
    }
    rate = (sum_xy - sum_x * sum_y / d_samples) / (sum_xs - sum_x * sum_x / d_samples);
    return rate;
}

void PID::retune(double Kp_in, double Ki_in = 0.0, double Kd_in = 0.0){
    Kp = Kp_in;
    Ki = Ki_in;
    Kd = Kd_in;
}
