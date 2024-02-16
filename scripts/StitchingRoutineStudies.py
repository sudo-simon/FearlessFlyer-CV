import cv2
import numpy as np
import math

# Read input images
img1 = cv2.imread('/home/lor3n/Dev/RTMP/scripts/1.png')
img2 = cv2.imread('/home/lor3n/Dev/RTMP/scripts/2.png')

def warp_perspective_no_cut(src_image, transformation_matrix):
    # Get the height and width of the source image
    height, width = src_image.shape[:2]

    # Define the four corners of the source image
    src_corners = np.array([[0, 0], [width, 0], [width, height], [0, height]], dtype=np.float32).reshape(-1, 1, 2)

    # Use the warpPerspective function to transform the corners
    dst_corners = cv2.perspectiveTransform(src_corners, transformation_matrix)

    # Find the bounding box that contains all the transformed corners
    min_x, min_y = np.min(dst_corners, axis=0).ravel()
    max_x, max_y = np.max(dst_corners, axis=0).ravel()

    # Calculate the size of the destination image
    dst_width, dst_height = int(max_x - min_x), int(max_y - min_y)

    # Adjust the transformation matrix to shift the image to the positive quadrant
    shift_matrix = np.array([[1, 0, -min_x], [0, 1, -min_y], [0, 0, 1]])
    adjusted_matrix = np.dot(shift_matrix, transformation_matrix)

    # Use warpPerspective with the adjusted transformation matrix and new destination size
    dst_image = cv2.warpPerspective(src_image, adjusted_matrix, (dst_width, dst_height))

    return dst_image


# Detect key points and extract descriptors
detector = cv2.ORB_create()
keypoints1, descriptor1 = detector.detectAndCompute(img1, None)
keypoints2, descriptor2 = detector.detectAndCompute(img2, None)

# Match descriptors using a FLANN based matcher
matcher = cv2.BFMatcher(cv2.NORM_HAMMING)
matches = matcher.match(descriptor1, descriptor2)

# Sort matches based on distance
matches = sorted(matches, key=lambda x: x.distance)

# Keep only the top matches (you may need to adjust this threshold)
numGoodMatches = int(len(matches) * 0.15)
matches = matches[:numGoodMatches]

# Homography matrix
points1 = np.float32([keypoints1[m.queryIdx].pt for m in matches]).reshape(-1, 1, 2)
points2 = np.float32([keypoints2[m.trainIdx].pt for m in matches]).reshape(-1, 1, 2)

H, _ = cv2.findHomography(points1, points2, cv2.RANSAC)

imgWarped = warp_perspective_no_cut(img2, H)


dx = H[0,2]*-1
dy = H[1, 2]*-1

# Ablend two images: 
# 1. take the first image and add borders following the translation components
# 2. inizia a copiare l'immagine a (sign(x_vec)*ceil(abs(x-vec)), sign(y_vec)*ceil(abs(y_vec)))
bottom = 0
left = 0
right = 0
top = 0

if(dy>0):
    top = math.ceil(dy)
    bottom = 0
else:
    top = 0
    bottom = math.ceil(abs(dy))

if(dx>0):
    left=0
    right=math.ceil(dx)
else:
    left=math.ceil(abs(dx))
    right=0

image_with_border = cv2.copyMakeBorder(img1, top, bottom, left, right, cv2.BORDER_CONSTANT, value=(0,0,0))

x_offset, y_offset = int(np.sign(dx)*math.ceil(abs(dx))),int(np.sign(dy)*math.ceil(abs(dy)))  # Adjust these values based on where you want to place the smaller image

#calcolo dello scarto

y_error = imgWarped.shape[0]-img1.shape[0]
x_error = imgWarped.shape[1]-img1.shape[1]

#blending
for i in range(y_offset, imgWarped.shape[0]+y_offset-y_error):
    for j in range(x_offset, imgWarped.shape[1]+x_offset-x_error):

        if np.all(imgWarped[i-y_offset][j-x_offset] == [0, 0, 0]):
            continue
        image_with_border[i][j] = imgWarped[i-y_offset][j-x_offset]



cv2.imwrite("res.png", image_with_border)
