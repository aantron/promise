type promise('a) = Js.Promise.t('a);
type t('a) = promise('a);

let new_ = executor =>
  Js.Promise.make((~resolve, ~reject) => {
    let resolve = (value) => [@bs] resolve(value);
    ignore(reject);
    executor(~resolve);
  });

let resolve = Js.Promise.resolve;

let then_ = Js.Promise.then_;

let ready_callbacks: ref(list(unit => unit)) = ref([]);
