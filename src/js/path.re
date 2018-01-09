type t = {.
  "dir": string,
  "root": string,
  "base": string,
  "name": string,
  "ext": string
};

[@bs.module "path"]
external basename: (~path: string, ~ext: string) => string = "basename";

[@bs.module "path"]
external delimiter: string = "delimiter";

[@bs.module "path"]
external dirname: string => string = "dirname";

[@bs.module "path"]
external extname: string => string = "extname";

[@bs.module "path"]
external format: t => string = "format";

[@bs.module "path"]
external isAbsolute: string => bool = "isAbsolute";

[@bs.module "path"]
external join: array(string) => string = "join";

[@bs.module "path"]
external normalize: string => string = "normalize";

[@bs.module "path"]
external parse: string => t = "parse";

[@bs.module "path"]
external relative: (~from: string, ~to_: string) => string = "relative";

[@bs.module "path"]
external resolve: array(string) => string = "resolve";

[@bs.module "path"]
external sep: string = "sep";
