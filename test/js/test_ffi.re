let test = Framework.test;

let suite = Framework.suite("node", [
  test("path.delimiter", () => {
    switch Node_path.delimiter {
    | ":" | ";" => Repromise.resolve(true)
    | _ => Repromise.resolve(false)
    };
  }),
]);
