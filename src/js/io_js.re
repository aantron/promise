let run = ignore;

type fd = int;

let open_ = filename =>
  Promise.new_((~resolve) =>
    Node_fs.open_(filename, resolve));

let read = (~fd, ~length) =>
  Promise.new_((~resolve) =>
    Node_fs.read(~fd, ~length, ~position = 0, resolve));
