from io import BytesIO
import json
import random
import math
import time
from PIL import Image
from flask import Flask, Response, send_from_directory, jsonify

app = Flask(__name__)

def get_image():
    cnt = 0
    while True:
        try:
            with open('pic.jpg', "rb") as f:
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

            with open("pic.jpg", "rb") as f:
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
    return Response(get_image(), mimetype='multipart/x-mixed-replace; boundary=frame')


prev_ax = 0.0
prev_ay = 0.0
prev_az = 0.0
prev_vx = 0.0
prev_vy = 0.0
prev_vz = 0.0
prev_time = None

# API endpoint to provide sensor data
@app.route('/api/sensor_data')
def sensor_data():
    global prev_vx, prev_vy, prev_vz, prev_time, prev_ax, prev_ay, prev_az
    
    with open('data.json', 'r') as file:
        print('file opened')
        loadData = json.load(file)
    
    print(loadData)
    
    # Heart rate between 60-180 BPM
    bpm = float(loadData['bpm'])
    avg_bpm = int(loadData['bpm_avg'])
    
    # Acceleration values (-10 to 10 m/s²) - limiting to smaller range for smoother display
    ax = float(loadData['a_x'])
    ay = float(loadData['a_y'])
    az = float(loadData['a_z'])
    
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

        
    # IMU values (Yaw, Pitch, Roll in degrees) - limiting to smaller range for display consistency
    roll = float(loadData['g_x']) * 180.0 / math.pi
    pitch = float(loadData['g_y']) * 180.0 / math.pi
    yaw = float(loadData['g_z']) * 180.0 / math.pi
    
    # Position on rink (x: 0-30m, y: 0-60m for hockey rink dimensions)
    pos_x = round(random.uniform(0, 29.9), 1)
    pos_y = round(random.uniform(0, 59.9), 1)
    
    return jsonify({
        'bpm': bpm,
        'avg_bpm': avg_bpm,
        'ax': ax,
        'ay': ay,
        'az': az,
        'vx': vx,
        'vy': vy,
        'vz': vz,
        'temp': f"{temp}°C",
        'stress': stress,
        'yaw': yaw,
        'pitch': pitch,
        'roll': roll,
        'pos_x': pos_x,
        'pos_y': pos_y,
        'timestamp': time.time()
    })

# Serve static files (images, etc.)
@app.route('/<path:filename>')
def static_files(filename):
    return send_from_directory('.', filename)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=False, threaded=True)