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


function handleMouseEnter(){
    h1.innerText = "Mouse is here!";
    //console.log("mouse is here !");
}
function handleMouseLeave(){
    h1.innerText = "Mouse is gone!";
}
function handleWindowResize(){
    document.body.style.backgroundColor = "tomato";
}
function handleWindowCopy(){
    alert("copier !");
}
function handleWindowOffline(){
    alert("SOS no WIFI");
}
function handleWindowOnline(){
    alert("WIFI good!");
}


h1.addEventListener("mouseenter", handleMouseEnter); //<-> removeEventListener
h1.addEventListener("mouseleave", handleMouseLeave);
window.addEventListener("resize", handleWindowResize);
window.addEventListener("copy", handleWindowCopy);
window.addEventListener("offline", handleWindowOffline);
window.addEventListener("online", handleWindowOnline);

