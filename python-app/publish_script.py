import paho.mqtt.client as mqtt
import time

import tkinter as tk
from tkinter import simpledialog
from tkinter import filedialog as fd
from tkinter.messagebox import showinfo

import ctypes
import time

from PIL import Image


# ====================== MQTT broker details ============================ #
broker_address = "192.168.1.172"
broker_port = 1883
topic_for_publishing = "fromLyraT"
topics_for_subscribing = ["fromESP32","fromLyraT","Forward pic to esp32"]
# ====================== MQTT broker details ============================ #

filename = ""


# CONVERTING IMAGE TO HEX CODE READY FOR PUBLISHING TO ESP32
def image_to_hex(name):
    original_image = Image.open(name)

    new_size = (128, 64)
    resized_image = original_image.resize(new_size)

    monochrome_image = resized_image.convert("1")
    monochrome_image.save("bitmap.bmp", "BMP")

    image_data = bytearray()

    for y in range(64):  # Iterate through rows
        for x in range(0, 128, 8):  # Iterate through columns of 8 pixels
            byte = 0
            for bit in range(8):  # Iterate through bits in each byte
                pixel_value = monochrome_image.getpixel((x + bit, y))
                if pixel_value > 50:  # Threshold for black or white
                    byte |= (1 << (7 - bit))  # Invert bit order
            image_data.append(byte)

    hex_code = ''.join(f'{byte:02X}' for byte in image_data)
    return hex_code

# PUBLISHING CONVERTED IMAGE IN THREE PARTS TO ESP32
def publish_image(hex_code):
    first_third = "11" + hex_code[0:680]
    second_thrid = "22" + hex_code[680:1360]
    thrid_thrid = "33" + hex_code[1360:]
    client.publish("picture", first_third)
    print(len(first_third))
    print(first_third)
    client.publish("picture", second_thrid)
    print(len(second_thrid))
    print(second_thrid)
    client.publish("picture", thrid_thrid)
    print(len(thrid_thrid))
    print(thrid_thrid)
    global filename
    filename = ""

# CALL FUNCTION FOR THE PUBLISH MESSAGE BUTTON
def publish_message():
    user_input = entry.get()
    if user_input:
        client.publish(topic_for_publishing, user_input)
        entry.delete(0, tk.END)

# CONNECT HANDLER
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    for topic in topics_for_subscribing:
        client.subscribe(topic)

# RECIEVED MESSAGE HANDLER
def on_message(client, userdata, msg):
    received_message = msg.payload.decode("utf-8")
    print("Recieved  (" +msg.topic+ ")  ~>  " + received_message)
    result_label.config(text="\nRecieved  (" +msg.topic+ ")  ~>  " + received_message)  #display message
    if(str(msg.topic) == "Forward pic to esp32"):
        global filename
        if filename != "":
            hex_code = image_to_hex(filename)
            publish_image(hex_code)
        else:
            file_label.config(text="Image not selected!")

# CHOOSE FILE BUTTON FUNCTION
def select_file():
    global filename
    filename = fd.askopenfilename(
        title='Open a file',
        initialdir='/')

    file_label.config(text=filename.split('/')[-1])


# ====================== CONNECTING TO CLIENT ============================ #
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(broker_address, broker_port, 60)
client.loop_start()
# ====================== CONNECTING TO CLIENT ============================ #

# ====================== CONFIGURING GUI ============================ #
root = tk.Tk()
root.title("MQTT app")

root.columnconfigure(0, weight=1, minsize=75)
root.rowconfigure(0, weight=1, minsize=50)
root.columnconfigure(1, weight=1, minsize=75)
root.rowconfigure(1, weight=1, minsize=50)

frame_a = tk.Frame(relief=tk.RAISED,borderwidth=5)
frame_b = tk.Frame(relief=tk.RAISED,borderwidth=5)
frame_c = tk.Frame(relief=tk.FLAT,borderwidth=5)
frame_d = tk.Frame(relief=tk.FLAT,borderwidth=5)

    # FRAME A
publish_message_label = tk.Label(master=frame_a, text=" Enter a message to publish to esp32", font=("Helvetica", 18))
publish_message_label.pack(pady = 10, padx = 10)

entry = tk.Entry(master=frame_a, font=("Helvetica", 18))
entry.pack(pady=15, padx = 10)

publish_message_button = tk.Button(master=frame_a, text="Publish Message", font=("Helvetica", 12), command=publish_message)
publish_message_button.pack(pady=10)


    # FRAME B
recieved_label = tk.Label(master=frame_b, text="Choose an image to show on screen", font=("Helvetica", 18))
recieved_label.grid(row=0, column=0, sticky="n", pady=10, padx=10)

file_label = tk.Label(master=frame_b, text="", font=("Helvetica", 14))
file_label.grid(row=1, column=0, sticky="n", pady=10, padx=10)

file_button = tk.Button(master = frame_b, text = "Choose image file", font=("Helvetica", 12), command=select_file)
file_button.grid(row=2, column=0, sticky="n", pady=10, padx=10)


    # FRAME C
recieved_label = tk.Label(master=frame_c, text="Message traffic", font=("Helvetica", 18))
recieved_label.grid(row=0, column=0, sticky="n", pady=10, padx=10)

result_label = tk.Label(master=frame_c, text="", font=("Helvetica", 16))
result_label.grid(row=1, column=0, sticky="s", pady=10, padx = 10)


    # FRAME D
available_commands = ["Say Hello", "What time is it?", "Show me the temperature", "I'm sending you an image!"]
comm_label = tk.Label(master=frame_d, text="Available commands:", font=("Helvetica", 18), relief=tk.FLAT,borderwidth=3, pady=10)
comm_label.pack()
for command in available_commands:
    label = tk.Label(master=frame_d, text=" "+str(command), font=("Helvetica", 18), relief=tk.RAISED,borderwidth=3)
    label.pack(anchor="w")

frame_a.grid(row=0, column=0, sticky="n", pady=30, padx=10)
frame_b.grid(row=0, column=1, sticky="n", pady=35, padx=10)
frame_c.grid(row=1, column=1, sticky="n", pady=30, padx=10)
frame_d.grid(row=1, column=0, sticky="n", pady=30, padx = 10)
# ====================== CONFIGURING GUI ============================ #

if(client.is_connected()):
    root.mainloop()
else:
    print("Not connected to the client! Restart.")

# Disconnect from the broker
client.disconnect()
