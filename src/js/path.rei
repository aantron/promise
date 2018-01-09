type t = {.
  "dir": string,
  "root": string,
  "base": string,
  "name": string,
  "ext": string
};

let basename: (~path: string, ~ext: string) => string;

let delimiter: string;

let dirname: string => string;

let extname: string => string;

let format: t => string;

let isAbsolute: string => bool;

let join: array(string) => string;

let normalize: string => string;

let parse: string => t;

let relative: (~from: string, ~to_: string) => string;

let resolve: array(string) => string;

let sep: string;
