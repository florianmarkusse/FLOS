package serial

import (
	"cmd/common/configuration"
	"cmd/common/flags"
	"flag"
	"fmt"
)

const SERIAL_LONG_FLAG = "serial"

func DefaultSerial() bool {
	return false
}

func DisplaySerial() {
	flags.DisplayLongFlagArgumentInput(SERIAL_LONG_FLAG, "Turn on logging to serial", fmt.Sprint(DefaultSerial()))
}

func AddSerialAsFlag(serial *bool) {
	flag.BoolVar(serial, SERIAL_LONG_FLAG, *serial, "")
}

func DisplaySerialConfiguration(serial bool) {
	configuration.DisplayBoolArgument(SERIAL_LONG_FLAG, serial)
}
