const path = require("path");
const child = require("child_process");
const fs = require("fs");

const sushi = path.join(__dirname, "..", "build", "sushi");
const cppHarnessFilepath = path.join(
  __dirname,
  "clang_harness",
  "clang_harness.cpp"
);

async function sushiCompile({ filepath, cwd }) {
  const absFilepath = path.resolve(filepath);

  return new Promise((resolve, reject) => {
    child.exec(
      `${sushi} ${absFilepath}`,
      {
        cwd: cwd,
      },
      (err, stdout, stderr) => {
        if (err) {
          console.log(stdout, err, stderr);
          reject(new Error("Sushi rejected.\n" + stdout));
          return;
        }

        console.log(stdout);
        resolve(stdout);
      }
    );
  });
}

async function clangCompile({ objectFiles, cwd }) {
  const cmd = `clang++ ${cppHarnessFilepath} ${objectFiles.join(" ")} -o test`;

  return new Promise((resolve, reject) => {
    child.exec(
      cmd,
      {
        cwd: cwd,
      },
      (err, stdout, stderr) => {
        if (err) {
          console.log(err, stderr);
          reject(new Error(`Failed to compile '${cmd}'`));
          return;
        }

        resolve(stdout);
      }
    );
  });
}

async function run({ binary, cwd }) {
  return new Promise((resolve, reject) => {
    child.exec(
      `./${binary}`,
      {
        cwd: cwd,
      },
      (err, stdout, stderr) => {
        if (err) {
          reject();
          return;
        }

        resolve(stdout);
      }
    );
  });
}

async function compileAndRun({ filepath, cwd }) {
  const delFolder = createTestFolder({ cwd: cwd });
  try {
    await sushiCompile({ filepath: filepath, cwd: cwd });

    await clangCompile({ objectFiles: ["output.o"], cwd: cwd });

    const result = await run({ binary: "test", cwd: cwd });

    return result;
  } catch (e) {
    throw e;
  } finally {
    delFolder();
  }
}

function createTestFolder({ cwd }) {
  if (!fs.existsSync(cwd)) {
    fs.mkdirSync(cwd);
  }

  return () => {
    fs.rmSync(cwd, { recursive: true });
  };
}

module.exports = {
  compileAndRun,
};
