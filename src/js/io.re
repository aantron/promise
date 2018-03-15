let run = ignore;

type fd = Fs.fd;

[@bs.val]
external node_buffer_alloc: int => Fs.buffer = "Buffer.alloc";

[@bs.send]
external node_buffer_to_string:
  (Fs.buffer, string, int, int) => string = "toString";

let open_ = filename => {
  let (p, resolve) = Repromise.new_();
  Fs.open_(~path=filename, ~flags="r", ~mode=438, (_error, result) =>
    resolve(result));
  p;
};


let read = (~fd, ~length) => {
  let buffer = node_buffer_alloc(length);
  let (p, resolve) = Repromise.new_();
  Fs.read_(~fd, ~buffer, ~offset=0, ~length, ~position=0,
      (_error, result, _buffer) => {
    let s = node_buffer_to_string(buffer, "utf8", 0, result);
    resolve(s);
  });
  p;
};
