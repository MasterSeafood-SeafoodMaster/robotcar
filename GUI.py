import tkinter as tk
import threading as t
import time
import socket
import os, sys
import numpy as np

HOST = "192.168.137.110"  # The server's hostname or IP address
PORT = 8000  # The port used by the server

RobotKey="n"
FullScreenState=True
CarPosition=[700, 400]
Canvas=None
Height = 0
Movement = 'n'
CarAngleWithPlane = 90
Tilt = 0

X = 0
Y = []
Z = []
Prev = 0
R = G = B = 255
wall = 'n'

SIZE = 5
STEP = 7
        
        
        
    

def send():
    global X, wall, Movement
    while True:
        try:
            msg = RobotKey
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((HOST, PORT))
                s.sendall(bytes(msg, encoding='utf-8'))
                # print("send", msg)
                # data = s.recv(32)
                # Roll & Pitch are the angles which rotate by the axis X and Y
                # X and Z angle are equal
                axis = s.recv(64).decode('utf8') # without encode there is a b'' infront of string
                # print("receive data:", axis)
                X = round(float(axis.split(',')[0]), 1)  # 車子傾斜角度
                wall = s.recv(1).decode('utf8')
                Movement = s.recv(1).decode('utf8')
                print('wall:', wall)
                print('Movement:', Movement)
                if Movement != 'n':
                    app.carMove()
                # print(wall)
                
            # print("-------")
            # print(f"Received {data}")
            # print(f"Current axis {axis}")
            # print("-------")
            time.sleep(0.01)
        except:
            pass
            

t1 = t.Thread(target=send)
t1.daemon = True
t1.start()

class MyApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.initUI()
        

    def initUI(self):
        self.title('GUI')
        self.geometry('480x480')
        self.attributes('-fullscreen', FullScreenState) 

        self.setButton("A", (50, 100), self.buttonPress, self.buttonRelease)	
        self.setButton("S", (100, 100), self.buttonPress, self.buttonRelease)
        self.setButton("W", (100, 50), self.buttonPress, self.buttonRelease)
        self.setButton("D", (150, 100), self.buttonPress, self.buttonRelease)
        
        self.setButton("G", (300, 100), self.buttonPress, self.buttonRelease)
        self.setButton("J", (450, 100), self.buttonPress, self.buttonRelease)
        
        self.setCanvas((50, 200))

    def setLabel(self, text, pos):
        label = tk.Label(self, text=text, font=("MV Boli", 16), bg="white")
        label.place(x=pos[0], y=pos[1])

    def setEntry(self, re, pos, default, width):
        entry = tk.Entry(self, font=("MV Boli", 16), textvariable=re, width=width)
        entry.insert(-1, default)
        entry.place(x=pos[0], y=pos[1])

    def setButton(self, text, pos, pFun, rFun):
        button = tk.Button(self, text=text, font=("MV Boli", 16), command=lambda: pFun(text), width=3, height=1)
        button.place(x=pos[0], y=pos[1])

        pkey = "<KeyPress-"+text.lower()+">"
        self.bind(pkey,lambda event:pFun(text))

        rkey = "<KeyRelease-"+text.lower()+">"
        self.bind(rkey,lambda event:rFun(text))
        
    def setCanvas(self, pos):
        global Canvas
        color = f'#{self.getTerrainHeightColor()}'
        Canvas=tk.Canvas(self, width=1400, height=750, bg='#FFFFFF')
        Canvas.place(x=pos[0], y=pos[1])
        Canvas.create_oval(CarPosition[0], CarPosition[1], CarPosition[0] + SIZE, CarPosition[1] + SIZE, width = 0.3, fill = 'black', outline = color)
        
    def buttonPress(self, text):
        global RobotKey, FullScreenState, CarPosition
        #print("buttonPressed", text)
        if text=="W":
            RobotKey="w"
            # CarPosition[1] = CarPosition[1] - STEP
        elif text=="A":
            RobotKey="a"
            # CarPosition[0] = CarPosition[0] - STEP
        elif text=="S":
            RobotKey="s"
            # CarPosition[1] = CarPosition[1] + STEP
        elif text=="D":
            RobotKey="d"
            # CarPosition[0] = CarPosition[0] + STEP
        elif text=="G":
            pass
        elif text=="J":
            FullScreenState = not FullScreenState
            self.attributes('-fullscreen', FullScreenState) 
        
        # self.carMove()

    def buttonRelease(self, text):
        global RobotKey
        RobotKey="n"
        print("buttonReleased", text)
    
    def carMove(self):
        global CarAngleWithPlane, Movement, CarPosition
        CarBodyLength = 5
        # Movement = RobotKey
        
        if Movement=="a":
            TurningAngle = 5
        elif Movement=="d":
            TurningAngle = -5
        else:
            TurningAngle = 0
        
        if Movement=="s":
            CarPosition[0] = (CarPosition[0] + 
                             self.cos( TurningAngle + CarAngleWithPlane ) + 
                             self.sin( TurningAngle ) * self.sin( CarAngleWithPlane ))
            CarPosition[1] = (CarPosition[1] + 
                             self.sin( TurningAngle + CarAngleWithPlane ) - 
                             self.sin( TurningAngle ) * self.cos( CarAngleWithPlane ))
        else:
            CarPosition[0] = (CarPosition[0] - 
                             self.cos( TurningAngle + CarAngleWithPlane ) + 
                             self.sin( TurningAngle ) * self.sin( CarAngleWithPlane ))
            CarPosition[1] = (CarPosition[1] - 
                             self.sin( TurningAngle + CarAngleWithPlane ) - 
                             self.sin( TurningAngle ) * self.cos( CarAngleWithPlane ))
                         
        CarAngleWithPlane = CarAngleWithPlane - np.degrees(np.arcsin( (2 * self.sin( TurningAngle )) / CarBodyLength))
        # print(CarPosition)
        self.updateTilt()
        self.drawMap()
        
        
    def carMoveOld(self):
        global Movement
        # print('move:', Movement)
        if Movement=="w":
            CarPosition[1] = CarPosition[1] - STEP
        elif Movement=="a":
            CarPosition[0] = CarPosition[0] - STEP
        elif Movement=="s":
            CarPosition[1] = CarPosition[1] + STEP
        elif Movement=="d":
            CarPosition[0] = CarPosition[0] + STEP
        self.updateTilt()
        self.drawMap()
        
    def cos(self, angle):
        if (angle == 90) or (angle == 270):
            return 0
        else:
            return np.cos( np.radians( angle )  )

    def sin(self, angle):
        if (angle == 180) or (angle == 360):
            return 0
        else:
            return np.sin(np.radians(angle))
    
    def drawMap(self):
        # print('move')
        global Canvas
        global wall
        color = f'#{self.getTerrainHeightColor()}'
        Canvas.create_oval(CarPosition[0], CarPosition[1], CarPosition[0] + SIZE, CarPosition[1] + SIZE, width = 1, fill = 'black', outline = color)
        if wall == 'y':
            self.drawBarrier()
        Canvas.update()
        # print(CarPosition)
        
    def drawBarrier(self):
        global Movement
        BarrieSize = SIZE + 3
        BarrieStep = BarrieStep + 3
        if Movement=="w":
            Canvas.create_rectangle(CarPosition[0], CarPosition[1] - BarrieStep, 
                                    CarPosition[0] + BarrieSize, CarPosition[1] - BarrieStep + BarrieSize, 
                                    width = 1, fill = 'black')
        elif Movement=="a":
            Canvas.create_rectangle(CarPosition[0] - BarrieStep, CarPosition[1], 
                                    CarPosition[0] - BarrieStep + BarrieSize, CarPosition[1] + BarrieSize, 
                                    width = 1, fill = 'black')      
        elif Movement=="s":
            Canvas.create_rectangle(CarPosition[0], CarPosition[1] + BarrieStep, 
                                    CarPosition[0] + BarrieSize, CarPosition[1] + BarrieStep + BarrieSize, 
                                    width = 1, fill = 'black')
        elif Movement=="d":
            Canvas.create_rectangle(CarPosition[0] + BarrieStep, CarPosition[1], 
                                    CarPosition[0] + BarrieStep + BarrieSize, CarPosition[1] + BarrieSize, 
                                    width = 1, fill = 'black')
                                    
    def updateTilt(self):
        # 上坡 X 0.11 up up 0.22代表斜度越大
        # 下坡 X 255.0 down down
        # 靜止不動 0.11 0.12 0.10
        global Tilt, X
        currentCarTilt = 0
        if X == 0.1:
            # print('平面')
            currentCarTilt = 0
        elif X > 255:
            # print('上坡')
            currentCarTilt = X - 255
        elif X > 0.1:
            # print('下坡')
            currentCarTilt = -(X)
        
        # print(X)
        
        Tilt = int(Tilt + currentCarTilt * 10)
        # print(Tilt)
        
        
    def getTerrainHeightColor(self):
        global Tilt
        global R, G, B
        # Tilt += 3
        if Tilt > 255:
            Tilt = 255
            G -= 10
        if Tilt < 0:
            Tilt = 0
            R -= 10
            
        if R < 0:
            R = 0
            
        if G < 0:
            G = 0
        return '%02x%02x%02x' % (R, G, B - Tilt)
        
        

app = MyApp() 
app.mainloop()

