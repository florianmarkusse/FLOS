package architecture

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/converter"
	"cmd/common/flags"
	"flag"
	"fmt"
)

const ARCHITECTURE_LONG_FLAG = "architecture"
const ARCHITECTURE_SHORT_FLAG = "a"

type Architecture string

const (
	X86 Architecture = "X86"
)

var PossibleArchitectures = []string{
	string(X86),
}

func IsValidArchitecture(architecture string) bool {
	for _, validArchitecture := range PossibleArchitectures {
		if architecture == validArchitecture {
			return true
		}
	}
	return false
}

func DefaultArchitecture() string {
	return string(X86)
}

func DisplayArchitecture() {
	// Not sure why go doesnt understand string lengths of this one, but whatever
	var architectureDescription = fmt.Sprintf("Set the architecture (%s%s%s)                                  ", common.WHITE,
		converter.ArrayIntoPrintableString(PossibleArchitectures[:]), common.RESET)
	flags.DisplayArgumentInput(ARCHITECTURE_SHORT_FLAG, ARCHITECTURE_LONG_FLAG, architectureDescription, DefaultArchitecture())
}

func AddArchitectureAsFlag(architecture *string) {
	flag.StringVar(architecture, ARCHITECTURE_LONG_FLAG, *architecture, "")
	flag.StringVar(architecture, ARCHITECTURE_SHORT_FLAG, *architecture, "")
}

func DisplayArchitectureConfiguration(architecture string) {
	configuration.DisplayStringArgument(ARCHITECTURE_LONG_FLAG, architecture)
}
