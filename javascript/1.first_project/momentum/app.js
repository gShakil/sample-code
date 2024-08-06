const title = document.querySelector("div.hello:first-child h1");
//Change text color
console.dir(title);
title.style.color = "blue" // text color -> blue

//Event
function handleTitleClick(){
    console.log("title was clicked!");
}

title.addEventListener("click", handleTitleClick); 
