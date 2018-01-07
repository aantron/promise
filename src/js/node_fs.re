type fd = int;

type buffer;

[@bs.val]
external node_buffer_alloc: int => buffer = "Buffer.alloc";

[@bs.send]
external node_buffer_to_string:
  (buffer, string, int, int) => string = "toString";

[@bs.module "fs"]
external access: (~path: string, ~mode: int, unit => unit) => unit = "access";

[@bs.module "fs"]
external appendFile: (~file: string, ~data: string, ~options: string, unit => unit) => unit = "appendFile";

[@bs.module "fs"]
external chmod: (~path: string, ~mode: int, unit => unit) => unit = "chmod";

[@bs.module "fs"]
external chown: (~path: string, ~uid: int, ~gid: int, unit => unit) => unit = "chown";

[@bs.module "fs"]
external close: (~fd: fd, unit => unit) => unit = "close";

[@bs.module "fs"]
external copyFile: (~src: string, ~dest: string, ~flags: int, unit => unit) => unit = "copyFile";

[@bs.module "fs"]
external exists: (~path: string, bool => unit) => unit = "exists";

[@bs.module "fs"]
external fchmod: (~fd: fd, ~mode: int, unit => unit) => unit = "fchmod";

[@bs.module "fs"]
external fchown: (~fd: fd, ~uid: int, ~gid: int, unit => unit) => unit = "fchown";

[@bs.module "fs"]
external fdatasync: (~fd: fd, unit => unit) => unit = "fdatasync";

/*
  Needs to type second arg of callback to a Js.t in node.js - fs.Stats
  https://nodejs.org/dist/latest-v9.x/docs/api/fs.html#fs_class_fs_stats
*/
[@bs.module "fs"]
external fstat: (~fd: fd, (unit, 'a) => unit) => unit = "fstat";

[@bs.module "fs"]
external fsync: (~fd: fd, unit => unit) => unit = "fsync";

[@bs.module "fs"]
external ftruncate: (~fd: fd, ~len: int, unit => unit) => unit = "ftruncate";

[@bs.module "fs"]
external futimes: (~fd: fd, ~atime: int, ~mtime: int, unit => unit) => unit = "futimes";

[@bs.module "fs"]
external lchmod: (~path: string, ~mode: int, unit => unit) => unit = "lchmod";

[@bs.module "fs"]
external lchown: (~path: string, ~uid: int, ~gid: int, unit => unit) => unit = "lchown";

[@bs.module "fs"]
external link: (~existingPath: string, ~newPath: string, unit => unit) => unit = "link";

[@bs.module "fs"]
external lstat: (~path: string, (unit, 'a) => unit) => unit = "lstat";

[@bs.module "fs"]
external mkdir: (~path: string, ~mode: int, unit => unit) => unit = "mkdir";

[@bs.module "fs"]
external mkdtemp: (~prefix: string, ~options: string, (unit, string) => unit) => unit = "mkdir";

[@bs.module "fs"]
external open_: (~path: string, ~flags: string, ~mode: int, (unit, int) => unit) => unit = "open";

[@bs.module "fs"]
external read_:
  (~fd: fd, ~buffer: buffer, ~offset: int, ~length: int, ~position: int, (unit, int, buffer) => unit) => unit = "read";

[@bs.module "fs"]
external readdir: (~path: string, ~options: string, (unit, array(string)) => unit) => unit = "readdir";

[@bs.module "fs"]
external readFile: (~path: string, ~options: string, (unit, string) => unit) => unit = "readFile";

[@bs.module "fs"]
external readLink: (~path: string, ~options: string, (unit, string) => unit) => unit = "readLink";

[@bs.module "fs"]
external realpath: (~path: string, ~options: string, (unit, string) => unit) => unit = "realpath";

[@bs.module "fs"]
external rename: (~oldPath: string, ~newPath: string, unit => unit) => unit = "rename";

[@bs.module "fs"]
external rmdir: (~path: string, (unit) => unit) => unit = "rmdir";

[@bs.module "fs"]
external stat: (~path: string, (unit, 'a) => unit) => unit = "stat";

[@bs.module "fs"]
external symlink: (~target: string, ~path: string, ~typ: string, unit => unit) => unit = "symlink";

[@bs.module "fs"]
external truncate: (~path: string, ~len: int, unit => unit) => unit = "truncate";

[@bs.module "fs"]
external unlink: (~path: string, unit => unit) => unit = "unlink";

[@bs.module "fs"]
external utimes: (~path: string, ~atime: float, ~mtime: float, unit => unit) => unit = "utimes";

[@bs.module "fs"]
external write: (~fd: fd, ~str: string, ~position: int, ~encoding: string, (unit, int, string) => unit) => unit = "write";

[@bs.module "fs"]
external writeFile: (~file: string, ~data: string, ~options: string, unit => unit) => unit = "writeFile";

/* let open_ = (path, callback) =>
  open_(path, "r", 438, (_error, result) => callback(result)); */

/* let read = (~fd, ~length, ~position, callback) => {
  let buffer = node_buffer_alloc(length);
  read_(fd, buffer, 0, length, position, (_error, result, _buffer) => {
    let s = node_buffer_to_string(buffer, "utf8", 0, result);
    callback(s);
  });
}; */
