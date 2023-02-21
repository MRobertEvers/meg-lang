# Enum Unions

I want to support enum unions/tagged unions, etc. The syntax from rust is kind of clunky, can it be better?


```rust
enum Coin {
    Penny,
    Nickel { x: i32, y: i32 },
    Dime(i32),
    Quarter(i32)
}

fn value_in_cents(coin: Coin) -> u8 {
    match coin {
        Coin::Penny => 1,
        Coin::Nickel => 5,
        Coin::Dime => 10,
        Coin::Quarter(state) => {
            println!("State quarter from {:?}!", state);
            25
        }
    }
}

let coin: Coin;

if let Coin::Quarter(max) = coin {
    println!("The maximum is configured to be {}", max);
}
```

The match syntax is pretty good. The `if` syntax is bad. It's backwards, it's a forced yoda statement. I get why they did it, you're declaring a var in the let statement. Also looks like you're calling a function.

```
if (coin is Coin::Quarter(max)) {

}

if (coin is Coin::Quarter; let [max] = coin) {

}

if (coin is Coin::Quarter) => (quarter: Coin::Quarter) {
    
}

if (is_true && coin is Coin::Quarter && is_green) => (quarter: Coin::Quarter) {

}

if (is_true) {

}
```

Talked to Andy and we settled on this, call it arrow if.

```
if (coin is Coin::Quarter) => (quarter: Coin::Quarter) {
    
}

switch (coin) {
    case Coin::Quarter => handle_quarter;
    case Coin::Penny {

    }
}
```