const fs = require("fs")

/*
const start = process.hrtime();
const rs = fs.createReadStream("./large_file_js.txt");

let count = 0;
rs.on("data", (buf) => {
  count += Buffer.byteLength(buf)

});

rs.on("end", () => {
  console.log("total count: ", count);
  const diff = process.hrtime(start);
  console.log("time consumes: ", Math.floor(diff[0] * 1e3 + diff[1] / 1e6));
});
*/

const BUF_SIZE = 4096 * 3;
function main() {
  const buf = Buffer.alloc(BUF_SIZE);
  try {
    const start = process.hrtime();
    const fd = fs.openSync("./large_file_js.txt", "r");
    let count = 0;
    for(;;) {
      try {
        const bytesRead = fs.readSync(fd, buf, 0, BUF_SIZE);
        if (bytesRead == 0) {
          console.log("read file ends");
          break;
        }
        count += bytesRead;
      } catch (err) {
        console.log(err);
        break;
      }
    }
    console.log("total count: ", count);
    const diff = process.hrtime(start);
    console.log("time consumes: ", Math.floor(diff[0] * 1e3 + diff[1] / 1e6));
  } catch (err) {
    console.log(err);
  }
}

main();
