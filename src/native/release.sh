set -e
set -x

VERSION=$(git describe --abbrev=0)
RELEASE=promise-$VERSION

rm -rf $RELEASE $RELEASE.tar $RELEASE.tar.gz
mkdir -p $RELEASE
cp -r dune-project LICENSE.md promise.opam README.md src test $RELEASE
rm -rf $RELEASE/src/js
rm -rf $RELEASE/src/native/release.sh
rm -rf $RELEASE/test/bundle
rm -rf $RELEASE/test/framework/js
rm -rf $RELEASE/test/isoresult/js
rm -rf $RELEASE/test/js
sed -i "s/version: \"dev\"/version: \"$VERSION\"/" $RELEASE/promise.opam
tar cf $RELEASE.tar $RELEASE
ls -l $RELEASE.tar
gzip -9 $RELEASE.tar
mkdir -p _release
cp $RELEASE.tar.gz _release
(cd _release && tar xf $RELEASE.tar.gz)
opam install --verbose --with-test -y _release/$RELEASE/promise.opam
opam remove -y promise
opam pin remove -y promise
colordiff -u promise.opam $RELEASE/promise.opam || true
opam lint $RELEASE
ls -l $RELEASE.tar.gz
md5sum $RELEASE.tar.gz
