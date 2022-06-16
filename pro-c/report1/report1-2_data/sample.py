import os

f = open("./data/groupA")

lines = f.readlines()

line_index = 1
for line in lines:
  print("{}行目: ".format(line_index), line, end='')
  line_index += 1
  
f.close()