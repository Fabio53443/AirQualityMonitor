#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <curl/curl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

#define I2C_ADDRESS 0x77 // BME860 sensor I2C address
#define I2C_BUS "/dev/i2c-2" // I2C bus

// Function to read data from BME860 sensor
void readBME860(int& temperature, int& humidity, int& pressure)
{
    int fd = open(I2C_BUS, O_RDWR);
    ioctl(fd, I2C_SLAVE, I2C_ADDRESS);

    // Read temperature
    uint8_t cmd[2] = {0xFE, 0x11}; // Temperature command
    write(fd, cmd, sizeof(cmd));
    usleep(10000);
    char buf[6] = {0};
    read(fd, buf, sizeof(buf));
    temperature = ((buf[3] & 0xFF) << 8) | (buf[4] & 0xFF);

    // Read humidity
    cmd[0] = 0xFE;
    cmd[1] = 0x21; // Humidity command
    write(fd, cmd, sizeof(cmd));
    usleep(10000);
    read(fd, buf, sizeof(buf));
    humidity = ((buf[1] & 0xFF) << 8) | (buf[2] & 0xFF);

    // Read pressure
    cmd[0] = 0xFE;
    cmd[1] = 0x31; // Pressure command
    write(fd, cmd, sizeof(cmd));
    usleep(10000);
    read(fd, buf, sizeof(buf));
    pressure = ((buf[1] & 0xFF) << 16) | ((buf[2] & 0xFF) << 8) | (buf[3] & 0xFF);

    close(fd);
}

// Function to write data to CSV file
void writeCSV(int temperature, int humidity, int pressure)
{
    auto now = chrono::system_clock::now();
    time_t timestamp = chrono::system_clock::to_time_t(now);
    char filename[80];
    strftime(filename, sizeof(filename), "airquality_%Y%m%d.csv", localtime(&timestamp));
    ofstream outfile(filename, ios::app);
    outfile << timestamp << "," << temperature << "," << humidity << "," << pressure << endl;
    outfile.close();
}

// Function to send data via POST request
void sendPOST(string data)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/data");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}

int main()
{
    int temperature, humidity, pressure;
    while (true) {
        readBME860(temperature, humidity, pressure);
        writeCSV(temperature, humidity, pressure);
        sleep(1800); // Sleep for 30 minutes
        time_t now = time(0);
    if (now % 72000 == 0) { // Send data every 20 hours
        string data = "{\"temperature\":" + to_string(temperature) + ",\"humidity\":" + to_string(humidity) + ",\"pressure\":" + to_string(pressure) + "}";
        sendPOST(data);
    }
}
    return 0;
}