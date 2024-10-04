#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "DHT.h"
#include "RTClib.h"

#define SD_CS 5
#define SD_SCK 18
#define SD_MOSI 23
#define SD_MISO  19
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DHTPIN 25
#define DHTTYPE DHT22 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

SPIClass sd_spi(HSPI);

File file;

const char* path = "/TESSS.txt";

DHT dht(DHTPIN, DHTTYPE);

RTC_DS3231 rtc;

char dataHari[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
String hari;
int tanggal, bulan, tahun, jam, menit, detik, sensor, sensor1;
float h, t;

float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

const unsigned long interval = 2000;  // Interval 2 detik
unsigned long previousMillis = 0;
int functionIndex = 0;

// FUNGSI OLED
// Select I2C BUS
void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(0x70);  // TCA9548A address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

void OLED_print(String baris1, String baris2) {
  TCA9548A(0);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(5, 10);
  display.print(baris1);
  display.setTextSize(2);
  display.setCursor(5, 30);
  display.print(baris2);
  display.display();
}

void OLED1_print(String baris3, String baris4) {
  TCA9548A(1);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(5, 10);
  display.print(baris3);
  display.setTextSize(2);
  display.setCursor(5, 30);
  display.print(baris4);
  display.display();
}

void oleds() {
  // Start I2C communication with the Multiplexer
  Wire.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);  // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  // Tampilkan pesan awal di OLED
  OLED_print("CO2", "PURIFIER");
  OLED1_print("LAB.", "MATERIAL");
}

// FUNGSI SDCARD
void sdcs() {
  sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, sd_spi)) {
    Serial.println("Gagal Memuat Kartu SD");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("Tidak Ada Kartu SD");
    return;
  }
  Serial.println("Menginisialisasi kartu SD...");

  // Periksa apakah file dapat dibuat atau dibuka
  if (!SD.exists(path)) {
    file = SD.open(path, FILE_WRITE);
    if (file) {
      Serial.println("File baru dibuat");
      file.close();
    } else {
      Serial.println("Gagal membuat file");
    }
  }
}

// Fungsi untuk menulis data ke SD card dengan data waktu
void sdcl(String waktu, String data1, String data2) {
  Serial.printf("Writing file: %s\n", path);

  file = SD.open(path, FILE_APPEND);

  if (file) {
    // Menyusun format data yang akan ditulis
    String logData = waktu + " | " + data1 + " | " + data2 + "\n";
    if (file.print(logData)) {
      Serial.println("File written");
    } else {
      Serial.println("Write failed");
    }
    file.close();
  } else {
    Serial.println("Error opening file");
  }
}

// FUNGSI RTC
void rtcs() {
  if (!rtc.begin()) {
    Serial.println("RTC Tidak Ditemukan");
    Serial.flush();
    abort();
  }
}

void rtcl() {
  DateTime now = rtc.now();
  hari = dataHari[now.dayOfTheWeek()];
  tanggal = now.day();
  bulan = now.month();
  tahun = now.year();
  jam = now.hour();
  menit = now.minute();
  detik = now.second();
}

// Fungsi untuk mendapatkan waktu sebagai string
String getWaktu() {
  return hari + ", " + String(tanggal) + "/" + String(bulan) + "/" + String(tahun) + " " + String(jam) + ":" + String(menit) + ":" + String(detik);
}

// FUNGSI DHT22
void dhts() {
  dht.begin();
}

void dhtl() {
  h = dht.readHumidity();
  t = dht.readTemperature();
}

// FUNGSI 2 SENSOR
void sensors() {
  // set the ADC attenuation to 11 dB (up to ~3.3V input)
  analogSetAttenuation(ADC_11db);
}

void sensorl() {
  // read the input on analog pin GPIO36:
  int analogValue = analogRead(32);
  int analogValue1 = analogRead(33);

  // Rescale to potentiometer's voltage (from 0V to 3.3V):
  float voltage = floatMap(analogValue, 0, 4095, 0, 3.3);
  float voltage1 = floatMap(analogValue1, 0, 4095, 0, 3.3);

  //Rescale to PPM CO2 Range
  sensor = map(voltage, 0, 3.3, 400, 5000);
  sensor1 = map(voltage1, 0, 3.3, 400, 5000);
}

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  oleds();
  delay(2000);
  rtcs();
  sdcs();
  sensors();
  dhts();
}

// the loop routine runs over and over again forever:
void loop() {
  unsigned long currentMillis = millis();
  rtcl();    // Update waktu saat ini
  sensorl(); // Update pembacaan sensor
  dhtl();    // Update pembacaan suhu dan kelembapan

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Format data waktu
    String waktu = getWaktu();

    // Panggil fungsi berdasarkan functionIndex
    switch (functionIndex) {
      case 0:
        OLED_print("CO2 Inlet", String(sensor) + " PPM");
        OLED1_print("CO2 Outlet", String(sensor1) + " PPM");
        sdcl(waktu, "CO2 Inlet = " + String(sensor) + " PPM", "CO2 Outlet = " + String(sensor1) + " PPM");
        break;
      case 1:
        OLED_print("Suhu Udara", String(t) + " C");
        OLED1_print("Kelembapan", String(h) + " %");
        sdcl(waktu, "Suhu Udara = " + String(t) + " C", "Kelembapan = " + String(h) + " %");
        break;
    }
    // Ubah functionIndex untuk memanggil fungsi berikutnya
    functionIndex++;
    if (functionIndex > 1) {  // Perbaikan index range (seharusnya > 1 bukan 2)
      functionIndex = 0;  // Kembali ke fungsi pertama
    }
  }
}
