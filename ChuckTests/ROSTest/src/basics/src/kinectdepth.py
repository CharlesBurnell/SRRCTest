#!/usr/bin/env python

import rospy
import freenect
import cv2
import frame_convert2
import numpy as np
from blobsDetectionLib import radialSearch
def show_depth(depth,window):
    """
    passes a raw depth from the kinect and displays it
    ont a image called window
    """
    #depth = depth.astype(np.uint8)
    depth = frame_convert2.pretty_depth_cv(depth)
    cv2.imshow(window, depth)


def show_video(image, window):
    """
    passes a raw image  from the kinect and displays it
    ont a image called window
    """
    cv2.imshow(window, image)

def findObjects(image):
    """
    takes a bgr image and then does hsv scans
    based on the hue search sizeval as a plus
    minus.

    it returns an array of masks
    (this might need to be changed if it gets to large)
    """
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    searchSize = 15
    maxHVal = 360/2

    hsvLow = 240/2
    hsvHigh = 60/2

    lowerSVal = 75
    upperSVal = 255

    lowerVVal = 100
    upperVVal = 255

    returnedMasks = []
    i = hsvLow + searchSize
    # i<hsvHigh so that it will stop at the max without overshooting it
    while(i>=hsvLow or i < hsvHigh):

        lowerSubsearchHVal = i - searchSize
        higherSubsearchHVal = i + searchSize

        lowerSubmatrix = np.array([lowerSubsearchHVal,lowerSVal,lowerVVal])
        upperSubmatrix = np.array([higherSubsearchHVal,upperSVal,upperVVal])

        mask = cv2.inRange(hsv, lowerSubmatrix, upperSubmatrix)
        res = cv2.bitwise_and(image,video, mask= mask)
        i = (i+searchSize) % maxHVal
        returnMasks.append(mask)
        #findBlobs(mask)
    return returnMasks
    """
    removed findBlobs(mask)
    to instead send back an array of masks so that the masks can be
    handled by the main function
    """
def findBlobs(mask):
    """
    This takes a mask and then returns keypoints of the objects detected
    """
    blurSize = 9 #Must be odd
    mask = 255 - mask #this is to invert it becuase the blob detection
                      # finds black and doesn't find blobs of white
    bluredIm = cv2.GaussianBlur(mask,(blurSize,blurSize),0)

    # Set up the detector with parameters.
    params = cv2.SimpleBlobDetector_Params()
    params.filterByArea = True
    params.minArea = 100
    params.maxArea = 150000
    params.filterByCircularity = False
    #params.filterByInertia = False
    params.minInertiaRatio = 0.01;
    params.filterByConvexity = False

    # Detect blobs.
    detector = cv2.SimpleBlobDetector_create(params)
    keypoints = detector.detect(bluredIm)
    return keypoints

def ourCannyBlob(mask):
    """
    This takes a mask and returns a cannyEdge image
    """
    blurSize = 5
    mask = 255-mask
    bluredIm = cv2.GaussianBlur(mask,(blurSize,blurSize),0)

    #canny edge detector
    sigma = .33
    v = np.median(bluredIm)
    lower = int(max(0, (1.0 - sigma) * v))
    upper = int(min(255, (1.0 + sigma) * v))
    cannyEdges = cv2.Canny(bluredIm, lower, upper, 5)
    return cannyEdges


def createPerimeter(bluredIm, cannyEdges, point):
    """
    This takes a blured
    """
    pointxy = point.pt
    print pointxy
    pointx = pointxy[0]
    pointy = pointxy[1]
    bluredSize = 3
    #mask[pointy,pointx]=[0, 255, 0]
    cannyEdges = cv2.GaussianBlur(cannyEdges, (bluredSize,bluredSize), 0)
    #print cannyEdges
    pointEdgeArray, cannyEdges = radialSearch(pointx, pointy, cannyEdges)
    #print pointEdgeArray
    for edgePoint in pointEdgeArray:
        plusminus = 2
        edgePointx = edgePoint[0]
        edgePointy = edgePoint[1]
        for i in xrange(- plusminus, plusminus):
            for j in xrange(-plusminus,plusminus):
                try:
                    bluredIm[edgePointy+j,edgePointx+i] = [120,120,0]
                except:
                    pass
    #findMaxMinAxis(pointEdgeArray,bluredIm)
    return bluredIm, pointEdgeArray

#TODO add the ability for it to make sure that the depth
#doesn't start at to low or two high becuase it just
#tries to find a progressively further thing away.
#TODO make this look at options of going left or right.
def isBlocked(depth):
    """
    Takes a depth and does some VERY basic searching to
    see if it ther is an object in front of it
    """
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
