


def inner_looper():
    i = 0
    inc = 1
    while True:
        i += inc
        print('before')
        inc = yield i
        print('after')
        if i > 8:
            return

def looper():
    i = 0

    yield from inner_looper()
    
    return


def task():
    it = looper()
    # Does nothing until next(it) is called
    
    it.send(1)



task()