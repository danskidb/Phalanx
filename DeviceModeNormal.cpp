#include "DeviceModeNormal.h"
#include <ESP8266WiFi.h>

bool DeviceModeNormal::Start()
{
	EEPROM.get(0, saveData);

    timeClient = new NTPClient(ntpUDP, "pool.ntp.org");

	timeClient->begin();
	timeClient->setUpdateInterval(1800000); // 30 min
	timeClient->setTimeOffset(0);

    return true;
}

bool DeviceModeNormal::Stop()
{
	timeClient->end();
	delete timeClient;
    return true;
}

void DeviceModeNormal::OnTick()
{
	timeClient->update();
	int hours = timeClient->getHours();
	if(saveData.time_12hmode && hours > 12)
		hours -= 12;

	display->ShiftCurrentTime(hours, timeClient->getMinutes(), timeClient->getSeconds());
    
	delay(delayBetweenTicks);
	timeSinceLastSettingsSync += delayBetweenTicks;
	if(timeSinceLastSettingsSync >= delayBetweenSettingsSync)
	{
		EEPROM.get(0, saveData);
		timeSinceLastSettingsSync = 0;
	}
}