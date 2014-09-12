import serial
from PIL import Image
from PIL import ImageTk
from tkFileDialog import askopenfilenames
import Tkinter
import threading
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
listimgs = askopenfilenames(parent=root).split(" ")

class MyApp (threading.Thread):
        def run(self):
                for imgpath in listimgs:
                        print "opening", imgpath
                        img = Image.open(imgpath)
                        imgb = img.tobytes()
                        print "sending", len(imgb), "bytes"
                        serial.write(img.tobytes())
                        print "sent", serial.read(16), "bytes";
                        imgb =  serial.read(320*240/8- 1) + " "
                        img2 = Image.frombytes("1", (320,240), imgb)
                        serial.read(1);
                        print "received"
                        labimage.paste(img2)
                sleep(5)
                exit()

thr = MyApp()
thr.start()
Tkinter.mainloop()	
