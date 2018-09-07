# IoT Smart Garden
Using the Sparkfun Thing, a soil moisture sensor, a UV light sensor, and a 12V water pump, this IOT project monitors the state of a plant, knows the optimal watering date and time, automates watering, and presents data over a mobile application through Blynk. 

While this project has more functionality for outdoor plants, it still has usage for indoor ones. It can tell whether the plant is indoors my measuring the UV index, and then water it when the soil moisture falls below a certain threshold. For outdoor plants, once the software detects that the plant is too dry, it predicts the weather for the following two days using the OpenWeather API and then using the JsonListener library to parse the JSON it returns. It then determines which day is cooler and waters it that day at 8am, as a result of mornings being the optimal watering time. 


## DEMO
https://www.youtube.com/watch?v=S-8N-nDglmo
