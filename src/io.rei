type never = Repromise.never;

let run: unit => unit;

type fd;

let open_: string => Repromise.t(fd, never);
let read: (~fd: fd, ~length: int) => Repromise.t(string, never);
