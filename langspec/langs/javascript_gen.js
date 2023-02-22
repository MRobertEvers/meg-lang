function* looper() {
  let i = 0;
  while (true) {
    i += 1;
    console.log(i);
    yield i;
  }
}

let it = looper();
// Does nothing until next is called
it.next();
