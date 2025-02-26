package cmake

import (
	"bufio"
	"cmd/common"
	"cmd/common/exit"
	"cmd/common/iwyu"
	"cmd/common/project"
	"fmt"
	"os"
	"strings"
)

const EXECUTABLE = "cmake"

func includeWhatYouUseFlag(flag string) string {
	return fmt.Sprintf("-Xiwyu;%s;", flag)
}

func AddDefaultConfigureOptions(options *strings.Builder, proj *project.ProjectStructure, buildDirectory string, buildMode string, buildTests bool, projectTargetsFile string, architecture string) {
	fmt.Fprintf(options, "-S %s", proj.CodeFolder)
	fmt.Fprintf(options, "-B %s", buildDirectory)

	result := strings.TrimPrefix(proj.Folder, common.REPO_PROJECTS)
	result = result[1:] // remove '/' xxx
	fmt.Fprintf(options, "-D PROJECT_FOLDER=%s", result)

	fmt.Fprintf(options, "-D CMAKE_C_COMPILER=%s", proj.CCompiler)
	fmt.Fprintf(options, "-D CMAKE_LINKER=%s", proj.Linker)
	fmt.Fprintf(options, "-D CMAKE_BUILD_TYPE=%s", buildMode)
	fmt.Fprintf(options, "-D ENVIRONMENT=%s", proj.Environment)
	fmt.Fprintf(options, "-D ARCHITECTURE=%s", architecture)
	fmt.Fprintf(options, "-D BUILD_OUTPUT_PATH=%s", buildDirectory)
	fmt.Fprintf(options, "-D REPO_ROOT=%s", common.REPO_ROOT)
	fmt.Fprintf(options, "-D REPO_DEPENDENCIES=%s", common.REPO_DEPENDENCIES)
	fmt.Fprintf(options, "-D REPO_PROJECTS=%s", common.REPO_PROJECTS)
	fmt.Fprintf(options, "-D PROJECT_TARGETS_FILE=%s", projectTargetsFile)
	fmt.Fprintf(options, "-D FLOAT_OPERATIONS=%t", proj.FloatOperations)

	var build string
	if buildTests {
		build = "UNIT_TEST"
	} else {
		build = "PROJECT"
	}
	fmt.Fprintf(options, "-D BUILD=%s", build)
	fmt.Fprintf(options, "--graphviz=%s/output.dot", proj.CodeFolder)
	fmt.Fprintf(options, "%s", iwyu.FlagsForCMake(proj.Environment, proj.ExcludedIWYUMappings))
}

func AddDefaultBuildOptions(options *strings.Builder, buildDirectory string, projectTargetsFile string, threads int, targets []string, verbose bool) bool {
	fmt.Fprintf(options, "--build %s", buildDirectory)
	fmt.Fprintf(options, "--parallel %d", threads)
	if verbose {
		fmt.Fprintf(options, "-v")
	}

	targetsString := strings.Builder{}
	if len(targets) > 0 {
		for _, target := range targets {
			targetsString.WriteString(target)
			targetsString.WriteString(" ")
		}
	} else {
		file, err := os.Open(projectTargetsFile)
		if err != nil {
			os.Exit(exit.EXIT_TARGET_ERROR)
		}
		defer file.Close()

		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			var target = scanner.Text()
			targetsString.WriteString(target)
			targetsString.WriteString(" ")
		}

		if err := scanner.Err(); err != nil {
			os.Exit(exit.EXIT_TARGET_ERROR)
		}

		if targetsString.Len() == 0 {
			return false
		}

	}

	if targetsString.Len() > 0 {
		fmt.Fprintf(options, "--target %s", targetsString.String())
	}

	return true
}
