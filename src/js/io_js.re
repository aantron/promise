let run = ignore;

type fd = Node_fs.fd;

[@bs.val]
external node_buffer_alloc: int => Node_fs.buffer = "Buffer.alloc";

[@bs.send]
external node_buffer_to_string:
  (Node_fs.buffer, string, int, int) => string = "toString";

let open_ = filename =>
  Promise.new_((~resolve) =>
    Node_fs.open_(~path=filename, ~flags="r", ~mode=438, (_error, result) => resolve(result)));


let read = (~fd, ~length) => {
  let buffer = node_buffer_alloc(length);
  Promise.new_((~resolve) =>
    Node_fs.read_(~fd, ~buffer, ~offset=0, ~length, ~position=0, (_error, result, _buffer) => {
      let s = node_buffer_to_string(buffer, "utf8", 0, result);
      resolve(s);
    })
  )
};