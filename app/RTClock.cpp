#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Debug.h>
#include <globals.h>
#include <AppSettings.h>
#include "RTClock.h"

#if PLATFORM_TYPE == PLATFORM_TYPE_GENERIC
#include "Sodaq_DS3231.h"
#elif PLATFORM_TYPE == PLATFORM_TYPE_SDSHIELD
#include "RTClib.h"
RTC_DS1307 rtc1307;
#else
    #error "Unknown platform type"
#endif

void RTClock::checkState()
{
#if PLATFORM_TYPE == PLATFORM_TYPE_GENERIC
    SystemClock.setTime(rtc.now().getEpoch(), eTZ_UTC);

    //convert current temperature into registers
    rtc.convertTemperature();
    //read registers and display the temperature
    Debug.printf("Plug temperature %02f deg C\n",
                 rtc.getTemperature()); 
    if (changeDlg)
        changeDlg("RTC-temperature", String(rtc.getTemperature()));

#elif PLATFORM_TYPE == PLATFORM_TYPE_SDSHIELD
    SystemClock.setTime(rtc1307.now().unixtime(), eTZ_UTC);
#endif
}

void RTClock::begin(RTCChangeDelegate dlg)
{
    byte error;

    changeDlg = dlg;

#if PLATFORM_TYPE == PLATFORM_TYPE_GENERIC
    Wire.lock();
    Wire.beginTransmission(0x68);
    error = Wire.endTransmission();
    Wire.unlock();

    WDT.alive(); //Make doggy happy

    if (error == 0)
    {
        RTCFound = TRUE;
        SystemClock.setTimeZone(1); //TODO make this configurable!
        Debug.printf("Found RTC DS3213 at address 0x68\n");
        rtc.begin(0x68);
        SystemClock.setTime(rtc.now().getEpoch(), eTZ_UTC);
        Debug.print(" Time = ");
        Debug.println(SystemClock.getSystemTimeString());

        rtc.convertTemperature();             //convert current temperature into registers
        Debug.printf(" %02f deg C\n", rtc.getTemperature()); //read registers and display the temperature
    }

#else
    Wire.lock();

    Wire.beginTransmission(0x68);
    error = Wire.endTransmission();

    WDT.alive(); //Make doggy happy

    if (error == 0)
    {
        RTCFound = TRUE;
        SystemClock.setTimeZone(1); //TODO make this configurable!
        Debug.printf("Found RTC DS1307 at address 0x68\n");
        rtc1307.begin();
        SystemClock.setTime(rtc1307.now().unixtime(), eTZ_UTC);
        Debug.print(" Time = ");
        Debug.println(SystemClock.getSystemTimeString());
    }

    Wire.unlock();
#endif

    if (RTCFound)
    {
        checkTimer.initializeMs(60000, TimerDelegate(&RTClock::checkState, this)).start(true);
    }
    else
    {
        Debug.printf("No RTC found\n");
    }
}

void RTClock::setTime(uint32_t ts)
{
#if PLATFORM_TYPE == PLATFORM_TYPE_GENERIC
    if (RTCFound)
        rtc.setEpoch(ts);
#else
    if (RTCFound)
        rtc1307.adjust(MyDateTime2(ts));
#endif
}

uint32_t RTClock::getTime()
{
#if PLATFORM_TYPE == PLATFORM_TYPE_GENERIC
    if (RTCFound)
        return rtc.now().getEpoch();
    return 0;
#else
    if (RTCFound)
        return rtc1307.now().unixtime();
    return 0;
#endif
}
