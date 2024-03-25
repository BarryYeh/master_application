import cv2
import mediapipe as mp
import numpy as np

# Initialize Mediapipe Pose
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(min_detection_confidence=0.5, min_tracking_confidence=0.5)
mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles

meaure_depth = 0
meaure_X = 0

def detect_distance(depth, frame_rgb, f_x, f_z, filtered_z, filtered_x, alpha=0.6, scale_x=639, scale_y=479, img_show=False):
    # Process the frame with the Pose model
    results = pose.process(frame_rgb)
    if results.pose_landmarks:
        person = True
        # Access and convert landmark X-coordinates (pixel)
        landmark_names = [mp_pose.PoseLandmark.NOSE,
                            mp_pose.PoseLandmark.LEFT_HIP,
                            mp_pose.PoseLandmark.RIGHT_HIP,
                            mp_pose.PoseLandmark.LEFT_SHOULDER,
                            mp_pose.PoseLandmark.RIGHT_SHOULDER]

        # frames = pipeline.wait_for_frames()
        # depth = frames.get_depth_frame()

        landmarks_x, landmarks_y, landmarks_z, landmarks_c = [], [], [], []

        for landmark_name in landmark_names:
            landmark = results.pose_landmarks.landmark[landmark_name]
            landmarks_x.append(min(max(int(scale_x * landmark.x), 0), scale_x))
            landmarks_y.append(min(max(int(scale_y * landmark.y), 0), scale_y))
            #landmarks_z.append(depth.get_distance(landmarks_x[-1], landmarks_y[-1]))
            landmarks_c.append(landmark.visibility)
        pixel_x = np.mean(landmarks_x)
        pixel_y = np.mean(landmarks_y) 
        landmarks_z = depth.get_distance(int(pixel_x), int(pixel_y))

        distance = landmarks_z
        X = pixel_x

        # Kalman filter prediction and update 
        filtered_z = distance if filtered_z is None else alpha * distance + (1 - alpha) * filtered_z
        filtered_x = X if filtered_x is None else 0.7 * X + (1 - 0.7) * filtered_x
        #print(f'z: {filtered_z},x: {filtered_x}')

        if img_show:
            mp_drawing.draw_landmarks(
            frame_rgb,
            results.pose_landmarks,
            mp_pose.POSE_CONNECTIONS,
            landmark_drawing_spec=mp_drawing_styles.get_default_pose_landmarks_style())
            cv2.imshow('Tracking', frame_rgb)
    else:
        person = False

    return person, f_x, f_z, filtered_z, filtered_x


