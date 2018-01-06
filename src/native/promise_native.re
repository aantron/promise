type promise('a) =
  ref([
    | `Resolved('a)
    | `Pending(list('a => unit))
  ]);

type t('a) = promise('a);

let ready_callbacks: ref(list(unit => unit)) = ref([]);

let new_ = executor => {
  let promise = ref(`Pending([]));
  let resolve = (value) =>
    switch promise^ {
      | `Resolved(_) => ()
      | `Pending callbacks => {
        let call_with_value = f => (() => f(value));
        let callbacks = List.map(call_with_value, callbacks);
        ready_callbacks := ready_callbacks^ @ callbacks;
        promise := `Resolved(value);
      }
    };
  executor(~resolve);
  promise
};

let resolve = value => ref(`Resolved(value));

let then_ = (callback, promise) =>
  new_((~resolve) => {
    let run_callback = (value) => {
      let nested_promise = callback(value);
      switch nested_promise^ {
        | `Resolved(value) =>
          resolve(value);
        | `Pending(callbacks) =>
          nested_promise := `Pending(callbacks @ [resolve])
      }
    };

    switch promise^ {
      | `Resolved(value) =>
        ready_callbacks := ready_callbacks^ @ [() => run_callback(value)]
      | `Pending(callbacks) =>
        promise := `Pending(callbacks @ [run_callback])
    }
  })
