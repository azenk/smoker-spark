// This #include statement was automatically added by the Spark IDE.
#include "mypwm.h"

// This #include statement was automatically added by the Spark IDE.
#include "AD7194.h"

// This #include statement was automatically added by the Spark IDE.
#include "pid-control.h"

// Math
#include <math.h>

// This #include statement was automatically added by the Spark IDE.
#include "thermocouple.h"

#include "buildtime.h"

constexpr unsigned long buildtime = BUILDTIME;

// Setup for pushingbox notifactions
//const char server[] = "api.pushingbox.com";
//const char wood_url[] = "/pushingbox?devid=v3F721A007235DC9";  //replace with your devid
#define SERIAL_DEBUG 1

//SYSTEM_MODE(MANUAL);

double coldjunction = 0.0;
int _cj_accum = 0;
double tc_temperature = 0.0;
double tc_temperature2 = 0.0;
double tc_voltage = 0.0;
double tc_voltage2 = 0.0;
double te_voltage = 0.0;

 
double web_kp,web_ki,web_kd;
double web_sp = 107.22;

PID p = PID(PID_MODE_HEAT,web_sp,16,0.0025,0.0);
int output = 0;
double output_f,diff,integral;

AD7194 adc = AD7194(A2);

double thermistor_voltage1 = 0.0;
double thermistor_voltage2 = 0.0;
int _therm_accum1 = 0;
int _therm_accum2 = 0;
double thermistor_temp1 = 0.0;
double thermistor_temp2 = 0.0;
double batvoltage = 0.0;
int _batvoltage_accum = 0.0;
double sense_r = 0.0;
unsigned int loopcount = 0;

#define arraysize 40
char text[arraysize];
char varname[arraysize];
byte server[4] = {192,168,0,65};

void sendpulse(int pin, float microseconds) {
    digitalWrite(pin,HIGH);
    delayMicroseconds(microseconds);
    digitalWrite(pin,LOW);
}

void setup() {
#ifdef SERIAL_DEBUG    
    Serial.begin(9600);
#endif

    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV128);
    SPI.setDataMode(SPI_MODE3);
    
    pinMode(A0,INPUT);
    pinMode(A1,INPUT);
    pinMode(A6,INPUT);
    pinMode(A7,INPUT);
    pinMode(A2,OUTPUT);
    pinMode(D0,OUTPUT);
    pinMode(D1,OUTPUT);
    pinMode(D7,OUTPUT);

#ifdef SERIAL_DEBUG    
    Serial.println("Resetting and calibrating ADC...");
#endif

    adc.reset();
    delay(2);
    adc.init();
    adc.oneshotread(AD7194_AIN5,AD7194_AIN6,32,true,false);
    adc.calibrate();
}

double steinhart(double thermistor_r){
    double temp;
    double trlog = log(thermistor_r);
    temp = 1.0/(5.36924e-4 + trlog * 1.91396e-4 + pow(trlog,3) * 6.60399e-8);
    temp = temp - 273.15;
    return temp;
}

void loop() {
    if (loopcount++ % 120 == 0){
        adc.reset();
        delay(10);
        adc.calibrate();
        delay(10);
    }

#ifdef SERIAL_DEBUG    
    Serial.println("Beginning loop()...");
#endif
    
    _cj_accum = 0;
    _therm_accum1 = 0;
    _therm_accum2 = 0;
    _batvoltage_accum = 0;
    int samples = 1024;

#ifdef SERIAL_DEBUG    
    Serial.println("Reading onboard ADC...");
#endif

    for (int i = 0; i < samples; i++){
        _cj_accum += analogRead(A0);
        _batvoltage_accum += analogRead(A1);
        _therm_accum1 += analogRead(A6);
        _therm_accum2 += analogRead(A7);
    }
    coldjunction = _cj_accum * 3.3 / 4095.0 * 1.0 / 5.5455 * 1.0 / 10e-3 * 1.0 / samples;

    batvoltage = _batvoltage_accum * 3.3 / 4096.0 * 11.286 / samples;

    thermistor_voltage1 = _therm_accum1 * 3.3 / 4096.0 * 1.0 / samples;
    thermistor_voltage2 = _therm_accum2 * 3.3 / 4096.0 * 1.0 / samples;
    
    if (thermistor_voltage1 < 3.3 && thermistor_voltage1 > 0){
        thermistor_temp1 = steinhart(989446.2 / (3.30 / thermistor_voltage1 - 1.0));
    } else {
        thermistor_temp1 = -273.15;
    }
    
    if (thermistor_voltage2 < 3.3 && thermistor_voltage2 > 0){
        thermistor_temp2 = steinhart(993381.7 / (3.30 / thermistor_voltage2 - 1.0));
    } else {
        thermistor_temp2 = -273.15;
    }
#
#ifdef SERIAL_DEBUG    
    Serial.println("Reading external ADC...");
#endif
    digitalWrite(D7,HIGH);
    tc_voltage = adc.oneshotread(AD7194_AIN5,AD7194_AIN6,32,true,false);
    tc_voltage2 = adc.oneshotread(AD7194_AIN7,AD7194_AIN8,32,true,false);
    if (tc_voltage <= -500.0 || tc_voltage2 <= -500.0){
        adc.reset();
        delay(10);
        adc.calibrate();
        delay(10);
        return;
    }
    digitalWrite(D7,LOW);
    
#ifdef SERIAL_DEBUG    
    Serial.println("External ADC read success.");
#endif
    
    tc_temperature = tc_temp(coldjunction,tc_voltage);
    
    tc_temperature2 = tc_temp(coldjunction,tc_voltage2);

    output_f = p.update(tc_temperature);
    
    output = (output * 224 + round((output_f) * 255.0/100.0) * 32) >> 8;

    if (output > 255) {
        output = 255;
    } else if (output < 0) {
        output = 0;
    }

    //compute servo position
    float servo_us = 0.0;
    float damper_angle = 0.0;
    
    if (output > 0) {
        damper_angle = 0.0;
    } else if (output_f < 0.00 && output_f > -84.28) {
        damper_angle = asin((14.105 - (100.0 + output_f) / 100.0 * 14.105) / 11.888) / 6.28 * 360.0;
        //damper_angle = output_f * -1.0 / 90.0;
    } else if (output_f <= -84.28) {
        damper_angle = 90.0;
    } else {
        damper_angle = 0.0;
    }    
    
    servo_us = (damper_angle + 5) * 1900.0 / 180.0 + 500.0;

    sendpulse(D0,servo_us);
    analogWrite2(D1,output);
    
    web_kp = p.get_kp();
    web_ki = p.get_ki();
    web_kd = p.get_kd();

#ifdef SERIAL_DEBUG    
    Serial.println("Connecting to server...");
#endif
    
    TCPClient dbserver;

    if (dbserver.connect("webserver.ad.overeducated.com", 9500)){
        float value;
        bool receivingvalues = true;
        int numrec = 0;
        int retrycount = 0;
        while(dbserver.connected() == false && retrycount < 100){
            sendpulse(D7,50000);
            delay(10);
            retrycount++;
        }
        
        if (retrycount >=100){
            return;
        }
        
#ifdef SERIAL_DEBUG    
        Serial.println("Connected to server.");
#endif
        
        sendpulse(D7,150000);
        
        while(receivingvalues){
            
#ifdef SERIAL_DEBUG    
            Serial.println("Reading parameter from server...");
#endif
        
            int i = 0;
            int emptycycles = 0;
            while(i < arraysize && emptycycles < 1000){
                if (dbserver.available()){
                    text[i] = dbserver.read();
                } else {
                    emptycycles++;
                    continue;
                }
                
                if (text[i] == '\n'){
                    text[i] = NULL;
                    break;
                } else if (text[i] == -1){
                    continue;
                }
                i++;
            }
            
            if (emptycycles >= 1000 || i == arraysize){
                receivingvalues = false;
#ifdef SERIAL_DEBUG    
                Serial.println("ERROR: server didn't send us the data quickly enough...skipping");
#endif
                continue;
            }
            
            numrec++;
            
            int fieldn = 0;
            text[arraysize-1] = NULL;
            
            char * param = strtok(text, ":");
            while (fieldn < 2 and param != NULL) {
                //get the values for 
                if (fieldn == 0) {  // value for 1st param
                   strncpy(varname,param,arraysize);
                } else if (fieldn == 1) {  // value for 3rd param (skip 2nd)
                   value = atof(param);
                }
                fieldn++;
                param = strtok(NULL, ":");
            }
            
            if (fieldn < 2){
                receivingvalues = false;
                continue;
            }
            
            //sscanf(text,"%s:%f",varname,value);
            if (strncmp(varname,"sendcomplete",arraysize) == 0){
                receivingvalues = false;
            } else if (strncmp(varname,"setpoint",arraysize) == 0){
                if (value != web_sp){
                    p.reset_setpoint(value);
                    web_sp = value;
                }
            } else if (strncmp(varname,"kp",arraysize) == 0){
                if (value != web_kp){
                    p.retune(value, web_ki, web_kd);
                    web_kp = value;
                }
            } else if (strncmp(varname,"ki",arraysize) == 0){
                if (value != web_ki){
                    p.retune(web_kp, value, web_kd);
                    web_ki = value;
                }
            } else if (strncmp(varname,"kd",arraysize) == 0){
                if (value != web_kd){
                    p.retune(web_kp, web_ki, value);
                    web_kd = value;
                }
            } else {
                receivingvalues = false;
            }
        }
        sendpulse(D7,150000);
        delay(10);
        sendpulse(D7,50000);

#ifdef SERIAL_DEBUG    
        Serial.println("Parameter read completed.\nSending data...");
#endif
        

        float damper_pct_open = 100.0 * (14.105 - sin(damper_angle / 360.0 * 6.28) * 11.888) / 14.105;
        
        snprintf(text,arraysize,"%s:%d","Build Time",buildtime);
        dbserver.println(text);

        snprintf(text,arraysize,"%s:%d","VariablesRec",numrec);
        dbserver.println(text);

        dbserver.println("framestart:1.0");

        snprintf(text,arraysize,"%s:%0.2f","coldtemp",coldjunction);
        dbserver.println(text);

        snprintf(text,arraysize,"%s:%0.2f","batvoltage",batvoltage);
        dbserver.println(text);

        if (thermistor_temp1 > -20){
            snprintf(text,arraysize,"%s:%0.2f","foodtemp1",thermistor_temp1);
            dbserver.println(text);
        }

        if (thermistor_temp2 > -20){
            snprintf(text,arraysize,"%s:%0.2f","foodtemp2",thermistor_temp2);
            dbserver.println(text);
        }
        
        snprintf(text,arraysize,"%s:%0.2f","tctemp",tc_temperature);
        dbserver.println(text);

        snprintf(text,arraysize,"%s:%0.2f","firetemp",tc_temperature2);
        dbserver.println(text);

        snprintf(text,arraysize,"%s:%0.2f","output_pct",output * 100.0 / 255.0);
        dbserver.println(text);

        snprintf(text,arraysize,"%s:%0.2f","damper_pct_open",damper_pct_open);
        dbserver.println(text);

        snprintf(text,arraysize,"%s:%f","kp",web_kp);
        dbserver.println(text);

        snprintf(text,arraysize,"%s:%f","ki",web_ki);
        dbserver.println(text);

        snprintf(text,arraysize,"%s:%f","kd",web_kd);
        dbserver.println(text);

        snprintf(text,arraysize,"%s:%0.2f","setpoint",web_sp);
        dbserver.println(text);

        dbserver.println("frameend:1.0");
        dbserver.println();

        dbserver.flush();
        //dbserver.stop();
    }

#ifdef SERIAL_DEBUG    
    Serial.println("Loop completed.\nDelaying for 100ms...");
#endif

    delay(100);
}
