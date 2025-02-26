package argument

import (
	"cmd/common"
	"fmt"
	"io"
	"log"
	"os/exec"
)

func ExecCommand(command string) {
	ExecCommandWriteOutput(command)
}

func ExecCommandWriteOutput(command string, writers ...io.Writer) {
	fmt.Printf("%s%s%s\n", common.BOLD, command, common.RESET)

	cmd := exec.Command("bash", "-c", command)

	cmd.Stdout = log.Writer()
	if len(writers) > 0 {
		cmd.Stderr = io.MultiWriter(append([]io.Writer{log.Writer()}, writers...)...)
	} else {
		cmd.Stderr = log.Writer()
	}

	var err = cmd.Run()
	if err != nil {
		log.Fatalf("Command failed, aborting!")
	}
}
