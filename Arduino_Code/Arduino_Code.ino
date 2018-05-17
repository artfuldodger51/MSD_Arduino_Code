//Curtin University
//Mechatronics Engineering
//Bluetooth I/O Card - Driver Code

// This driver handles serial transmission of data for the four ports of the Bluetooth I/O card.
// Each command or response is sent as a four byte package, formatted as follows:
// <startByte><commandByte><dataByte><checkByte>
// The start byte identifies the first byte of a new message. Ideally this value would be reserved for the start byte 
// but as that is not possible the build of the message will be confirmed using a check sum.
// The command byte allows selection of which port to interact with.
// The data byte is the value set to or read from a port, this is the main information that needs to be transmitted.
// The check sum allows for confirmation that the correct four bytes have been received as a message. This is useful as it 
// confirm that the end of the message has been reached and that no bytes were lost during the transmission.

void encoderRead(void);
void encoderSend(byte, byte);


//Declare variables for each byte of the message.
byte startByte = 0;
byte commandByte = 0;
byte ADCByte = 0;
byte checkByte = 0;

// Declare constants for input mode
#define HTCLRST  0
#define ADC   1
#define sendDecode 2
#define deadband_tune 3
#define deadband_remove 4


//Declare variable for calculating the check sum which is used to confirm that the correct bytes were identified as the four message bytes.
byte checkSum = 0;

//Declare a constant for the start byte ensuring that the value is static.
const byte START = 255;

unsigned int low_byte= 0;
unsigned int high_byte = 0;

const byte RSTPIN = 5;

bool rising = false;

byte dead_upper = 0;
byte dead_lower = 0;
byte dead_low = 0;
byte controlAction = 0;

//Setup initialises pins as inputs or outputs and begins the bluetooth serial.
void setup() {
  Serial.begin(9600);
  DDRA = 0xFF; 
  DDRC = 0xFF;
  DDRF = 0x00;
  DDRK = 0x00;

}

//Main program manages setting/reading of ports via the bluetooth serial.
void loop() {

  if (Serial.available() >= 4) // Check that a full package of four bytes has arrived in the buffer.
  {
    startByte = Serial.read(); // Get the first available byte from the buffer, assuming that it is the start byte.

    if(startByte == START) // Confirm that the first byte was the start byte, otherwise begin again checking the next byte.
    {
      //Read the remaining three bytes of the package into the respective variables.
      commandByte = Serial.read();
      ADCByte = Serial.read();
      checkByte = Serial.read();

      checkSum = startByte + ADCByte + commandByte; // Calculate the check sum, this is also calculated
                                                     // in visual studio and is sent as he final byte of the package.

      if(checkByte == checkSum) //Confirm that the calculated and sent check sum match, if so it is safe to process the data.
      {

        switch(commandByte)
        {
          case HTCLRST:
            PORTC |= (1<<RSTPIN);
            break;
			
          case ADC:
			if (remove)
			{
				if (ADCByte < dead_upper && ADCByte > 127)
				{
					controlAction = dead_upper;
				}
				else if (ADCByte > dead_lower && ADCByte < 127)
				{
					controlAction = dead_lower;
				}
				else
				{
					controlAction = ADCByte;
				}	
			}
			else
			{
				controlAction = ADCByte;
			}
			
            PORTA = controlAction;       // Output byte to ADC
          break;
		  
		  case sendDecode:
		    decoderRead();
			decoderSend(low_byte, high_byte);
			break;
			
		  case deadband_tune:
			PORTA = 127;
			rising = true;
			while(rising == true)
			{
				PORTA++;
				delay(100);
				dead_low = decoderRead();
				if (dead_low > prev_low_byte)
				{
					rising = false;
					falling = true;
					dead_upper = PORTA;
					PORTA = 127;
					
				}
				prev_low_byte = dead_low;
				
			}
			
			while (falling == true)
			{
				PORTA--
				delay(100);
				dead_low = decoderRead();
				if (dead_low < prev_low_byte)
				{
					falling = false;
					dead_lower = PORTA;
					PORTA = 127;
				}
				prev_low_byte = dead_low;
			}
			decoderSend(69, 69);
			break;
			
		  case deadband_remove:
			remove = !remove;
			break;
        }
        
      }
      
    }    
  }

}

byte decoderRead(void)
{

  // Set OE to high to disable output, and set SEL to low to output the high byte
  PORTC = 0b00100000;
  // Clear OE bit which will enable the output
  delay(10);

  // Read the output from the decoder into tempCount
  high_byte = PINF;

  // Set the SEL bit high to read the low byte
  PORTC = 0b10100000;
  delay(10);

  low_byte = PINF;

  // Clear OE to enable output
  PORTC =0b11100000;
  return low_byte;

}

void decoderSend(byte low_b, byte high_b)
{
  Serial.write(START); //Send the start byte indicating the start of a package.
  Serial.write(low_b); //Send the value read from Port F.
  Serial.write(high_b); //Send the value read from Port F.

  int checkSum1 = START + low_byte + high_byte; //Calculate the check sum.
  Serial.write(checkSum1); //Send the check sum.
}

//Function to reverse the order of the bits.
byte bitFlip(byte value)
{
  byte bFlip = 0;
  byte j=7;
  for (byte i=0; i<8; i++)
  { 
    bitWrite(bFlip, i, bitRead(value, j));
    j--;
  }
  return bFlip;
}





