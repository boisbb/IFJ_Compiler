#inputi
def power(base, exponent):
    x = 1
    ret = base
    while x < exponent:
        print(ret)
        ret = ret * base
        x = x + 1
    return ret


fct = 'power'
if fct == 'power':
    b = 2
    e = 5
    res = power(b,e)

print('result:', res)
