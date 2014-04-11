from pyx import *
text.set(mode="latex") 
text.preamble(r"\renewcommand{\familydefault}{\sfdefault}")

attrs = [style.linewidth(0.001), style.linecap.square]
attrs_helper = attrs + [color.rgb.red]
# y
# ^
# |
# |
# #--->x
def draw_mounting_hole(canvas, x, y):
    c.stroke(path.circle(x, y, .3/2.), attrs)
    c.stroke(path.line(x - .5, y - .5, x + .5, y + .5), attrs)
    c.stroke(path.line(x + .5, y - .5, x - .5, y + .5), attrs)

def draw_mounting_hole2(canvas, x, y):
    draw_mounting_hole(canvas, 20 - x/10., y/10.)

    
c = canvas.canvas()
holes = ((10,10), (10,140), (29.5, 50), (55, 104.5), (87.2, 63), (87.05, 129.2), (143.85, 62.35), (152.65, 140.55), (190, 140), (190, 10))

for hole in holes:
    draw_mounting_hole2(c, hole[0], hole[1])

c.writePDFfile("mount")
c.writeEPSfile("mount")

