#include "AD7194.h"

AD7194::AD7194(int cs_pin){
    this->cs_pin = cs_pin; 
}

void AD7194::write_comm_register(bool read, int reg_addr, bool cont_read){
    char reg_val = 0x00;
    if (read){
        reg_val |= 0x40;
    }
    
    reg_val |= (reg_addr << 3);
    
    if (cont_read){
        reg_val |= 0x04;
    }
    
    digitalWrite(this->cs_pin, LOW);
    
    SPI.transfer(reg_val);

    digitalWrite(this->cs_pin, LOW);
}

void AD7194::write_mode_register(){
    char mr23_16 = 0x00;    
    char mr15_8 = 0x00;    
    char mr7_0 = 0x00;

    mr23_16 |= (this->mode << 5);
    
    if (this->status){
        mr23_16 |= 0x20;
    }
    
    // use internal clock
    mr23_16 |= 0x08;
    
    mr23_16 |= (avg & 0x00000003);
    
    if (this->sinc3){
        mr15_8 |= 0x80;
    }
    
    if (this->enparity){
        mr15_8 |= 0x20;
    }
    
    if (this->clk_div){
        mr15_8 |= 0x10;
    }
    
    if (this->single_cycle){
        mr15_8 |= 0x08;
    }
    
    if (this->rej60){
        mr15_8 |= 0x04;
    }
    
    mr15_8 |= (fs & 0x00000300) >> 8;
    
    mr7_0 = (fs & 0x000000FF);

    digitalWrite(cs_pin,LOW);
    this->write_comm_register(false,AD7194_REG_MODE,false);    
    SPI.transfer(mr23_16);
    SPI.transfer(mr15_8);
    SPI.transfer(mr7_0);
    digitalWrite(cs_pin,HIGH);    
}

void AD7194::write_config_register(int pos_input, int neg_input, int gain, bool buffered, bool unipolar){
    char con23_16 = 0x00;
    char con15_8 = 0x00;
    char con7_0 = 0x00;
    
    if (this->chop){
        con23_16 |= 0x80;
    }
    
    if (this->refsel == AD7194_REF2){
        con23_16 |= 0x10;
    }
    
    if (this->pseudo){
        con23_16 |= 0x04;
    }

    con15_8 |= (pos_input & 0x0000000F) << 4;
    con15_8 |= (neg_input & 0x0000000F);
    
    if (this->burn){
        con7_0 |= 0x80;
    }
    
    if (this->refdet) {
        con7_0 |= 0x40;
    }
    
    if (buffered){
        con7_0 |= 0x10;
    }
    
    if (unipolar){
        con7_0 |= 0x80;
    }
    
    switch(gain){
        case 128:
            con7_0 |= 0x07;
            break;
        case 64:
            con7_0 |= 0x06;
            break;
        case 32:
            con7_0 |= 0x05;
            break;
        case 16:
            con7_0 |= 0x04;
            break;
        case 8:
            con7_0 |= 0x03;
            break;
        default:
            con7_0 |= 0x00;
    }

    digitalWrite(cs_pin,LOW);
    this->write_comm_register(false,AD7194_REG_CONFIG,false);    
    SPI.transfer(con23_16);
    SPI.transfer(con15_8);
    SPI.transfer(con7_0);
    digitalWrite(cs_pin,HIGH);    
}

int  AD7194::read_data_register(){
    int data = 0;
    digitalWrite(cs_pin,LOW);
    this->write_comm_register(true,AD7194_REG_DATA,false);    
    data |= SPI.transfer(0x00) << 16;
    data |= SPI.transfer(0x00) << 8;
    data |= SPI.transfer(0x00);
    digitalWrite(cs_pin,HIGH);
    
    return data;
}

int  AD7194::read_status_register(){
    int data = 0;
    digitalWrite(cs_pin,LOW);
    this->write_comm_register(true,AD7194_REG_STATUS,false);    
    data |= SPI.transfer(0x00);
    digitalWrite(cs_pin,HIGH);
    return data;
}

bool AD7194::dataready(){
    int data = this->read_status_register();
    return ((data & 0x00000080) == 0);
}

void AD7194::reset(){
    // RESET by sending 40 1s to the chip 
    digitalWrite(this->cs_pin,LOW);
    for (int i = 0; i < 5; i++){
        SPI.transfer(0xFF);
    }
    digitalWrite(this->cs_pin,HIGH);
}

void AD7194::init(){
    this->fs = 96;
    this->rej60 = true;
    this->write_mode_register();
}

double AD7194::value2voltage(int value, int gain, bool unipolar){
    double voltage;
    if (! unipolar){
        voltage = double(value - (1<<23)) / double(1<<23) * this->ref_voltage / gain;
    } else {
        voltage = double(value) / double(1<<24) * this->ref_voltage / gain;
    }
    return voltage;
}

double AD7194::oneshotread(int pos_input, int neg_input, int gain, bool buffered, bool unipolar){
    this->write_config_register(pos_input,neg_input,gain,buffered,unipolar);
    delay(1);
    int count = 0;
    while (! this->dataready()){
        // wait
        delay(1);
        if (count++ > 500){
            return -500.0; // should do something better for errors like this
        }
    }
    int value = this->read_data_register();
    return this->value2voltage(value,gain,unipolar);
}

void AD7194::calibrate(){
    int prev_mode = this->mode;
    int dataready_counter = 0;
    this->mode = AD7194_MODE_INT_ZERO_CAL;
    this->write_mode_register();
    while (! this->dataready() && dataready_counter < 2000000){
        // spin waiting
        dataready_counter++;
    }
    
    if (dataready_counter >= 2000000){
        // TODO: do something
    }
    
    this->mode = AD7194_MODE_INT_FS_CAL;
    this->write_mode_register();
    dataready_counter = 0;
    while (! this->dataready() && dataready_counter < 2000000){
        // spin waiting
        dataready_counter++;
    }

    if (dataready_counter >= 2000000){
        // TODO: do something
    }
    
    this->mode = prev_mode;
    this->write_mode_register();
}
    
