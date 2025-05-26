package uefiimage

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/project"
	"fmt"
)

func CreateUefiImage(buildMode string, architecture string, serial bool) {
	copyEfiCommand := fmt.Sprintf("find %s -executable -type f -name \"%s\" -exec cp {} %s \\;", project.BuildDirectoryRoot(project.PROJECT_STRUCTURES[project.OS_LOADER], buildMode, architecture, serial), project.OS_LOADER, common.FLOS_EFI_FILE)
	argument.ExecCommand(copyEfiCommand)

	copyKernelCommand := fmt.Sprintf("find %s -executable -type f -name \"kernel.bin\" -exec cp {} %s \\;", project.BuildDirectoryRoot(project.PROJECT_STRUCTURES[project.KERNEL], buildMode, architecture, serial), common.FLOS_KERNEL_FILE)
	argument.ExecCommand(copyKernelCommand)

	createTestImageCommand := fmt.Sprintf("find %s -type f -name \"%s\" -exec {} --phys 512 --efi %s --kernel %s \\;", project.BuildDirectoryRoot(project.PROJECT_STRUCTURES[project.IMAGE_BUILDER], buildMode, architecture, serial), project.IMAGE_BUILDER, common.FLOS_EFI_FILE, common.FLOS_KERNEL_FILE)
	argument.ExecCommand(createTestImageCommand)
}
