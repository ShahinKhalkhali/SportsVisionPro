from io import BytesIO
import json
import random
import math
import time
from PIL import Image
from flask import Flask, Response, send_from_directory, jsonify
import numpy as np
import shutil
import os

STREAM_FILE = 'stream.jpg'

CALIB_AX = 1.5
CALIB_AY = 0.6

app = Flask(__name__)

def get_stream():
    cnt = 0
    while True:
        try:
            with open(STREAM_FILE, "rb") as f:
                image_bytes = f.read()
            image = Image.open(BytesIO(image_bytes))
            img_io = BytesIO()
            image.save(img_io, 'JPEG')
            img_io.seek(0)
            img_bytes = img_io.read()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + img_bytes + b'\r\n')

        except Exception as e:
            print("encountered an exception: ")
            print(e)

            with open(STREAM_FILE, "rb") as f:
                image_bytes = f.read()
            image = Image.open(BytesIO(image_bytes))
            img_io = BytesIO()
            image.save(img_io, 'JPEG')
            img_io.seek(0)
            img_bytes = img_io.read()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + img_bytes + b'\r\n')
            continue

# Serve the HTML page at the root URL
@app.route('/')
def index():
    try:
        with open('index.html', 'r') as f:
            html_content = f.read()
        return html_content
    except Exception as e:
        return f"Error loading index.html: {str(e)}"

# Serve the image stream
@app.route('/video_feed')
def video_feed():
    return Response(get_stream(), mimetype='multipart/x-mixed-replace; boundary=frame')


roll, pitch, yaw = 0.0, 0.0, 0.0
prev_ax, prev_ay, prev_az = 0.0, 0.0, 0.0
prev_vx, prev_vy, prev_vz = 0.0, 0.0, 0.0
prev_time = None

# API endpoint to provide sensor data
@app.route('/api/sensor_data')
def sensor_data():
    global prev_vx, prev_vy, prev_vz, prev_time, prev_ax, prev_ay, prev_az
    global roll, pitch, yaw
    
    with open('data.json', 'r') as file:
        print('file opened')
        loadData = json.load(file)
    
    print(loadData)
    
    # Heart rate between 60-180 BPM
    bpm = float(60.0)
    avg_bpm = int(loadData['bpm'])
    
    # Acceleration values (-10 to 10 m/s²) - limiting to smaller range for smoother display
    ax = float(loadData['acc']['x'])
    ay = float(loadData['acc']['y'])
    az = float(loadData['acc']['z'])
    
    if np.abs(ax) < CALIB_AX: ax = 0.0
    elif ax < 0: ax += CALIB_AX
    else: ax -= CALIB_AX
    
    if np.abs(ay) < CALIB_AY: ay = 0.0
    elif ay < 0: ay += CALIB_AY
    else: ay -= CALIB_AY
    
    # accel = np.sqrt(ax * ax + ay * ay + az * az)
    accel = np.sqrt(ax * ax + ay * ay)
    
    curr_time = time.time()
    
    # Calculate time difference (Δt)
    if prev_time is not None:
        dt = curr_time - prev_time
        dt = max(dt, 1e-6)  # Prevent division by zero
    else:
        dt = 0  # First call, no time difference
        
    # Velocity values (-5 to 5 m/s)
        
    if (ax != prev_ax):
        vx = prev_vx + ax * dt
    else:
        vx = 0.0
    
    if (ay != prev_ay):
        vy = prev_vy + ay * dt
    else:
        vy = 0.0
    
    if (az != prev_az):
        vz = prev_vz + az * dt
    else:
        vz = 0.0
    
    # IMU values (Yaw, Pitch, Roll in degrees) - limiting to smaller range for display consistency
    dRoll    = float(loadData['gyro']['x'])
    dPitch   = float(loadData['gyro']['y'])
    dYaw     = float(loadData['gyro']['z'])
    if np.abs(dRoll) > 0.01:    roll    += dRoll * dt
    if np.abs(dPitch) > 0.01:   pitch   += dPitch * dt
    if np.abs(dYaw) > 0.01:     yaw     += dYaw * dt

    prev_vx, prev_vy, prev_vz, prev_time = vx, vy, vz, curr_time
    prev_ax, prev_ay, prev_az = ax, ay, az
    
    # Temperature (35-40°C for body temp)
    temp = float(loadData['temp'])
    
    # Stress level (1-10)
    if (avg_bpm < 90.0):
        stress = 'Low'
    elif (avg_bpm < 120.0):
        stress = 'Medium'
    else:
        stress = 'High'

    # Position on rink (x: 0-30m, y: 0-60m for hockey rink dimensions)
    pos_x = 0.0
    pos_y = 0.0
    
    return jsonify({
        'bpm': bpm,
        'avg_bpm': avg_bpm,
        'ax': accel,
        'ay': ay,
        'az': az,
        'vx': vx,
        'vy': vy,
        'vz': vz,
        'temp': f"{temp:2.1f}°C",
        'stress': stress,
        'yaw': yaw * 180.0 / np.pi,
        'pitch': pitch * 180.0 / np.pi,
        'roll': roll * 180.0 / np.pi,
        'pos_x': pos_x,
        'pos_y': pos_y,
        'timestamp': time.time()
    })

# Serve static files (images, etc.)
@app.route('/<path:filename>')
def static_files(filename):
    return send_from_directory('.', filename)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=80, debug=False, threaded=True)