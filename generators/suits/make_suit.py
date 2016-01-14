from PIL import Image


WHITE = (255, 255, 255)
WHITE_T = (255, 255, 255, 255)

im = Image.open("clubs.png")

pixelAccessor = im.load()
width, height = im.size

# assume width <= 16 pixels

print "{{"
for y in range(height):
	byte = "0b"
	for x in range(width):
		pixel = pixelAccessor[x, y]
		if pixel == WHITE or pixel == WHITE_T:
			byte += "0"
		else:
			byte += "1"
	print byte + ","
print "}}"
