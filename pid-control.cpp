#include "pid-control.h"

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
    setpoint = new_setpoint;
}

double PID::update(double process_value){
    double err = setpoint - process_value;
    update_history(err);
    return mode * (Kp * err + Ki * calculate_integral() + Kd * calculate_rate());
}

void PID::update_history(double err){
    current_sample = (current_sample + 1) % PID_HISTORY_SIZE;
    err_hist[current_sample] = err;
    err_i = err_i + err;
}

double PID::calculate_integral(){
    double integral = 0.0;
    //for (int i = 0; i < i_samples ; i++){
    //    integral = integral + err_hist[(current_sample + PID_HISTORY_SIZE - i) % PID_HISTORY_SIZE];
    //}
    integral = err_i;
    return integral;
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
