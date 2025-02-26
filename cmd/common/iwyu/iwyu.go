package iwyu

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/flags/environment"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"os"
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

type CompileCommands struct {
	Directory string `json:"directoory"`
	Command   string `json:"command"`
	File      string `json:"file"`
	Output    string `json:"output"`
}

var COMPILE_COMMANDS_FILE = "compile_commands.json"

func FixIncludes(isWetRun bool, iwyuFileString string) {
	var fixIncludesCommand = strings.Builder{}
	fmt.Fprintf(&fixIncludesCommand, "%s/include-what-you-use/fix_includes.py ", common.REPO_DEPENDENCIES)
	if !isWetRun {
		fmt.Fprintf(&fixIncludesCommand, "--dry_run ")
	}
	fmt.Fprintf(&fixIncludesCommand, "--nocomments --reorder ")
	fmt.Fprintf(&fixIncludesCommand, "< %s", iwyuFileString)

	argument.ExecCommand(fixIncludesCommand.String())
}

func runIWYUOnProjectFiles(codeFolder string, iwyuFlags string, iwyuFileString string) {
	iwyuFile, err := os.Create(iwyuFileString)
	if err != nil {
		log.Fatalf("Failed to create file to redirect iwyu to")
	}

	iwyuCommand := strings.Builder{}
	fmt.Fprintf(&iwyuCommand, "include-what-you-use")
	fmt.Fprintf(&iwyuCommand, " %s", iwyuFlags)

	var compileCommandsLocation = fmt.Sprintf("%s/%s", codeFolder, COMPILE_COMMANDS_FILE)

	compileCommandsFile, err := os.Open(compileCommandsLocation)
	if err != nil {
		log.Fatalf("Error opening file: %v", err)
	}
	defer compileCommandsFile.Close()

	data, err := io.ReadAll(compileCommandsFile)
	if err != nil {
		log.Fatalf("Error reading file: %v", err)
	}

	var commands []CompileCommands
	if err := json.Unmarshal(data, &commands); err != nil {
		log.Fatalf("Error unmarshalling JSON: %v", err)
	}

	for _, cmd := range commands {
		if strings.HasPrefix(cmd.File, codeFolder) {
			iwyuCommandForFile := strings.Builder{}
			iwyuCommandForFile.WriteString(iwyuCommand.String())
			iwyuCommandForFile.WriteString(cmd.Command)

			argument.ExecCommandWriteOutput(iwyuCommandForFile.String(), iwyuFile)
		}
	}
}

func includeWhatYouUseFlag(flag string) string {
	return fmt.Sprintf("-Xiwyu %s ", flag)
}

func RunIWYUOnProject(codeFolder string, env string, isWetRun bool, excludedMappings []IWYUMapping) {
	var iwyuFlags = strings.Builder{}
	if env == string(environment.Freestanding) || env == string(environment.Efi) {
		iwyuFlags.WriteString(includeWhatYouUseFlag("--no_default_mappings"))
	}

	for _, possibleMapping := range possibleMappings {
		var excludeMapping = false
		for _, excludedMapping := range excludedMappings {
			if possibleMapping == excludedMapping {
				excludeMapping = true
			}
		}

		if !excludeMapping {
			iwyuFlags.WriteString(includeWhatYouUseFlag(fmt.Sprintf("--mapping_file=%s/%s", IWYU_ROOT, possibleMapping)))
		}
	}

	var iwyuFileString = fmt.Sprintf("%s/iwyu.txt", codeFolder)
	runIWYUOnProjectFiles(codeFolder, iwyuFlags.String(), iwyuFileString)
	FixIncludes(isWetRun, iwyuFileString)
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
