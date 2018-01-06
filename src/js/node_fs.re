type fd = int;

type buffer;

[@bs.val]
external node_buffer_alloc: int => buffer = "Buffer.alloc";

[@bs.send]
external node_buffer_to_string:
  (buffer, string, int, int) => string = "toString";

[@bs.module "fs"]
external access: (string, int, unit => unit) => unit = "access";

/* 
  This takes a optional field that I ommitted because we need to type it to a Js.t
  https://nodejs.org/dist/latest-v9.x/docs/api/fs.html#fs_fs_appendfile_file_data_options_callback
*/
[@bs.module "fs"]
external appendFile: (string, string, unit => unit) => unit = "appendFile";

[@bs.module "fs"]
external chmod: (string, int, unit => unit) => unit = "chmod";

[@bs.module "fs"]
external chown: (string, int, int, unit => unit) => unit = "chown";

[@bs.module "fs"]
external close: (int, (unit) => unit) => unit = "close";

[@bs.module "fs"]
external copyFile: (string, string, int, (unit) => unit) => unit = "copyFile";

[@bs.module "fs"]
external exists: (string, (bool) => unit) => unit = "exists";

[@bs.module "fs"]
external fchmod: (int, int, (unit) => unit) => unit = "fchmod";

[@bs.module "fs"]
external fchown: (int, int, int, (unit) => unit) => unit = "fchown";

[@bs.module "fs"]
external fdatasync: (int, (unit) => unit) => unit = "fdatasync";

/*
  Needs to type second arg of callback to a Js.t in node.js - fs.Stats
  https://nodejs.org/dist/latest-v9.x/docs/api/fs.html#fs_class_fs_stats
*/
[@bs.module "fs"]
external fstat: (int, (unit, 'a) => unit) => unit = "fstat";

[@bs.module "fs"]
external fsync: (int, (unit) => unit) => unit = "fsync";

[@bs.module "fs"]
external ftruncate: (fd, int, (unit) => unit) => unit = "ftruncate";

[@bs.module "fs"]
external futimes: (int, int, int, (unit) => unit) => unit = "futimes";

[@bs.module "fs"]
external lchmod: (string, int, (unit) => unit) => unit = "lchmod";

[@bs.module "fs"]
external lchown: (string, int, int, (unit) => unit) => unit = "lchown";

[@bs.module "fs"]
external link: (string, string, (unit) => unit) => unit = "link";

[@bs.module "fs"]
external lstat: (string, (unit, 'a) => unit) => unit = "lstat";

[@bs.module "fs"]
external mkdir: (string, int, (unit) => unit) => unit = "mkdir";
/*
 Ommitted an optional argument that required typing a Js.t
 https://nodejs.org/dist/latest-v9.x/docs/api/fs.html#fs_fs_mkdtemp_prefix_options_callback
*/
[@bs.module "fs"]
external mkdtemp: (string, (unit, string) => unit) => unit = "mkdir";

[@bs.module "fs"]
external open_: (string, string, (unit, int) => unit) => unit = "open";

[@bs.module "fs"]
external read_:
  (int, buffer, int, int, int, (unit, int, buffer) => unit) => unit = "read";

[@bs.module "fs"]
external readdir: (string, (unit, array(string)) => unit) => unit = "readdir";

[@bs.module "fs"]
external readFile: (string, string, (unit, string) => unit) => unit = "readFile";

[@bs.module "fs"]
external readLink: (string, string, (unit, string) => unit) => unit = "readLink";

[@bs.module "fs"]
external realpath: (string, string, (unit, string) => unit) => unit = "realpath";

[@bs.module "fs"]
external rename: (string, string, (unit) => unit) => unit = "rename";

[@bs.module "fs"]
external rmdir: (string, (unit) => unit) => unit = "rmdir";

[@bs.module "fs"]
external stat: (string, string, (unit, 'a) => unit) => unit = "stat";

[@bs.module "fs"]
external symlink: (string, string, string, (unit) => unit) => unit = "symlink";

[@bs.module "fs"]
external truncate: (string, int, (unit) => unit) => unit = "truncate";

[@bs.module "fs"]
external unlink: (string, (unit) => unit) => unit = "unlink";

[@bs.module "fs"]
external utimes: (string, float, float, (unit) => unit) => unit = "utimes";

[@bs.module "fs"]
external write: (fd, string, int, int, int, (unit, int, string) => unit) => unit = "write";

[@bs.module "fs"]
external writeFile: (string, string, string, (unit) => unit) => unit = "writeFile";

[@bs.module "fs"]
external open_: (string, string, (unit, int) => unit) => unit = "open";

let open_ = (path, callback) =>
  open_(path, "r", (_error, result) => callback(result));

let read = (~fd, ~length, ~position, callback) => {
  let buffer = node_buffer_alloc(length);
  read_(fd, buffer, 0, length, position, (_error, result, _buffer) => {
    let s = node_buffer_to_string(buffer, "utf8", 0, result);
    callback(s);
  });
};
