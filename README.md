# UEFI-bootloader & OS

run

```
cmd/install-dependencies.sh
cmd/build-go.sh
# The below command is necessary to create the kernel magic file. This
dependency will be removed once I have better hardware that has a more
up-to-date UEFI implementation >2017 instead of 2015...
cmd/compile.elf -p kernel
cmd/compile-run.elf
```

## To run as a standalone operating system

```
# Plug in USB or other bootable device into your computer
# Find out first on what file system your device is
lsblk # You should see there your device if it is connected
# Fill out the of command with the right path from the above command
cmd/hardware.elf --file /dev/sdb1
# Restart your computer with the device still in there
# Go to the boot menu and you should find it there, may need to hit F12
# In my case: UEFI: <name of the USB stick>
# but motherboards do things in mysterious ways
```

## x86_64 only

## Using iwyu

```
# First compile with -e flag to create stderr.txt file for a project.
dependencies/include-what-you-use/fix_includes.py --reorder --nocomments < projects/image-builder/code/stderr.txt
```

### UEFI improvements once upgraded to new firmware !!

1. Use `PartitionInformationProtocol`
2. Use custom Memory Type to mark free kernel memory

### CPU improvements once upgraded to new firmware !!

1. Update calibrating timer code:
   - Use leaf 0x15 on Intel for timer
   - Use AMD extended info for timer
   - Instead of the waiting stuff it's doing now
2. Use XSAVES instead of XSAVEOPT
   - In idt.c / x86/init.c and run-qemu.go , all need updates
