let run: unit => unit;

type fd;

let open_: string => Promise.t(fd);
let read: (~fd: fd, ~length: int) => Promise.t(string);
