const clock = document.querySelector("h2#clock");

function getClock(){
    const date = new Date();
    const hours   = String(date.getHours()).padStart(2, "0"); // padStart 2: 최소 두 글자. "0": 앞쪽에 0으로 채움. <-> padEnd
    const minutes = String(date.getMinutes()).padStart(2,"0");
    const seconds = String(date.getSeconds()).padStart(2,"0");

    clock.innerText = `${hours}:${minutes}:${seconds}`;
    //clock.innerText = `${date.getHours()}:${date.getMinutes()}:${date.getSeconds()}`;
    
}

getClock();
setInterval(getClock, 1000);
//setTimeout(sayHello2,5000); // 한 번만 호출