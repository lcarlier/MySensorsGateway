#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Debug.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <globals.h>

#include <AppSettings.h>
#include "i2c.h"
#include "mqtt.h"

#include <SHA204.h>
#include <SHA204Definitions.h>
#include "SHA204I2C.h"

Adafruit_SSD1306 display(-1); // reset Pin required but later ignored if set to False

void MyI2C::showOLED()
{
	Wire.lock();

	display.clearDisplay();
	// text display tests
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
        display.println("MySensors gateway");
	display.setTextSize(1);
	display.setCursor(0,9);
        if (WifiStation.isConnected())
        {
          display.print("AP  :");
          display.println(WifiStation.getIP().toString());
        } 
        else
        {
	  display.setTextColor(BLACK, WHITE); // 'inverted' text
          display.println("connecting ...");
	  display.setTextColor(WHITE);
        }
	display.setCursor(0,18);
        if (isMqttConfigured())
        {
          display.print("MQTT:");
          display.println(MqttServer());
        }
        else
        {
	  display.setTextColor(BLACK, WHITE); // 'inverted' text
          display.println("configure MQTT !");
	  display.setTextColor(WHITE);
        }

	display.setCursor(0,27);
        display.println(SystemClock.getSystemTimeString().c_str());
	display.setCursor(0,36);
        display.print("HEAP :");
	display.setTextColor(BLACK, WHITE); // 'inverted' text
        display.println(system_get_free_heap_size());

	display.setTextColor(WHITE);

	//display.setTextColor(BLACK, WHITE); // 'inverted' text
	//display.setTextSize(3);
	display.display();

	Wire.unlock();
}

void MyI2C::begin(I2CChangeDelegate dlg)
{
    byte error, address;

    changeDlg = dlg;

    // Belows works on Wemos with swapped ATSHA204 pinning as in datasheet !
    Wire.pins(4, 5); // needed to swap : SCL, SDA  : will fix PCB !!!!
    // Belows works on Wemos with ATSHA204 pinning as in datasheet
    //Wire.pins(5, 4); // needed to swap : SCL, SDA  : will fix PCB !!!!
    Wire.begin();

    for (address=0; address < 127; address++)
    {
        Wire.beginTransmission(address);
	error = Wire.endTransmission();

	WDT.alive(); //Make doggy happy

	if (error == 0)
	{
	    if (address >= 0x20 && address <= 0x26)
            {
                Debug.printf("Found MCP23017 expander at %x\n", address);
            }
#if 0
            else if (address == 0x27)
            {
                Debug.printf("Found LCD at address %x\n", address);
                lcd.begin(20, 4);
                lcd.setCursor(0, 0);
                lcd.print((char *)"   cloud-o-mation   ");
                lcd.setCursor(0, 2);
                lcd.print((char *)build_git_sha);
                lcdFound = TRUE;
            }
#endif
            else if (address >= 0x48 && address <= 0x4f)
            {
                Debug.printf("Found PCF8591 expander at %x\n", address);
            }
            else if (address == 0x68)
            {
#if PLATFORM_TYPE == PLATFORM_TYPE_GENERIC
                Debug.printf("Found RTC DS3213 at address %x\n", address);
#elif PLATFORM_TYPE == PLATFORM_TYPE_SDSHIELD
                Debug.printf("Found RTC DS1307 at address %x\n", address);
#else
    #error "Unknown platform type"
#endif
            } 
            else if (address == 0x57)
            {
                Debug.printf("Found ATtiny %x\n", address);
            }
            else if (address == 0x64)
            {
                Debug.printf("Found Atsha204 %x\n", address);
                uint8_t response[SHA204_RSP_SIZE_MIN];
                byte returnValue;
                SHA204I2C sha204dev;
                uint8_t serialNumber[9];
                // TODO : MUTEX !!! 
                // On my Wemos proto, ATSHA is the only component on the bus.

                sha204dev.init(); // Be sure to wake up device right as I2C goes up otherwise you'll have NACK issues 

                //BEGIN of TESTCODE
                returnValue = sha204dev.serialNumber(serialNumber);
                for (int i=0; i<9; i++) {
                    Debug.print(serialNumber[i], HEX);
                    Debug.print(" ");
                }
                Debug.println();
                Debug.println("Asking the SHA204's serial number. Response should be:");
                Debug.println("1 23 x x x x x x x EE");
                Debug.println("-------"); 

                returnValue = sha204dev.resync(4, &response[0]);
                for (int i=0; i<SHA204_RSP_SIZE_MIN; i++) {
                    Debug.print(response[i], HEX);
                    Debug.print(" ");
                }
                Debug.println();
                //END of TESTCODE

            }
            else if (address == 0x3c)
            {
                OLEDFound = TRUE;
                Debug.printf("Found OLED %x\n", address);
                // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)`
                // initialize with the I2C addr 0x3D (for the 128x64)
                // bool:reset set to TRUE or FALSE depending on you display
                display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, FALSE);
                // display.begin(SSD1306_SWITCHCAPVCC);
                display.display();
            }
            else
            {
                Debug.printf("Unexpected I2C device found @ %x\n", address);
            }
        }
    }

    if (OLEDFound)
    {
        i2cOLEDTimer.initializeMs(1000, TimerDelegate(&MyI2C::showOLED, this)).start(true);
    }
}
