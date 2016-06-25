import cv2
import numpy as np
from math import cos, sin, radians, sqrt
# this can be improved
# doing the vector form of lines but i dont want to rederive that
# so fix this sometime in the future
def drawLinesBetweenPoints(image, pointlist, colorArray):
    """
    Takes an image and a pointlist and a color array and draws
    lines between all of those points
    """
    for i in xrange(len(pointlist)):
        point1 = (int(pointlist[i][0]),int(pointlist[i][1]))
        point2 = (int(pointlist[(i+1)%len(pointlist)][0]),int(pointlist[(i+1)%len(pointlist)][1]))
        #print (point1, point2)
        cv2.line(image, point1, point2, colorArray, 1)

def radialSearch(x, y, edgeMat):
    """
    takes an x,y to start at and then looks for when a canny
    edgeMat value goes above 0 (note it should be blured before this to avoid
    the search from going out of bounds and 0 is an arbitrary choice)
    it returns a edgePointArray an array like [(x1,y1),(x2,y2),...,(xn,yn)]
    """
    edgemaxx = len(edgeMat[0])
    edgemaxy = len(edgeMat)
    pointEdgeArray = []
    numberOfSections = 36
    degreeStep = 360 / numberOfSections
    edgeBuf = 10
    if edgeMat[y, x] != 0:
        return pointEdgeArray, edgeMat
    for i in xrange(0,360 , degreeStep):
        sinStep = sin(radians(i))
        cosStep = cos(radians(i))
        #print (cosStep, sinStep, i)
        currentx = x
        currenty = y
        #print "before"
        #print edgeMat[int(round(currenty)),int(round(currentx))]
        while (int(edgeMat[int(round(currenty)),int(round(currentx))]) == 0):
            #print (edgeMat[int(round(currenty)),int(round(currentx))], currentx, currenty)
            currentx += cosStep
            currenty += sinStep
            if round(currentx)>=edgemaxx or round(currenty)>= edgemaxy or round(currentx) < 0 or round(currenty) < 0:
                break
            #pointEdgeArray.append((currentx ,currenty))
            #print (currentx,currenty)
        pointEdgeArray.append((currentx + edgeBuf*cosStep ,currenty +edgeBuf*sinStep))
        #edgeMat[int(currenty),int(currentx)] = int(255/2)
    #print type(pointEdgeArray)
    #print type(edgeMat)
    return pointEdgeArray, edgeMat

def findMaxDist(pointlist, image):
    """
    takes a pointlist and returns the closest and furthest points away
    and draws lines between them on the image

    Note: Mins were taken out because it was not well thought out
    this should become its own function later.
    """
    maxDist = 0
    maxPoints = []
#    minDist = sqrt(len(image)*len(image) + len(image[0])*len(image[0]))
#    print len(image)
#    print len(image[0])
#    print minDist
#    minPoints =[]
    for i in xrange(0, len(pointlist)):
        point1 = pointlist[i]
        for j in xrange(i,len(pointlist)):
            point2 = pointlist[j]
            distanceBetween = sqrt(pow(point1[0]-point2[0],2)+pow(point1[1]-point2[1],2))
            if distanceBetween>maxDist:
                maxDist = distanceBetween
                maxPoints = (point1,point2)
#            elif distanceBetween<minDist:
#                minPoints = (point1,point2)
#                minDist = distanceBetween
    drawLinesBetweenPoints(image,maxPoints,[0,100,100])
#   drawLinesBetweenPoints(image,minPoints,[0,200,200])
