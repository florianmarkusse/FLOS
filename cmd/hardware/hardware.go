package main

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/exit"
	"cmd/common/flags"
	"cmd/common/flags/architecture"
	"cmd/common/flags/buildmode"
	"cmd/common/flags/help"
	"cmd/common/uefiimage"
	"cmd/compile/builder"
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

const OUTPUT_FILE_LONG_FLAG = "file"
const OUTPUT_FILE_SHORT_FLAG = "f"

var outputFile = ""

var isHelp = false

func main() {
	help.AddHelpAsFlag(&isHelp)

	flag.StringVar(&outputFile, OUTPUT_FILE_LONG_FLAG, "", "")
	flag.StringVar(&outputFile, OUTPUT_FILE_SHORT_FLAG, "", "")

	flag.Usage = usage
	flag.Parse()

	var showHelpAndExit = false

	if len(outputFile) == 0 {
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

	var result = builder.Build(&builder.RunBuildArgs)
	if result == builder.Failure {
		fmt.Println("Failed to build project")
		os.Exit(exit.EXIT_TARGET_ERROR)
	}

	uefiimage.CreateUefiImage(buildmode.DefaultBuildMode(), architecture.DefaultArchitecture(), false)

	writeToUSBCommand := fmt.Sprintf("sudo dd if=%s/%s of=%s conv=notrunc", common.REPO_ROOT, common.FLOS_UEFI_IMAGE_FILE, outputFile)
	argument.ExecCommand(writeToUSBCommand)
}

func usage() {
	flags.DisplayUsage("")
	fmt.Printf("\n")
	flags.DisplayNoDefaultArgumentInput(OUTPUT_FILE_LONG_FLAG, OUTPUT_FILE_SHORT_FLAG, "Output file to write OS to")
	help.DisplayHelp()
	fmt.Printf("\n")
	exit.DisplayExitCodes()
	exit.DisplayExitCode(exit.EXIT_SUCCESS)
	exit.DisplayExitCode(exit.EXIT_MISSING_ARGUMENT)
	exit.DisplayExitCode(exit.EXIT_CLI_PARSING_ERROR)
	exit.DisplayExitCode(exit.EXIT_TARGET_ERROR)
	fmt.Printf("\n")
	flags.DisplayExamples()
	fmt.Printf("  %s --%s=%s\n", filepath.Base(os.Args[0]), OUTPUT_FILE_LONG_FLAG, "/dev/sdc1")
	fmt.Printf("\n")
}
