# Attendance-System

This Arduino sketch provides functions to enroll, verify, and delete fingerprints using a fingerprint sensor module connected to an Arduino Uno. It is intended to work together with a Python program that sends commands over serial communication.

Setup
Connect the fingerprint sensor to the Arduino using the TX, RX, Voltage and GND pins.
Upload the Arduino sketch to the board.
The baudrate should be sent to 9600.

Status messages
The Arduino will perform the operation and respond over serial with a message indicating

- Success - ending with 'Success' indicates operation ended as it should.
- Error - ending with 'Error' indicates operation didn't end as it should.
- Information

Usage
Upon connection, the following messages can be sent:
FingerprintSensorSuccess - indicates there is a valid fingerprint sensor.
FingerprintSensorError - indicates there is no valid fingerprint sensor. No command can be carried out if this message is sent. Check connections(i.e. TX, RX, Voltage and GND pins) for fingerprint sensor and reconnect. If it persists, consult hardware engineer.

The message is sent about two seconds after connection.

The Python program sends the following commands over serial:

Enroll - Enroll a new fingerprint
Verify - Verify a fingerprint
BurstEnroll/Verify - Perform operation multiple times until told to stop
Delete - Delete a single fingerprint
DeleteAll - Delete all fingerprints
Stop - Stops a command's execution mid-operation

Enroll
The enroll process requires the fingerprint to be scanned twice. Follow the prompts in the serial monitor.
This can be canceled mid-enrollment by sending 'Stop' over serial.

The following messages can be sent:
FingerprintStorageFull - indicates the no more fingerprints can be stored. Check fingerprint sensor product name to know storage capacity. Check ID selection for more information. You can run the 'Delete' or 'DeleteAll' command to free up space.
OperationStopped - indicates that a 'Stop' command was sent.
FingerprintFirstCapture - indicates an enrollee's finger is being captured the first time.
FingerprintSecondCapture - indicates an enrollee's finger is being captured the second time.
FingerprintConversionError - indicates fingerprint image could not be converted. If it persists, consult hardware engineer.
FingerprintEnrollMismatch - indicates fingerprints from first capture and second capture didn't match. Clean fingerprint sensor surface as well as finger of enrollee and try again. Ensure enrollee's finger is the same and is placed properly. If it persists, consult hardware engineer.
FingerprintEnrollError - indicates an error occurred during fingerprint model creation or storage. If it persists, consult hardware engineer.

Verify
Simply place an already enrolled finger on the sensor when prompted.
This can be canceled mid-enrollment by sending 'Stop' over serial.

The following messages can be sent:
OperationStopped - indicates that a 'Stop' command was sent.
FingerprintConversionError - indicates fingerprint image could not be converted. If it persists, consult hardware engineer.
FingerprintNotFound - indicates that fingerprint is not registered on fingerprint sensor. If it persists, try enrolling again. If it still persists, consult hardware engineer.
FingerprintVerifySuccess - indicates fingerprint was found.

Delete
No fingerprint scan is needed. Deletes either a single fingerprint or all fingerprints depending on the command.

The following messages can be sent:
FingerprintStorageEmpty - indicates no fingerprint are in storage. Register fingerprints by running the 'Enroll' or 'BurstEnroll' command. If it still persists, consult hardware engineer.
FingerprintDelete(All)Success - indicates fingerprint(s) has been successfully deleted.

ID selection
The fingerprint storage works using a LIFO mechanism.
