const quotes = [
{
    quote: "blabla1",
    author: "author1",
},
{
    quote: "blabla2",
    author: "author2",
},
{
    quote: "blabla3",
    author: "author3",
},
{
    quote: "blabla4",
    author: "author4",
},
{
    quote: "blabla5",
    author: "author5",
},
{
    quote: "blabla6",
    author: "author6",
},
{
    quote: "blabla7",
    author: "author7",
},
{
    quote: "blabla8",
    author: "author8",
},
{
    quote: "blabla9",
    author: "author9",
},
{
    quote: "blabla10",
    author: "author10",
},
];

const quote = document.querySelector("#quote span:first-child");
const author = document.querySelector("#quote span:last-child");

const todayQuote = quotes[Math.floor(Math.random() * quotes.length)];
quote.innerText = todayQuote.quote;
author.innerText = todayQuote.author;