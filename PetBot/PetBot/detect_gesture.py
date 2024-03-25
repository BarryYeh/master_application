
from cvzone.HandTrackingModule import HandDetector
import cv2

detector = HandDetector(staticMode=False, maxHands=2, modelComplexity=1, detectionCon=0.5, minTrackCon=0.5)
def gesture_name(fingers1, fingers2=None):
    if fingers2 != None:
        if (fingers1 == [1,0,0,0,0] or fingers1 == [1,0,0,0,1]) or (fingers2 == [1,0,0,0,0] or fingers2 == [1,0,0,0,1]):
            gesture = 2 # 'happy'
        elif fingers1 == [1,1,1,1,1] and fingers2 == [1,1,1,1,1]:
            gesture = 4 # 'playground_start'
        elif fingers1 == [0,1,0,0,0] and fingers2 == [0,1,0,0,0]:
            gesture = 1
        else:
            gesture = 6 # 'unknown'
    else:
        if fingers1 == [0,1,0,0,0] or fingers1 == [0,1,1,0,0] or fingers1 == [1,1,0,0,0]:
            gesture = 1 # 'stop'
        elif fingers1 == [0,0,0,0,0]:
            gesture = 5 # 'playground_stop'
        elif fingers1 == [1,1,1,1,1] or fingers1 == [0,1,1,1,1]:
            gesture = 0 # 'tracking'
        elif fingers1 == [0,0,1,1,1] or fingers1 == [1,0,1,1,1]:
            gesture = 3 # 'turn_around'
        else:
            gesture = 6 # 'unknown'
    return gesture

def detect_gesture(img, gesture_pre, mode, count, img_show=False):
    # img = cv2.flip(img, 1)
    hands, img = detector.findHands(img, draw=True, flipType=True)
    if img_show:
        cv2.imshow("Gesture Detection", img)
    # Check if any hands are detected
    if hands:
        hand1 = hands[0]  # hand1["lmList", "bbox", "center", "type"]
        # Count the number of fingers up for the first hand
        fingers1 = detector.fingersUp(hand1)
        if len(hands) > 1:
            hand2 = hands[1]  # hand2["lmList", "bbox", "center", "type"]
            fingers2 = detector.fingersUp(hand2)
            gesture = gesture_name(fingers1, fingers2)
            # print(gesture, fingers1, fingers2)
        else:
            gesture = gesture_name(fingers1)
            # print(gesture, fingers1)
    else:
        # print(6)
        gesture = 6 # 'unknown'

    # keeps running the previous mode when gesture is unknown
    if gesture == gesture_pre and gesture != 6: 
        count += 1
    else:
        count = 0
    if count >= 3:
        # when petbot is in playground mode and get the playground_stop gesutre
        if mode == 4 and gesture == 5: 
            mode = 1
        elif mode != 4 and gesture < 5:
            mode = gesture # 0~4
    # print('in detect_gesture:', gesture, mode)
    return gesture, mode, count