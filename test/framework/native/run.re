/* This file is part of Promise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/promise/blob/master/LICENSE.md. */



let rec main_loop = () => {
  if (!Promise.ReadyCallbacks.callbacksPending()) {
    ()
  }
  else {
    let callbackSnapshot = Promise.ReadyCallbacks.snapshot();
    Promise.ReadyCallbacks.call(callbackSnapshot);
    main_loop();
  }
};
