#!/bin/sh
# $1: $$PWD
# $2: $$DESTDIR
# $3: $$TARGET
# $4: $$VER_ARG

echo "Prepare translation files…"
lrelease $1/$3.pro
echo "Complete"

echo "Copy translation files…"
cp $1/*.qm $2$3".app/Contents/MacOS"
echo "Complete"

echo "Deploying…"
macdeployqt $2$3".app" -dmg
mv $2$3".dmg" $2$3"_"$4".dmg"
echo ""
echo "Done! $2$3"_"$4".dmg"
echo ""


