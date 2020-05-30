#!/usr/bin/env sh

APP_NAME="gradovi-srbije"
SOURCEDIR="$( cd "$(dirname "$0")" ; pwd -P )"

clean_up () {
	./uninstall-linux.sh
}

echo -n "Running make ......"
if [ -f "${SOURCEDIR}/Makefile" ]; then
	make -s
else
	echo "${SOURCEDIR} doesn't have a Makefile!"
	exit 1
fi

if [ -f "${SOURCEDIR}/${APP_NAME}" ]; then
	echo "[done]"
else
	echo "[failed]"
	exit 1
fi

echo -n "Copying executable to /usr/bin/ ......"
cp -f "${SOURCEDIR}/${APP_NAME}" "/usr/bin"
if [ -f "/usr/bin/${APP_NAME}" ]; then
	echo "[done]"
else
	echo "[failed]"
	clean_up
	exit 1
fi

echo -n "Copying icons to /usr/share/icons/hicolor/*/apps/ ......"
for i in 16 32 48 128 256
do
	cp -f "${SOURCEDIR}/resources/images/${APP_NAME}-${i}.png" "/usr/share/icons/hicolor/${i}x${i}/apps/${APP_NAME}.png"
done
update-icon-caches /usr/share/icons/*
echo "[done]"

echo -n "Copying .desktop file to /usr/share/applications......"
cp -rf "${SOURCEDIR}/resources/${APP_NAME}.desktop" "/usr/share/applications"

if [ -f "/usr/share/applications/gradovi-srbije.desktop" ]; then
	echo "[done]"
else
	echo "[failed]"
	clean_up
	exit 1
fi

echo "Successfully installed ${APP_NAME}!"
