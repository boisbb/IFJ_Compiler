def print_line(width):
  line = 'IFJ_TEST--->'
  x = 0
  while x < width:
    q = x // 2
    p = q * 2
    modulo = x - p
    if modulo == 1:
        line = line + 'S'
    else:
        line = line + 'T'
    x = x + 1
  line = line + '<---IFJ_TEST'
  print('\n', line, '\n')



print_line(30)
