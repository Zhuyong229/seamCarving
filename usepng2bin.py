import png2bin
from PIL import Image

if __name__ == '__main__':
    im2 = Image.open("HJoceanSmall.png") # read a png image using PIL
    im2.show() # display the image
    png2bin.write_image(im2, "HJoceanSmall.bin") # write the image to a bin file

    
    im = png2bin.read_image("HJoceanSmall.bin") # read a bin image using png2bin
    im.show() # display the image

    

