type t = {.
  "dir": string,
  "root": string,
  "base": string,
  "name": string,
  "ext": string
};

[@bs.module "fs"]
external basename: (~path: string, ~ext: string) => string = "basename";

[@bs.module "fs"]
external delimeter: string = "delimeter";

[@bs.module "fs"]
external dirname: string => string = "dirname";

[@bs.module "fs"]
external extname: string => string = "extname";

[@bs.module "fs"]
external format: t => string = "format";

[@bs.module "fs"]
external isAbsolute: string => bool = "isAbsolute";

[@bs.module "fs"]
external join: array(string) => string = "join";

[@bs.module "fs"]
external normalize: string => string = "normalize";

[@bs.module "fs"]
external parse: string => t = "parse";

[@bs.module "fs"]
external relative: (~from: string, ~to_: string) => string = "relative";