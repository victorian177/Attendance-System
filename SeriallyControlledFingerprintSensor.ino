// TODO: Implement timeout for enroll and verify operations

#include <Adafruit_Fingerprint.h>
#include <StopWatch.h>

#define FINGERPRINT_ADDRESS_SIZE 127
#define HAS_FINGERPRINT 0
#define HAS_NO_FINGERPRINT 12
#define TIMEOUT 2000

const int TX = 2;
const int RX = 3;

SoftwareSerial sensorSerial(TX, RX);
Adafruit_Fingerprint fingerprintSensor = Adafruit_Fingerprint(&sensorSerial);
StopWatch stopWatch;

bool isSensor;
uint8_t id;
String command;
String result;

bool isOperationEnd = false;

void setup()
{
  // Serial setup
  Serial.begin(9600);
  while (!Serial)
    ;
  delay(2000);
  fingerprintSensor.begin(57600);

  // Check if fingerprint is connected
  isSensor = fingerprintSensor.verifyPassword();
  if (isSensor)
    Serial.println("FingerprintSensorSuccess");
  else
    Serial.println("FingerprintSensorError");
}

void loop()
{
  if (isSensor)
  {
    if (Serial.available() > 0)
    {
      command = Serial.readStringUntil("\n");
      stopWatch.reset();

      // Enroll
      if (command == "Enroll")
      {
        result = enrollFingerprint();
        Serial.println(result);
      }

      else if (command == "BurstEnroll")
      {
        Serial.println("BurstEnroll");
        while (true)
        {
          result = enrollFingerprint();
          Serial.println(result);

          if (isOperationEnd)
            break;
        }
      }

      // Verify
      else if (command == "Verify")
      {
        result = verifyFingerprint();
        Serial.println(result);
      }

      else if (command == "BurstVerify")
      {
        Serial.println("BurstVerify");
        while (true)
        {
          result = verifyFingerprint();
          Serial.println(result);

          if (isOperationEnd)
            break;
        }
      }

      // Delete
      else if (command == "Delete")
      {
        result = deleteFingerprint(false);
        Serial.println(result);
      }

      else if (command == "DeleteAll")
      {
        result = deleteFingerprint(true);
        Serial.println(result);
      }
    }
  }
}

// Helper functions
bool shouldStop()
{
  if (Serial.available() > 0)
  {
    String stoppingCommand = Serial.readStringUntil("\n");
    if (stoppingCommand == "Stop")
      return true;
  }
  return false;
}
bool shouldTimeout()
{
  if (!stopWatch.isRunning())
    stopWatch.start();

  if (stopWatch.elapsed() >= TIMEOUT)
    return true;

  return false;
}

int readId(bool isEnroll)
{
  int id = 0;

  if (isEnroll)
  {
    for (int addr = 1; addr <= FINGERPRINT_ADDRESS_SIZE; addr++)
    {
      if (fingerprintSensor.loadModel(addr) == HAS_NO_FINGERPRINT)
      {
        id = addr;
        break;
      }
    }
  }
  else
  {
    for (int addr = FINGERPRINT_ADDRESS_SIZE; addr >= 1; addr--)
    {
      if (fingerprintSensor.loadModel(addr) == HAS_FINGERPRINT)
      {
        id = addr;
        break;
      }
    }
  }

  return id;
}

// Command operations
String enrollFingerprint()
{
  id = 0;
  id = readId(true);
  if (!id)
    return "FingerprintStorageFull";

  int p = -1; // Status checker

  // Await first fingerprint image
  Serial.println("FingerprintFirstCapture");

  // Check whether to stop
  isOperationEnd = shouldStop();
  if (isOperationEnd)
    return "OperationStopped";

  isOperationEnd = shouldTimeout();
  if (isOperationEnd)
    return "OperationTimeout";

  // First fingerprint image capture
  while (p != FINGERPRINT_OK)
  {
    p = fingerprintSensor.getImage();

    // Check whether to stop
    isOperationEnd = shouldStop();
    if (isOperationEnd)
      return "OperationStopped";

    isOperationEnd = shouldTimeout();
    if (isOperationEnd)
      return "OperationTimeout";
  }

  // First fingerprint image conversion
  p = fingerprintSensor.image2Tz(1);
  if (p != FINGERPRINT_OK)
    return "FingerprintConversionError";

  // Waiting for removal of finger
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
    p = fingerprintSensor.getImage();
  p = -1;

  // Await second fingerprint image
  Serial.println("FingerprintSecondCapture");

  // Second fingerprint image capture
  while (p != FINGERPRINT_OK)
  {
    p = fingerprintSensor.getImage();

    // Check whether to stop
    isOperationEnd = shouldStop();
    if (isOperationEnd)
      return "OperationStopped";

    isOperationEnd = shouldTimeout();
    if (isOperationEnd)
      return "OperationTimeout";
  }

  // Second fingerprint image conversion
  p = fingerprintSensor.image2Tz(2);
  if (p != FINGERPRINT_OK)
    return "FingerprintConversionError";

  // Fingerprint model creation
  p = fingerprintSensor.createModel();
  if (p != FINGERPRINT_OK)
  {
    if (p == FINGERPRINT_ENROLLMISMATCH)
      return "FingerprintEnrollMismatch";

    else
      return "FingerprintEnrollError";
  }

  // Fingerprint model storage
  p = fingerprintSensor.storeModel(id);
  if (p != FINGERPRINT_OK)
    return "FingerprintEnrollError";

  // Fingerprint success
  return "FingerprintEnrollSuccess";
}

String verifyFingerprint()
{
  int p = -1; // Status checker

  // Fingerprint image capture
  while (p != FINGERPRINT_OK)
  {
    p = fingerprintSensor.getImage();

    // Check whether to stop
    isOperationEnd = shouldStop();
    if (isOperationEnd)
      return "OperationStopped";

    isOperationEnd = shouldTimeout();
    if (isOperationEnd)
      return "OperationTimeout";
  }

  // Fingerprint image conversion
  p = fingerprintSensor.image2Tz();
  if (p != FINGERPRINT_OK)
    return "FingerprintConversionError";

  // Fingerprint search
  p = fingerprintSensor.fingerSearch();
  if (p != FINGERPRINT_OK)
    return "FingerprintNotFound";

  return "FingerprintVerifySuccess";
}

String deleteFingerprint(bool shouldDeleteAll)
{
  id = 0;
  id = readId(false);
  if (!id)
    return "FingerprintStorageEmpty";

  if (shouldDeleteAll)
  {
    fingerprintSensor.emptyDatabase();
    return "FingerprintDeleteAllSuccess";
  }

  else
  {
    fingerprintSensor.deleteModel(id);
    return "FingerprintDeleteSuccess";
  }
}
