let () = {
  ignore ({
    let%await fd = Io.open_("test/demo/demo.re");
    let%await data = Io.read(~fd, ~length = 1024);
    print_endline(data);
    Promise.resolve(());
  });

  Io.run();
};
