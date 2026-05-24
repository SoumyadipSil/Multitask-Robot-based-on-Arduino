#include <AFMotor.h>  // Include Adafruit Motor Shield library
#include <Servo.h>    // Servo library

#define Echo A0
#define Trig A1
#define SERVO_PIN 10
#define MOTOR_SPEED 200  // Define motor speed as a constant
#define TURN_DURATION 3000  // Turn duration for regular turns

// Motor Definitions
AF_DCMotor M1(1); 
AF_DCMotor M2(2); 
AF_DCMotor M3(3); 
AF_DCMotor M4(4); 

Servo servo;
bool automaticModeEnabled = false;

void setup() {
    Serial.begin(9600);  // Serial for Bluetooth communication
    pinMode(Echo, INPUT);
    pinMode(Trig, OUTPUT);
    servo.attach(SERVO_PIN);
    servo.write(90);  // Start with servo facing front

    // Initialize motors
    M1.setSpeed(MOTOR_SPEED);
    M2.setSpeed(MOTOR_SPEED);
    M3.setSpeed(MOTOR_SPEED);
    M4.setSpeed(MOTOR_SPEED);

    Serial.println("Bluetooth communication started…");
    Serial.println("Waiting for messages…");
}

// Ultrasonic distance measurement
long getDistance() {
    digitalWrite(Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trig, LOW);
    long distance = pulseIn(Echo, HIGH) / 58; // Convert time to cm
    return distance;
}

// Forward Motion with Obstacle Detection
void moveForward() {
    // Before moving, check if there's an obstacle
    long currentDistance = getDistance();

    if (currentDistance < 15) {  // Check if obstacle is too close
        Serial.println("Obstacle detected ahead! Cannot move forward.");
        return;  // Don't move forward
    }

    Serial.println("Moving Forward");
    M1.run(FORWARD);
    M2.run(FORWARD);
    M3.run(FORWARD);
    M4.run(FORWARD);

    // Continuous detection happens in loop() not here
}

// Backward Motion
void moveBackward() {
    Serial.println("Moving Backward");
    M1.run(BACKWARD);
    M2.run(BACKWARD);
    M3.run(BACKWARD);
    M4.run(BACKWARD);
}

// Turn Left
void turnLeft() {
    Serial.println("Turning Left");
    M1.run(FORWARD);
    M3.run(FORWARD);
    M2.run(BACKWARD);
    M4.run(BACKWARD);
}

// Turn Right
void turnRight() {
    Serial.println("Turning Right");
    M1.run(BACKWARD);
    M3.run(BACKWARD);
    M2.run(FORWARD);
    M4.run(FORWARD);
}

// Stop Movement
void stopMotors() {
    Serial.println("Stopping");
    M1.run(RELEASE);
    M2.run(RELEASE);
    M3.run(RELEASE);
    M4.run(RELEASE);
}

// Look in a specific direction and get distance
long lookAndMeasure(int angle) {
    servo.write(angle);
    delay(300);  // Reduced delay for smoother operation
    return getDistance();
}

// Performs a detailed scan in 20-degree increments and returns the best angle to turn
int preciseScan() {
    long distances[10]; // Array to store distances at different angles
    int angles[10];     // Array to store corresponding angles
    int readings = 0;   // Counter for valid readings

    Serial.println("Performing precise scan…");

    // Scan from 0 to 180 degrees in 20-degree increments
    for (int angle = 0; angle <= 180; angle += 20) {
        long distance = lookAndMeasure(angle);
        delay(200); // Allow time for stable reading

        Serial.print("Angle: "); Serial.print(angle);
        Serial.print(", Distance: "); Serial.print(distance);
        Serial.println(" cm");

        // Store this reading
        distances[readings] = distance;
        angles[readings] = angle;
        readings++;
    }

    // Return servo to center
    servo.write(90);
    delay(300);

    // Find the best direction
    long maxDistance = 0;
    int bestAngle = 90; // Default to forward if no good option

    // First, check right side (90-180 degrees) as it has priority
    for (int i = 0; i < readings; i++) {
        if (angles[i] > 90 && distances[i] > maxDistance && distances[i] > 15) {
            maxDistance = distances[i];
            bestAngle = angles[i];
        }
    }

    // If no good path found on right, check left side (0-90 degrees)
    if (maxDistance == 0) {
        for (int i = 0; i < readings; i++) {
            if (angles[i] < 90 && distances[i] > maxDistance && distances[i] > 15) {
                maxDistance = distances[i];
                bestAngle = angles[i];
            }
        }
    }

    // Report the best angle found
    Serial.print("Best angle: "); Serial.print(bestAngle);
    Serial.print(", with distance: "); Serial.print(maxDistance);
    Serial.println(" cm");

    return bestAngle;
}

// Automatic Obstacle Avoidance Mode with Precise Scanning
// Modified Automatic Obstacle Avoidance Mode
void automaticMode() {
    static bool isCurrentlyMoving = false;

    // First, check if there's an obstacle in front
    long frontDistance = getDistance();

    // If obstacle detected in front, perform precise scanning
    if (frontDistance <= 15) {
        // Stop immediately
        stopMotors();
        isCurrentlyMoving = false;
        Serial.println("Obstacle detected ahead! Stopping");
        delay(200);

        // Perform precise scanning
        int bestAngle = preciseScan();

        // If front is still blocked, back up and scan again
        frontDistance = lookAndMeasure(90);  // Look forward
        if (frontDistance <= 15) {
            Serial.println("Front still blocked. Moving backward");
            moveBackward();
            delay(800); // Move back for 800ms (1 step)
            stopMotors();
            delay(200);

            // Perform another precise scan after backing up
            bestAngle = preciseScan();
        }

        // Turn to the best direction with increased turn duration
        if (bestAngle < 90) {
            // Turn left
            Serial.print("Turning left based on best angle: ");
            Serial.println(bestAngle);
            turnLeft();

            // Calculate turn duration based on how far from center
            int turnTime = map(bestAngle, 0, 90, TURN_DURATION * 1.5, TURN_DURATION * 0.75);

            Serial.print("Turn duration: ");
            Serial.print(turnTime);
            Serial.println(" ms");

            delay(turnTime);
            stopMotors();
            delay(200);

            // After turning, move forward but don't stop
            Serial.println("Turn complete, moving forward");
            moveForward();
            isCurrentlyMoving = true;

        } else if (bestAngle > 90) {
            // Turn right
            Serial.print("Turning right based on best angle: ");
            Serial.println(bestAngle);
            turnRight();

            // Calculate turn duration based on how far from center
            int turnTime = map(bestAngle, 90, 180, TURN_DURATION * 0.75, TURN_DURATION * 1.5);

            Serial.print("Turn duration: ");
            Serial.print(turnTime);
            Serial.println(" ms");

            delay(turnTime);
            stopMotors();
            delay(200);

            // After turning, move forward but don't stop
            Serial.println("Turn complete, moving forward");
            moveForward();
            isCurrentlyMoving = true;

        } else {
            // No good direction (shouldn't happen often)
            Serial.println("No clear path. Backing up more and making a full turn");
            moveBackward();
            delay(1200);
            stopMotors();
            delay(200);

            // Make a full turn to try to find new path
            turnRight();
            delay(TURN_DURATION * 1.5); // Full 180-degree turn
            stopMotors();
            delay(200);

            // After turning, move forward but don't stop
            Serial.println("Turn complete, moving forward");
            moveForward();
            isCurrentlyMoving = true;
        }
    } else {
        // No obstacle ahead, continue moving forward
        // Check if we're not already moving forward
        if (!isCurrentlyMoving) {
            Serial.println("Path clear. Moving forward");
            moveForward();
            isCurrentlyMoving = true;
        }
        // No need to stop since we want continuous movement
    }

    // A short delay is still good for system stability
    delay(100);
}

void loop() {
    // Continuous distance checking for manual forward movement
    static bool isMovingForward = false;
    static long lastDistance = 100;  // Initialize with a safe value

    // Check if motors are running in forward mode
    if (isMovingForward) {
        long currentDistance = getDistance();

        // If obstacle detected while moving forward, stop
        if (currentDistance < 15) {
            Serial.println("Obstacle detected while moving forward! Stopping");
            stopMotors();
            isMovingForward = false;
        }
    }

    // Check for automatic mode
    if (automaticModeEnabled) {
        automaticMode();

        // Check if there's a command to exit automatic mode
        if (Serial.available()) {
            String stopCommand = Serial.readString();
            stopCommand.trim();

            if (stopCommand.equals("stop")) {
                Serial.println("Exiting Automatic Mode");
                automaticModeEnabled = false;
                stopMotors();
            }
        }
    } 
    // Normal command processing
    else if (Serial.available()) {
        String receivedMessage = Serial.readString();
        receivedMessage.trim();  // Clean up spaces/newlines

        Serial.print("Received command: ");
        Serial.println(receivedMessage);

        // Movement Controls
        if (receivedMessage.equals("move forward")) {
            moveForward();
            isMovingForward = true;  // Mark that we're moving forward
        } else if (receivedMessage.equals("move back")) {
            moveBackward();
            isMovingForward = false;  // Not moving forward
        } else if (receivedMessage.equals("turn left")) {
            turnLeft();
            // For manual turning, run for a duration then stop
            delay(TURN_DURATION);
            stopMotors();
            isMovingForward = false;  // Not moving forward
        } else if (receivedMessage.equals("turn right")) {
            turnRight();
            // For manual turning, run for a duration then stop
            delay(TURN_DURATION);
            stopMotors();
            isMovingForward = false;  // Not moving forward
        } else if (receivedMessage.equals("stop")) {
            stopMotors();
            isMovingForward = false;  // Not moving forward
        } else if (receivedMessage.equals("automatic mode")) {
            Serial.println("Automatic Mode Enabled");
            automaticModeEnabled = true;
            isMovingForward = false;  // Reset forward tracking when entering auto mode
        } else {
            Serial.println("Replied: I don't understand.");
        }
    }

    delay(100); // Small delay to avoid buffer overflow
}