# TODO List
# make sure that the check doesnt go out of bounds
# maybe spin this off as its own function so it can be used
# in other functions

# Standard imports
import cv2
import numpy as np
from math import cos
from math import sin
from math import radians

# this can be improved
# doing the vector form of lines but i dont want to rederive that
# so fix this sometime in the future
def drawLinesBetweenPoints(image, pointlist, colorArray):
    #numPoints = 100
    for i in xrange(len(pointlist)):
        point1 = (int(pointlist[i][0]),int(pointlist[i][1]))
        point2 = (int(pointlist[(i+1)%len(pointlist)][0]),int(pointlist[(i+1)%len(pointlist)][1]))
        print (point1, point2)
        cv2.line(image, point1, point2, colorArray, 2)
        '''
        pointx1 = pointlist[i][0]
        pointx2 = pointlist[(i+i)%len(pointlist)][0]
        pointy1 = pointlist[i][1]
        pointy2 = pointlist[(i+i)%len(pointlist)][1]
        slope = (pointy2 - pointy1)/(pointx2 - pointx1)
        #for j in xrange(numPoints):
        currentx = pointx1
        currenty = pointy1
        '''
def radialSearch(x, y, edgeMat):
    if edgeMat[y, x] != 0:
        return
    pointEdgeArray = []
    numberOfSections = 16
    degreeStep = 360 / numberOfSections
    edgeBuf = 15
    for i in xrange(0,360 , degreeStep):
        sinStep = sin(radians(i))
        cosStep = cos(radians(i))
        #print (cosStep, sinStep, i)
        currentx = x
        currenty = y
        while (edgeMat[int(currenty),int(currentx)] == 0):
            currentx += cosStep
            currenty += sinStep
            #print (currentx,currenty)
        pointEdgeArray.append((currentx + edgeBuf*cosStep ,currenty +edgeBuf*sinStep))
        edgeMat[int(currenty),int(currentx)] = int(255/2)
    return pointEdgeArray, edgeMat

# Read image
im = cv2.imread("testnoise2.jpg", cv2.IMREAD_GRAYSCALE)

bluredIm = cv2.GaussianBlur(im,(91,91),0)
#this needs to be 5 for the canny edge detector
bluredIm2 = cv2.GaussianBlur(im,(5,5),0)
#bluredIm = cv2.medianBlur(im,19)
#ret,bluredIm = cv2.threshold(bluredIm,240,255,cv2.THRESH_BINARY)
#bluredIm = cv2.adaptiveThreshold(bluredIm,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,\
#        cv2.THRESH_BINARY,11,2)
# Set up the detector with default parameters.
params = cv2.SimpleBlobDetector_Params()
params.filterByArea = True
params.minArea = 1000
params.maxArea = 1500000
params.filterByCircularity = False
params.filterByInertia = False


detector = cv2.SimpleBlobDetector_create(params)

# Detect blobs.
#keypointsNoBlur = detector.detect(im)
keypointsBlur = detector.detect(bluredIm)
#print (bluredIm)
sigma = .33
v = np.median(bluredIm)
lower = int(max(0, (1.0 - sigma) * v))
upper = int(min(255, (1.0 + sigma) * v))
cannyEdges = cv2.Canny(bluredIm2, lower, upper, 5)



# Draw detected blobs as red circles.
# cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS ensures the size of the circle corresponds to the size of blob
#im_with_keypointsNoBlur = cv2.drawKeypoints(im, keypointsNoBlur, np.array([]), (0,0,255), cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
im_with_keypointsBlur = cv2.drawKeypoints(im, keypointsBlur, np.array([]), (0,0,255), cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
print type(im_with_keypointsBlur)
#print len(keypointsNoBlur)
#print len(keypointsBlur)
for point in keypointsBlur:
    pointxy = point.pt
    pointx = pointxy[0]
    pointy = pointxy[1]
    im_with_keypointsBlur[pointy,pointx]=[0, 255, 0]
    pointEdgeArray, cannyEdges = radialSearch(pointx, pointy, cannyEdges)
    #print pointEdgeArray
    for edgePoint in pointEdgeArray:
        plusminus = 2
        edgePointx = edgePoint[0]
        edgePointy = edgePoint[1]
        for i in xrange(- plusminus, plusminus):
            for j in xrange(-plusminus,plusminus):
                im_with_keypointsBlur[edgePointy+j,edgePointx+i] = [255,0,0]
    drawLinesBetweenPoints(im_with_keypointsBlur, pointEdgeArray, [255,0,0])
#    print (im_with_keypointsBlur[pointy,pointx])
#    print (point.pt)


# Show keypoints
#cv2.imshow("KeypointsNoBlur", im_with_keypointsNoBlur)
cv2.imshow("KeypointsBlue", im_with_keypointsBlur)
cv2.imshow("Canny Edges", cannyEdges)
cv2.waitKey(0)
