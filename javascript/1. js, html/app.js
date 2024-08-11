const h1 = document.querySelector("div.hello:first-child h1");
//Change text color
console.dir(h1);
h1.style.color = "blue" // text color -> blue

//Event
//console.dir(h1); // h1 -> on~~~ : event list
function handleTitleClick(){
    const currentColor = h1.style.color;
    let newColor;
    if(currentColor === "blue")
        newColor = "tomato";
    else
        newColor = "blue";
    h1.style.color = newColor;

}
h1.addEventListener("click", handleTitleClick); // same as [h1.onclick = handleTitleClick;]
