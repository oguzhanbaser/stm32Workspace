#ifndef DHT11_H
#define DHT11_H

#include "mbed.h"

#define DHTLIB_OK                0
#define DHTLIB_ERROR_CHECKSUM   -1
#define DHTLIB_ERROR_TIMEOUT    -2

/** Class for the DHT11 sensor.
 * 
 * Example:
 * @code
 * #include "mbed.h"
 * #include "Dht11.h"
 *
 * Serial pc(USBTX, USBRX);
 * Dht11 sensor(PTD7);
 * 
 * int main() {
 *     sensor.read()
 *     pc.printf("T: %f, H: %d\r\n", sensor.getFahrenheit(), sensor.getHumidity());
 * }
 * @endcode
 */
class Dht11
{
public:
    /** Construct the sensor object.
     *
     * @param pin PinName for the sensor pin.
     */
    Dht11(PinName const &p);
    
    /** Update the humidity and temp from the sensor.
     *
     * @returns
     *   0 on success, otherwise error.
     */
    int read();
    
    /** Get the temp(f) from the saved object.
     *
     * @returns
     *   Fahrenheit float
     */
    float getFahrenheit();
    
    /** Get the temp(c) from the saved object.
     *
     * @returns
     *   Celsius int
     */
    int getCelsius();
    
    /** Get the humidity from the saved object.
     *
     * @returns
     *   Humidity percent int
     */
    int getHumidity();

private:
    /// percentage of humidity
    int _humidity;
    /// celsius
    int _temperature;
    /// pin to read the sensor info on
    DigitalInOut _pin;
    /// times startup (must settle for at least a second)
    Timer _timer;
};

#endif
