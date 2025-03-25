#!/bin/bash
die() { echo "$*" 1>&2 ; exit 1; }

# make a download directory
mkdir .download
pushd .download

# Download Images
echo "> Downloading images from ftp.embeddedts.com"
curl -O "http://ftp.embeddedts.com/ftp/ts-x86-sbc/old-downloads/binaries/3100DISK.ZIP"
curl -O "http://ftp.embeddedts.com/ftp/ts-x86-sbc/old-downloads/binaries/3100BIOS.ZIP"
curl -O "http://ftp.embeddedts.com/ftp/ts-x86-sbc/old-downloads/binaries/DOS404.ZIP"
curl -O "http://ftp.embeddedts.com/ftp/ts-x86-sbc/old-downloads/Disks/TS-3100.ZIP"

# Check integrity
echo "> Checking integrity of images"
sha256sum -c ../checksums.sha256sum 2>&1 >/dev/null || die "Checksum of images does NOT MATCH!"
echo "> All files look good"

# Extract archives
echo "> Extracting archives"
unzip -o 3100DISK.ZIP 2>&1 >/dev/null || die "Failed to extract 3100DISK.ZIP"
unzip -o 3100BIOS.ZIP 2>&1 >/dev/null || die "Failed to extract 3100BIOS.ZIP"
unzip -o DOS404.ZIP 2>&1 >/dev/null || die "Failed to extract DOS404.ZIP"
unzip -o TS-3100.ZIP -d TS-3100 2>&1 >/dev/null || die "Failed to extract TS-3100.ZIP"
popd

# Generate flash.bin
echo "> Generating roms/flash.bin"
cat .download/3100DISK.BIN .download/DOS404.BIN .download/3100BIOS.BIN > roms/flash.bin

# Populate drivec.img
echo "> Populating roms/drivec.img"
mcopy -i roms/drivec.img@@1M .download/TS-3100/* ::

# Cleanup
rm -rf .download
echo "> Done"
