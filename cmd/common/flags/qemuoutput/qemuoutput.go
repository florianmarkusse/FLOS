package qemuoutput

import (
	"cmd/common/configuration"
	"cmd/common/flags"
	"flag"
	"fmt"
)

const QEMU_OUTPUT_LONG_FLAG = "file"
const QEMU_OUTPUT_SHORT_FLAG = "f"

func DefaultQemuOutput() bool {
	return false
}

func DisplayQemuOutput() {
	flags.DisplayArgumentInput(QEMU_OUTPUT_SHORT_FLAG, QEMU_OUTPUT_LONG_FLAG, "Redirect QEMU output to file", fmt.Sprint(DefaultQemuOutput()))
}

func AddQemuOutputAsFlag(qemuOutput *bool) {
	flag.BoolVar(qemuOutput, QEMU_OUTPUT_LONG_FLAG, *qemuOutput, "")
	flag.BoolVar(qemuOutput, QEMU_OUTPUT_SHORT_FLAG, *qemuOutput, "")
}

func DisplayQemuOutputConfiguration(qemuOutput bool) {
	configuration.DisplayBoolArgument(QEMU_OUTPUT_LONG_FLAG, qemuOutput)
}
