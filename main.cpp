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

int arrivedcount = 0;
const char* topic = "verner/feeds/message";
const char* toptemp = "verner/feeds/temperature";
I2C i2c(I2C1_SDA, I2C1_SCL);
uint8_t lm75_adress = 0x48 << 1;

/* Printf the message received and its configuration */
void messageArrived(MQTT::MessageData& md)
{
	MQTT::Message &message = md.message;
	printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
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
	data.MQTTVersion = 3;
	data.clientID.cstring = "test";
	data.username.cstring = "verner";
	data.password.cstring = "c777cfc4b0f34f93b1045ac4512cbac4";
	if ((rc = client.connect(data)) != 0)
		printf("rc from MQTT connect is %d\r\n", rc);

	// Subscribe to the same topic we will publish in
	if ((rc = client.subscribe(topic, MQTT::QOS2, messageArrived)) != 0)
		printf("rc from MQTT subscribe is %d\r\n", rc);



	// Send a test message:
	MQTT::Message message;
	// QoS 0
	char buf[100];
	sprintf(buf, "OSKOUUUUUR\r\n");

	message.qos = MQTT::QOS0;
	message.retained = false;
	message.dup = false;
	message.payload = (void*)buf;
	message.payloadlen = strlen(buf)+1;

	rc = client.publish(topic, message);

// Send temperature:
	while (true){
			MQTT::Message messagetemp;
			//rc = client.publish(topic, message);
			char cmd[2];
			cmd[0] = 0x00;

			i2c.write(lm75_adress, cmd, 1);
			i2c.read(lm75_adress, cmd, 2);

			float temperature = ((cmd[0] << 8 | cmd[1] ) >> 7) * 0.5;
			printf("Temperature : %f\n", temperature);

			//ThisThread::sleep_for(10000);

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

			wait_ms(5000);
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
