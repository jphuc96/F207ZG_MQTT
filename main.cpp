#include "mbed.h"
/*Network libraries*/
#include "EthernetInterface.h"
#include "MQTTNetwork.h"
#include "MQTTClient.h"
#include "MQTTmbed.h"

//LED interface
DigitalOut led_blue(PB_7);
DigitalOut led_red(PB_14);

// Network interface
EthernetInterface net;
MQTTNetwork mqtt_network(&net);
MQTT::Client<MQTTNetwork,Countdown,100,10> mqtt(mqtt_network);
int rc;

/*MQTT Client info*/
const char* mqtt_broker = "m14.cloudmqtt.com";
int mqtt_port = 16409;

/*Serial port*/
Serial pc(PD_5,PD_6,115200);

/*Threads declaration*/
Thread thread_1000ms(osPriorityNormal,OS_STACK_SIZE,NULL,"1000ms");

/*Mutex declaration*/
Mutex MQTTMutex;

/*Functions declaration*/
void task_1000ms();

void Net_Init();
void MQTT_Init();

void mqtt_publish_num(uint16_t mb_data, const char* mqtt_topic);
void mqtt_publish_str(const char* topic,const char* log_msg);



void MessageHandle(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    char topic[md.topicName.lenstring.len + 1];
    char payload[message.payloadlen + 1];
    sprintf(topic, "%.*s", md.topicName.lenstring.len, md.topicName.lenstring.data);
    sprintf(payload,"%.*s",message.payloadlen,(char*)message.payload);
    //pc.printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    pc.printf("%s: %s\r\r\n",topic,payload);

    uint8_t result;
    uint16_t data = atoi(payload);

    if(strcmp(topic,"mbed/topic_1") == 0)
    {

    }
}

int main()
{   
    wait(1);
    pc.printf("=========================\r\r\n");
    pc.printf("===mbed 5.9 MQTT demo ===\r\r\n");
    pc.printf("=========================\r\r\n");
  
    Net_Init();
    MQTT_Init();
    
    /*Start threads*/
    thread_1000ms.start(task_1000ms);

    while(1)
    {
        if(mqtt.yield(1000L) != 0)
        {
            while(1)
            {
                led_blue = 1;
                Thread::wait(1000);
                led_blue = 0;
                Thread::wait(1000);
            }
        }
    }
}

void task_1000ms()
{
    static uint32_t counter = 0;
    while(1)
    {
        char counter_buffer[10];
        sprintf(counter_buffer,"%ld",counter++);
        mqtt_publish_str("mbed/topic_1",counter_buffer);
        Thread::wait(1000);
    }
}

void Net_Init()
{
    net.connect();
    const char *ip = net.get_ip_address();
    const char *netmask = net.get_netmask();
    const char *gateway = net.get_gateway();
    pc.printf("IP address: %s\r\r\n", ip ? ip : "None");
    pc.printf("Netmask: %s\r\r\n", netmask ? netmask : "None");
    pc.printf("Gateway: %s\r\r\n", gateway ? gateway : "None");
    pc.printf("Connecting to %s:%d\r\r\n", mqtt_broker, mqtt_port);
    rc = mqtt_network.connect(mqtt_broker, mqtt_port);
    if(rc != 0) pc.printf("Failed to get network [Error %d]\r\r\n", rc);
    else pc.printf("Connected to network!\r\r\n");
}

void MQTT_Init()
{
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    MQTTPacket_willOptions LWT = MQTTPacket_willOptions_initializer;

    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed_F207ZG_1";
 
    data.username.cstring = "jrxanwut";
    data.password.cstring = "uDU6C6XHHsFF";
  
    rc = mqtt.connect(data);
    if(rc != 0) pc.printf("Failed to connect to MQTT [Error %d]\r\r\n", rc);
    else pc.printf("Connected to MQTT Broker!\r\r\n");

    rc =mqtt.subscribe("mbed/topic_1", MQTT::QOS0, MessageHandle);

    if(rc != 0) pc.printf("Failed to subcribed [Error %d]\r\r\n", rc);
    else{
        pc.printf("Subcribed successfully!\r\r\n");
    }
    mqtt_publish_str("status","Connected");
}

void mqtt_publish_num(const char* mqtt_topic,uint16_t num)
{
    MQTTMutex.lock();
    char buf[100];
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    sprintf(buf,"%d",num);
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf);
    mqtt.publish(mqtt_topic, message);
    MQTTMutex.unlock();
}

void mqtt_publish_str(const char* mqtt_topic,const char* msg)
{
    MQTTMutex.lock();
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)msg;
    message.payloadlen = strlen(msg);
    mqtt.publish(mqtt_topic, message);
    MQTTMutex.unlock();
}