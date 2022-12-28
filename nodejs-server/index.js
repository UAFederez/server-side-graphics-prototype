const express = require("express");
const app = express();
const net = require("net");
const http = require("http");

const server = http.createServer(app);

const { Server } = require("socket.io");
const io = new Server(server);

function collectArraysAsOneBuffer(arrays) {
    const byteBuffers = arrays.map((array) => new Uint8Array(array.buffer));
    const totalSize = byteBuffers.reduce(
        (accumulated, buffer) => accumulated + buffer.length,
        0
    );
    const mergedBuffers = new Uint8Array(totalSize);
    byteBuffers.reduce((accumulated, buffer) => {
        mergedBuffers.set(buffer, accumulated);
        return accumulated + buffer.length;
    }, 0);
    return mergedBuffers;
}

const RADIUS = 2.0;

io.on("connection", (socket) => {
    console.log("Client connected");
    socket.on("requestFrame", (info) => {
        const renderSocket = net.connect(
            { host: "127.0.0.1", port: 3434 },
            () => {
                console.log("Connected to server");
                const imageDimensions = Uint32Array.from([
                    info.width,
                    info.height,
                ]);
                console.log(info);
                const lightPosition = Float32Array.from([
                    Math.sin(info.idx) * RADIUS,
                    2.0,
                    Math.cos(info.idx) * RADIUS,
                ]);
                const spherePosition = Float32Array.from([0, 0.0, 2.0]);

                const message = collectArraysAsOneBuffer([
                    imageDimensions,
                    lightPosition,
                    spherePosition,
                ]);
                renderSocket.write(message);
            }
        );

        renderSocket.on("data", (data) => {
            console.log("Received response");
            socket.emit("responseFrame", {
                error: null,
                data: data.toString(),
            });
        });

        renderSocket.on("error", (error) => {
            socket.emit("responseFrame", {
                error: `No connection: ${error}`,
                data: [],
            });
        });
    });
});

app.get("/", (request, response) => {
    response.sendFile(__dirname + "/index.html");
});

server.listen(3000, () => {
    console.log("Listening on port 3000");
});
