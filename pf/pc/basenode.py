import os
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.neighbors import KNeighborsRegressor
import serial as ser
import requests
import json
import threading
import time
import numpy as np
from matplotlib import pyplot as plt
import pandas as pd
import matplotlib
matplotlib.use('TkAgg')

os.chdir(os.path.dirname(os.path.abspath(__file__)))


url = "https://api.tago.io/data"


headers = {
    'device-token': '2f6dd09c-7e19-44bc-995b-39f79ea0b21a',
    'Content-Type': 'application/json'
}


def calculate_air_quality(temp, humidity, co2, tvoc):
    # Define maximum "safe" values and minimum "unsafe" values for each variable
    # Updated max and min values for each variable
    max_temp = 120  # degrees Celsius
    min_temp = -40  # degrees Celsius
    max_humidity = 100  # percent
    min_humidity = 0  # percent
    max_tvoc = 1187  # ppb
    min_tvoc = 0  # ppb
    max_co2 = 8192  # ppm
    min_co2 = 400  # ppm

    # Convert each variable to a score out of 100, where the min value scores 0 and the max value scores 100
    temp_score = ((float(temp) - min_temp) / (max_temp - min_temp)) * 100
    humidity_score = ((float(humidity) - min_humidity) /
                      (max_humidity - min_humidity)) * 100
    tvoc_score = ((max_tvoc - float(tvoc)) / (max_tvoc - min_tvoc)
                  ) * 100  # reversed as higher TVOC is worse
    co2_score = ((max_co2 - float(co2)) / (max_co2 - min_co2)) * \
        100  # reversed as higher CO2 is worse

    # Create a weighted average of the scores
    # Here, we're giving temperature and humidity 15% of the weight each,
    # and tvoc and co2 35% of the weight each
    quality_score = (0.15 * temp_score + 0.15 * humidity_score +
                     0.35 * tvoc_score + 0.35 * co2_score)

    # Clip the score to the range 0-100
    quality_score = max(0, min(100, quality_score))

    return quality_score


def send_values_to_rest_api(temp, humidity, tvoc, fo2, air_quality):
    # Replace this with your RESTful API endpoint'

    payload = json.dumps([
        {
            "variable": "temperature",
            "value": temp,
            "group": "1631814703673",
            "unit": "C"
        },
        {
            "variable": "humidity",
            "value": humidity,
            "group": "1631814703673",
            "unit": "m"
        },
        {
            "variable": "tvoc",
            "value": tvoc,
            "group": "1631814703672",
            "unit": "ppb"
        },
        {
            "variable": "co2",
            "value": fo2,
            "group": "1631814703672",
            "unit": "ppm"
        },
        {
            "id": "6143842f561c4e00199f517b",
            "variable": "airquality",
            "value": air_quality,
            "unit": "%"
        },
    ])
    response = requests.request("POST", url, headers=headers, data=payload)
    print(response.text)


# Set up the serial connection
# Replace this with your specific serial port
serial_port = '/dev/cu.usbmodem1303'
baud_rate = 115200
ser = ser.Serial(serial_port, baud_rate)

while True:

    line = ser.readline().decode('utf-8').strip()
    print(line)
    try:
        temp, humidity, fo2, tvoc = line.split(',')
        air_quality = calculate_air_quality(temp, humidity, fo2, tvoc)
        print(f"Temperature: {temp}")
        print(f"Humidity: {humidity}")
        print(f"TVOC: {tvoc}")
        print(f"FO2: {fo2}")
        print((f"Air Quality: {air_quality}"))
        send_values_to_rest_api(temp, humidity, tvoc, fo2, air_quality)

    except ValueError:
        print("Incorrect data format, expecting 'Temperature,humidity,tvoc,fo2'")
