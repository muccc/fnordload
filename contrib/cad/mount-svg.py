import svgwrite

#attrs = [style.linewidth(0.001), style.linecap.square]
#attrs_helper = attrs + [color.rgb.red]


#dwg.add(dwg.line((0, 0), (10, 0), stroke=svgwrite.rgb(10, 10, 16, '%')))
#dwg.add(dwg.text('Test', insert=(0, 0.2), fill='red'))


# #--->x
# |
# |
# y
def draw_mounting_hole(drawing, x, y):
    #c.stroke(path.circle(x, y, .3/2.), attrs)
    #c.stroke(path.line(x - .5, y - .5, x + .5, y + .5), attrs)
    #c.stroke(path.line(x + .5, y - .5, x - .5, y + .5), attrs)
    #drawing.add(drawing.circle((x,y), r = 3, stroke=svgwrite.rgb(10, 10, 16, '%')))
    drawing.add(drawing.circle((str(x)+'mm',str(y)+'mm'), r = "1.5mm", stroke="black", stroke_width="0.1mm", fill="white"))
    dwg.add(dwg.line((str(x-5)+"mm", str(y-5)+"mm"), (str(x+5)+"mm", str(y+5)+"mm"), stroke="black", stroke_width="0.1mm"))
    dwg.add(dwg.line((str(x+5)+"mm", str(y-5)+"mm"), (str(x-5)+"mm", str(y+5)+"mm"), stroke="black", stroke_width="0.1mm"))

def draw_mounting_hole2(drawing, x, y):
    draw_mounting_hole(drawing, 200 - x, 150 - y)

    
dwg = svgwrite.Drawing('mount.svg', profile='tiny')
holes = ((10,10), (10,140), (29.5, 50), (55, 104.5), (87.2, 63), (87.05, 129.2), (143.85, 62.35), (152.65, 140.55), (190, 140), (180, 20))

for hole in holes:
    draw_mounting_hole2(dwg, hole[0], hole[1])

dwg.add(dwg.line(("0mm", "0mm"), ("200mm", "0mm"), stroke="black", stroke_width="0.1mm"))
dwg.add(dwg.line(("200mm", "0mm"), ("200mm", "150mm"), stroke="black", stroke_width="0.1mm"))
dwg.add(dwg.line(("200mm", "150mm"), ("0mm", "150mm"), stroke="black", stroke_width="0.1mm"))
dwg.add(dwg.line(("0mm", "150mm"), ("0mm", "0mm"), stroke="black", stroke_width="0.1mm"))

dwg.save()

