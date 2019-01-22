#include <nRF5x_BLE_API.h>

// Device will be displayed as OxiiLabsBLE
#define DEVICE_NAME       "OxiiLabsBLE"
#define TXRX_BUF_LEN      20

// Create ble instance
BLE                       ble;

// Create a timer task
Ticker                    ticker1s;

#define ledPin            D13 
#define buttonPin         D3 
#define analogPin         D4

static volatile int buttonState = 0;

uint8_t pressureReading = 0;
uint8_t value[2] = {0x00, pressureReading};

// The uuid of service and characteristics
static const uint8_t service1_uuid[]         = {0x99, 0x99, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t service1_chars3_uuid[]  = {0x99, 0x99, 0, 4, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};

// Used in advertisement
static const uint8_t uart_base_uuid_rev[]    = {0x1E, 0x94, 0x8D, 0xF1, 0x48, 0x31, 0x94, 0xBA, 0x75, 0x4C, 0x3E, 0x50, 0, 0, 0x99, 0x99};

// Initialize value of chars
uint8_t chars3_value[TXRX_BUF_LEN] = {0};

// Create characteristic
GattCharacteristic  characteristic3(service1_chars3_uuid, chars3_value, 1, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic *uartChars[] = {&characteristic3};

//Create service
GattService         uartService(service1_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));

DeviceInformationService *deviceInfo;

void disconnectionCallBack(const Gap::DisconnectionCallbackParams_t *params) {
  Serial.println("Disconnected!");
  Serial.println("Restarting the advertising process");
  ble.startAdvertising();
}

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

void setAdvertisement(void) {

  ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
  // Add short name to advertisement
  ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,(const uint8_t *)"TXRX", 4);
  // Add complete 128bit_uuid to advertisement
  ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,(const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid_rev));
  // Add complete device name to scan response data
  ble.accumulateScanResponse(GapAdvertisingData::COMPLETE_LOCAL_NAME,(const uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME) - 1);
}

void ISR_button() {
  buttonState = digitalRead(buttonPin);
  digitalWrite(ledPin, buttonState);
  periodicCallback();
}

void periodicCallback() {
  uint8_t buf[3];
uint16_t report_value = analogRead(A3);
buf[0] = (0x00);
buf[1] = (report_value >> 8);
buf[2] = (report_value);
ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), buf, 3);

} 


// put your setup code here, to run once
void setup() {
  
  Serial.begin(9600);
  Serial.println("Start ");

  Serial.print("BLE stack verison is : ");
  Serial.println(ble.getVersion());

  //Set on board LED as output to show on-off status
  //Set D1 as test read pin
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin,INPUT);

  attachInterrupt(buttonPin,ISR_button,FALLING);

  
  // Init timer task
  ticker1s.attach(periodicCallback, 1);
  
  // Init ble
  ble.init();
  
//  ble.onConnection(connectionCallBack);

  // Callback when bluetooth disconnected
  ble.onDisconnection(disconnectionCallBack);
  
//  attachInterrupt(BUTTON_PIN,button_handle,FALLING);
//  ticker1s.attach(periodicCallback, 1);

  setAdvertisement();

  ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  // add service
//  hrService  = new HeartRateService(ble, hrmCounter, HeartRateService::LOCATION_FINGER);
  deviceInfo = new DeviceInformationService(ble, "ARM", "Model1", "SN1", "hw-rev1", "fw-rev1", "soft-rev1");
  ble.addService(uartService);
  // set device name
  ble.setDeviceName((const uint8_t *)DEVICE_NAME);
  // set tx power,valid values are -40, -20, -16, -12, -8, -4, 0, 4
  ble.setTxPower(4);
  // set adv_interval, 100ms in multiples of 0.625ms.
  ble.setAdvertisingInterval(160);
  // set adv_timeout, in seconds
  ble.setAdvertisingTimeout(0);
  // ger BLE stack version
  Serial.print("BLE stack verison is : ");
  Serial.println(ble.getVersion());
  // start advertising
  ble.startAdvertising();
  Serial.println("start advertising ");
}


// put your main code here, to run repeatedly:
void loop() {
  ble.waitForEvent();
}
