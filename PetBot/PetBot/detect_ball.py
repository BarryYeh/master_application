from collections import deque
from imutils.video import VideoStream
import numpy as np
import cv2
import imutils
import time

# define the lower and upper boundaries of the color
colorLower = (30, 30, 0)
colorUpper = (50, 180, 200)
buffer = 64

pts = deque(maxlen=buffer)

def detect_ball(frame, img_show=False):
	blurred = cv2.GaussianBlur(frame, (11, 11), 0)
	hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
	# construct a mask for the color
	mask = cv2.inRange(hsv, colorLower, colorUpper)
	mask = cv2.erode(mask, None, iterations=2)
	mask = cv2.dilate(mask, None, iterations=2)
	# find contours in the mask and initialize the current
	cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
	cnts = imutils.grab_contours(cnts)
	center = None
	
	# only proceed if at least one contour was found
	if len(cnts) > 0:
		# compute the minimum enclosing circle and centroid
		c = max(cnts, key=cv2.contourArea)
		((x, y), radius) = cv2.minEnclosingCircle(c)
		M = cv2.moments(c)
		center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
		# only proceed if the radius meets a minimum size
		if radius > 10:
			# draw the circle and centroid on the frame
			cv2.circle(frame, (int(x), int(y)), int(radius),
				(0, 255, 255), 2)
			cv2.circle(frame, center, 5, (0, 0, 255), -1)
			# update the points queue
			pts.appendleft(center)

	# loop over the set of tracked points
	for i in range(1, len(pts)):
		if pts[i - 1] is None or pts[i] is None:
			continue
		thickness = int(np.sqrt(buffer / float(i + 1)) * 2.5)
		cv2.line(frame, pts[i - 1], pts[i], (0, 0, 255), thickness)
	# show the frame to our screen
	if img_show:
		cv2.imshow("Frame", frame)
	if pts:
		turn = 1 if pts[0][0]-320 > 50 else(-1 if pts[0][0]-320 < -50 else 0)
		if pts[0][0]-320 > 280:
			return turn, 320+190
		elif pts[0][0]-320 < -280:
			return turn, 320-190
		return turn, pts[0][0]
	
	else:
		return 0, 0
