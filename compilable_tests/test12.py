# Fibonacci
def Fibonacci(n):
    if n == 1:
        return 0

    if n == 2:
        return 1

    if n > 2:
        first_n = n - 1
        second_n = n - 2
        a = Fibonacci(first_n)
        b = Fibonacci(second_n)
        return a + b


res = Fibonacci(9)
print(res)
