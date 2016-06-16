# Standard imports
import cv2
import numpy as np;

 # Read image
im = cv2.imread("testnoise.jpg", cv2.IMREAD_GRAYSCALE)

bluredIm = cv2.GaussianBlur(im,(5,5),0)
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
keypointsNoBlur = detector.detect(im)
keypointsBlur = detector.detect(bluredIm)
    # Draw detected blobs as red circles.
    # cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS ensures the size of the circle corresponds to the size of blob
im_with_keypointsNoBlur = cv2.drawKeypoints(im, keypointsNoBlur, np.array([]), (0,0,255), cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
im_with_keypointsBlur = cv2.drawKeypoints(im, keypointsBlur, np.array([]), (0,0,255), cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)

     # Show keypoints
cv2.imshow("KeypointsNoBlur", im_with_keypointsNoBlur)
cv2.imshow("KeypointsBlue", im_with_keypointsBlur)
cv2.waitKey(0)
