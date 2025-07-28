const { Kqueue } = require("./build/Release/kqueue");
const fs = require("fs");

const kq = new Kqueue();

// 1. Create a named pipe (FIFO)
const fifoPath = "/tmp/myfifo";
try {
  fs.unlinkSync(fifoPath);
} catch {}
require("child_process").execSync(`mkfifo ${fifoPath}`);

// 2. Open FIFO in read-only non-blocking mode (we want FD)
const fd = fs.openSync(fifoPath, "r");
console.log("Watching FIFO:", fifoPath, "FD:", fd);

// 3. Register FD with kqueue
kq.addReadFD(fd);

// 4. Block on wait (simulate server waiting for input)
console.log("🔒 Blocking on kqueue...");
const event = kq.wait();
console.log("✅ Event triggered:", event);

// 5. Read data
const data = fs.readFileSync(fd, "utf-8");
console.log("📥 Read from FIFO:", data);
