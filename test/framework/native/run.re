/* This file is part of Repromise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/repromise/blob/master/LICENSE.md. */



let rec main_loop = () => {
  if (!Repromise.ReadyCallbacks.callbacksPending()) {
    ()
  }
  else {
    let callbackSnapshot = Repromise.ReadyCallbacks.snapshot();
    Repromise.ReadyCallbacks.call(callbackSnapshot);
    main_loop();
  }
};
