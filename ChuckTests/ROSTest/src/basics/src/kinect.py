#!/usr/bin/env python

import rospy
import freenect
import cv2
import frame_convert2
import numpy as np
from std_msgs.msg import Bool

debug = True
#debug = False
def show_depth(depth):
    #depth = depth.astype(np.uint8)
    depth = frame_convert2.pretty_depth_cv(depth)
    cv2.imshow('Depth', depth)


def show_video(video):
    cv2.imshow('Video', video)



def findObjects(video):
    hsv = cv2.cvtColor(video, cv2.COLOR_BGR2HSV)
    searchSize = 15
    maxHVal = 360/2
    hsvLow = 240/2
    hsvHigh = 60/2
    i = hsvLow + searchSize
    # i<hsvHigh so that it will stop at the max without overshooting it
    while(i>=hsvLow or i < hsvHigh):
        lowerSubsearchHVal = i - searchSize
        higherSubsearchHVal = i + searchSize
        #print (i-searchSize,i+searchSize)
        lowerSVal = 50
        upperSVal = 255

        lowerVVal = 100
        upperVVal = 255
        lowerSubmatrix = np.array([lowerSubsearchHVal,lowerSVal,lowerVVal])
        upperSubmatrix = np.array([higherSubsearchHVal,upperSVal,upperVVal])

        mask = cv2.inRange(hsv, lowerSubmatrix, upperSubmatrix)
        res = cv2.bitwise_and(video,video, mask= mask)
        i = (i+searchSize) % maxHVal
        if debug:
            #cv2.imshow('Mask', mask)
            #cv2.imshow('HSVScan',res)
            findBlobs(mask)
            cv2.waitKey(100)

def findBlobs(image):
    image = 255 - image
    bluredIm = cv2.GaussianBlur(image,(5,5),0)
    #ret,bluredIm = cv2.threshold(bluredIm,240,255,cv2.THRESH_BINARY)
    #bluredIm = cv2.adaptiveThreshold(bluredIm,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,\
    #        cv2.THRESH_BINARY,11,2)
    # Set up the detector with default parameters.
    params = cv2.SimpleBlobDetector_Params()
    params.filterByArea = True
    params.minArea = 100
    params.maxArea = 150000
    params.filterByCircularity = False
    #params.filterByInertia = False
    params.minInertiaRatio = 0.1;
    params.filterByConvexity = False

    detector = cv2.SimpleBlobDetector_create(params)

    # Detect blobs.
    keypointsBlur = detector.detect(bluredIm)
    detector = cv2.SimpleBlobDetector_create(params)
    keypoints = detector.detect(image)
    keypointsImage = cv2.drawKeypoints(image, keypoints, np.array([]), (0,255,0), cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
    cv2.imshow("Keypoints", keypointsImage)
    cv2.waitKey(1)

def isBlocked(depth):
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

cv2.namedWindow('Keypoints')
#cv2.namedWindow('Depth')
cv2.namedWindow('Video')
#cv2.namedWindow('HSVScan')
#cv2.namedWindow('Mask')
print('Press ESC in window to stop')


rospy.init_node('kinect')
pub = rospy.Publisher('isClear',Bool)

if debug:
    rate = rospy.Rate(.5)
else:
    rate = rospy.Rate(1)

if debug:
    Counter = 0


while not rospy.is_shutdown():
    if debug:
        if Counter %100 == 0:
            print Counter
        if Counter > 12:
            break
        Counter += 1
    depth, timestamp = freenect.sync_get_depth()
    depth,safe = isBlocked(depth)
    video = frame_convert2.video_cv(freenect.sync_get_video()[0])
    pub.publish(safe)
    findObjects(video)
    #show_depth(depth)
    show_video(video)
    if cv2.waitKey(10) == 27:
        break
    rate.sleep()
