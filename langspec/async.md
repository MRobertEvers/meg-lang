# Async Syntax
await can only block on a single promise at a time.

If you have several concurrent promises, you can't use it to react to the soonest of them, without an additional primitive like Promise.race. (Promise.race doesn't need to a primitive with the simple but annoying to use .then chains)

Go basically took this approach, with blocking on channels (possibly in a select) as the "idiomatic" way to handle blocking and concurrency, though this still struggles just a bit because the select primitive is limited to a small, fixed number of channels.

So `await` needs to be something like `await future, future`