#ifndef AD7194_h
#define AD7194_h

#include <application.h>


#define AD7194_AIN1 0b0000
#define AD7194_AIN2 0b0001
#define AD7194_AIN3 0b0010
#define AD7194_AIN4 0b0011
#define AD7194_AIN5 0b0100
#define AD7194_AIN6 0b0101
#define AD7194_AIN7 0b0110
#define AD7194_AIN8 0b0111
#define AD7194_AIN9 0b1000
#define AD7194_AIN10 0b1001
#define AD7194_AIN11 0b1010
#define AD7194_AIN12 0b1011
#define AD7194_AIN13 0b1100
#define AD7194_AIN14 0b1101
#define AD7194_AIN15 0b1110
#define AD7194_AIN16 0b1111
#define AD7194_REF1 1
#define AD7194_REF2 2

// Possible Converter modes, see table 20 in datasheet.
#define AD7194_MODE_CONTINUOUS  0b000
#define AD7194_MODE_SINGLE      0b001
#define AD7194_MODE_IDLE        0b010
#define AD7194_MODE_POWER_DOWN  0b011
#define AD7194_MODE_INT_ZERO_CAL    0b100
#define AD7194_MODE_INT_FS_CAL      0b101
#define AD7194_MODE_SYS_ZERO_CAL    0b110
#define AD7194_MODE_SYS_FS_CAL      0b111

// Register addresses
#define AD7194_REG_COMM     0b000
#define AD7194_REG_STATUS     0b000
#define AD7194_REG_MODE     0b001
#define AD7194_REG_CONFIG   0b010
#define AD7194_REG_DATA     0b011
#define AD7194_REG_ID       0b100
#define AD7194_REG_GPOCON   0b101
#define AD7194_REG_OFFSET   0b110
#define AD7194_REG_FSCALE   0b111

class AD7194 {
    
    // Chipselect Pin
    int cs_pin;
    
    // Enable chopping?  (this swaps inputs per sample, CON23)
    bool chop = true;
    
    // Use REFIN1 or REFIN2 (CON20)
    int refsel = AD7194_REF1;
    
    // Differential vs. pseudo differential (CON18)
    bool pseudo = false;
    
    // Reference Detect, Enable reference detection?
    bool refdet = false;
    
    // Enable 500nA current sources in input path?
    bool burn = false;
    
    // Enable status register transmission? (MR20, DAT_STA)
    bool status = false;
    
    // Mode
    int mode = AD7194_MODE_CONTINUOUS;
    
    // Fast settling average filter
    int avg = 0;
    
    // Use sinc3 filter?  Otherwise sinc4
    bool sinc3 = false;
    
    // Enable parity?  ENPAR
    bool enparity = false;
    
    // Clock Divide-by-2?  Used primarily by internal calibrations
    bool clk_div = false;
    
    // Single cycle conversion?  When true, the converter will only
    // output fully settled values.
    bool single_cycle = false;
    
    // 60 hz notch filter?
    bool rej60 = false;
    
    // Conversion rate.  Output data rate = (MCLK/1024)/FS
    // FS is a 10 bit unsigned decimal number
    int fs = 0x060;
    
    // reference voltage
    double ref_voltage = 2.500;
    
    public:
    
        AD7194(int cs_pin);
        double oneshotread(int pos_input, int neg_input, int gain, bool buffered, bool unipolar);
        void setup_read(int pos_input, int neg_input, int gain, bool buffered, bool unipolar);
        double read();
        bool dataready();
        double readtemp();
        
        void calibrate();
        void reset();
        void init();
    
    private:
        void write_comm_register(bool read, int reg_addr, bool cont_read);
        int  read_status_register();
        void write_mode_register();
        void read_mode_register();
        void write_config_register(int pos_input, int neg_input, int gain, bool buffered, bool unipolar);
        void read_config_register();
        int  read_data_register();
        void write_gpocon_register();
        void read_gpocon_register();
        int  read_id_register();
        void write_offset_register(int offset);
        int  read_offset_register();
        void write_fullscale_register(int full_scale);
        int  read_fullscale_register();
        double value2voltage(int value, int gain, bool unipolar);
        
    
};

#endif
