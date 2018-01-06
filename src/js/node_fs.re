type fd = int;

[@bs.module "fs"]
external node_fs_open_: (string, string, (unit, int) => unit) => unit = "open";

let open_ = (path, callback) =>
  node_fs_open_(path, "r", (_error, result) => callback(result));

type buffer;

[@bs.val]
external node_buffer_alloc: int => buffer = "Buffer.alloc";

[@bs.send]
external node_buffer_to_string:
  (buffer, string, int, int) => string = "toString";

[@bs.module "fs"]
external node_fs_read:
  (int, buffer, int, int, int, (unit, int, buffer) => unit) => unit = "read";

let read = (~fd, ~length, ~position, callback) => {
  let buffer = node_buffer_alloc(length);
  node_fs_read(fd, buffer, 0, length, position, (_error, result, _buffer) => {
    let s = node_buffer_to_string(buffer, "utf8", 0, result);
    callback(s);
  });
};
