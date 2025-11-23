# ESP32S3 Weather Station project

I wanted to make a custom weather station using an esp32s3. More specifically, I am using the so-called Cheap Yellow Display. The chip is perfect for displaying screens that are mostly just static, refreshing the weather api once per 1 hour maybe.
Alongside that, the chip already offers wifi with very minimal setup, so its perfect for calling an API every once in a while.


## Goals of this project:
- Step 1) Setup wifi connection (and reconnection), HTTP API calls and JSON parsing.
- Step 2) Design UI in figma
- Step 3) Implement UI with LVGL
- Future expansions in no particular order) 
    - battery power
    - in-home temperature with sensor
    - weather (precipation) map display
    - battery powered esp32c3 with temperature sensor and wireless protocol to send measured outside temperature data to the station.

### Notes:
- OpenWeatherMap offers 1M free calls of their API per month. This is my calculation:
    ```
    1000000 calls / month

    32258 calls / day (assuming 31 days in month)

    1344 calls / hour

    22,4 calls / min
    Also note hard limit of max 60 calls / min
    ```
    
### TODO:
- v1.0:
    - Finish backend (call TimeZoneDB and parse time and date)
    - Connect data recieved by backend to ui
    - Model and print the case

- v1.1:
    - Make the weather forecast screen
    - Upgrade backend to fetch 5 day weather forecast data