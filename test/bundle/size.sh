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

CONTROL=`stat -c '%s' test/bundle/control.js`
PROMISE=`stat -c '%s' test/bundle/uses_promise.js`
DIFFERENCE=`expr $PROMISE - $CONTROL`
LIMIT=4096

if [ $DIFFERENCE -gt $LIMIT ]
then
  echo "Bundle size $DIFFERENCE exceeds $LIMIT"
  exit 1
else
  echo "Bundle size: $DIFFERENCE"
fi
