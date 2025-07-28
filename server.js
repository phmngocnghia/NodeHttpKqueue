const { Kqueue } = require("./build/Release/kqueue");
const { spawn } = require("child_process");

// Create kqueue instance with callback
const kq = new Kqueue((fd) => {
  console.log("Kqueue event fired for fd:", fd);
});

// Spawn a child process with pipes (cat just echoes input)
const child = spawn("cat", [], {
  stdio: ["pipe", "pipe", "pipe"],
});

// Get the underlying file descriptor for stdout
const readFD = child.stdout._handle.fd;
console.log("Child stdout FD =", readFD);

// Register the FD with kqueue
kq.addReadFD(readFD);

// Send data to child after a short delay
setTimeout(() => {
  console.log("Sending data to child process...");
  child.stdin.write("hello from JS\n");
  child.stdin.end(); // Signal end of input
}, 1000);

// Listen to output from the child
child.stdout.on("data", (data) => {
  console.log("Child output:", data.toString());
});

child.on("exit", (code) => {
  console.log("Child exited with code", code);
  kq.close();
});
