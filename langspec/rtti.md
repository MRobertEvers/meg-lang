# RTTI

I want to facilitate message passing. Similar to tagged enums but with an open set of types.

```
struct IMessage : @RTTI(i32) {

}

struct AlertMessage : IMessage {

}

fn on_message(msg: IMessage*) {
    if (msg is AlertMessage) => (quarter: AlertMessage) {
        
    }
}
```