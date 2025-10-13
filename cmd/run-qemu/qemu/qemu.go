package qemu

import (
	"cmd/common"
	"cmd/common/argument"
	"fmt"
	"strings"
)

const QEMU_EXECUTABLE = "qemu-system-x86_64"
const FILE_OUTPUT = "qemu-output.log"

type QemuArgs struct {
	OsLocation   string
	UefiLocation string
	OutputToFile bool
	Verbose      bool
	Debug        bool
	Graphic      bool
}

var DefaultQemuArgs = QemuArgs{
	OsLocation:   common.REPO_ROOT + "/" + common.FLOS_UEFI_IMAGE_FILE,
	UefiLocation: common.REPO_ROOT + "/" + common.BIOS_FILE,
	OutputToFile: false,
	Verbose:      false,
	Debug:        false,
	Graphic:      false,
}

func Run(args *QemuArgs) {
	qemuOptions := strings.Builder{}
	argument.AddArgument(&qemuOptions, "-m 512")
	argument.AddArgument(&qemuOptions, "-machine q35")
	argument.AddArgument(&qemuOptions, "-no-reboot")
	argument.AddArgument(&qemuOptions, fmt.Sprintf("-drive \"format=raw,file=%s\"", args.OsLocation))
	argument.AddArgument(&qemuOptions, fmt.Sprintf("-bios %s", args.UefiLocation))
	if args.OutputToFile {
		argument.AddArgument(&qemuOptions, fmt.Sprintf("-serial file:%s", FILE_OUTPUT))
	} else {
		argument.AddArgument(&qemuOptions, "-serial mon:stdio")
	}
	if !args.Graphic {
		argument.AddArgument(&qemuOptions, "-nographic")
	}

	argument.AddArgument(&qemuOptions, "-smp 1")
	argument.AddArgument(&qemuOptions, "-usb")
	argument.AddArgument(&qemuOptions, "-vga std")
	argument.AddArgument(&qemuOptions, "-enable-kvm")

	if args.Verbose {
		argument.AddArgument(&qemuOptions, "-d int,cpu_reset")
	}

	if args.Debug {
		argument.AddArgument(&qemuOptions, "-s -S")
		// NOTE: Ensure this is the same architecture as what you are trying to
		// build for :)))
		argument.AddArgument(&qemuOptions,
			"-cpu EPYC-Genoa-v1,pdpe1gb=on,tsc-frequency=4000000000,+invtsc")
	} else {
		// TODO: When upgraded, add the xsavec , xsaves, xrstors
		argument.AddArgument(&qemuOptions, "-cpu host,+xsave,+avx,+xsaveopt,+invtsc")
	}

	if args.OutputToFile {
		fmt.Printf("%sYour output is redirected to file %s%s%s\n\n", common.BOLD, common.YELLOW, FILE_OUTPUT, common.RESET)
	}
	argument.ExecCommand(fmt.Sprintf("%s %s", QEMU_EXECUTABLE, qemuOptions.String()))
}
