#!/usr/bin/env python

import rospy
import freenect
import cv2
import frame_convert2
import numpy as np
import kinectdepth as k
import blobDetectionLib as bd
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
    rate = rospy.Rate(15)
else:
    rate = rospy.Rate(1)

if debug:
    Counter = 0
while not rospy.is_shutdown():
    rate.sleep()
    if debug:
        if Counter % 100 == 0:
            print(Counter)
        #if Counter > 30:
        #    break
        Counter += 1
    depth, timestamp = freenect.sync_get_depth()
    #depth,safe = isBlocked(depth)
    video = frame_convert2.video_cv(freenect.sync_get_video()[0])

    pub.publish(True)

    maskArray = k.findObjects(video)
    for mask in maskArray:
        keypointsArray, bluredIm = k.findBlobs(mask)
        #this is added up here becuase as part ofthis bluredIm goes from a 1 dimensional
        # ndarray to 3 channels which we need later
        # TODO Figure out how to do this without that.
        bluredIm = cv2.drawKeypoints(bluredIm, keypointsArray, np.array([]), (0,0,255),
                                              cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
        cannyEdges = k.ourCannyBlob(mask)
        for point in keypointsArray:
            #try:
            #bluredIm = k.convert_BWnumpy_to_BGR(bluredIm)
            video, pointEdgeArray = k.createPerimeter(video, cannyEdges, point)
            #bd.findMaxDist(pointEdgeArray, video)
            #TODO: change this to cover the only the out of bounds exception it should cover

            #except:
            #    pass
        cv2.imshow( "Keypoints", bluredIm)
        #This is so that you can hold escape to end
        if cv2.waitKey(10) == 27:
            break
    k.show_depth(depth, "Depth")
    k.show_video(video, "Video")
    #This is so that you can hold escape to end
    if cv2.waitKey(10) == 27:
        break
