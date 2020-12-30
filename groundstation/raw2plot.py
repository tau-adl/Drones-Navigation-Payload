import os
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image


MY_PATH = r'D:\\'
CURR_FILE = '35572.bmp'
HEIGHT_PADDING = 32
WIDTH = 320
HEIGHT = 240  # 32 lines added as patting

with open(os.path.join(MY_PATH, CURR_FILE)) as f:
    bin_img = np.fromfile(f, dtype='uint8', count=-1, sep='', offset=0)
    bin_img.shape = (HEIGHT+HEIGHT_PADDING, WIDTH)
    bin_img = bin_img[0:HEIGHT, 0:WIDTH]
    im = Image.fromarray(bin_img)
    im.save(MY_PATH + '\\' + CURR_FILE + '.bmp')

    fig = plt.figure()
    plt.title('%s' % CURR_FILE)
    plt.imshow(bin_img, cmap='gray')

    fig.savefig(MY_PATH + '\\' + CURR_FILE + '.jpg')
    plt.show()







