def multiply(a,b):
    print("Will compute", a, "times", b)
    c = 0
    for i in range(0, a):
        c = c + b
    return c

import os

def mmm(str):
    print(str)
    f = open(str, 'w')
    f.write("fuxk")
    f.close()