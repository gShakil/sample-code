const loginForm = document.querySelector("#login-form");
const loginInput = document.querySelector("#login-form input");

function onLoginSubmit(event){
    event.preventDefault();
    console.log(event);
    console.log(event);
    //const username = loginInput.value;
    //console.log(username);
}

loginForm.addEventListener("submit", onLoginSubmit)
/*
// HTML에서 유효성 검사를 실시하므로 아래 검사 코드는 필요없음
//Version2 - find input, button
const loginInput = document.querySelector("#login-form input");
const loginButton = document.querySelector("#login-form button");

function onLoginBtnClick(){
    const username = loginInput.value;
    
    if(username === ""){
        alert("Please write your name");
    }else if(username.length > 15){
        alert("Your name is too long.");
    } else{
        
    }
    
        
    console.log(username);
}

loginButton.addEventListener("click", onLoginBtnClick);
*/

/*
//Version 1 - find input, button
//const loginForm = document.getElementsById("login-form"); // same as
const loginForm = document.querySelector("#login-form"); // # : id 
const loginInput = loginForm.querySelector("input");
const loginButton = loginForm.querySelector("button");
*/