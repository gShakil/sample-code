const loginForm = document.querySelector("#login-form");
const loginInput = document.querySelector("#login-form input");
const link = document.querySelector("a");
const greeting = document.querySelector("#greeting");

const HIDDEN_CLASSNAME = "hidden";
const USERNAME_KEY = "username";

function onLoginSubmit(event){
    event.preventDefault();
    console.log(event);
    loginForm.classList.add(HIDDEN_CLASSNAME); // hidden: css

    const username = loginInput.value;
    localStorage.setItem(USERNAME_KEY, username); // exist if refresh(F5)
    paintGreetings();
}
function handleLinkClick(event){
    event.preventDefault();
    console.log(event);
}
function paintGreetings(username){
    greeting.innerText = `Hello ${username}`; // same as "Hello " + username
    greeting.classList.remove(HIDDEN_CLASSNAME);
}


link.addEventListener("click", handleLinkClick);

const savedUsername = localStorage.getItem(USERNAME_KEY);
if(savedUsername === null){
    //show the form
    loginForm.classList.remove(HIDDEN_CLASSNAME);
    loginForm.addEventListener("submit", onLoginSubmit)
} else{
    // show greeting
    paintGreetings(savedUsername);
}
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