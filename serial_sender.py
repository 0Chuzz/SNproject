import serial
from PIL import Image
from PIL import ImageTk
from PIL import ImageDraw
from PIL.ImageColor import getrgb
from tkFileDialog import askopenfilenames
from Tkinter import *
from PIL import Image, ImageTk
import Tkinter
import threading
import sys
from time import sleep
serial = serial.Serial("COM3", 115200)
print "reading banner"
test = serial.readline()
print "test: ", test
serial.write("aaaaaaaa")
print serial.readline()
root = Tkinter.Tk()

labimage = ImageTk.PhotoImage("RGB", (320,240))
lab = Tkinter.Label(root, image=labimage)
lab.pack()
listimgs = askopenfilenames(parent=root)
try:
        listimgs = listimgs.split(" ")
except:
        pass

colors = [
        (255,0,0)
        ,(0,255,0)
        ,(0,0,255)
        ,(0,255,255)
        ,(255,0,255)
        ,(255,255, 0)
        ,(50,50,125)
        ,(125,50,50)
        ,(50,125,50)
        ,(0,125,125)
        ,(125,0,125)
        ,(125,125,0)
        ,(125,0,255)
        ]

class MyApp (threading.Thread):
        def run(self):
                for imgpath in listimgs:
                        print "opening", imgpath
                        img = Image.open(imgpath)
                        imgb = img.tobytes()
                        print "sending", len(imgb), "bytes"
                        serial.write(img.tobytes())
                        print "sent", serial.read(16), "bytes";
                        readedresponse = serial.read(1);
                        while readedresponse != "$":
                                sys.stdout.write(readedresponse)
                                readedresponse = serial.read(1);
                        """ When exiting from this loop read:
                        1. The number of objects tracked (i.e. the number of bbox that will be received
                        2. All the bounding box
                        3. Retrieve original image, then draw the bounding box on them and finally show
                        """
                        readedresponse = serial.read(1)
                        boundingBoxes =  []
                        print "just readed ", readedresponse
                        while readedresponse != "$":
                                ident = serial.read(3).strip();
                                topX = int(serial.read(3))
                                topY = int(serial.read(3))
                                botX = int(serial.read(3))
                                botY = int(serial.read(3))
                                boundingBoxes.append((ident, topX, topY, botX, botY))
                                readedresponse = serial.read(1)
                                
                        #img = Image.open(imgpath.replace("foreground_mask", "original")).convert("RGB")
                        img = img.convert("RGB")
                        drw = ImageDraw.Draw(img)
                        
                        for bbox in boundingBoxes:
                                ident, topX, topY, botX, botY = bbox
                                col = colors[int(ident)%len(colors)]
                                drw.rectangle([(topX, topY), (botX, botY)], outline=col)
                                drw.text(((topX + botX)/2, (topY+botY)/2), ident, fill=col)
                                print bbox

                        labimage.paste(img)
                        #sleep(5)

                sleep(5)
                exit()

thr = MyApp()
thr.start()
Tkinter.mainloop()
