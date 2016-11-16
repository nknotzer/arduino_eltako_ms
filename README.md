# arduino_eltako_ms
Arduino webserver. Displays measurement results from "Eltako Multisensor MS" (http://www.eltako.com/fileadmin/downloads/en/_datasheets/Datasheet_MS.pdf) obtained via RS 485 bus as html-file. You need to connect your arduino hardware serial interface to the RS485 bus of the Eltako via a RS 485 converter (e.g. MAX485) chip.

Sample output:

RawData.value W+02.5000000J00000.6J?151515151515?1841
CheckSum.value 1841
RecvSum.value 1841
Temperature.value 2.50
Daylight.value 0
Dawn.value 1
Rain.value 1
Windspeed.value 0.60
SunSouth.value 0
SunWest.value 0
SunEast.value 0
