#!/usr/local/bin/python
# -*- coding: utf-8 -*-
################################################################################
# show_result.py
# Description:  show yolo result
#
# Author:       shoz
# Date:         Sun Nov 08 23:13:22 2020
# Last revised: Sun Nov 08 23:13:22 2020
# Application:  Python 3.7.4
################################################################################

#<IMPORT>
import os,sys
import argparse
import re
from PIL import Image, ImageDraw

#<CLASS>########################################################################
# Description:  
# Dependencies: 
################################################################################

#<SUBROUTINE>###################################################################
# Function:     load box table
# Description:  
# Dependencies: 
################################################################################
def load_boxes(fname):
  current = None
  classes = dict()
  with open(fname, "r") as f:
    for line in f:
      m = re.match(r"([\w ]+):", line)
      if m:
        current = m.group(1)
        classes[current] = []
      else:
        box = tuple(map(float, line.split()))
        classes[current].append(box)
  return classes

#<SUBROUTINE>###################################################################
# Function:     draw box on image
# Description:  
# Dependencies: 
################################################################################
def draw_boxes(im, boxes, scale, transposed=False, color='#FFFFFF', width=3):
  w, h = scale
  draw = ImageDraw.Draw(im)
  
  for x1,y1,x2,y2 in boxes:
    if transposed:
        draw.rectangle((int(w*y1), int(h*x1), int(w*y2), int(h*x2)), outline=color, width=width)
    else:
        draw.rectangle((int(w*x1), int(h*y1), int(w*x2), int(h*y2)), outline=color, width=width)

#<SUBROUTINE>###################################################################
# Function:     draw box on image
# Description:  
# Dependencies: 
################################################################################
def parse_item(str, color='#FFFFFF'):
  m = re.match(r"([\w ]+)(#[0-9A-Fa-f]{6})?", str)
  if m:
    return m.groups(color)
  else:
    return None

#<MAIN>#########################################################################
# Function:     CLI Main
# Description:  
# Dependencies: 
################################################################################
if __name__ == '__main__':
  parser = argparse.ArgumentParser(description="Gathering changed files")
  parser.add_argument('img',
                      help="image file")
  parser.add_argument('tbl',
                      help="boxes table file")
  parser.add_argument('output', nargs='?',
                      help="output image file")
  parser.add_argument('-i', '--items', nargs='+',
                      help="select item classes")
  parser.add_argument('-q', '--quiet', action='store_true',
                      help="don't show image")
  parser.add_argument('-l', '--list', action='store_true',
                      help="print item classes")
  parser.add_argument('-c', '--color', default='#FFFFFF',
                      help="default color")
  parser.add_argument('-b', '--border', type=int, default=3,
                      help="border width of box")
  parser.add_argument('-t', '--transposed', action='store_true',
                      help="x,y axis transposed")
  parser.add_argument('-n', '--normalized', action='store_true',
                      help="normalized scale")
  
  args = parser.parse_args()
  
  classes = load_boxes(args.tbl)

  if args.list:
    print(list(classes))
    exit()

  if args.items:
    items = [parse_item(item, args.color) for item in args.items]
  else:
    items = [(item, args.color) for item in list(classes)]

  im = Image.open(args.img)
  if args.normalized:
      scale = im.size
  else:
      scale = (1.0, 1.0)

  for item,color in items:
    draw_boxes(im, classes[item], scale, args.transposed, color, args.border)
  
  if not args.quiet:
    im.show()
  
  if args.output:
    im.save(args.output)

# show_nsm.py
