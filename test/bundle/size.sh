#!/bin/bash

set -e

npm run build
npx webpack \
  --display none --mode production --optimize-minimize \
  --entry ./lib/js/test/bundle/control.js \
  --output ./test/bundle/control.js
npx webpack \
  --display none --mode production --optimize-minimize \
  --entry ./lib/js/test/bundle/uses_promise.js \
  --output ./test/bundle/uses_promise.js
gzip -9f test/bundle/control.js
gzip -9f test/bundle/uses_promise.js

CONTROL=`stat -c '%s' test/bundle/control.js.gz`
PROMISE=`stat -c '%s' test/bundle/uses_promise.js.gz`
DIFFERENCE=`expr $PROMISE - $CONTROL`
LIMIT=1152

if [ $DIFFERENCE -gt $LIMIT ]
then
  echo "Bundle size (incremental) $DIFFERENCE exceeds $LIMIT"
  exit 1
else
  echo "Bundle size (incremental): $DIFFERENCE"
fi
