import matplotlib.pyplot as plt
from socket import *
from PIL import Image
from io import BytesIO

MSG_TYPE_TEXT       = 0
MSG_TYPE_STREAM     = 1
MSG_TYPE_BPM        = 2
MSG_TYPE_BPM_AVG    = 3
MSG_TYPE_ACC_X      = 4
MSG_TYPE_ACC_Y      = 5
MSG_TYPE_ACC_Z      = 6
MSG_TYPE_GYRO_X     = 7
MSG_TYPE_GYRO_Y     = 8
MSG_TYPE_GYRO_Z     = 9
MSG_TYPE_TEMP       = 10

def updateImg(msgData):
    with Image.open(BytesIO(msgData)) as img:
        img_display.set_data(img)
        plt.draw()
        plt.pause(0.000001)

try:
    serverName = '192.168.2.18'
    serverPort = 12000
    clientSocket = None
    
    fig, ax = plt.subplots()
    ax.axis('off')
    img_display = ax.imshow([[0, 0], [0, 0]], animated=True)
    data = {
        MSG_TYPE_BPM : b'',
        MSG_TYPE_BPM_AVG : b'',
        MSG_TYPE_ACC_X : b'',
        MSG_TYPE_ACC_Y : b'',
        MSG_TYPE_ACC_Z : b'',
        MSG_TYPE_GYRO_X : b'',
        MSG_TYPE_GYRO_Y : b'',
        MSG_TYPE_GYRO_Z : b'',
        MSG_TYPE_TEMP : b''
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
                    print(f'  BPM( {data[MSG_TYPE_BPM_AVG].decode()} )  A( ({data[MSG_TYPE_ACC_X].decode()}, {data[MSG_TYPE_ACC_Y].decode()}, {data[MSG_TYPE_ACC_Z].decode()} )  G( {data[MSG_TYPE_GYRO_X].decode()}, {data[MSG_TYPE_GYRO_X].decode()}, {data[MSG_TYPE_GYRO_X].decode()} )  TEMP( {data[MSG_TYPE_TEMP].decode()} )', end='')
                    
                    if msgType == MSG_TYPE_TEXT:
                        print(f' --- received TEXT {msgLen} B: {msgData}', end='')
                        
                    elif msgType == MSG_TYPE_STREAM:
                        print(f' --- received STREAM {msgLen} B: (JPEG {len(msgData)} B)', end='')
                        updateImg(msgData)
                        # with open('pic.jpg', 'wb') as file:
                            # file.write(msgData)

                    elif msgType == MSG_TYPE_BPM:
                        print(f' --- received BPM {msgLen} B: {msgData}', end='')
                        data[MSG_TYPE_BPM] = msgData;
                        
                    elif msgType == MSG_TYPE_BPM_AVG:
                        print(f' --- received BPM_AVG {msgLen} B: {msgData}', end='')
                        data[MSG_TYPE_BPM_AVG] = msgData;
                        
                    elif msgType == MSG_TYPE_ACC_X:
                        print(f' --- received ACC_X {msgLen} B: {msgData}', end='')
                        data[MSG_TYPE_ACC_X] = msgData;

                    elif msgType == MSG_TYPE_ACC_Y:
                        print(f' --- received ACC_Y {msgLen} B: {msgData}', end='')
                        data[MSG_TYPE_ACC_Y] = msgData;

                    elif msgType == MSG_TYPE_ACC_Z:
                        print(f' --- received ACC_Z {msgLen} B: {msgData}', end='')
                        data[MSG_TYPE_ACC_Z] = msgData;
                        
                    elif msgType == MSG_TYPE_GYRO_X:
                        print(f' --- received GYRO_X {msgLen} B: {msgData}', end='')
                        data[MSG_TYPE_GYRO_X] = msgData;

                    elif msgType == MSG_TYPE_GYRO_Y:
                        print(f' --- received GYRO_Y {msgLen} B: {msgData}', end='')
                        data[MSG_TYPE_GYRO_Y] = msgData;                    

                    elif msgType == MSG_TYPE_GYRO_Z:
                        print(f' --- received GYRO_Z {msgLen} B: {msgData}', end='')
                        data[MSG_TYPE_GYRO_Z] = msgData;

                    elif msgType == MSG_TYPE_TEMP:
                        print(f' --- received TEMP {msgLen} B: {msgData}', end='')
                        data[MSG_TYPE_TEMP] = msgData;

                    else:
                        print(f' ---- received (len : {msgLen} B, type : {msgType}) : {msgData}', end='')
                        
                    print()

                print('Connection to client lost...')
                clientSocket.close()

except KeyboardInterrupt:
    print('Keyboard Interrupt. Exiting...')
    if clientSocket != None:
        clientSocket.close()
