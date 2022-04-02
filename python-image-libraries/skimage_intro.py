"""
scikit-image is an image processing library.
"""

import matplotlib.pyplot as plt
from skimage import data, io, filters
from skimage.exposure import histogram
from skimage.feature import canny
from scipy import ndimage as ndi
from skimage import morphology
from skimage.filters import sobel
import numpy as np
from skimage import segmentation
from skimage.color import label2rgb
from skimage import color


# This loads a supplied image as a ndarray.
camera = data.camera()
print(type(camera))
print(camera.shape)

# Apply filters
edges = filters.sobel(camera)

# Display the image (matplotlib under the hood).
if False:
    io.imshow(camera)
    plt.figure()
    io.imshow(edges)

# Thresholding seperates pixels into two separate classes. Otsu's method maxmizes the variance between the two classes.
coins = data.coins()
threshold_value = filters.threshold_otsu(coins)
print(threshold_value)

# You can read any other image into a numpy array using scikit-image.

# The pixel data has a uniform type across the image (or band of an image).
print(coins.dtype) # uint8

"""
Image Segmentation Example.
"""

# We want to detect all the coins in the coins image.
# The histogram plots the number of pixels with each uint8_t value in the image.

hist, hist_centers = histogram(coins)

fig, axes = plt.subplots(1, 2, figsize=(8, 3))
axes[0].imshow(coins, cmap=plt.cm.gray)
axes[0].axis('off')
axes[1].plot(hist_centers, hist, lw=2)
axes[1].set_title('histogram of gray values')

# You can use thresholding. From observation, the coins a more lightly colored than the background.
# But it is rough and only captures parts of the coins, or it can capture part of the background if
# the threshold no set proerly.

fig, axes = plt.subplots(1, 2, figsize=(8, 3), sharey=True)

axes[0].imshow(coins > 100, cmap=plt.cm.gray)
axes[0].set_title('coins > 100')

axes[1].imshow(coins > 150, cmap=plt.cm.gray)
axes[1].set_title('coins > 150')

for a in axes:
    a.axis('off')

plt.tight_layout()

# For edge based detection we first highlight the edges in the image. Then these edges are filled in using
# mathemetical morphology. We set a minimum valid size to clear out some of the invalid small specs.
# One coin is lost in this process otherwize it worked well.

edges = canny(coins)

fig, ax = plt.subplots(figsize=(4, 3))
ax.imshow(edges, cmap=plt.cm.gray)
ax.set_title('Canny detector')
ax.axis('off')

fill_coins = ndi.binary_fill_holes(edges)

fig, ax = plt.subplots(figsize=(4, 3))
ax.imshow(fill_coins, cmap=plt.cm.gray)
ax.set_title('filling the holes')
ax.axis('off')

coins_cleaned = morphology.remove_small_objects(fill_coins, 21)

fig, ax = plt.subplots(figsize=(4, 3))
ax.imshow(coins_cleaned, cmap=plt.cm.gray)
ax.set_title('removing small objects')
ax.axis('off')

# For region based imaging, we first do edge detection using the sobel operatpr.
# Next we mark bright pixels with 2, dark pizels with 1, and the rest with 0.
# Then use te watershed transform to create the regions.

elevation_map = sobel(coins)

fig, ax = plt.subplots(figsize=(4, 3))
ax.imshow(elevation_map, cmap=plt.cm.gray)
ax.set_title('elevation map')
ax.axis('off')

markers = np.zeros_like(coins)
markers[coins < 30] = 1
markers[coins > 150] = 2

fig, ax = plt.subplots(figsize=(4, 3))
ax.imshow(markers, cmap=plt.cm.nipy_spectral)
ax.set_title('markers')
ax.axis('off')

segmentation_coins = segmentation.watershed(elevation_map, markers)

fig, ax = plt.subplots(figsize=(4, 3))
ax.imshow(segmentation_coins, cmap=plt.cm.gray)
ax.set_title('segmentation')
ax.axis('off')

# There are some refinements to this last method.

# This takes our last segmentation and gets rid of the hole in one of the coins.
segmentation_coins = ndi.binary_fill_holes(segmentation_coins - 1)

labeled_coins, _ = ndi.label(segmentation_coins)
image_label_overlay = label2rgb(labeled_coins, image=coins, bg_label=0)

fig, axes = plt.subplots(1, 2, figsize=(8, 3), sharey=True)
axes[0].imshow(coins, cmap=plt.cm.gray)
axes[0].contour(segmentation_coins, [0.5], linewidths=1.2, colors='y')
axes[1].imshow(image_label_overlay)

for a in axes:
    a.axis('off')

plt.tight_layout()

"""
An example of using SLIC segmentation
"""

# Input data
img = data.immunohistochemistry()

# Compute a mask
lum = color.rgb2gray(img)
mask = morphology.remove_small_holes(
    morphology.remove_small_objects(
        lum < 0.7, 500),
    500)

mask = morphology.opening(mask, morphology.disk(3))

# SLIC result
slic = segmentation.slic(img, n_segments=200, start_label=1)

# maskSLIC result
m_slic = segmentation.slic(img, n_segments=100, mask=mask, start_label=1)

# Display result
fig, ax_arr = plt.subplots(2, 2, sharex=True, sharey=True, figsize=(10, 10))
ax1, ax2, ax3, ax4 = ax_arr.ravel()

ax1.imshow(img)
ax1.set_title('Original image')

ax2.imshow(mask, cmap='gray')
ax2.set_title('Mask')

ax3.imshow(segmentation.mark_boundaries(img, slic))
ax3.contour(mask, colors='red', linewidths=1)
ax3.set_title('SLIC')

ax4.imshow(segmentation.mark_boundaries(img, m_slic))
ax4.contour(mask, colors='red', linewidths=1)
ax4.set_title('maskSLIC')

for ax in ax_arr.ravel():
    ax.set_axis_off()

plt.tight_layout()

io.show()
