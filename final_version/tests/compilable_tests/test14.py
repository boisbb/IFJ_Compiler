def sum_of_series(n):
    q = n // 2
    p = q * 2
    modulo = n - p
    x = 0
    if modulo == 0:
        x = (n / 2) * (n + 1)
    else:
        x = ((n + 1) / 2) * n
    return (x * x)

res = sum_of_series(5)
if res == 225.0:
    res = 225
else:
    print('CHYBA')

print(res)
