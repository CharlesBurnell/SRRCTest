#!/usr/bin/env python

import rospy
import freenect
import cv2
import frame_convert2
import numpy as np
import kinectdepth as k
from std_msgs.msg import Bool

debug = True
#debug = False
cv2.namedWindow('Keypoints')
cv2.namedWindow('Depth')
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
openingDelay = rospy.Rate(5)
if debug:
    Counter = 0

openingDelay.sleep()
while not rospy.is_shutdown():
    rate.sleep()
    if debug:
        if Counter %100 == 0:
            print Counter
        if Counter > 30:
            break
        Counter += 1
    depth, timestamp = freenect.sync_get_depth()
    #depth,safe = isBlocked(depth)
    video = frame_convert2.video_cv(freenect.sync_get_video()[0])
    #pub.publish(safe)
    maskArray = k.findObjects(video)
    for mask in maskArray:
        keypointsArray, bluredIm = k.findBlobs(mask)
        cannyEdges = k.ourCannyBlob(mask)

        for point in keypointsArray:
            try:
                bluredIm, pointEdgeArray = k.createPerimeter(bluredIm, cannyEdges, point)
            except:
                pass
            cv2.imshow( "Keypoints", bluredIm)
            if cv2.waitKey(10) == 27:
                break
    k.show_depth(depth, "Depth")
    k.show_video(video, "Video")
    if cv2.waitKey(10) == 27:
        break
