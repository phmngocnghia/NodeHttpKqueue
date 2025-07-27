const { Kqueue } = require("./build/Release/kqueue");
const { spawn } = require("child_process");

const kq = new Kqueue();

// 1. Spawn "cat" with pipes
const child = spawn("cat", [], { stdio: ["pipe", "pipe", "pipe"] });

const readFD = child.stdout._handle.fd;
console.log("readFD =", readFD);

// 2. Register stdout FD to kqueue
kq.addReadFD(readFD);

// 3. Send some data to cat
child.stdin.write("hello\n");
child.stdin.end(); // End input so "cat" exits after echoing

// 4. Wait for the output to be ready
console.log("Waiting for kqueue event...");
const event = kq.wait();
console.log("Event triggered:", event);

// 5. Read the actual data
child.stdout.on("data", (data) => {
  console.log("Child output:", data.toString());
});

// 6. Exit cleanly
child.on("exit", (code) => {
  console.log("Child exited with code", code);
  process.exit(0);
});
