


def looper():
    i = 0
    while True:
        i += 1
        print(i)
        yield i


def task():
    it = looper()
    # Does nothing until next(it) is called
    next(it)



task()