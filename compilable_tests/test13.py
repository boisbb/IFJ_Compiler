def squaresum(n):
    i = 1
    finish = n + 1
    sm = 0
    while i < finish:
        sm = sm + (i * i)
        i = i + 1

    return sm

res = squaresum(10)
print(res)
