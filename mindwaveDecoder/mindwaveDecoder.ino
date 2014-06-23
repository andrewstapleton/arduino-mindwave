
// Based on helloattention.pde - Show attention level (EEG) with LED color.
// (c) Kimmo Karvinen & Tero Karvinen http://MindControl.BotBook.com
// Disconnect TX and RX jump wires from Arduino when uploading from IDE.


// This is updated code that gets full data payload from Mindwave Packets and fixes the issues with setColor()
// by having the rgb values directly enter the analogWrite for each of the 3 pins


// (c) Andrew Stapleton


const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;
const int tinyLedPin = 13;
const int speakerPin = 12;

int tinyLedState = HIGH;

void setup () {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    pinMode(tinyLedPin, OUTPUT);
    pinMode(speakerPin, OUTPUT);
    Serial.begin(115200); // bit/s //
    connectHeadset(); //
}
void loop()
{
    float att = getAttention(); //
    if (att > 0) //
    {
        setBlueToRed(att);
        Serial.print("Attention: ");
        Serial.println(att);
    }
    
    //if (brain.update())
    //{
    //  Serial.println(brain.readCSV());
    //}
    
    toggleTinyLed(); //
}
/*** Headset ***/
void connectHeadset()
{
    setGreen();
    delay(3000);
    Serial.write(0xc2); //
    setWhite();
}
byte readOneByte()
{
    while (!Serial.available()) { //
        delay(5); // ms
    };
    return Serial.read();
}
float getAttention()
{ // return attention percent (0.0 to 1.0)
    // negative (-1, -2...) for error
    byte generatedChecksum = 0; //
    byte checksum = 0;
    int payloadLength = 0;
    byte payloadData[64] = {0};
    
    
    int poorQuality = 0;
    float attention = 0;
    unsigned int delta = 0;
    unsigned int theta = 0;
    unsigned int low_alpha = 0;
    unsigned int high_alpha = 0;
    unsigned int low_beta = 0;
    unsigned int high_beta = 0;
    unsigned int low_gamma = 0;
    unsigned int mid_gamma = 0;
    
    
    
    
    Serial.flush(); // prevent serial buffer from filling up //
    /* Sync */
    if (170 != readOneByte()) return -1; //
    if (170 != readOneByte()) return -1;
    /* Length */
    payloadLength = readOneByte();
    if (payloadLength > 169) return -2; //
    /* Checksum */
    generatedChecksum = 0;
    for (int i = 0; i < payloadLength; i++) { //
        // Read payload into array:
        payloadData[i] = readOneByte();
        generatedChecksum += payloadData[i];
    }
    generatedChecksum = 255 - generatedChecksum;
    checksum = readOneByte();
    if (checksum != generatedChecksum) return -3; //
    /* Payload */
    for (int i = 0; i < payloadLength; i++) { //
        switch (payloadData[i]) {
            case 0xD0:
                sayHeadsetConnected();
                break;
            case 4: //
                i++; //
                attention = payloadData[i]; //
                break;
            case 2:
                i++;
                poorQuality = payloadData[i];
                if (200 == poorQuality) {
                    setYellow(); //
                    return -4;
                }
                break;
            case 0xD1: // Headset Not Found
            case 0xD2: // Headset Disconnected
            case 0xD3: // Request Denied
            case -70:
                wave(speakerPin, 900, 500);
                setWhite();
                return -5;
                break;
                
            case 0x80: // skip RAW //
                i = i + 3; // skip 3 bytes
                break;
            case 0x83: // get payload data
                
                byte buff[4];
                
                
                    
                    // read 3
                    for(int d = 0; d<3; d++)
                    {
                     i++;
                     buff[d] = payloadData[i];
                    }
                
                    delta = (( (unsigned long)buff[0] << 24)
                         +  ( (unsigned long)buff[1] << 16)
                		 +  ( (unsigned long)buff[2] << 8 )
                		 +  ( (unsigned long)buff[0])); // need to check this (plus endian-ness) // ensure values make sense
                
                Serial.print("delta: ");
                Serial.println(delta);
                
                
                /*
                    // read 3
                    theta
                    
                    // read 3
                    low_alpha
                    
                    // read 3
                	high_alpha
                    
                    // read 3
                    low_beta
                    
                    // read 3
                    high_beta
                    
                    // read 3
                    low_gamma
                    
                    // read 3
                    mid_gamma
               
                */
        
                // advance to push the head past Mr. Crackles
                i = i + 22;
                
                break;
                
                
        } // switch
    } // for
    return (float)attention / 100; //
}


/*** Outputs ***/
void setBlueToRed(float redPercent)
{
    int red = redPercent * 255;
    int blue = (1 - redPercent) * 255;
    setColor(red, 0, blue);
}

void setGreen()
{
    setColor(0, 255, 0);
}
void setYellow()
{
    setColor(255, 255, 0);
}
void setWhite()
{
    setColor(100, 100, 100);
}


void sayHeadsetConnected()
{
    wave(speakerPin, 440, 40);
    delay(25);
    wave(speakerPin, 300, 20);
    wave(speakerPin, 540, 40);
    delay(25);
    wave(speakerPin, 440, 20);
    wave(speakerPin, 640, 40);
    delay(25);
    wave(speakerPin, 540, 40);
    delay(25);
}
void setColor(int red, int green, int blue)
{
    // updated 16-Apr-2014
    // all analogWrite values now the 'theColor' instead of '255 - theColor'
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
}
void toggleTinyLed()
{
    tinyLedState = !tinyLedState;
    digitalWrite(tinyLedPin, tinyLedState);
}
void wave(int pin, float frequency, int duration)
{
    float period = 1 / frequency * 1000 * 1000; // microseconds
    long int startTime = millis();
    while (millis() - startTime < duration) {
        digitalWrite(pin, HIGH);
        delayMicroseconds(period / 2);
        digitalWrite(pin, LOW);
        delayMicroseconds(period / 2);
    }
}
