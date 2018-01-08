let eol: string;

let arch: string;

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

let cpus: unit => array(cpuCore);

let endianness: unit => string;

let freemem: unit => int;

let homedir: unit => string;

let hostname: unit => string;

let loadavg: unit => array(int);

let platform: unit => string;

let release: unit => string;

let tmpdir: unit => string;

let totalmem: unit => string;

let typ: unit => string;

let uptime: unit => int;
