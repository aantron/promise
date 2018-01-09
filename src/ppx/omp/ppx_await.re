module Ast = Ast_406;

module Ast_helper = Ast.Ast_helper;
module Ast_mapper = Ast.Ast_mapper;
module Asttypes = Ast.Asttypes;
module Location = Ast.Location;
module Longident = Ast.Longident;
module Parsetree = Ast.Parsetree;

open Ast_mapper;
open Parsetree;

let mapper = {
  ...default_mapper,
  expr: (mapper, e) =>
    switch e.pexp_desc {
      | [@implicit_arity] Pexp_extension({Asttypes.txt: "await", _},
          PStr([{pstr_desc:
            Pstr_eval({pexp_desc:
              Pexp_let(Asttypes.Nonrecursive, [binding], e'), _}, _), _}])) =>

        let e'' = mapper.expr(mapper, binding.pvb_expr);
        let e' = mapper.expr(mapper, e');

        let open Ast_helper.Exp;

        let then_ = ident(Location.mknoloc(Longident.parse("Promise.then_")));
        let x = binding.pvb_pat;
        let fun_ = fun_(Asttypes.Nolabel, None, x, e');

        apply(then_, [(Asttypes.Nolabel, fun_), (Asttypes.Nolabel, e'')])

      | _ =>
        default_mapper.expr(mapper, e)
    }
};

let () = {
  let open Migrate_parsetree;
  Driver.register(
    ~name = "ppx_await", Versions.ocaml_406, (_config, _cookies) => mapper);
};
