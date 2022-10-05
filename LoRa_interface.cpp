/* this marks the start of a library
* ZHAW VT1
* Clement Stoquet
* Betreuer: Ruprecht Altenburger
*/
#include "LoRa_interface.h"
//SPI spi(MOSI_PIN, MISO_PIN, SCLK_PIN);

RFM95::RFM95(PinName chip_select_pin, PinName int_pin, SPI *LR_spi) :
    _cs_pin(chip_select_pin),
    isrLora(int_pin)
{
    _cs_pin = 1;
    _spi = LR_spi;
    
}




bool RFM95::init(){
    // set some parameters to properly use LoRa Module
    // Set operation mode
    write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE);
    event = NO_EVENT;
    mode = IDLE;
    rxBad = 0;
    uint8_t data;
    int i = 1;
    while(1){
        data = read(RH_RF95_REG_01_OP_MODE);
        if(data == (RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE)){
            //printf("mode set after %d tries\n", i);
            break;
        } else {
            i++;
        }
        if(i > 10){
            printf("mode init failed\n");
            return 0;
        }
    }
    // set up fifo
    write(RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0);
    write(RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0);

    // Packet format is preamble + explicit-header + payload + crc
    // Explicit Header Mode
    // payload is TO + FROM + ID + FLAGS + message data
    // RX mode is implmented with RXCONTINUOUS
    // max message data length is 255 - 4 = 251 octets

    // set to idle mode
    setModeIdle();


    // define preamble
    uint16_t preamb = 8;
    write(RH_RF95_REG_20_PREAMBLE_MSB, preamb >> 8);
    write(RH_RF95_REG_21_PREAMBLE_LSB, preamb & 0xff);

    //modem config
    write(RH_RF95_REG_1D_MODEM_CONFIG1, RH_RF95_BW_125KHZ | RH_RF95_CODING_RATE_4_5);
    write(RH_RF95_REG_1E_MODEM_CONFIG2, RH_RF95_SPREADING_FACTOR_128CPS);


    //set up interrupt
    isrLora.rise(callback(this, &RFM95::isr_flagger));

    flags = 0x80; // set first bit to signal init was used properly

    return 1;
}

event_ RFM95::event_handler(){
    //printf("flags = 0x%x\n", flags);
    if((flags & 0x40) != 0x40){
        //printf("0x%x\n",(flags & !0x40));
        return NO_EVENT;
    }
    flags = flags & !0x80; // clear flags --> correct flag gets set in this function
    //printf("Interrupt was successfull\n");

    uint8_t reg_flags = read(RH_RF95_REG_12_IRQ_FLAGS);
    uint8_t crc_present = read(RH_RF95_REG_1C_HOP_CHANNEL);
    clearInt(); // reset interrupt register on lora module

    if(RH_RF95_RX_DONE & reg_flags){ ///////////////////////////////check if INT was a reception
	    
        if(reg_flags & (RH_RF95_RX_TIMEOUT | RH_RF95_PAYLOAD_CRC_ERROR)){ //check if payload was bad or not
            rxBad++;
            
            return RX_BAD;
        }
        readRxData();

        return RX_DONE;
    }

    
    if(reg_flags & RH_RF95_TX_DONE){ //////////////////////////////check if INT was a transmission
        
		setModeIdle();
        return TX_DONE;
    }

    return NO_EVENT;
}

bool RFM95::waitForTransmission(){
    int i = 0;
    Timer t;
    t.start();
    while(1){
        //printf("int reg content: 0x%x\n",read(RH_RF95_REG_12_IRQ_FLAGS));

        //printf("reg content = 0x%x\n",read(RH_RF95_REG_12_IRQ_FLAGS));
        if(event_handler() == TX_DONE){
            //printf("transmission complete\n");
            return 1;
        }
        if(t.elapsed_time() >= 5s){
            t.stop();
            return 0;
        }
        ThisThread::sleep_for(10ms);
    }
    return 0;
}

void RFM95::isr_flagger(){

    flags = flags | 0x40; // set flag B0[X]00 0000 to signal isr has been called

}

bool RFM95::setModeTX(){
    uint8_t reg = read(RH_RF95_REG_01_OP_MODE); // read current register
    reg = reg & 0xF7; //mask out stuff to keep

    write(RH_RF95_REG_01_OP_MODE,RH_RF95_MODE_TX | reg); //write mode
    write(RH_RF95_REG_40_DIO_MAPPING1,0x40); //set INT mapping so we can get an TxDone Interupt

    /*check if mode is correct
    if(read(RH_RF95_REG_01_OP_MODE) != (RH_RF95_MODE_TX | reg)){
        printf("couldnt enter TX mode\n");
        return false;
    }
    */
    mode = TX_SINGLE;
    
    return true;
}

bool RFM95::setModeRX(){
    uint8_t reg = read(RH_RF95_REG_01_OP_MODE); // read current register
    reg = reg & 0xF7; //mask out stuff to keep

    write(RH_RF95_REG_01_OP_MODE,RH_RF95_MODE_RXSINGLE | reg); //set mode
    write(RH_RF95_REG_40_DIO_MAPPING1,0x00); //set correct INT mapping so we can get RxDone Interrupt

    /*check if mode is correct
    if(read(RH_RF95_REG_01_OP_MODE) != (RH_RF95_MODE_RXSINGLE | reg)){
        printf("couldnt enter RX mode\n");
        return false;
    }
    */
	mode = RX_SINGLE;
    return true;
}
bool RFM95::setModeContRX(){
    uint8_t reg = read(RH_RF95_REG_01_OP_MODE); // read current register
    reg = reg & 0xF7; //mask out stuff to keep

    write(RH_RF95_REG_01_OP_MODE,RH_RF95_MODE_RXCONTINUOUS | reg);
    write(RH_RF95_REG_40_DIO_MAPPING1,0x00); //set correct INT mapping so we can get RxDone Interrupt

    /*check if mode is correct
    if(read(RH_RF95_REG_01_OP_MODE) != (RH_RF95_MODE_RXCONTINUOUS | reg)){
        printf("couldnt enter RX mode\n");
        return false;
    }
    */
    mode = RX_CONT;
    return true;
}

bool RFM95::setModeIdle(){
    uint8_t reg = read(RH_RF95_REG_01_OP_MODE); // read current register
    reg = reg & 0xF7; //mask out stuff to keep

    write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_STDBY | reg);
    write(RH_RF95_REG_40_DIO_MAPPING1,0x00); //set correct INT mapping so we can get RxDone Innterrupt

    /*printf("adding some delay xoxoxox haha message go brrrrrrr\n");
    uint8_t data = read(RH_RF95_REG_01_OP_MODE);
    /check if mode is correct
    if(data != (RH_RF95_MODE_STDBY | reg)){
        printf("couldnt enter IDLE mode\n");
        return false;
    }
    */
	mode = IDLE;
    return true;
}

void RFM95::clearInt(){
    write(RH_RF95_REG_12_IRQ_FLAGS,0xff);
}

// the cool funtions
bool RFM95::transmit(uint8_t* data, uint8_t len){

    //printf("transmitting\n");

    if(len > RH_RF95_MAX_MESSAGE_LEN){
        printf("message to long, idiot\n");
        return false;
    }

    len = uint8_t(len);

    if(mode != IDLE){ //check if module is doing anything
        return false;
    }
    //setModeIdle(); // make sure module isnt sending or resceiving stuff aka mess up the fifo buffer

    
    write(RH_RF95_REG_0D_FIFO_ADDR_PTR, 0);
    // The headers
    write(RH_RF95_REG_00_FIFO, 0x00);   //to
    write(RH_RF95_REG_00_FIFO, 0xdc);   //from
    write(RH_RF95_REG_00_FIFO, 0x01);   //id
    write(RH_RF95_REG_00_FIFO, flags);  //Flags
    // The message data
    //printf("one\n");
    burstwrite(RH_RF95_REG_00_FIFO, data, len);
    //printf("two\n");

    write(RH_RF95_REG_22_PAYLOAD_LENGTH, len + RH_RF95_HEADER_LEN);
    //printf("fifo write complete\n");
    setModeTX(); // Start the transmitter

    return true;
}

bool RFM95::receive(uint8_t *buf, uint8_t *len){
    //printf("receiver signalled incoming data\n");
    //flags = flags | 0x02; //set internal flag to reception for further data handling
    
    memcpy(buf, _buf, *len);
    
    return true;
}

void RFM95::readRxData(){

    uint8_t len = read(RH_RF95_REG_13_RX_NB_BYTES);
    //start reading received data
    write(RH_RF95_REG_0D_FIFO_ADDR_PTR, read(RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR)); // set some pointers
    burstread(RH_RF95_REG_00_FIFO, _buf, len); // read fifo and save to temp buffer
    _bufLen = len;

    _lastSNR = (int8_t)read(RH_RF95_REG_19_PKT_SNR_VALUE) / 4;// quality of packet signal to noise ratio
	_lastRssi = read(RH_RF95_REG_1A_PKT_RSSI_VALUE);//no clue what this is
}


/////////////////////////basic function to read a register

uint8_t RFM95::write(uint8_t addr, uint8_t data){
    //__disable_irq(); //disable interrupts during transmission
    
    _cs_pin = 0;                            // select device
    _spi->write(addr | SPI_WRITE_MASK);     // write address on bus with MSB set to 1 to indicate write request
    data = _spi->write(data);               // write data to device
    _cs_pin = 1;                            // deselect device

    //__enable_irq(); //reenable IRQ
    return data;
}

uint8_t RFM95::read(uint8_t addr){
    //__disable_irq(); //disable interrupts during transmission

    uint8_t data = 0x00;
    _cs_pin = 0;
    _spi->write(addr & 0x7F);    
    data = _spi->write(data);    //read data
    _cs_pin = 1;

    //__enable_irq(); //reenable IRQ
    return data;
}

uint8_t RFM95::burstwrite(uint8_t addr, uint8_t* data, uint8_t length){
    //__disable_irq(); //disable interrupts during transmission

    _cs_pin = 0;
    _spi->write(addr | SPI_WRITE_MASK);
    for(int i = 0; i < length; i++){
        _spi->write(data[i]);
    }
    _cs_pin = 1;

    //__enable_irq(); //reenable IRQ
    return 0;
}

uint8_t RFM95::burstread(uint8_t addr, uint8_t* data, uint8_t length){
    //__disable_irq(); //disable interrupts during transmission

    _cs_pin = 0;
    _spi->write(addr);
    for(int i = 0; i < length; i++){
        data[i] = _spi->write(0x00);
    }
    _cs_pin = 1;

    //__enable_irq(); //reenable IRQ
    return 0;
}
