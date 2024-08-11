const h1 = document.querySelector("div.hello:first-child h1");

//Event
function handleTitleClick(){
    //Version3
    h1.classList.toggle("clicked");
    /*
    //Version2
    // className: class를 교체, classList: 리스트 관리
    const clickedClass = "clicked";
    if(h1.classList.contains(clickedClass))
        h1.classList.remove(clickedClass);
    else
        h1.classList.add(clickedClass);
    */
    /*
    //Version 1
    const clickedClass = "clicked";
    if(h1.className === clickedClass){
        h1.className = "";
    }else{
        h1.className = clickedClass; // css에서 생성된 active 클래스 부여. 색상 변경됌
    }
    */
}
h1.addEventListener("click", handleTitleClick); // same as [h1.onclick = handleTitleClick;]
