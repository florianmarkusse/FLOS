package main

import (
	"cmd/common/configuration"
	"cmd/common/exit"
	"cmd/common/flags"
	"cmd/common/flags/help"
	"cmd/common/iwyu"
	"cmd/common/project"
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

const WET_RUN_LONG_FLAG = "wet-run"
const WET_RUN_SHORT_FLAG = "w"

var projectToIWYUString string
var projectsToIWYU []string

var isWetRun = false

var isHelp = false

func main() {
	project.AddProjectAsFlag(&projectToIWYUString)
	help.AddHelpAsFlag(&isHelp)

	flag.BoolVar(&isWetRun, WET_RUN_LONG_FLAG, isWetRun, "")
	flag.BoolVar(&isWetRun, WET_RUN_SHORT_FLAG, isWetRun, "")

	flag.Usage = usage
	flag.Parse()

	var showHelpAndExit = false

	if !project.ValidateAndConvertProjects(projectToIWYUString, &projectsToIWYU) {
		showHelpAndExit = true
	}

	if isHelp {
		showHelpAndExit = true
	}

	if showHelpAndExit {
		usage()
		if isHelp {
			os.Exit(exit.EXIT_SUCCESS)
		}
		os.Exit(exit.EXIT_MISSING_ARGUMENT)
	}

	configuration.DisplayConfiguration()
	project.DisplayProjectConfiguration(projectsToIWYU)
	configuration.DisplayBoolArgument(WET_RUN_LONG_FLAG, isWetRun)

	var projectsToBuild = project.GetAllProjects(projectsToIWYU)
	for _, project := range projectsToBuild {
		iwyu.RunIWYUOnProject(project.CodeFolder, project.Environment, isWetRun, project.ExcludedIWYUMappings)
	}

	fmt.Printf("\n")

	os.Exit(exit.EXIT_SUCCESS)

	// var result = builder.Build(&buildArgs)
	//
	// switch result {
	// case builder.Success:
	// 	{
	// 		os.Exit(exit.EXIT_SUCCESS)
	// 	}
	// case builder.Failure:
	// 	{
	// 		os.Exit(exit.EXIT_TARGET_ERROR)
	// 	}
	// }
}

func usage() {
	flags.DisplayUsage("")
	fmt.Printf("\n")

	flags.DisplayOptionalFlags()

	project.DisplayProject()
	flags.DisplayArgumentInput(WET_RUN_SHORT_FLAG, WET_RUN_LONG_FLAG, "Do a wet run", fmt.Sprint(isWetRun))
	help.DisplayHelp()

	fmt.Printf("\n")
	exit.DisplayExitCodes()
	exit.DisplayExitCode(exit.EXIT_SUCCESS)
	exit.DisplayExitCode(exit.EXIT_CLI_PARSING_ERROR)
	fmt.Printf("\n")
	flags.DisplayExamples()
	fmt.Printf("  %s\n", filepath.Base(os.Args[0]))
	fmt.Printf("  %s --%s %s,%s\n", filepath.Base(os.Args[0]),
		project.PROJECTS_LONG_FLAG, project.KERNEL, project.OS_LOADER)
	fmt.Printf("\n")
}
