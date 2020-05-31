#!/usr/bin/env sh

APP_NAME="gradovi-srbije"

echo -n "Running make clean ......"
make -s clean
echo "[done]"

echo -n "Removing executable from /usr/bin/ ......"
rm -f "/usr/bin/${APP_NAME}"
echo "[done]"

echo -n "Removing icons from /usr/share/icons/hicolor/*/apps/${APP_NAME}.png ......"
for i in 16 32 48 128 256
do
	rm -f "/usr/share/icons/hicolor/${i}x${i}/apps/${APP_NAME}.png"
done
gtk-update-icon-cache /usr/share/icons/hicolor/
echo  "[done]"

echo -n "Removing .desktop file from /usr/share/applications ......"
rm -f "/usr/share/applications/${APP_NAME}.desktop"
echo "[done]"

echo "Successfully removed ${APP_NAME}!"
