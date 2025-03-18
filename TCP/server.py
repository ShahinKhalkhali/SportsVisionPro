import matplotlib.pyplot as plt
from socket import *
from PIL import Image
from io import BytesIO

MSG_TYPE_TEXT = 0
MSG_TYPE_STREAM = 1

def updateImg(msgData):
    with Image.open(BytesIO(msgData)) as img:
        img_display.set_data(img)
        plt.draw()
        plt.pause(0.001)

try:
    serverName = '192.168.2.18'
    serverPort = 12000
    clientSocket = None
    
    fig, ax = plt.subplots()
    ax.axis('off')
    img_display = ax.imshow([[0, 0], [0, 0]], animated=True)

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
                    
                    if msgType == MSG_TYPE_TEXT:
                        print(f'received TEXT {msgLen} B: {msgData}')
                    elif msgType == MSG_TYPE_STREAM:
                        print(f'received STREAM {msgLen} B: JPEG {len(msgData)} B')
                        updateImg(msgData)
                        # with open('pic.jpg', 'wb') as file:
                            # file.write(msgData)
                    else:
                        print(f'received {msgLen} B --- TYPE {msgType} --- DataSize: {len(msgData)}')

                print('Connection to client lost...')
                clientSocket.close()

except KeyboardInterrupt:
    print('Keyboard Interrupt. Exiting...')
    if clientSocket != None:
        clientSocket.close()
