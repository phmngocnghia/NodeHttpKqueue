const fs = require("fs");

const fifoPath = "/tmp/myfifo";

// Give the server time to block first (1s delay)
setTimeout(() => {
  console.log("💥 Sending data to FIFO...");
  fs.writeFileSync(fifoPath, "Hello from trigger.js\n");
}, 1000);
