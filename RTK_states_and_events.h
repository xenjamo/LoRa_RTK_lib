


// statemachine stuff
typedef enum{
    SLEEP = 0,
    IDLE,
    FREQ_SYN_TX,
    TX_SINGLE,
    FREQ_SYN_RX,
    RX_CONT,
    RX_SINGLE,
    CAD
}state_;

typedef enum{
	NO_EVENT,
	//interrupts
	RX_TIMEOUT,
	RX_DONE,
	PAYLOADCRCERROR,
	VALIDHEADER,
	TX_DONE,
	FHSS_CHANGE_CHANNEL,
	CAD_DETECTED,
	//custom
	FIFO_WRITE_COMPLETE,
	RX_BAD,
    TX_TIMEOUT
}event_;

