#ifndef PID_CONTROL_H
#define PID_CONTROL_H

#define PID_HISTORY_SIZE 32

class PID {
    double Kp = 0.0;
    double Ki = 0.0;
    double Kd = 0.0;
    
    double setpoint = 0.0;
    
    int i_samples = PID_HISTORY_SIZE;
    int d_samples = 10;
    double err_hist[PID_HISTORY_SIZE];// = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double err_i = 0.0;
    int mode;
    
    public:
        int current_sample = -1;

        PID(int,double,double,double,double);
        void reset_setpoint(double);
        void reset();
        double update(double);
        double calculate_rate();
        double calculate_integral();
        void retune(double,double,double);
        double get_kp();
        double get_ki();
        double get_kd();
        
    private:
        void update_history(double);
};

#define PID_MODE_HEAT 1.0
#define PID_MODE_COOL -1.0

#endif
