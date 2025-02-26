package main

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/exit"
	"cmd/common/flags"
	"cmd/common/flags/environment"
	"cmd/common/flags/help"
	"cmd/common/iwyu"
	"cmd/common/project"
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

var projectToIWYUString string
var projectsToIWYU []string

var env string = ""

var isHelp = false

func main() {
	project.AddProjectAsFlag(&projectToIWYUString)
	environment.AddEnvironmentAsFlag(&env)
	help.AddHelpAsFlag(&isHelp)

	flag.Usage = usage
	flag.Parse()

	var showHelpAndExit = false

	if !project.ValidateAndConvertProjects(projectToIWYUString, &projectsToIWYU) {
		showHelpAndExit = true
	}

	if !environment.IsValidEnvironment(env) && env != "" {
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
	environment.DisplayEnvironmentConfiguration(env)
	project.DisplayProjectConfiguration(projectsToIWYU)

	var projectsToBuild = project.GetAllProjects(projectsToIWYU)
	for name, project := range projectsToBuild {
		var selectedEnvironment = env
		if env == "" {
			selectedEnvironment = project.Environment
		}
		fmt.Printf("Running iwyu on %s%s%s\n", common.CYAN, name, common.RESET)
		fmt.Printf("project %s\n", selectedEnvironment)

		iwyu.RunIWYUOnProject(project.CodeFolder, selectedEnvironment, project.ExcludedIWYUMappings)
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

	environment.DisplayEnvironment()
	project.DisplayProject()
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
