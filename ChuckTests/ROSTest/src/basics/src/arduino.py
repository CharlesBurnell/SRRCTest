#!/usr/bin/env python

import rospy
import serial
from std_msgs.msg import Bool
s = serial.Serial('/dev/ttyACM0', 9600, timeout=5)

def callback(msg):
    print msg.data
    if msg.data:
        s.write("0")
    else:
        s.write("1")


rospy.init_node('arduino')
sub = rospy.Subscriber('isClear', Bool, callback)

rospy.spin()
