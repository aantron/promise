[@bs.module "os"]
external eol: string = "EOL";

[@bs.module "os"]
external arch: string = "arch";

type cpuCore = {.
  "model": string, 
  "speed": int,
  "times": {.
    "user": int,
    "nice": int,
    "sys": int,
    "idle": int,
    "irq": int
  }
};

[@bs.module "os"]
external cpus: unit => array(cpuCore) = "cpus";

[@bs.module "os"]
external endianness: unit => string = "endianness";

[@bs.module "os"]
external freemem: unit => int = "freemem";

[@bs.module "os"]
external homedir: unit => string = "homedir";

[@bs.module "os"]
external hostname: unit => string = "hostname";

[@bs.module "os"]
external loadavg: unit => array(int) = "loadavg";

[@bs.module "os"]
external platform: unit => string = "platform";

[@bs.module "os"]
external release: unit => string = "release";

[@bs.module "os"]
external tmpdir: unit => string = "tmpdir";

[@bs.module "os"]
external totalmem: unit => string = "totalmem";

[@bs.module "os"]
external typ: unit => string = "type";

[@bs.module "os"]
external uptime: unit => int = "uptime";