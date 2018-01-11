let run: unit => unit;

type fd;

let open_: string => Repromise.t(fd);
let read: (~fd: fd, ~length: int) => Repromise.t(string);
