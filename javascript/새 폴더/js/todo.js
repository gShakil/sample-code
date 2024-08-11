const todoFrom =document.getElementById("todo-form");
const todoInput = todoFrom.querySelector("input"); // = document.querySelector("#todo-form input");
const todoList = document.getElementById("todo-list");

function paintToDo(newTodo){

}

function handleToDoSubmit(event){
    event.preventDefault();
    const newTodo = todoInput.value;
    todoInput.value ="";
    paintToDo(newTodo);
}

todoFrom.addEventListener("submit", handleToDoSubmit)