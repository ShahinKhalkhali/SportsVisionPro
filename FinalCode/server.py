from socket import *
import json

MSG_TYPE_TEXT   = 0
MSG_TYPE_STREAM = 1
MSG_TYPE_BPM    = 2
MSG_TYPE_IMU    = 3
MSG_TYPE_TAG    = 4

BUFFER_SIZE     = 1280
STREAM_FILENAME = 'stream.jpg'
JSON_FILENAME   = 'data.json'


try:
    serverIP = '192.168.2.18'
    serverPort = 12000
    clientSocket = None
    
    data = {
        'bpm' : '0.0',
        'acc' : {
            'x' : '0.0',
            'y' : '0.0',
            'z' : '0.0'
        },
        'gyro' : {
            'x' : '0.0',
            'y' : '0.0',
            'z' : '0.0'
        },
        'mag' : {
            'x' : '0.0',
            'y' : '0.0',
            'z' : '0.0'            
        },
        'temp' : '0.0',
        'pos' : {
            'x' : '0.0',
            'y' : '0.0'
        }
    }
    
    with socket(AF_INET, SOCK_DGRAM) as s:
        s.bind((serverIP, serverPort))
        s.settimeout(1.0)
        print(f'> UDP server started on {serverIP}:{serverPort}')
        
        timeoutCnt = 0
        while True:
            try:
                msgHeader, addr = s.recvfrom(BUFFER_SIZE)
                timeoutCnt = 0
                if len(msgHeader) != 5: continue
                
                msgLen = int.from_bytes(msgHeader[:4], 'little')
                msgType = msgHeader[4]
                
                msgData = b''
                while len(msgData) < msgLen:
                    chunk, addr = s.recvfrom(BUFFER_SIZE)
                    msgData += chunk
                
                if msgType == MSG_TYPE_TEXT:
                    print(f'> Received  TEXT  {msgLen} B  {msgData}')
                    
                elif msgType == MSG_TYPE_STREAM:
                    print(f'> Received  STREAM  {msgLen} B  (JPEG {len(msgData)})')
                    if len(msgData) != msgLen:
                        print(f'> ERROR: JPG size mismatch!')
                        continue
                    with open(STREAM_FILENAME, 'wb') as file:
                        file.write(msgData)
                    
                elif msgType == MSG_TYPE_BPM:
                    print(f'> Received  BPM  {msgLen} B  {msgData}')
                    try:
                        data['bpm'] = msgData.decode()
                    except UnicodeDecodeError:
                        print('> ERROR: could not decode BPM')
                    
                elif msgType == MSG_TYPE_IMU:
                    print(f'> Received  IMU  {msgLen} B  {msgData}')
                    try :
                        imu = msgData.decode().split(', ')
                        if len(imu) != 10:
                            print(f'> ERROR: unexpected IMU length ({len(imu)})')
                            continue
                        data['acc']['x'] = imu[0]
                        data['acc']['y'] = imu[1]
                        data['acc']['z'] = imu[2]
                        data['gyro']['x'] = imu[3]
                        data['gyro']['y'] = imu[4]
                        data['gyro']['z'] = imu[5]
                        data['mag']['x'] = imu[6]
                        data['mag']['y'] = imu[7]
                        data['mag']['z'] = imu[8]
                        data['temp'] = imu[9]
                    except UnicodeDecodeError:
                        print('> ERROR: could not decode IMU')
                    
                elif msgType == MSG_TYPE_TAG:
                    print(f'> Received  TAG  {msgLen} B  {msgData}')
                    try:
                        tag = msgData.decode().split(', ')
                        if len(tag) != 3:
                            print(f'> ERROR: unexpected TAG length ({len(TAG)})')
                        # get pos from tag ranges 0, 1, 2
                        data['pos']['x'] = 0.0
                        data['pos']['y'] = 0.0
                    except UnicodeDecodeError:
                        print('> ERROR: could not decode TAG')
                    
                else:
                    print(f'> Received  ?? {msgType} ??  {msgLen} B  {msgData}')
                    
                if msgType >= MSG_TYPE_BPM and msgType <= MSG_TYPE_TAG:
                    with open(JSON_FILENAME, 'w') as file:
                        json.dump(data, file, indent=4)
            
            except timeout:
                timeoutCnt += 1
                print(f'recv timeout {timeoutCnt}')
                if timeoutCnt >= 20:
                    print('Nobody there...')
                    break
                continue
    
except KeyboardInterrupt:
    print('Keyboard Interrupt! Exiting...')
    if clientSocket != None:
        clientSocket.close()
    s.close()

print('Goodbye!')