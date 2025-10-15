#!/bin/bash
# Usage: ./make_icns.sh icon.png MyApp

SRC_ICON="$1"
APP_NAME="${2:-AppIcon}"
ICONSET="${APP_NAME}.iconset"

if [ ! -f "$SRC_ICON" ]; then
  echo "❌ Source PNG '$SRC_ICON' not found!"
  echo "Usage: $0 icon.png MyApp"
  exit 1
fi

mkdir -p "$ICONSET"

echo "🖼️  Generating icon sizes..."
sips -z 16 16     "$SRC_ICON" --out "$ICONSET/icon_16x16.png"
sips -z 32 32     "$SRC_ICON" --out "$ICONSET/icon_16x16@2x.png"
sips -z 32 32     "$SRC_ICON" --out "$ICONSET/icon_32x32.png"
sips -z 64 64     "$SRC_ICON" --out "$ICONSET/icon_32x32@2x.png"
sips -z 128 128   "$SRC_ICON" --out "$ICONSET/icon_128x128.png"
sips -z 256 256   "$SRC_ICON" --out "$ICONSET/icon_128x128@2x.png"
sips -z 256 256   "$SRC_ICON" --out "$ICONSET/icon_256x256.png"
sips -z 512 512   "$SRC_ICON" --out "$ICONSET/icon_256x256@2x.png"
sips -z 512 512   "$SRC_ICON" --out "$ICONSET/icon_512x512.png"
cp "$SRC_ICON" "$ICONSET/icon_512x512@2x.png"

echo "📦  Building .icns file..."
iconutil -c icns "$ICONSET" -o "${APP_NAME}.icns"

if [ $? -eq 0 ]; then
  echo "✅ Done! Created ${APP_NAME}.icns"
  rm -rf "$ICONSET"
else
  echo "❌ Failed to create .icns"
fi