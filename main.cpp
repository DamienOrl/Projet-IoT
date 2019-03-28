#include "mbed.h"
#include "zest-radio-atzbrf233.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

namespace {
#define PERIOD_MS 500
}

// Network interface
NetworkInterface *net;

static DigitalOut led1(LED1);
static AnalogIn humidite(ADC_IN1);
int arrivedcount = 0;
const char* topic = "verner/feeds/message";
const char* toptemp = "verner/feeds/temperature";
const char* topled = "verner/feeds/led";
const char* tophum =  "verner/feeds/humidite";
I2C i2c(I2C1_SDA, I2C1_SCL);
uint8_t lm75_adress = 0x48 << 1;

/* Printf the message received and its configuration */
void messageArrived(MQTT::MessageData& md)
{
	MQTT::Message &message = md.message;
	if (message.payloadlen == 2) {
		led1 = 1;
	}
	if (strcmp((const char *)message.payload, "ON") == 0) {
		led1 = 1;
	}
	else if (strcmp((const char *)message.payload, "OFF") == 0) {
		led1 = 0;
	}

	printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
	++arrivedcount;
}

// MQTT demo
int main() {
	int result;

	// Add the border router DNS to the DNS table
	nsapi_addr_t new_dns = {
			NSAPI_IPv6,
			{ 0xfd, 0x9f, 0x59, 0x0a, 0xb1, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x01 }
	};
	nsapi_dns_add_server(new_dns);

	printf("Starting MQTT demo\n");

	// Get default Network interface (6LowPAN)
	net = NetworkInterface::get_default_instance();
	if (!net) {
		printf("Error! No network inteface found.\n");
		return 0;
	}

	// Connect 6LowPAN interface
	result = net->connect();
	if (result != 0) {
		printf("Error! net->connect() returned: %d\n", result);
		return result;
	}

	// Build the socket that will be used for MQTT
	MQTTNetwork mqttNetwork(net);

	// Declare a MQTT Client
	MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

	// Connect the socket to the MQTT Broker
	const char* hostname = "io.adafruit.com";
	uint16_t port = 1883;
	printf("Connecting to %s:%d\r\n", hostname, port);
	int rc = mqttNetwork.connect(hostname, port);
	if (rc != 0)
		printf("rc from TCP connect is %d\r\n", rc);

	// Connect the MQTT Client
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.MQTTVersion = 4;
	data.clientID.cstring = "test";
	data.username.cstring = "verner";
	data.password.cstring = "c777cfc4b0f34f93b1045ac4512cbac4";
	if ((rc = client.connect(data)) != 0)
		printf("rc from MQTT connect is %d\r\n", rc);

	// Subscribe to the same topic we will publish in
	if ((rc = client.subscribe(topled, MQTT::QOS2, messageArrived)) != 0)
		printf("rc from MQTT subscribe is %d\r\n", rc);



	// Send a test message:
	MQTT::Message message;
	// QoS 0
	char buf[100];
	sprintf(buf, "It just works! Probably...\r\n");

	message.qos = MQTT::QOS0;
	message.retained = false;
	message.dup = false;
	message.payload = (void*)buf;
	message.payloadlen = strlen(buf)+1;

	rc = client.publish(topic, message);



	// Send temperature + humidité:
	while (true){
		MQTT::Message messagetemp;
		char cmd[2];
		cmd[0] = 0x00;

		i2c.write(lm75_adress, cmd, 1);
		i2c.read(lm75_adress, cmd, 2);

		float temperature = ((cmd[0] << 8 | cmd[1] ) >> 7) * 0.5;
		printf("Temperature : %f\n", temperature);

		char buf2[100];
		sprintf(buf2, "%f", temperature);

		messagetemp.qos = MQTT::QOS0;
		messagetemp.retained = false;
		messagetemp.dup = false;
		messagetemp.payload = (void*)buf2;
		messagetemp.payloadlen = strlen(buf2)+1;

		rc = client.publish(toptemp, messagetemp);
		client.yield();
		printf("RC %d\n", rc);


		MQTT::Message messagehum;
		float mesure = (humidite.read()*3.3) * 100 / 2.8;
		char buf3[100];
		printf("Humidité : %f\n", mesure);
		sprintf(buf3,"%.2f",mesure);
		messagehum.qos = MQTT::QOS0;
		messagehum.retained = false;
		messagehum.dup = false;
		messagehum.payload = (void*)buf3;
		messagehum.payloadlen = strlen(buf3);

		rc = client.publish(tophum, messagehum);

		wait_ms(3000);
	}


	// yield function is used to refresh the connexion
	// Here we yield until we receive the message we sent
	while (arrivedcount < 1)
		client.yield(100);

	// Disconnect client and socket
	client.disconnect();
	mqttNetwork.disconnect();

	// Bring down the 6LowPAN interface
	net->disconnect();
	printf("Done\n");


}
