set -x



install_opam () {
    sudo add-apt-repository -y ppa:avsm/ocaml42+opam12
    sudo apt-get update -qq
    sudo apt-get install opam
}



build_with_npm () {
    npm install bs-platform
    npm install
    npm run build
    (cd test && npm install && npm run build && npm run tests)
}

build_with_esy () {
    npm install -g esy

    rm -f esy.lock.json
    esy install
    esy dune exec test/test_main.exe
}

build_with_opam () {
    install_opam
    opam init -ya --compiler=4.06.1
    eval `opam config env`

    opam pin add -y --no-action repromise .
    opam install -y --deps-only repromise

    dune exec test/test_main.exe
}



case $BUILD_SYSTEM in
    npm) build_with_npm;;
    esy) build_with_esy;;
    opam) build_with_opam;;
    *) echo Unknown build system $BUILD_SYSTEM; exit 1;;
esac
