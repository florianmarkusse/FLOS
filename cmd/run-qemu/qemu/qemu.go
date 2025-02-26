package qemu

import (
	"cmd/common"
	"cmd/common/argument"
	"fmt"
	"strings"
)

const QEMU_EXECUTABLE = "qemu-system-x86_64"

type QemuArgs struct {
	OsLocation   string
	UefiLocation string
	Verbose      bool
	Debug        bool
}

var DefaultQemuArgs = QemuArgs{
	OsLocation:   common.REPO_ROOT + "/" + common.FLOS_UEFI_IMAGE_FILE,
	UefiLocation: common.REPO_ROOT + "/" + common.BIOS_FILE,
	Verbose:      false,
	Debug:        false,
}

func Run(args *QemuArgs) {
	qemuOptions := strings.Builder{}
	fmt.Fprintf(&qemuOptions, "-m 512")
	fmt.Fprintf(&qemuOptions, "-machine q35")
	fmt.Fprintf(&qemuOptions, "-no-reboot")
	fmt.Fprintf(&qemuOptions, "-drive \"format=raw,file=%s\"", args.OsLocation)
	fmt.Fprintf(&qemuOptions, "-bios %s", args.UefiLocation)
	fmt.Fprintf(&qemuOptions, "-serial stdio")
	fmt.Fprintf(&qemuOptions, "-smp 1")
	fmt.Fprintf(&qemuOptions, "-usb")
	fmt.Fprintf(&qemuOptions, "-vga std")

	if args.Verbose {
		fmt.Fprintf(&qemuOptions, "-d int,cpu_reset")
	}

	if args.Debug {
		fmt.Fprintf(&qemuOptions, "-s -S")
		// NOTE: Ensure this is the same architecture as what you are trying to
		// build for :)))
		fmt.Fprintf(&qemuOptions, "-cpu Haswell-v4")
		fmt.Fprintf(&qemuOptions, "-accel \"tcg\"")
	} else {
		fmt.Fprintf(&qemuOptions, "-cpu host")
		fmt.Fprintf(&qemuOptions, "-enable-kvm")
	}

	argument.ExecCommand(fmt.Sprintf("%s %s", QEMU_EXECUTABLE, qemuOptions.String()))
}
