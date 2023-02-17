



type Color = {
    type: "black"
    value: "FFF"
} | {
    type: "green"
    value: "GGG"
    height: 3,
    age: 3
}


let color: Color = {type: 'black', value: 'FFF'};

if (color.type === 'black') {
    color.value
}
