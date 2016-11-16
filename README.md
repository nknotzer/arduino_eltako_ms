# arduino_eltako_ms
Arduino webserver. Displays measurement results from "Eltako Multisensor MS" (http://www.eltako.com/fileadmin/downloads/en/_datasheets/Datasheet_MS.pdf) obtained via RS 485 bus as html-file. You need to connect your arduino hardware serial interface to the RS485 bus of the Eltako via a RS 485 converter (e.g. MAX485) chip.

Sample output:

RawData.value W+02.5000000J00000.6J?151515151515?1841<br>
CheckSum.value 1841<br>
RecvSum.value 1841<br>
Temperature.value 2.50<br>
Daylight.value 0<br>
Dawn.value 1<br>
Rain.value 1<br>
Windspeed.value 0.60<br>
SunSouth.value 0<br>
SunWest.value 0<br>
SunEast.value 0<br>

