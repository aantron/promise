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
