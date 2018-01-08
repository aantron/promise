let abort: unit => unit;

let argv: array('a);

let argv0: string;

let chdir: string => unit;

type usage = {.
  "user": int,
  "system": int
};

let cpuUsage: usage => usage;

let cwd: unit => string;

let exit: int => unit;

let getegid: unit => int;

let geteuid: unit => int;

let getgid: unit => int;

let getgroups: unit => int;

let getuid: unit => int;

let hrtime: array(int) => array(int);

let initgroups: string => string;

let kill: (int, string) => unit;

type memory = {.
  "rss": int,
  "heapTotal": int,
  "heapUsed": int,
  "external": int
};

let memoryUsage: unit => memory;

let pid: int;

let platform: string;

let ppid: int;

let setegid: int => unit;

let seteuid: int => unit;

let setgid: int => unit;

let setgroups: array(int) => unit;

let setuid: int => unit;

let title: string;

let version: string;