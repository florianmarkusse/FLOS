package builder

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/cmake"
	"cmd/common/flags/architecture"
	"cmd/common/flags/buildmode"
	"cmd/common/project"
	"fmt"
	"io"
	"log"
	"os"
	"runtime"
	"strings"
)

func findAndRunTests(selectedTargets []string, proj *project.ProjectStructure, buildMode string, architecture string) BuildResult {
	var buildDirectory = project.BuildDirectoryRoot(proj, buildMode, architecture)
	if len(selectedTargets) > 0 {
		for _, target := range selectedTargets {
			var findCommand = fmt.Sprintf("find %s -executable -type f -name \"%s\" -exec {} \\;", buildDirectory, target)
			argument.ExecCommand(findCommand)
		}

		return Success
	}

	var findCommand = fmt.Sprintf("find %s -executable -type f -name \"*-tests*\" -exec {} \\;", buildDirectory)
	argument.ExecCommand(findCommand)

	return Success
}

func copyCompileCommands(buildDirectory string, codeDirectory string) {
	findOptions := strings.Builder{}
	fmt.Fprintf(&findOptions, "%s", buildDirectory)
	fmt.Fprintf(&findOptions, "-maxdepth 1")
	fmt.Fprintf(&findOptions, "-name \"compile_commands.json\"")
	// NOTE: Using copy here because sym linking gave issues???
	fmt.Fprintf(&findOptions, "-exec cp -f {} %s \\;", codeDirectory)

	argument.ExecCommand(fmt.Sprintf("find %s", findOptions.String()))
}

func populateErrorWriter(errorsToFile bool, codeDirectory string) []io.Writer {
	if !errorsToFile {
		return []io.Writer{}
	}

	var errorFile = fmt.Sprintf("%s/stderr.txt", codeDirectory)
	file, err := os.Create(errorFile)
	if err != nil {
		log.Fatalf("Failed to create file to redirect errors to")
	}

	var result = []io.Writer{file}
	return result
}

func buildProject(args *BuildArgs, proj *project.ProjectStructure) {
	if args.Environment != "" {
		proj.Environment = args.Environment
	}

	var errorWriters = populateErrorWriter(args.ErrorsToFile, proj.CodeFolder)

	configureOptions := strings.Builder{}

	var buildDirectory = project.BuildDirectoryRoot(proj, args.BuildMode, args.Architecture)
	var projectTargetsFile = project.BuildProjectTargetsFile(proj.CodeFolder)
	cmake.AddDefaultConfigureOptions(&configureOptions, proj, buildDirectory, args.BuildMode, args.BuildTests, projectTargetsFile, args.Architecture)
	argument.ExecCommandWriteOutput(fmt.Sprintf("%s %s", cmake.EXECUTABLE, configureOptions.String()), errorWriters...)

	buildOptions := strings.Builder{}
	if cmake.AddDefaultBuildOptions(&buildOptions, buildDirectory, projectTargetsFile, args.Threads, args.SelectedTargets, args.Verbose) {
		argument.ExecCommandWriteOutput(fmt.Sprintf("%s %s", cmake.EXECUTABLE, buildOptions.String()), errorWriters...)
	}
	copyCompileCommands(buildDirectory, proj.CodeFolder)
}

type BuildArgs struct {
	BuildMode        string
	Environment      string
	ErrorsToFile     bool
	Threads          int
	SelectedTargets  []string
	SelectedProjects []string
	BuildTests       bool
	RunTests         bool
	Architecture     string
	Verbose          bool
}

type BuildResult uint8

const (
	Success BuildResult = iota
	Failure
)

var RunBuildArgs = BuildArgs{
	BuildMode:        buildmode.DefaultBuildMode(),
	Environment:      "",
	ErrorsToFile:     false,
	Threads:          runtime.NumCPU(),
	SelectedTargets:  []string{},
	SelectedProjects: []string{project.KERNEL, project.IMAGE_BUILDER, project.OS_LOADER},
	BuildTests:       false,
	RunTests:         false,
	Architecture:     architecture.DefaultArchitecture(),
	Verbose:          false,
}

var DefaultBuildArgs = BuildArgs{
	BuildMode:        buildmode.DefaultBuildMode(),
	Environment:      "",
	ErrorsToFile:     false,
	Threads:          runtime.NumCPU(),
	SelectedTargets:  []string{},
	SelectedProjects: []string{},
	BuildTests:       false,
	RunTests:         false,
	Architecture:     architecture.DefaultArchitecture(),
	Verbose:          false,
}

func Build(args *BuildArgs) BuildResult {
	// NOTE: Using waitgroups is currently slower than just running synchronously

	var projectsToBuild = project.GetAllProjects(args.SelectedProjects)
	for name, project := range projectsToBuild {
		fmt.Printf("Building %s%s%s\n", common.CYAN, name, common.RESET)
		buildProject(args, project)
	}

	if args.BuildTests {
		if !args.RunTests {
			return Success
		}
		for name, project := range projectsToBuild {
			fmt.Printf("Testing %s%s%s\n", common.CYAN, name, common.RESET)
			findAndRunTests(args.SelectedTargets, project, args.BuildMode, args.Architecture)
		}
	}

	return Success
}
