let test = Framework.test;

let suite = Framework.suite("node", [
  test("path.delimiter", () => {
    switch Node_path.delimiter {
    | ":" | ";" => Promise.resolve(true)
    | _ => Promise.resolve(false)
    };
  }),
]);
