#!/bin/sh
# @LICENCE("Open Kernel Labs, Inc.", "2008")@
errors=0

# Create dir to mount on
mkdir -p /disktest/mnt
echo -n "Mounting /dev/sda1 on /disktest/mnt... "
mount -t vfat /dev/sda1 /disktest/mnt
if [ "$?" = "0" ]; then
	echo "OK"
else
	echo "FAILED!"
	errors=$(($errors+1))
	exit 1
fi

echo -n "Touching file /disktest/mnt/foo... "
touch /disktest/mnt/foo
if test -e /disktest/mnt/foo; then
	echo "OK"
else
	echo "FAILED!"
	errors=$(($errors+1))
fi

echo -n "Making a copy of image.elf... "
cp -f /disktest/mnt/boot/grub/image.elf /disktest/mnt/copy.elf
if test -e /disktest/mnt/copy.elf; then
	echo "OK"
else
	echo "FAILED!"
	errors=$(($errors+1))
fi

echo -n "Comparing image.elf with copy.elf... "
cmp /disktest/mnt/boot/grub/image.elf /disktest/mnt/copy.elf
if [ "$?" = "0" ]; then
	echo "OK"
else
	echo "FAILED!"
	errors=$(($errors+1))
fi

echo -n "Cleaning up... "
rm -f /disktest/mnt/foo /disktest/mnt/copy.elf
echo "OK"

echo -n "Unmounting... "
umount /disktest/mnt
if [ "$?" = "0" ]; then
	echo "OK"
else
	echo "FAILED!"
	errors=$(($errors+1))
fi

# Finished
if [ "$errors" = "0" ]; then
	echo "disktest completed... exiting"
	exit 0
else
	echo "disktest failed with $errors errors... exiting"
	exit 1
fi
