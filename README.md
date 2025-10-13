# UEFI-bootloader & OS

run

```
cmd/install-dependencies.sh
cmd/build-go.sh
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
# or run sudo systemctl reboot --firmware-setup # that will reboot and you 'should' go to your firmware setup
# In my case: UEFI: <name of the USB stick>
# but motherboards do things in mysterious ways and milage may vary sadly
```

## x86_64 only

## Using iwyu

```
# First compile with -e flag to create stderr.txt file for a project.
dependencies/include-what-you-use/fix_includes.py --reorder --nocomments < projects/image-builder/code/stderr.txt
```

## Using pahole

Ensure you have built with debug mode on!

```
find code/build -type f -name *.c.o -exec pahole -a -d -R -S {} \; > ~/Desktop/FLOS/pahole.txt
```

### CPU improvements once upgraded to new firmware !!

1. detected vendor
2. intel - known
3. amd: 1. check for invariant tsc
   mov eax, 0x80000007
   cpuid
   test edx, (1 << 8)
   jnz .tsc_invariant 2. check msr value and calculate tsc frequency through there
   25 mhz? + 16? + 1? look at the AMD manual

4. Update calibrating timer code:
   - Use leaf 0x15 on Intel for timer
   - Use AMD extended info for timer
   - Instead of the waiting stuff iDt's doing now
