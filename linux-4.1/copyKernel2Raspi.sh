#!/bin/sh

# This script copies the kernel and the modules to a RaspberryPi.
# The IP must be commited as the first parameter.
# If the RPi shall be rebooted, you have to commit "reboot" as second parameter.
# The modules must be available in /tmp/ram/rpi

IP=$1

echo "====================================================================="
echo "Mount partitions on RPi..."
ssh root@$IP 'mount -o remount,rw /'
ssh root@$IP 'mkdir /tmp/boot'
ssh root@$IP 'mount /dev/mmcblk0p1 /tmp/boot'
ssh root@$IP 'cp /tmp/boot/zImage /tmp/boot/zImage.bak'

echo "====================================================================="
echo "Copy new kernel tp RPi..."
rm -f zImage
scripts/mkknlimg arch/arm/boot/zImage zImage
scp zImage root@$IP:/tmp/boot/zImage

echo "====================================================================="
exho "Copy device trees to RPi..."
scp arch/arm/boot/dts/bcm2709-rpi-2-b.dtb root@$IP:/tmp/boot/
scp arch/arm/boot/dts/overlays/*.dtb* root@$IP:/tmp/boot/overlays/
scp arch/arm/boot/dts/overlays/README root@$IP:/tmp/boot/overlays/


echo "====================================================================="
echo "Copy modules to RPi..."
rm  /tmp/ram/rpi/lib/modules/4.1.13-v7+/build
rm  /tmp/ram/rpi/lib/modules/4.1.13-v7+/source
scp -r /tmp/ram/rpi/* root@$IP:/


echo "====================================================================="
echo "Remount RPi read only..."
ssh root@$IP 'mount -o remount,ro /'
ssh root@$IP 'sync'


if [ $2 = "reboot" ]
then
	echo "====================================================================="
	echo "Reboot RPi..."
	ssh root@$IP 'reboot'
fi

exit 0
