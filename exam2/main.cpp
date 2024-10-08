#include "mbed.h"
#include "mbed_rpc.h"
#include "math.h"
#include "uLCD_4DGL.h"
#include "stm32l475e_iot01_accelero.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"


#include "accelerometer_handler.h"
#include "config.h"
#include "magic_wand_model_data.h"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
int seq[10];
//DigitalOut myled(LED3);
RpcDigitalOut myled1(LED1, "myled1"); // for gesture indicate
RpcDigitalOut myled2(LED2, "myled2"); // for detection indicate
RpcDigitalOut myled3(LED3, "myled3");
//void doGesture(Arguments *in, Reply *out);
//void doAngledetect(Arguments *in, Reply *out);
void getAcc(Arguments *in, Reply *out);
RPCFunction rpcAcc(&getAcc, "getAcc");
//RPCFunction rpcgesture(&doGesture, "doGesture");
//RPCFunction rpcangledetect(&doAngledetect, "doAngledetect");
BufferedSerial pc(USBTX, USBRX);
//Thread gesturethread(osPriorityLow);
Thread anglethread(osPriorityLow);
Thread ACCthread(osPriorityLow);
uLCD_4DGL uLCD(D1, D0, D2);
int check = 1;
double_t theta;
Timeout angle_mqtt;
int angle[3];
int f = 0;
double_t confirmmmm;
int countcount = 0;
Thread RPC_mqtt_half(osPriorityHigh);
Thread WIFI_half(osPriorityHigh);

WiFiInterface *wifi = WiFiInterface::get_default_instance();
//InterruptIn btn2(USER_BUTTON);
volatile int message_num = 0;
volatile int arrivedcount = 0;
volatile bool closed = false;
NetworkInterface* net = WiFiInterface::get_default_instance();
const char* topic = "Mbed";

MQTTNetwork mqttNetwork(net);
MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);
Thread mqtt_thread(osPriorityHigh);
EventQueue mqtt_queue;

void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client) {
    message_num++;
    MQTT::Message message;
    char buff[100];

    sprintf(buff, "%02d %d%d%d%d%d%d%d%d%d%d", countcount, seq[0], seq[1], seq[2], seq[3], seq[4], seq[5], seq[6], seq[7], seq[8], seq[9]);

    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*) buff;
    message.payloadlen = strlen(buff) + 1;
    int rc = client->publish(topic, message);

    //printf("rc:  %d\r\n", rc);
    printf("%s\n", buff);
}

void close_mqtt() {
    closed = true;
}

void halfcode() {
    char buf[256], outbuf[256];

    FILE *devin = fdopen(&pc, "r");
    FILE *devout = fdopen(&pc, "w");
    while(1) {
        memset(buf, 0, 256);
        for (int i = 0; ; i++) {
            char recv = fgetc(devin);
            if (recv == '\n') {
                printf("\r\n");
                break;
            }
            buf[i] = fputc(recv, devout);
        }
        RPC::call(buf, outbuf);
        printf("%s\r\n", outbuf);
    }
}


constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

// Return the result of the last prediction
int PredictGesture(float* output) {
  // How many times the most recent gesture has been matched in a row
  static int continuous_count = 0;
  // The result of the last prediction
  static int last_predict = -1;

  // Find whichever output has a probability > 0.8 (they sum to 1)
  int this_predict = -1;
  for (int i = 0; i < label_num; i++) {
    if (output[i] > 0.8) this_predict = i;
  }

  // No gesture was detected above the threshold
  if (this_predict == -1) {
    continuous_count = 0;
    last_predict = label_num;
    return label_num;
  }

  if (last_predict == this_predict) {
    continuous_count += 1;
  } else {
    continuous_count = 0;
  }
  last_predict = this_predict;

  // If we haven't yet had enough consecutive matches for this gesture,
  // report a negative result
  if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {
    return label_num;
  }
  // Otherwise, we've seen a positive result, so clear all our variables
  // and report it
  continuous_count = 0;
  last_predict = -1;

  return this_predict;
}

float S, D, U;
/*
int main() {
    RPC_mqtt_half.start(halfcode);
////////////////////////////////////////////
    WIFI_half.start(wifi_halfcode);
    
}*/

int main(int argc, char* argv[]) {
    RPC_mqtt_half.start(halfcode);
////////////////////////////////////////////
    //WIFI_half.start(wifi_halfcode);
    //wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
            printf("ERROR: No WiFiInterface found.\r\n");
            return -1;
    }


    printf("\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
            printf("\nConnection error: %d\r\n", ret);
            return -1;
    }



    //TODO: revise host to your IP
    const char* host = "172.20.10.5";
    printf("Connecting to TCP network...\r\n");

    SocketAddress sockAddr;
    sockAddr.set_ip_address(host);
    sockAddr.set_port(1883);

    printf("address is %s/%d\r\n", (sockAddr.get_ip_address() ? sockAddr.get_ip_address() : "None"),  (sockAddr.get_port() ? sockAddr.get_port() : 0) ); //check setting

    int rc = mqttNetwork.connect(sockAddr);//(host, 1883);
    if (rc != 0) {
            printf("Connection error.");
            return -1;
    }
    printf("Successfully connected!\r\n");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "Mbed";

    if ((rc = client.connect(data)) != 0){
            printf("Fail to connect MQTT\r\n");
    }

    mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
    //btn2.rise(mqtt_queue.event(&publish_message, &client));
    //btn3.rise(&close_mqtt);

    int num = 0;
    while (num != 5) {
            client.yield(100);
            ++num;
    }

    while (1) {
            if (closed) break;
            client.yield(500);
            ThisThread::sleep_for(500ms);
    }

    printf("Ready to close MQTT Network......\n");

    if ((rc = client.unsubscribe(topic)) != 0) {
            printf("Failed: rc from unsubscribe was %d\n", rc);
    }
    if ((rc = client.disconnect()) != 0) {
    printf("Failed: rc from disconnect was %d\n", rc);
    }

    mqttNetwork.disconnect();
    printf("Successfully closed!\n");
    return 0;

}


int16_t ppDataXYZ[3] = {0};
double_t refXYZ[3] = {0};
double_t lengthr;

void angle_mode()
{
    double_t mDataXYZ[3] = {0};
    double_t lengtha;
    //double_t lengthb;
    double_t dot;
    int i = 0, j = 0, k = 0;
    int seq[10];
    uint8_t XYZ[30];
    confirmmmm = 30;
    while(1) {
        for (i = 0; i < 30; i++) {
            XYZ[i] = tensor_arena[i];
        }
        while (j + 2 < 30) {
            ppDataXYZ[0] = XYZ[j];
            ppDataXYZ[1] = XYZ[j + 1];
            ppDataXYZ[2] = XYZ[j + 2];
            mDataXYZ[0] = ppDataXYZ[0]/1024.0;
            mDataXYZ[1] = ppDataXYZ[1]/1024.0;
            mDataXYZ[2] = ppDataXYZ[2]/1024.0;
            dot = mDataXYZ[0] * refXYZ[0] + mDataXYZ[1] * refXYZ[1] + mDataXYZ[2] * refXYZ[2];
            lengtha = sqrt(mDataXYZ[0] * mDataXYZ[0] + mDataXYZ[1] * mDataXYZ[1] + mDataXYZ[2] * mDataXYZ[2]);
            if (lengtha < 0) lengtha = -lengtha;
            theta = acos(dot/lengtha/lengthr)/3.14*180;
            if (theta > confirmmmm) {
                seq[k] = 1;
            } else {
                seq[k] = 0;
            }
            j += 3;
            k += 1;
        }
    }
}

void ACC_mode()
{
    bool should_clear_buffer = false;
    bool got_data = false;

    // The gesture index of the prediction
    int gesture_index;
    double_t mDataXYZ[3] = {0};
    double_t lengtha;
    //double_t lengthb;
    double_t dot;
    int i = 0, j = 0, k = 0;
    uint8_t XYZ[30];
    confirmmmm = 30;
    
    // Set up logging.
    static tflite::MicroErrorReporter micro_error_reporter;
    tflite::ErrorReporter* error_reporter = &micro_error_reporter;

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report(
            "Model provided is schema version %d not equal "
            "to supported version %d.",
            model->version(), TFLITE_SCHEMA_VERSION);
        return -1;
    }

    // Pull in only the operation implementations we need.
    // This relies on a complete list of all the ops needed by this graph.
    // An easier approach is to just use the AllOpsResolver, but this will
    // incur some penalty in code space for op implementations that are not
    // needed by this graph.
    static tflite::MicroOpResolver<6> micro_op_resolver;
    micro_op_resolver.AddBuiltin(
        tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
        tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                                tflite::ops::micro::Register_MAX_POOL_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                                tflite::ops::micro::Register_CONV_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                                tflite::ops::micro::Register_FULLY_CONNECTED());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                                tflite::ops::micro::Register_SOFTMAX());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE,
                                tflite::ops::micro::Register_RESHAPE(), 1);

    // Build an interpreter to run the model with
    static tflite::MicroInterpreter static_interpreter(
        model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
    tflite::MicroInterpreter* interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors
    interpreter->AllocateTensors();

    // Obtain pointer to the model's input tensor
    TfLiteTensor* model_input = interpreter->input(0);
    if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
        (model_input->dims->data[1] != config.seq_length) ||
        (model_input->dims->data[2] != kChannelNumber) ||
        (model_input->type != kTfLiteFloat32)) {
        error_reporter->Report("Bad input tensor parameters in model");
        return -1;
    }

    int input_length = model_input->bytes / sizeof(float);

    TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
    if (setup_status != kTfLiteOk) {
        error_reporter->Report("Set up failed\n");
        return -1;
    }
    
    //anglethread.start(angle_mode);
    //error_reporter->Report("Set up successful...\n");
    while (true) {
        // Attempt to read new data from the accelerometer
        got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                    input_length, should_clear_buffer);

        // If there was no new data,
        // don't try to clear the buffer again and wait until next time
        if (!got_data) {
        should_clear_buffer = false;
        continue;
        }

        // Run inference, and report any error
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
        error_reporter->Report("Invoke failed on index: %d\n", begin_index);
        continue;
        }

        // Analyze the results to obtain a prediction
        gesture_index = PredictGesture(interpreter->output(0)->data.f);

        // Clear the buffer next time we read data
        should_clear_buffer = gesture_index < label_num;

        // Produce an output
        //if (gesture_index < label_num) {
        //error_reporter->Report(config.output_message[gesture_index]);
        //}
        countcount++;
        for (i = 0; i < 30; i++) {
            XYZ[i] = tensor_arena[i];
        }
        k = 0;
        j = 0;
        while (j + 2 < 30) {
            ppDataXYZ[0] = XYZ[j];
            ppDataXYZ[1] = XYZ[j + 1];
            ppDataXYZ[2] = XYZ[j + 2];
            mDataXYZ[0] = ppDataXYZ[0]/1024.0;
            mDataXYZ[1] = ppDataXYZ[1]/1024.0;
            mDataXYZ[2] = ppDataXYZ[2]/1024.0;
            dot = mDataXYZ[0] * refXYZ[0] + mDataXYZ[1] * refXYZ[1] + mDataXYZ[2] * refXYZ[2];
            lengtha = sqrt(mDataXYZ[0] * mDataXYZ[0] + mDataXYZ[1] * mDataXYZ[1] + mDataXYZ[2] * mDataXYZ[2]);
            if (lengtha < 0) lengtha = -lengtha;
            theta = acos(dot/lengtha/lengthr)/3.14*180;
            if (theta > confirmmmm) {
                seq[k] = 1;
            } else {
                seq[k] = 0;
            }
            j += 3;
            k += 1;
        }
        angle_mqtt.attach(mqtt_queue.event(&publish_message, &client), 2ms);
        if (gesture_index == 0) {
            uLCD.cls();
            uLCD.text_height(1);
            uLCD.printf("%3d", 1);
        } else if (gesture_index == 1) {
            uLCD.cls();
            uLCD.text_height(1);
            uLCD.printf("%3d", 2);
        } else if (gesture_index == 2) {
            uLCD.cls();
            uLCD.text_height(1);
            uLCD.printf("%3d", 3);
        }
    }
}


void getAcc(Arguments *in, Reply *out) {
   bool success = true;
    char strings[20];
    char buffer[200], outbuf[256];
    int z = in->getArg<int>();
    BSP_ACCELERO_Init();
    
    if (z) {
        sprintf(strings, "/myled1/write 0");
        strcpy(buffer, strings);
        RPC::call(buffer, outbuf);
        if (success) {
            out->putData(buffer);
        } else {
            out->putData("Failed to execute LED control");
        }
        sprintf(strings, "/myled2/write 1");
        strcpy(buffer, strings);
        RPC::call(buffer, outbuf);
        if (success) {
            out->putData(buffer);
        } else {
            out->putData("Failed to execute LED control");
        }
        ACCthread.start(ACC_mode);

    } else {
        sprintf(strings, "/myled1/write 0");
        strcpy(buffer, strings);
        RPC::call(buffer, outbuf);
        if (success) {
            out->putData(buffer);
        } else {
            out->putData("Failed to execute LED control");
        }
        sprintf(strings, "/myled2/write 0");
        strcpy(buffer, strings);
        RPC::call(buffer, outbuf);
        if (success) {
            out->putData(buffer);
        } else {
            out->putData("Failed to execute LED control");
        }
        anglethread.terminate();
        ACCthread.terminate();
    }
}
