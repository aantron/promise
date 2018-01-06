let () = {
  ignore ({
    let%await fd = Io.open_("test/test.re");
    let%await data = Io.read(~fd, ~length = 1024);
    print_endline(data);
    Promise.resolve(());
  });

  Io.run();
};
