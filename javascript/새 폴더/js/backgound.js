const images = [
    "bg1.jpg",
    "bg2.jpg",
    "bg3.jpg",
];

const chosenImage = images[Math.floor(Math.random() * images.length)];
//console.log(chosenImage);
const bgImage = document.createElement("img");
//console.log(bgImage);
bgImage.src = `background/${chosenImage}`;
//console.log(bgImage);
document.body.appendChild(bgImage);