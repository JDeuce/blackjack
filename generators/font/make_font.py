from PIL import Image

chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.,:"


WHITE = (255, 255, 255)
WHITE_T = (255, 255, 255, 255)

im = Image.open("font.png")

pixelAccessor = im.load()
width, height = im.size

# assume width <= 16 pixels
GLYPH_WIDTH = 14

for i,char in enumerate(chars):
	print "{{","//",char
	for y in range(height):
		byte = "0b"
		for glyphx in range(GLYPH_WIDTH):
			x = ((GLYPH_WIDTH+0)*i) + glyphx
			pixel = pixelAccessor[x, y]
			if pixel == WHITE or pixel == WHITE_T:
				byte += "0"
			else:
				byte += "1"
		print byte + ","
	print "}},"
