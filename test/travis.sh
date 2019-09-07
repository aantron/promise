set -e
set -x



build_with_npm () {
    date

    npm install

    date

    make bucklescript-coverage
    ./node_modules/.bin/bisect-ppx-report send-to Coveralls

    date
}

build_with_esy () {
    date

    npm install --no-save esy
    WD=`pwd`
    export PATH="$WD/node_modules/.bin:$PATH"

    date

    esy install

    date

    # The separate build command is a workaround. See
    #   https://github.com/aantron/repromise/commit/c2f5aa7dd7e7bc5f521a6ab8c8275687f5fc8639#commitcomment-29955448.
    esy dune build test/test_main.exe

    date

    esy dune exec test/test_main.exe

    date
}

build_with_opam () {
    date

    wget https://github.com/ocaml/opam/releases/download/2.0.5/opam-2.0.5-x86_64-linux
    sudo mv opam-2.0.5-x86_64-linux /usr/local/bin/opam
    sudo chmod a+x /usr/local/bin/opam

    date

    opam init -ya --compiler=4.08.1 --disable-sandboxing
    eval `opam env`
    ocaml -version

    date

    opam lint
    opam install -y --deps-only .

    date

    make native-coverage
    bisect-ppx-report send-to Coveralls

    date
}



case $BUILD_SYSTEM in
    npm) build_with_npm;;
    esy) build_with_esy;;
    opam) build_with_opam;;
    *) echo Unknown build system $BUILD_SYSTEM; exit 1;;
esac
