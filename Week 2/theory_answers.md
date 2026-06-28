# Q19 & Q20: Arduino Fundamentals Theory

## Q19: Difference Between analogWrite() and analogRead(); What is PWM?

### **analogWrite() vs analogRead()**

#### **analogRead(pin)**
- **Purpose**: Read analog voltage from an input pin (A0-A5 on Arduino UNO)
- **Return Value**: Integer from 0 to 1023 (10-bit ADC resolution)
  - 0 = 0V (GND)
  - 1023 = 5V (VCC)
- **Usage**: Sensor input (potentiometer, light sensor, temperature sensor)
- **Example**:
  ```cpp
  int sensor_value = analogRead(A0); // Reads pot voltage as 0-1023
  int voltage_mV = map(sensor_value, 0, 1023, 0, 5000);
  ```

#### **analogWrite(pin, value)**
- **Purpose**: Output analog-like voltage via PWM (digital pulse variation)
- **Value Range**: 0 to 255
  - 0 = always OFF (0% duty cycle)
  - 127 = 50% ON/OFF (50% duty cycle)
  - 255 = always ON (100% duty cycle)
- **Usage**: Actuator control (LED brightness, motor speed, servo position)
- **Example**:
  ```cpp
  analogWrite(9, 128);  // LED at ~50% brightness
  ```

**Key Difference**: `analogRead()` is **input** (ADC), `analogWrite()` is **output** (PWM)

---

### **What is PWM (Pulse-Width Modulation)?**

PWM simulates analog output on a digital pin by rapidly switching between ON (HIGH, 5V) and OFF (LOW, 0V).

#### **PWM Concept:**
```
Duty Cycle = (ON time) / (Total period)

Example: 50% duty cycle on a 1ms period:
  ON for 0.5ms (HIGH 5V)
  OFF for 0.5ms (LOW 0V)
  Result: Average voltage = 2.5V (perceived as mid-brightness)
```

#### **Visual Representation:**
```
5V ┌─┐ ┌─┐ ┌─┐
   │ │ │ │ │ │
0V └─┴─┴─┴─┴─┴─  (25% duty cycle, dim LED)

5V ┌───┐ ┌───┐
   │   │ │   │
0V └───┴─┴───┴─  (50% duty cycle, medium brightness)

5V ┌─────┐ ┌───
   │     │ │
0V └─────┴─┴───  (75% duty cycle, bright LED)
```

#### **Arduino PWM Pins**:
- UNO: pins 3, 5, 6, 9, 10, 11 support PWM
- ESP32: all GPIO pins support PWM
- Default frequency: ~490 Hz (fast enough human eye sees steady brightness)

---

### **Real-World IoT Example:**

**Smart Lighting System (Q23: Smart Street Light)**
```cpp
if (dark && motion_detected) {
  analogWrite(LED_PIN, 255);  // Full brightness (100% ON)
} else if (dark && !motion_detected) {
  analogWrite(LED_PIN, 51);   // 20% brightness (fade after 30s)
} else {
  analogWrite(LED_PIN, 0);    // Off (daylight)
}
```
This saves energy by dimming the street light when no motion is detected at night.

---

### **Summary Table:**

| Feature | analogRead() | analogWrite() |
|---------|--------------|---------------|
| Direction | Input | Output |
| Hardware | ADC (Analog-to-Digital Converter) | PWM (Pulse-Width Modulation) |
| Value Range | 0-1023 | 0-255 |
| Voltage Mapping | 0V → 0, 5V → 1023 | 0 → 0%, 255 → 100% |
| Common Usage | Sensors (pot, LDR, temp) | Actuators (LED, motor, servo) |
| Pin Example (UNO) | A0-A5 | 3, 5, 6, 9, 10, 11 |

---

---

## Q20: Understanding setup() and loop(); Why delay() Blocks; Non-Blocking Timing with millis()

### **setup() Function**
- **Runs Once**: Executes only once when Arduino starts or resets
- **Purpose**: Initialize pins, Serial, variables, sensors
- **Example**:
  ```cpp
  void setup() {
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(9600);
  }
  ```

### **loop() Function**
- **Runs Repeatedly**: Executes continuously after setup() completes
- **Purpose**: Main program logic, read sensors, control actuators
- **Frame Rate**: Frequency depends on code complexity (typically 1000s-10000s Hz)
- **Example**:
  ```cpp
  void loop() {
    int sensor = analogRead(A0);
    digitalWrite(LED_PIN, sensor > 512 ? HIGH : LOW);
  }
  ```

---

### **Problem: Why delay() Blocks Everything**

When you call `delay(1000)`, the entire microcontroller stops for 1 second:

```cpp
void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);              // ⚠️ BLOCKS! Nothing else happens for 1000ms
  
  digitalWrite(LED_PIN, LOW);
  delay(1000);              // ⚠️ BLOCKS AGAIN!
  
  // Serial input ignored during delays
  // Button presses missed during delays
  // Sensor readings missed during delays
}
```

**Effect on Sensor Responsiveness:**
- **Blocked for 2 seconds** per loop iteration
- **Button press lost** if it occurs during delay() call
- **Serial commands ignored** while delayed
- **Poor real-world performance** in multi-tasking scenarios

---

### **Solution: Non-Blocking Timing with millis()**

`millis()` returns elapsed milliseconds since Arduino startup (0-4,294,967,295 then wraps).

#### **Comparison:**

```cpp
// ❌ BLOCKING APPROACH (BAD)
void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);              // Processor stuck here!
  digitalWrite(LED_PIN, LOW);
  delay(1000);              // Processor stuck here!
}

// ✅ NON-BLOCKING APPROACH (GOOD)
unsigned long led_time = 0;

void loop() {
  unsigned long now = millis();
  
  if ((now - led_time) >= 1000) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle LED
    led_time = now;
  }
  
  // This code runs every iteration (responsive!):
  check_buttons();
  read_sensors();
  process_serial_commands();
}
```

---

### **Non-Blocking Pattern (General Template):**

```cpp
// Global variables
unsigned long last_action_time = 0;
const unsigned long ACTION_INTERVAL = 1000; // milliseconds

void loop() {
  unsigned long current_time = millis();
  
  // Check if interval has elapsed
  if ((current_time - last_action_time) >= ACTION_INTERVAL) {
    // Time to act!
    perform_action();
    last_action_time = current_time;
  }
  
  // Other responsive code here (always executes every loop):
  read_sensors();
  check_buttons();
  respond_to_commands();
}
```

---

### **Real-World Example: Multi-Sensor Monitoring**

**Blocking (Poor):**
```cpp
void loop() {
  // Read temp, ignore other sensors during delay
  float temp = read_temp();
  delay(5000);  // ⚠️ Ignores motion, light, distance sensors for 5s!
  
  // When we get here, motion might have been missed
}
```

**Non-Blocking (Excellent for IoT):**
```cpp
unsigned long temp_time = 0, motion_time = 0, light_time = 0;

void loop() {
  unsigned long now = millis();
  
  // Read temperature every 5 seconds
  if ((now - temp_time) >= 5000) {
    float temp = read_temp();
    temp_time = now;
  }
  
  // Read motion every 100ms (20x per second)
  if ((now - motion_time) >= 100) {
    bool motion = read_pir();
    if (motion) handle_motion();  // Responds immediately!
    motion_time = now;
  }
  
  // Read light every 200ms
  if ((now - light_time) >= 200) {
    int brightness = read_ldr();
    light_time = now;
  }
  
  // All happens simultaneously, responsive to all events
}
```

**Benefit**: Motion detected within 100ms instead of waiting 5+ seconds!

---

### **Timing Diagram (5-second scenario):**

```
=== BLOCKING APPROACH ===
Time | Action
0s   | Read temp, delay 5s
5s   | Motion event MISSED! ❌
5s   | Read temp, delay 5s
10s  | Motion event MISSED! ❌

=== NON-BLOCKING APPROACH ===
Time | Action
0s   | Check temp (5s timer) → read
0-5s | Check motion every 100ms → DETECTED IMMEDIATELY ✓
1s   | Motion detected → LED ON within 100ms
2s   | Motion ends → LED OFF within 100ms
5s   | Check temp (timer reset) → read
```

---

### **Summary: Key Takeaways for Q20**

| Concept | Key Point |
|---------|----------|
| **setup()** | Runs once; initialize hardware |
| **loop()** | Runs repeatedly; main logic |
| **delay() Problem** | Blocks entire processor; misses events; unresponsive |
| **millis() Solution** | Non-blocking; responsive; simultaneous multi-tasking |
| **IoT Applications** | Always use millis() (Q22 bonus mark requirement!) |
| **Pattern** | Check `(now - last_time) >= interval`, then act |

---

### **Arduino UNO Timing Constraints**

- **millis() precision**: ±1-2ms (affected by interrupts)
- **Loop frequency**: 1000-10000+ iterations/second (depends on code complexity)
- **Button debounce**: Recommend 20-50ms delay between reads
- **Sensor polling**: Typically 100-500ms intervals (varies by sensor)
- **Serial communication**: 9600 baud standard for this course

---

**For This Course (Q14, Q22, Q23, Q33, Q34):** Always use `millis()` for timing instead of `delay()` to maintain responsive IoT systems.

---

**Author**: [YOUR NAME]  
**Date**: [DATE]  
**Questions**: Q19, Q20
