def print_line(width):
  line = '+'
  x = 0
  while x < width:
    q = x // 2
    p = q * 2
    modulo = x - p
    line = line + '-'
    x = x + 1
  line = line + '+'
  print(line)

print_line(30)
