# exam2
先將exam2/exam_main/mbed_app.json中的SSID及PASSWORD改成可用自己可用的WiFi AP， 接著exam2/exam_main/main.cpp第177行改成自己的IP， 
exam2/exam_main/wifi_mqtt/mqtt_client.py第13行改成自己的IP。
在main.cpp中的main是關於mqtt的code，但考試的時候沒辦法實作，所以那時候把它comment掉，用RPC傳指令停止分析手勢
在getAcc中有用LED2來顯示是否進入getAcc，然後再用thread到acc_mode，去偵測手勢，以lab8的方法然後我將結果以ring = 1，
slope = 2，rectangle = 3，的方式在uLCD上呈現，自己分析的部分用mqtt傳01的sequence到client。
