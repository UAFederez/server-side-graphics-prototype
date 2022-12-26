const express = require("express");
const app = express();
const http = require("http");

const server = http.createServer(app);

const { Server } = require("socket.io");
const io = new Server(server);

io.on("connection", (socket) => {
    console.log("Client connected");
    socket.on("requestFrame", (info) => {
        console.log(info);
    });
});

app.get("/", (request, response) => {
    response.sendFile(__dirname + "/index.html");
});

server.listen(3000, () => {
    console.log("Listening on port 3000");
});
