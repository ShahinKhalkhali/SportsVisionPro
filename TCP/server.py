import matplotlib.pyplot as plt
import json
from socket import *
from PIL import Image
from io import BytesIO

MSG_TYPE_TEXT       = 0
MSG_TYPE_STREAM     = 1
MSG_TYPE_BPM        = 2
MSG_TYPE_BPM_AVG    = 3
MSG_TYPE_IMU        = 4

def updateImg(msgData):
    with Image.open(BytesIO(msgData)) as img:
        img_display.set_data(img)
        plt.draw()
        plt.pause(0.001)

try:
    # serverName = '192.168.2.18'
    serverName = '192.168.43.130'
    serverPort = 12000
    clientSocket = None
    
    fig, ax = plt.subplots()
    ax.axis('off')
    img_display = ax.imshow([[0, 0], [0, 0]], animated=True)
    data = {
        'bpm' : '0.0',
        'bpm_avg' : '0',
        'a_x' : '0.0',
        'a_y' : '0.0',
        'a_z' : '0.0',
        'g_x' : '0.0',
        'g_y' : '0.0',
        'g_z' : '0.0',
        'temp' : '0.0'
    }

    with socket(AF_INET, SOCK_STREAM) as serverSocket:
        serverSocket.bind((serverName, serverPort))
        serverSocket.settimeout(1.0)
        serverSocket.listen(1)
        print('Server listening...')

        exitCnt = 0
        while True:
            try:
                clientSocket, clientAddr = serverSocket.accept()
            
            except timeout:
                exitCnt += 1
                print('Accept timeout', exitCnt)
                
                if exitCnt >= 15:
                    print('Nobody there... Goodbye!')
                    break
                
                continue
            
            exitCnt = 0
            clientSocket.settimeout(5.0)
            
            with clientSocket:
                print(f'Connection from { clientAddr }')
                
                while True:
                    try:
                        msgLen = int.from_bytes(clientSocket.recv(4), 'little')
                        if not msgLen: break
                        
                        msgType = int.from_bytes(clientSocket.recv(1))
                        
                        msgData = b''
                        while len(msgData) < msgLen:
                            chunk = clientSocket.recv(msgLen - len(msgData))
                            if not chunk:
                                break
                            msgData += chunk
                        
                    except timeout:
                        break
                    
                    # print(f'> BPM: {data[MSG_TYPE_BPM]}, AVG BPM: {data[MSG_TYPE_BPM_AVG]}', end='')
                    print(f'  BPM( {data['bpm_avg']} )  A( ({data['a_x']}, {data['a_y']}, {data['a_z']} )  G( {data['g_x']}, {data['g_y']}, {data['g_z']} )  TEMP( {data['temp']} )', end='')
                    
                    if msgType == MSG_TYPE_TEXT:
                        print(f' --- received TEXT {msgLen} B: {msgData}', end='')
                        
                    elif msgType == MSG_TYPE_STREAM:
                        print(f' --- received STREAM {msgLen} B: (JPEG {len(msgData)} B)', end='')
                        # updateImg(msgData)
                        with open('pic.jpg', 'wb') as file:
                            file.write(msgData)

                    elif msgType == MSG_TYPE_BPM:
                        print(f' --- received BPM {msgLen} B: {msgData}', end='')
                        data['bpm'] = msgData.decode();
                        
                    elif msgType == MSG_TYPE_BPM_AVG:
                        print(f' --- received BPM_AVG {msgLen} B: {msgData}', end='')
                        data['bpm_avg'] = msgData.decode();
                        
                    elif msgType == MSG_TYPE_IMU:
                        print(f' --- received IMU {msgLen} B: {msgData}', end='')
                        ddd = msgData.decode().split(', ')
                        data['a_x'] = ddd[0]
                        data['a_y'] = ddd[1]
                        data['a_z'] = ddd[2]
                        data['g_x'] = ddd[3]
                        data['g_y'] = ddd[4]
                        data['g_z'] = ddd[5]
                        data['temp'] = ddd[6]
                        with open('data.json', 'w') as file:
                            json.dump(data, file, indent=4)
                        
                    else:
                        print(f' ---- received (len : {msgLen} B, type : {msgType}) : {msgData}', end='')
                        
                    print()

                print('Connection to client lost...')
                clientSocket.close()

except KeyboardInterrupt:
    print('Keyboard Interrupt. Exiting...')
    if clientSocket != None:
        clientSocket.close()
