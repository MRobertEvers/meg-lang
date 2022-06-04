const { compileAndRun } = require("../../compile-and-run");

const path = require("path");

const cwd = __dirname;

describe("Assignments", () => {
  test("+=", async () => {
    const testdir = "plusequal.assignments.sushi.test";
    const testFile = path.join(__dirname, "plusequal.assignments.sushi");

    const result = await compileAndRun({
      filepath: testFile,
      cwd: path.join(cwd, testdir),
    });

    expect(result).toBe("5");
  });

  test("-=", async () => {
    const testdir = "minusequal.assignments.sushi.test";
    const testFile = path.join(__dirname, "minusequal.assignments.sushi");

    const result = await compileAndRun({
      filepath: testFile,
      cwd: path.join(cwd, testdir),
    });

    expect(result).toBe("5");
  });

  test("let a = N", async () => {
    const testdir = "assigninitialize.assignments.sushi.test";
    const testFile = path.join(__dirname, "assigninitialize.assignments.sushi");

    const result = await compileAndRun({
      filepath: testFile,
      cwd: path.join(cwd, testdir),
    });

    expect(result).toBe("10");
  });
});
