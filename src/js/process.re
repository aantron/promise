[@bs.module "process"]
external abort: unit => unit = "abort";

[@bs.module "process"]
external argv: array('a) = "argv";

[@bs.module "process"]
external argv0: string = "argv0";

[@bs.module "process"]
external chdir: string => unit = "chdir";

type usage = {.
  "user": int,
  "system": int
};

[@bs.module "process"]
external cpuUsage: usage => usage = "cpuUsage";

[@bs.module "process"]
external cwd: unit => string = "cwd";

[@bs.module "process"]
external exit: int => unit = "exit";

[@bs.module "process"]
external getegid: unit => int = "getegid";

[@bs.module "process"]
external geteuid: unit => int = "geteuid";

[@bs.module "process"]
external getgid: unit => int = "getgid";

[@bs.module "process"]
external getgroups: unit => int = "getgroups";

[@bs.module "process"]
external getuid: unit => int = "getuid";

[@bs.module "process"]
external hrtime: array(int) => array(int) = "hrtime";

[@bs.module "process"]
external initgroups: string => string = "initgroups";

[@bs.module "process"]
external kill: (int, string) => unit = "kill";

type memory = {.
  "rss": int,
  "heapTotal": int,
  "heapUsed": int,
  "external": int
};

[@bs.module "process"]
external memoryUsage: unit => memory = "memoryUsage";

[@bs.module "process"]
external pid: int = "pid";

[@bs.module "process"]
external platform: string = "platform";

[@bs.module "process"]
external ppid: int = "ppid";

[@bs.module "process"]
external setegid: int => unit = "setegid";

[@bs.module "process"]
external seteuid: int => unit = "seteuid";

[@bs.module "process"]
external setgid: int => unit = "setgid";

[@bs.module "process"]
external setgroups: array(int) => unit = "setgroups";

[@bs.module "process"]
external setuid: int => unit = "setuid";

[@bs.module "process"]
external title: string = "title";

[@bs.module "process"]
external version: string = "version";
