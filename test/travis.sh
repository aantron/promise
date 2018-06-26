set -x



install_libuv() {
    sudo add-apt-repository -y ppa:acooks/libwebsockets6
    sudo apt-get update -qq
    sudo apt-get install libuv1-dev
    sudo apt-get install texinfo
}

install_opam () {
    sudo add-apt-repository -y ppa:avsm/ocaml42+opam12
    sudo apt-get update -qq
    sudo apt-get install opam
}



build_with_npm () {
    install_opam
    opam init -y --compiler=4.02.3+buckle-master
    eval `opam config env`

    opam pin add -y --dev-repo reason

    npm install
    npm run build
    (cd test && npm install && npm run build)
    node ./test/lib/js/test.js
}

# The esy and opam builds both require libuv1, available in Ubuntu 16.04,
# (Xenial), which is still a work-in-progress from the Travis team. See
#   https://github.com/travis-ci/travis-ci/issues/7260
# In the meantime, we just let the commands fail, and add "|| true" to the
# testing commands. Most of the tests still pass.
# This may be undone if we remove the libuv tests, or move libuv to a separate
# repo.

build_with_esy () {
    install_libuv
    npm install -g esy

    esy install
    esy jbuilder exec test/test.exe
}

build_with_opam () {
    install_libuv
    install_opam
    opam init -ya --compiler=4.06.1
    eval `opam config env`

    opam pin add -y --dev-repo reason
    opam pin add -y --no-action repromise .
    opam pin add -y --no-action ppx_await .
    opam install -y --deps-only repromise
    opam install -y --deps-only ppx_await

    jbuilder exec test/test.exe
}



case $BUILD_SYSTEM in
    npm) build_with_npm;;
    esy) build_with_esy;;
    opam) build_with_opam;;
    *) echo Unknown build system $BUILD_SYSTEM; exit 1;;
esac
