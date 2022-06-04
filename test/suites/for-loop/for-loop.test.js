const { compileAndRun } = require("../../compile-and-run");

const path = require("path");

const cwd = __dirname;

describe("For Loop", () => {
  test("For loop decrement", async () => {
    const filename = "incrementing.for-loop.sushi.test";
    const testFile = path.join(__dirname, "incrementing.for-loop.sushi");

    const result = await compileAndRun({
      filepath: testFile,
      cwd: path.join(cwd, filename),
    });

    expect(result).toBe("15");
  });
});
