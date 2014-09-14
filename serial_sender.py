import serial
from PIL import Image
from PIL import ImageTk
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

labimage = ImageTk.PhotoImage("1", (320,240))
lab = Tkinter.Label(root, image=labimage)
lab.pack()
listimgs = askopenfilenames(parent=root)

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
                                ident = serial.read(3);
                                topX = serial.read(3);
                                topY = serial.read(3);
                                botX = serial.read(3);
                                botY = serial.read(3);
                                boundingBoxes.append((ident, topX, topY, botX, botY))
                                readedresponse = serial.read(1)
                        for bbox in boundingBoxes:
                                print bbox
                        
                        #trackedImage.createRectangle(10, 10, 10, outline="red")
                        #imgb =  serial.read(320*240/8- 1) + " "
                        #img2 = Image.frombytes("1", (320,240), imgb)
                        #serial.read(1);
                        #print "received"
                        #labimage.paste(img2)
                sleep(5)
                exit()

thr = MyApp()
thr.start()
Tkinter.mainloop()
