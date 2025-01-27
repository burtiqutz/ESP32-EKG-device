#include <Wire.h>
#include <U8g2lib.h>

#define LO_PLUS_PIN 25       // Setup for leads off detection LO+
#define LO_MINUS_PIN 26      // Setup for leads off detection LO-
#define ANALOG_INPUT_PIN 34  // Analog input for AD8232 signal

// Define custom SDA and SCL pins
#define SDA_PIN 32  // Use GPIO 32 for SDA
#define SCL_PIN 33  // Use GPIO 33 for SCL

// Initialize the display with the SH1106 driver using custom SDA and SCL pins
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SDA_PIN, SCL_PIN, /* reset=*/ U8X8_PIN_NONE);

// Array to store ECG data (graph data buffer)
int ecgData[128];  // 128 data points for the ECG graph
int dataIndex = 0;     // To keep track of the data index (renamed variable)

// Define moving average size
#define MOVING_AVERAGE_SIZE 7  // Increase the moving average window size to 7
int movingAverageBuffer[MOVING_AVERAGE_SIZE];  // Buffer to store the last N readings
int movingAverageIndex = 0;  // Index for the moving average buffer

// Low-pass filter parameters
float lastFilteredValue = 0.0;
float alpha = 0.1;  // Decrease the low-pass filter coefficient to 0.1 (less smoothing)

#define SIGNAL_BOOST_FACTOR 1.5  // Reduce the signal boost factor to 1.5 (instead of 2)

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing ECG...");
  pinMode(LO_PLUS_PIN, INPUT);
  pinMode(LO_MINUS_PIN, INPUT);
  
  analogSetAttenuation(ADC_0db); // Configure ADC for 0-3.3V range
  
  // Initialize the I2C communication
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize the OLED display
  Serial.println("Initializing display...");
  u8g2.begin();  // Initialize the display
  if (!u8g2.begin()) {
    Serial.println("Error: Display not found!");
    while (1);  // Halt execution if the display is not found
  }
  Serial.println("Display initialized successfully.");
}

void loop() {
  int loPlus = digitalRead(LO_PLUS_PIN);
  int loMinus = digitalRead(LO_MINUS_PIN);
  
  if (loPlus == HIGH || loMinus == HIGH) {
    Serial.println("! Lead off detected");
  } else {
    // Read the ECG signal from the AD8232
    int sensorValue = 0;
    for (int i = 0; i < 10; i++) { // Average 10 readings
      sensorValue += analogRead(ANALOG_INPUT_PIN);
    }
    sensorValue /= 10;  // Average the value
    
    // Apply signal processing techniques to improve the signal
    sensorValue = applySignalBoost(sensorValue); // Boost the signal if needed
    sensorValue = applyLowPassFilter(sensorValue); // Apply low-pass filter
    sensorValue = applyMovingAverage(sensorValue); // Apply moving average filter
    
    // Shift the ECG data for new data point
    for (int i = 0; i < 127; i++) {
      ecgData[i] = ecgData[i + 1];
    }
    ecgData[127] = sensorValue;  // Add new ECG value to the end of the array
    
    // Display the ECG graph
    displayECGGraph();
  }
  
  delay(10); // Add slight delay for stability
}

// Function to apply signal boost (amplifies the signal)
int applySignalBoost(int rawValue) {
  int boostedValue = rawValue * SIGNAL_BOOST_FACTOR;
  
  // Limit the boosted value to the maximum of 4095 (12-bit ADC max value)
  if (boostedValue > 4095) {
    boostedValue = 4095;
  }
  return boostedValue;
}

// Function to apply low-pass filter
int applyLowPassFilter(int currentValue) {
  lastFilteredValue = lastFilteredValue + alpha * (currentValue - lastFilteredValue);
  return (int)lastFilteredValue;
}

// Function to apply moving average filter
int applyMovingAverage(int newValue) {
  // Shift the old values in the buffer
  for (int i = 1; i < MOVING_AVERAGE_SIZE; i++) {
    movingAverageBuffer[i - 1] = movingAverageBuffer[i];
  }
  
  // Add the new value to the buffer
  movingAverageBuffer[MOVING_AVERAGE_SIZE - 1] = newValue;
  
  // Calculate the average of the values in the buffer
  int sum = 0;
  for (int i = 0; i < MOVING_AVERAGE_SIZE; i++) {
    sum += movingAverageBuffer[i];
  }
  int averageValue = sum / MOVING_AVERAGE_SIZE;  // Get the average
  
  return averageValue;
}

// Function to display the ECG graph
void displayECGGraph() {
  u8g2.clearBuffer();  // Clear the internal memory buffer

  // Set the display parameters
  u8g2.setFont(u8g2_font_ncenB08_tr); // Use a simple font

  // Move the label below the graph
  u8g2.drawStr(0, 10, "ECG Graph");

  // Draw ECG data on the graph (line by line)
  for (int i = 1; i < 128; i++) {
    // Map the sensor value to the display range (0-48) based on the data range
    int prevValue = map(ecgData[i - 1], 0, 4095, 0, 48);  // Map value to display height (lower scale)
    int currentValue = map(ecgData[i], 0, 4095, 0, 48);  // Map value to display height (lower scale)

    // Draw line between previous and current ECG data point
    u8g2.drawLine(i - 1, 64 - prevValue, i, 64 - currentValue);
  }

  u8g2.sendBuffer();  // Push the buffer to the screen
}
