type fd;

type buffer;

let access: (~path: string, ~mode: int, unit => unit) => unit;

let appendFile: (~file: string, ~data: string, ~options: string, unit => unit) => unit;

let chmod: (~path: string, ~mode: int, unit => unit) => unit;

let chown: (~path: string, ~uid: int, ~gid: int, unit => unit) => unit;

let close: (~fd: fd, unit => unit) => unit;

let copyFile: (~src: string, ~dest: string, ~flags: int, unit => unit) => unit;

let exists: (~path: string, bool => unit) => unit;

let fchmod: (~fd: fd, ~mode: int, unit => unit) => unit;

let fchown: (~fd: fd, ~uid: int, ~gid: int, unit => unit) => unit;

let fdatasync: (~fd: fd, unit => unit) => unit;

let fstat: (~fd: fd, (unit, 'a) => unit) => unit;

let fsync: (~fd: fd, unit => unit) => unit;

let ftruncate: (~fd: fd, ~len: int, unit => unit) => unit;

let futimes: (~fd: fd, ~atime: int, ~mtime: int, unit => unit) => unit;

let lchmod: (~path: string, ~mode: int, unit => unit) => unit;

let lchown: (~path: string, ~uid: int, ~gid: int, unit => unit) => unit;

let link: (~existingPath: string, ~newPath: string, unit => unit) => unit;

let lstat: (~path: string, (unit, 'a) => unit) => unit;

let mkdir: (~path: string, ~mode: int, unit => unit) => unit;

let mkdtemp: (~prefix: string, ~options: string, (unit, string) => unit) => unit;

let open_: (~path: string, ~flags: string, ~mode: int, (unit, int) => unit) => unit;

let read_: (~fd: fd, ~buffer: buffer, ~offset: int, ~length: int, ~position: int, (unit, int, buffer) => unit) => unit;

let readdir: (~path: string, ~options: string, (unit, array(string)) => unit) => unit;

let readFile: (~path: string, ~options: string, (unit, string) => unit) => unit;

let readLink: (~path: string, ~options: string, (unit, string) => unit) => unit;

let realpath: (~path: string, ~options: string, (unit, string) => unit) => unit;

let rename: (~oldPath: string, ~newPath: string, unit => unit) => unit;

let rmdir: (~path: string, (unit) => unit) => unit;

let stat: (~path: string, (unit, 'a) => unit) => unit;

let symlink: (~target: string, ~path: string, ~typ: string, unit => unit) => unit;

let truncate: (~path: string, ~len: int, unit => unit) => unit;

let unlink: (~path: string, unit => unit) => unit;

let utimes: (~path: string, ~atime: float, ~mtime: float, unit => unit) => unit;

let write: (~fd: fd, ~str: string, ~position: int, ~encoding: string, (unit, int, string) => unit) => unit;

let writeFile: (~file: string, ~data: string, ~options: string, unit => unit) => unit;