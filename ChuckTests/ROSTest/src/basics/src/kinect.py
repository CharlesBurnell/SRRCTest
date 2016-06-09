#!/usr/bin/env python

import rospy
import freenect
import cv2
import frame_convert2
import numpy as np
from std_msgs.msg import Bool

def show_depth(depth):
    cv2.imshow('Depth', depth)


def show_video():
    cv2.imshow('Video', frame_convert2.video_cv(freenect.sync_get_video()[0]))

def isObjectInFront(depth):
    vert = len(depth)
    hori = len(depth[0])
    safe = True
    for i in xrange(hori/4,hori/4+hori/2,5):
        testDepth = 0
        numOutOfRange = 15
        for j in xrange(vert-1,vert/2,-1):
            newDepth = depth[j][i]
            if newDepth-testDepth < 9:
                depth[j][i] = 255
            else:
                depth[j][i]=0
                numOutOfRange -= 1
            if numOutOfRange < 0:
                safe = False
            testDepth=newDepth
    return depth,safe

cv2.namedWindow('Depth')
cv2.namedWindow('Video')
print('Press ESC in window to stop')


rospy.init_node('kinect')
pub = rospy.Publisher('isClear',Bool)
rate = rospy.Rate(15)

while not rospy.is_shutdown():
    depth, timestamp = freenect.sync_get_depth()
    depth = depth.astype(np.uint8)
    depth,safe = isObjectInFront(depth)
    print safe
    pub.publish(safe)
    show_depth(depth)
    show_video()
    if cv2.waitKey(10) == 27:
        break
    rate.sleep()
