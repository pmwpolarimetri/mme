import numpy as np
import matplotlib.pyplot as plt

im: np.ndarray = np.load("image.npy")

print(im.size)
print(type(im))
print(im.dtype)

print(im)
print(im.ravel(order="K"))
print(im.flags)
fig, ax = plt.subplots(1,1)
ax.imshow(im)
plt.show()