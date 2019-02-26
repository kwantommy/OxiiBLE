/*Oxii Labs
   McMaster University - EE4BI6 Capstone
   Code to implement communication between Android Application and BLE Nano V2
   Authors: Christine Horner, Mark Suan, Aditya Thakkar, Tommy Kwan
   Supervised by: Dr. Aleksander Jeremic & Dr. Hubert de Bruin
   Last edit - Tuesday Feb 26
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
#define readPin           A5

volatile byte state = LOW;
int countData = 0;
int dataPoints = 50;

static const uint8_t service1_uuid[]         = {0x99, 0x99, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t service1_chars3_uuid[]  = {0x99, 0x99, 0, 4, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};

static const uint8_t uart_base_uuid_rev[]    = {0x1E, 0x94, 0x8D, 0xF1, 0x48, 0x31, 0x94, 0xBA, 0x75, 0x4C, 0x3E, 0x50, 0, 0, 0x3D, 0x71};

uint8_t chars1_value[TXRX_BUF_LEN] = {0};
uint8_t chars2_value[TXRX_BUF_LEN] = {1, 2, 3};
uint8_t chars3_value[TXRX_BUF_LEN] = {0};

GattCharacteristic  characteristic3(service1_chars3_uuid, chars3_value, 1, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic *uartChars[] = {&characteristic3};

GattService         uartService(service1_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));

DeviceInformationService *deviceInfo;

void disconnectionCallBack(const Gap::DisconnectionCallbackParams_t *params) {
  Serial.print("Disconnected hande : ");
  Serial.println(params->handle, HEX);
  Serial.print("Disconnected reason : ");
  Serial.println(params->reason, HEX);
  Serial.println("Restart advertising ");
  ble.startAdvertising();
}

void connectionCallBack( const Gap::ConnectionCallbackParams_t *params ) {
  uint8_t index;
  if (params->role == Gap::PERIPHERAL) {
    Serial.println("Peripheral ");
  }

  Serial.print("The conn handle : ");
  Serial.println(params->handle, HEX);
  Serial.print("The conn role : ");
  Serial.println(params->role, HEX);

  Serial.print("The peerAddr type : ");
  Serial.println(params->peerAddrType, HEX);
  Serial.print("  The peerAddr : ");
  for (index = 0; index < 6; index++) {
    Serial.print(params->peerAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  Serial.print("The ownAddr type : ");
  Serial.println(params->ownAddrType, HEX);
  Serial.print("  The ownAddr : ");
  for (index = 0; index < 6; index++) {
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
  for (index = 0; index < Handler->len; index++) {
    Serial.print(Handler->data[index], HEX);
  }
  Serial.println(" ");
}

void task_handle(void) {
  if (state == HIGH) {
    Serial.println(countData);
    countData++;
    if (countData > dataPoints) {
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
}

void setAdvertisement(void) {
  ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
  ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME, (const uint8_t *)"TXRX", 4);
  ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS, (const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid_rev));
  ble.accumulateScanResponse(GapAdvertisingData::COMPLETE_LOCAL_NAME, (const uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME) - 1);
}

void setup() {
  // put your setup code here, to run once
  Serial.begin(9600);
  Serial.println("Start ");
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(readPin, INPUT);
  state = LOW;
  ticker1s.attach(task_handle, 0.1);
  ble.init();
  ble.onConnection(connectionCallBack);
  ble.onDisconnection(disconnectionCallBack);
  ble.onDataWritten(gattServerWriteCallBack);
  setAdvertisement();
  ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  deviceInfo = new DeviceInformationService(ble, "ARM", "Model1", "SN1", "hw-rev1", "fw-rev1", "soft-rev1");
  ble.addService(uartService);
  ble.setDeviceName((const uint8_t *)DEVICE_NAME);
  ble.setTxPower(4);
  ble.setAdvertisingInterval(160);
  ble.setAdvertisingTimeout(0);
  Serial.print("BLE stack verison is : ");
  Serial.println(ble.getVersion());
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
}
