open Ast_mapper;
open Parsetree;

let mapper = {
  ...default_mapper,
  expr: (mapper, e) =>
    switch e.pexp_desc {
      | [@implicit_arity] Pexp_extension({Asttypes.txt: "await"},
          PStr([{pstr_desc:
            Pstr_eval({pexp_desc:
              Pexp_let(Asttypes.Nonrecursive, [binding], e'), _}, _)}])) =>

        let e'' = mapper.expr(mapper, binding.pvb_expr);
        let e' = mapper.expr(mapper, e');

        let open! Ast_helper.Exp;

        let andThen = ident(Location.mknoloc(Longident.parse("Repromise.andThen")));
        let x = binding.pvb_pat;
        let fun_ = fun_("", None, x, e');

        apply(andThen, [("", fun_), ("", e'')])

      | _ =>
        default_mapper.expr(mapper, e)
    }
};

let () =
  run_main(_argv => mapper);
