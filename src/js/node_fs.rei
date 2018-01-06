type fd;

let open_: (string, int => unit) => unit;
let read: (~fd: int, ~length: int, ~position: int, string => unit) => unit;
