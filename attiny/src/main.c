#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <Arduino.h>

#include "nrf24/projdefs.h"
#include "nrf24/nRF24L01.h"
#include "utils/utils.h"

#define CHANNEL 	76 // 0-125
#define FIVE_BYTES	5

#define DELAY	3	// (== 250us * (n+1))
#define RETRY	10	// 12

#define MIN_V 	700

volatile bool sleep = true;
volatile unsigned int voltage = 0;

const uint8_t PIPE0_ADDRESS_PGM[] PROGMEM = "0Node"; 
const uint8_t PIPE1_ADDRESS_PGM[] PROGMEM = "1Node";

void receive() {
	uint8_t payload[NRF24_MAX_SIZE];

	nrf24_writeReg(W_REGISTER | EN_AA,      NRF24_PIPE_0);  // en autoack
	nrf24_writeReg(W_REGISTER | NRF_STATUS, NRF24_STATUS_CLEAR_ALL);  // clear status
	
	// set RX mode, enable CRC with 2 bytes, mask all IRQs, power on nRF radio (POWER DOWN ==> STANDBY-1)
	nrf24_writeReg(W_REGISTER | NRF_CONFIG,
		NRF24_CFG_PWR_UP | NRF24_CFG_RX_MODE | NRF24_CFG_CRC_2B | NRF24_CFG_CRC_EN | NRF24_CFG_IRQ_MASK_ALL);

	_delay_ms(DELAY_POWER_UP_MILLIS);

	uint8_t status = nrf24_readReg(R_REGISTER | NRF_STATUS);

	if ( status & NRF24_STATUS_RX_DR ) {
		// read everything what has been received from RX FIFO
		// (make sure you read everything from RX FIFO otherwise RX FIFO will overflow eventually)
		// (if you are lazy, you can always do FLUSH_RX after each read but you loose unprocessed frames, RX has 3 FIFOs
		uint8_t bytes = nrf24_readReg(R_RX_PL_WID);

		nrf24_readRegs(R_RX_PAYLOAD, payload, bytes);	// read payload into a buffer
		nrf24_writeReg(W_REGISTER | NRF_STATUS, NRF24_STATUS_CLEAR_ALL); // clear received flag

		if (bytes < sizeof(payload)) {
			payload[bytes] = 0;
		} else {
			payload[sizeof(payload) - 1] = 0; // Ensure safe termination
		}

		if (payload[0] == 'U' && payload[1] == 'P') {
			digitalWrite(PB4, HIGH);
			sleep = false;
		}  else if (payload[0] == 'D' && payload[1] == 'O' && payload[2] == 'W' && payload[3] == 'N') {
			digitalWrite(PB4, LOW);
			sleep = true;
		}
	}

	nrf24_writeReg(W_REGISTER | NRF_CONFIG,
		NRF24_CFG_PWR_DOWN | NRF24_CFG_RX_MODE | NRF24_CFG_CRC_2B | NRF24_CFG_CRC_EN | NRF24_CFG_IRQ_MASK_ALL);
}

void send(uint8_t payload[]) {
	nrf24_writeReg(W_REGISTER | SETUP_RETR, (DELAY & 0xf) << ARD_SHIFT | (RETRY & 0xf) << ARC_SHIFT); // set waiting and retransmit 

	// TX mode, enable CRC with 2 bytes, mask all IRQs, power on nRF radio (POWER DOWN ==> STANDBY-1)
	nrf24_writeReg(W_REGISTER | NRF_CONFIG,
		NRF24_CFG_PWR_UP | NRF24_CFG_TX_MODE | NRF24_CFG_CRC_2B | NRF24_CFG_CRC_EN | NRF24_CFG_IRQ_MASK_ALL);
			
	_delay_ms(DELAY_POWER_UP_MILLIS); // there should be 5ms delay to bring nRF from the POWER DOWN to the POWER UP state (the worst case)

	nrf24_cmd(FLUSH_TX); // clean TX FIFOs thoroughly - in case there is a frame with exceeded retransmission (ARC)
	nrf24_writeReg(W_REGISTER | NRF_STATUS, NRF24_STATUS_CLEAR_ALL); // clear all status indications		
	nrf24_writeRegs(W_TX_PAYLOAD, payload, (int)strlen(payload)); // write message to the TX FIFO			
}

void sendVoltage() {
	char buffer[10];
	voltage = analogRead(A0);
	itoa(voltage, buffer, 10);
	send(buffer);
}

void setup() {
	pinMode(PB4, OUTPUT);

	nrf24_init(); // initialize radio (UNDEFINED ==> POWER ON RESET) 
	
	_delay_ms(20); // it takes 10.3ms to switch from POWER ON RESET ==> POWER DOWN
	
	nrf24_writeReg(W_REGISTER | RF_CH,    CHANNEL);
	nrf24_writeReg(W_REGISTER | RF_SETUP, NRF24_PWR_MAX | NRF24_SPEED_2Mbps); // 0dbm TX power, 250kbps
	
	nrf24_writeReg(W_REGISTER | EN_RXADDR, NRF24_PIPE_0);	// enable RX in pipe 1 for ACK packet
	nrf24_writeReg(W_REGISTER | DYNPD,     NRF24_PIPE_0);   // enable dynamic payload in pipe 0	x`
	nrf24_writeReg(W_REGISTER | FEATURE,   NRF24_FEATURE_EN_DPL); // enable dynamic payload length
	
	nrf24_writeRegs(W_REGISTER | TX_ADDR, PIPE0_ADDRESS_PGM, FIVE_BYTES | NRF24_PROGMEM_MASK); 	
	nrf24_writeRegs(W_REGISTER | RX_ADDR_P0, PIPE1_ADDRESS_PGM, FIVE_BYTES | NRF24_PROGMEM_MASK); 

	nrf24_cmd(FLUSH_TX); // clean TX FIFOs thoroughly
	nrf24_cmd(FLUSH_RX); // clean RX FIFOs thoroughly

	setSleep(SLEEP_4SEC);

	sendVoltage();
}

void loop(){
	receive();

	if (sleep) {
		sleep_mode();
	}
}
