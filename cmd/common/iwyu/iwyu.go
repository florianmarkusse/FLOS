package iwyu

import (
	"cmd/common"
	"cmd/common/flags/environment"
	"fmt"
	"strings"
)

var IWYU_ROOT = common.REPO_ROOT + "/iwyu-mappings"

type IWYUMapping string

const (
	MEMORY_MANIPULATION IWYUMapping = "memory-manipulation.imp"
	ARCHITECTURE        IWYUMapping = "architecture.imp"
)

var possibleMappings = []IWYUMapping{
	MEMORY_MANIPULATION,
	ARCHITECTURE,
}

var mappings = [...]string{
	"memory-manipulation.imp",
	"architecture.imp",
}

func includeWhatYouUseFlag(flag string) string {
	return fmt.Sprintf("-Xiwyu;%s;", flag)
}

func FlagsForCMake(env string, excludedMappings []IWYUMapping) string {
	var iwyuString = strings.Builder{}
	iwyuString.WriteString("-D CMAKE_C_INCLUDE_WHAT_YOU_USE=\"include-what-you-use;")
	if env == string(environment.Freestanding) || env == string(environment.Efi) {
		iwyuString.WriteString(includeWhatYouUseFlag("--no_default_mappings"))
	}

	for _, possibleMapping := range possibleMappings {
		var excludeMapping = false
		for _, excludedMapping := range excludedMappings {
			if possibleMapping == excludedMapping {
				excludeMapping = true
			}
		}

		if !excludeMapping {
			iwyuString.WriteString(includeWhatYouUseFlag(fmt.Sprintf("--mapping_file=%s/%s", IWYU_ROOT, possibleMapping)))
		}
	}

	iwyuString.WriteString("\"")

	return iwyuString.String()
}
