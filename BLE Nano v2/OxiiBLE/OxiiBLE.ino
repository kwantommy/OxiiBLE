/*Oxii Labs
 * McMaster University - EE4BI6 Capstone 
 * Code to implement communication between Android Application and BLE Nano V2
 * Authors: Christine Horner, Mark Suan, Aditya Thakkar, Tommy Kwan
 * Supervised by: Dr. Aleksander Jeremic & Dr. Hubert de Bruin
 * Last Edited - Tuesday March 26
 */ 
 
#include <nRF5x_BLE_API.h>

#define DEVICE_NAME       "OxiiBLE"
#define TXRX_BUF_LEN      20
// Create ble instance
BLE                       ble;
// Create a timer task
Ticker                    ticker1s;
#define ledPin            D13
#define buttonPin         D3
#define redPin            D0
#define greenPin          D1
#define bluePin           D2

volatile byte state = LOW;
int countData = 0;
int dataPoints = 250;
// The uuid of service and characteristics
static const uint8_t service1_uuid[]         = {0x99, 0x99, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t service1_chars3_uuid[]  = {0x99, 0x99, 0, 4, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
// Used in advertisement
static const uint8_t uart_base_uuid_rev[]    = {0x1E, 0x94, 0x8D, 0xF1, 0x48, 0x31, 0x94, 0xBA, 0x75, 0x4C, 0x3E, 0x50, 0, 0, 0x3D, 0x71};

// Initialize value of chars
uint8_t chars1_value[TXRX_BUF_LEN] = {0};
uint8_t chars2_value[TXRX_BUF_LEN] = {1,2,3};
uint8_t chars3_value[TXRX_BUF_LEN] = {0};

// Create characteristic

GattCharacteristic  characteristic3(service1_chars3_uuid, chars3_value, 1, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic *uartChars[] = {&characteristic3};
//Create service
GattService         uartService(service1_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));

HeartRateService         *hrService;
DeviceInformationService *deviceInfo;
// Init HRM to 100bps
static uint16_t hrmCounter = 100;
/** @brief  Disconnect callback handle
 *
 *  @param[in] *params   params->handle : connect handle
 *                       params->reason : CONNECTION_TIMEOUT                          = 0x08,
 *                                        REMOTE_USER_TERMINATED_CONNECTION           = 0x13,
 *                                        REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES = 0x14,  // Remote device terminated connection due to low resources.
 *                                        REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF     = 0x15,  // Remote device terminated connection due to power off.
 *                                        LOCAL_HOST_TERMINATED_CONNECTION            = 0x16,
 *                                        CONN_INTERVAL_UNACCEPTABLE                  = 0x3B,
 */

 void setColour(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}



void disconnectionCallBack(const Gap::DisconnectionCallbackParams_t *params) {
  Serial.print("Disconnected hande : ");
  Serial.println(params->handle, HEX);
  Serial.print("Disconnected reason : ");
  Serial.println(params->reason, HEX);
  Serial.println("Restart advertising ");
  ble.startAdvertising();
}

/** @brief  Connection callback handle
 *
 *  @param[in] *params   params->handle : The ID for this connection
 *                       params->role : PERIPHERAL  = 0x1, // Peripheral Role
 *                                      CENTRAL     = 0x2, // Central Role.
 *                       params->peerAddrType : The peer's BLE address type
 *                       params->peerAddr : The peer's BLE address
 *                       params->ownAddrType : This device's BLE address type
 *                       params->ownAddr : This devices's BLE address
 *                       params->connectionParams->minConnectionInterval
 *                       params->connectionParams->maxConnectionInterval
 *                       params->connectionParams->slaveLatency
 *                       params->connectionParams->connectionSupervisionTimeout
 */
void connectionCallBack( const Gap::ConnectionCallbackParams_t *params ) {
  uint8_t index;
  if(params->role == Gap::PERIPHERAL) {
    Serial.println("Peripheral ");
  }

 

  Serial.print("The conn handle : ");
  Serial.println(params->handle, HEX);
  Serial.print("The conn role : ");
  Serial.println(params->role, HEX);

  Serial.print("The peerAddr type : ");
  Serial.println(params->peerAddrType, HEX);
  Serial.print("  The peerAddr : ");
  for(index=0; index<6; index++) {
    Serial.print(params->peerAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  Serial.print("The ownAddr type : ");
  Serial.println(params->ownAddrType, HEX);
  Serial.print("  The ownAddr : ");
  for(index=0; index<6; index++) {
    Serial.print(params->ownAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  Serial.print("The min connection interval : ");
  Serial.println(params->connectionParams->minConnectionInterval, HEX);
  Serial.print("The max connection interval : ");
  Serial.println(params->connectionParams->maxConnectionInterval, HEX);
  Serial.print("The slaveLatency : ");
  Serial.println(params->connectionParams->slaveLatency, HEX);
  Serial.print("The connectionSupervisionTimeout : ");
  Serial.println(params->connectionParams->connectionSupervisionTimeout, HEX);
}

/** @brief  write callback handle of Gatt server
 *
 *  @param[in] *Handler   Handler->connHandle : The handle of the connection that triggered the event
 *                        Handler->handle : Attribute Handle to which the write operation applies
 *                        Handler->writeOp : OP_INVALID               = 0x00,  // Invalid operation.
 *                                           OP_WRITE_REQ             = 0x01,  // Write request.
 *                                           OP_WRITE_CMD             = 0x02,  // Write command.
 *                                           OP_SIGN_WRITE_CMD        = 0x03,  // Signed write command.
 *                                           OP_PREP_WRITE_REQ        = 0x04,  // Prepare write request.
 *                                           OP_EXEC_WRITE_REQ_CANCEL = 0x05,  // Execute write request: cancel all prepared writes.
 *                                           OP_EXEC_WRITE_REQ_NOW    = 0x06,  // Execute write request: immediately execute all prepared writes.
 *                        Handler->offset : Offset for the write operation
 *                        Handler->len : Length (in bytes) of the data to write
 *                        Handler->data : Pointer to the data to write
 */
void gattServerWriteCallBack(const GattWriteCallbackParams *Handler) {
  uint8_t index;

  Serial.print("Handler->connHandle : ");
  Serial.println(Handler->connHandle, HEX);
  Serial.print("Handler->handle : ");
  Serial.println(Handler->handle, HEX);
  Serial.print("Handler->writeOp : ");
  Serial.println(Handler->writeOp, HEX);
  Serial.print("Handler->offset : ");
  Serial.println(Handler->offset, HEX);
  Serial.print("Handler->len : ");
  Serial.println(Handler->len, HEX);
  for(index=0; index<Handler->len; index++) {
    Serial.print(Handler->data[index], HEX);
  }
  Serial.println(" ");

//  uint8_t buf[TXRX_BUF_LEN];
//  uint16_t bytesRead = 20;
//  Serial.println("Write Handle : ");
//  // Check the attribute belong to which characteristic
//  if (Handler->handle == characteristic1.getValueAttribute().getHandle()) {
//    // Read the value of characteristic
//    ble.readCharacteristicValue(characteristic1.getValueAttribute().getHandle(), buf, &bytesRead);
//    for(index=0; index<bytesRead; index++) {
//      Serial.print(buf[index], HEX);
//    }
//    Serial.println(" ");
//  }
}

/**
 * @brief  Timer task callback handle
 */
void task_handle(void) {
  if (state == HIGH) {
    Serial.println(countData);
    countData++;
    
    if (countData > dataPoints) { // 251st data point is FFFFFF to signifying start of next sequence
    uint8_t buf[3];
    buf[0] = (0xFF);
    buf[1] = (0xFF);
    buf[2] = (0xFF); 
    ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), buf, 3);
        Serial.println("resetting state to low");
      state = LOW;
      countData = 0;
      
      return;
    }
    uint8_t buf[3];
    uint16_t report_value = analogRead(A5);
    buf[0] = (0x00);
    buf[1] = (report_value >> 8);
    buf[2] = (report_value);
    ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), buf, 3);

  }
  

//  if (ble.getGapState().connected) {
//      setColour(255, 0, 255); // blue Colour
//      delay(100);
//  }
//  
}

void blink(int time){
  
}


/**
 * @brief  Set advertisement
 */
void setAdvertisement(void) {
  // A list of Advertising Data types commonly used by peripherals.
  //   FLAGS                              = 0x01, // Flags, refer to GapAdvertisingData::Flags_t.
  //   INCOMPLETE_LIST_16BIT_SERVICE_IDS  = 0x02, // Incomplete list of 16-bit Service IDs.
  //   COMPLETE_LIST_16BIT_SERVICE_IDS    = 0x03, // Complete list of 16-bit Service IDs.
  //   INCOMPLETE_LIST_32BIT_SERVICE_IDS  = 0x04, // Incomplete list of 32-bit Service IDs (not relevant for Bluetooth 4.0).
  //   COMPLETE_LIST_32BIT_SERVICE_IDS    = 0x05, // Complete list of 32-bit Service IDs (not relevant for Bluetooth 4.0).
  //   INCOMPLETE_LIST_128BIT_SERVICE_IDS = 0x06, // Incomplete list of 128-bit Service IDs.
  //   COMPLETE_LIST_128BIT_SERVICE_IDS   = 0x07, // Complete list of 128-bit Service IDs.
  //   SHORTENED_LOCAL_NAME               = 0x08, // Shortened Local Name.
  //   COMPLETE_LOCAL_NAME                = 0x09, // Complete Local Name.
  //   TX_POWER_LEVEL                     = 0x0A, // TX Power Level (in dBm).
  //   DEVICE_ID                          = 0x10, // Device ID.
  //   SLAVE_CONNECTION_INTERVAL_RANGE    = 0x12, // Slave Connection Interval Range.
  //   LIST_128BIT_SOLICITATION_IDS       = 0x15, // List of 128 bit service UUIDs the device is looking for.
  //   SERVICE_DATA                       = 0x16, // Service Data.
  //   APPEARANCE                         = 0x19, // Appearance, refer to GapAdvertisingData::Appearance_t.
  //   ADVERTISING_INTERVAL               = 0x1A, // Advertising Interval.
  //   MANUFACTURER_SPECIFIC_DATA         = 0xFF  // Manufacturer Specific Data.

  // AD_Type_Flag : LE_LIMITED_DISCOVERABLE = 0x01, Peripheral device is discoverable for a limited period of time
  //                LE_GENERAL_DISCOVERABLE = 0x02, Peripheral device is discoverable at any moment
  //                BREDR_NOT_SUPPORTED     = 0x03, Peripheral device is LE only
  //                SIMULTANEOUS_LE_BREDR_C = 0x04, Not relevant - central mode only
  //                SIMULTANEOUS_LE_BREDR_H = 0x05, Not relevant - central mode only
  ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
  // Add short name to advertisement
  ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,(const uint8_t *)"TXRX", 4);
  // Add complete 128bit_uuid to advertisement
  ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,(const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid_rev));
  // Add complete device name to scan response data
  ble.accumulateScanResponse(GapAdvertisingData::COMPLETE_LOCAL_NAME,(const uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME) - 1);

  
}

void setup() {
  // put your setup code here, to run once
  Serial.begin(9600);
  Serial.println("Start ");
  pinMode(D13, OUTPUT);
  
  // Init timer task
    pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(A5,INPUT);
  
  //LED pin outs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  //attachInterrupt(buttonPin, sampler, FALLING);
  state = LOW;
  ticker1s.attach(task_handle, 0.02); //introduces a 100 Hz noise when using a battery
  
  // Init ble
  ble.init();
  ble.onConnection(connectionCallBack);
  ble.onDisconnection(disconnectionCallBack);
  ble.onDataWritten(gattServerWriteCallBack);
  
  // set advertisement
  setAdvertisement();
  
  // set adv_type(enum from 0)
  //    ADV_CONNECTABLE_UNDIRECTED
  //    ADV_CONNECTABLE_DIRECTED
  //    ADV_SCANNABLE_UNDIRECTED
  //    ADV_NON_CONNECTABLE_UNDIRECTED
  ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  // add service
  hrService  = new HeartRateService(ble, hrmCounter, HeartRateService::LOCATION_FINGER);
  deviceInfo = new DeviceInformationService(ble, "ARM", "Model1", "SN1", "hw-rev1", "fw-rev1", "soft-rev1");
  ble.addService(uartService);
  // set device name
  ble.setDeviceName((const uint8_t *)DEVICE_NAME);
  // set tx power,valid values are -40, -20, -16, -12, -8, -4, 0, 4
  ble.setTxPower(4);
  // set adv_interval, 100ms in multiples of 0.625ms.
  ble.setAdvertisingInterval(160); //introduces a 6.25 Hz noise when using a battery
  
  // set adv_timeout, in seconds
  ble.setAdvertisingTimeout(0);
  // ger BLE stack version
  Serial.print("BLE stack verison is : ");
  Serial.println(ble.getVersion());
  // start advertising
  ble.startAdvertising();
  Serial.println("start advertising ");
}

void loop() {
  // put your main code here, to run repeatedly:
  ble.waitForEvent();
    if (digitalRead(buttonPin) == LOW) {
    state = HIGH;
    
  }
    
  digitalWrite(ledPin, state);
  if (state==HIGH){
      setColour(0, 255, 0); // Green Colour
  }
  if (state == LOW){
    setColour(0, 0, 200); // blue Colour
  }
  
}
