import cv2
import time
import numpy as np
import numpy as np
import pyrealsense2 as rs
from detect_gesture import detect_gesture
from detect_distance import detect_distance
from detect_home import detect_home
from detect_ball import detect_ball

# TODO: REPLACE WITH YOUR PATH
fname =  'C:/Users/florr/OneDrive/Desktop/share_virtualbox/input.txt'
open(fname, 'w').close()
f = open(fname, 'w', buffering=1)

# default
gesture = 6
turn = 1
distance_z = -1
distance_x = 320
person = False
filtered_z = None
filtered_x = None
f_x = [0,0,0,0,0]
f_z = [0,0,0,0,0]
mode = 0
gesture_count = 0
person_count = 0
flag = True

pipeline = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)
config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)
pipeline.start(config)

while True:
    frames = pipeline.wait_for_frames()
    depth = frames.get_depth_frame()
    color_frame = frames.get_color_frame()
    if depth and color_frame:
        frame = np.asanyarray(color_frame.get_data())
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        person, f_x, f_z, filtered_z, filtered_x = detect_distance(depth, frame_rgb, f_x, f_z, filtered_z, filtered_x)
        gesture, mode, gesture_count = detect_gesture(frame_rgb, gesture, mode, gesture_count)
        
        if person:
            person_count = 0
            if mode == 8:
                mode = 0
            flag = True
            if mode == 0: # 'tracking'
                distance_z = filtered_z
                distance_x = filtered_x # 0~640
                turn = 1 if distance_x-320 > 80 else (-1 if distance_x-320 < -80 else 0)
            elif mode == 1: # 'stop'
                turn = 0
                distance_z = 1
                distance_x = 0
            elif mode == 2: # 'happy'
                turn = 0
                distance_z = 1
                distance_x = 0
            elif mode == 3: # 'turn_around'
                turn = 0
                distance_z = 1
                distance_x = 0
            elif mode == 4: # 'playground'
                turn, distance_x = detect_ball(frame_rgb)
                distance_z = filtered_z
            else: # unknown
                pass

        else: # no one is in the frame
            if mode == 4:
                pass
            else:
                person_count += 1
                if person_count >= 3:
                    # turn = 1 # keeps turning right
                    # distance_z = -1
                    if distance_x-320 > 280:
                        distance_x = 320+250
                    elif distance_x-320 < -280:
                        distance_x = 320-250

                    if person_count >= 3 and flag:
                        start = time.time()
                        flag = False
                    now = time.time()
                    if now >= start + 5:
                        turn = 1 # keeps turning right
                        distance_z = -1
                        distance_x = 320
        
        file_str = f'{mode} {turn} {distance_z} {distance_x-320}'
        f.truncate(0)
        f.seek(0)
        f.write(f'{mode} {turn} {distance_z} {distance_x-320} ')
        f.flush()
        print(mode, gesture, turn, distance_z, distance_x-320)
        key = cv2.waitKey(1) & 0xFF
        if key == ord("q"):
            break

f.close()
cv2.destroyAllWindows()
