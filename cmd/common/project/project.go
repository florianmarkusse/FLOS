package project

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/converter"
	"cmd/common/flags"
	"cmd/common/flags/environment"
	"flag"
	"fmt"
	"strings"
)

const PROJECTS_LONG_FLAG = "projects"
const PROJECTS_SHORT_FLAG = "p"

func DisplayProject() {
	flags.DisplayArgumentInput(PROJECTS_SHORT_FLAG, PROJECTS_LONG_FLAG, "Select specific project(s, comma-separated) to be built", converter.ArrayIntoPrintableString(ConfiguredProjects))
}

func ValidateAndConvertProjects(projectsToBuild string, selectedProjects *[]string) bool {
	*selectedProjects = strings.FieldsFunc(projectsToBuild, func(r rune) bool {
		return r == ','
	})

	for _, selectedProject := range *selectedProjects {
		var isValidProjectName = false
		for _, configuredProjects := range ConfiguredProjects {
			if selectedProject == configuredProjects {
				isValidProjectName = true
			}
		}
		if !isValidProjectName {
			return false
		}
	}

	return true
}

func DisplayProjectConfiguration(selectedProjects []string) {
	var projectsConfiguration string
	if len(selectedProjects) > 0 {
		projectsConfiguration = converter.ArrayIntoPrintableString(selectedProjects[:])
	} else {
		projectsConfiguration = converter.ArrayIntoPrintableString(ConfiguredProjects)
	}
	configuration.DisplayStringArgument(PROJECTS_LONG_FLAG, projectsConfiguration)
}

func AddProjectAsFlag(project *string) {
	flag.StringVar(project, PROJECTS_LONG_FLAG, *project, "")
	flag.StringVar(project, PROJECTS_SHORT_FLAG, *project, "")
}

type Project int64

type ProjectStructure struct {
	CCompiler       string
	Linker          string
	Folder          string
	CodeFolder      string
	Environment     string
	FloatOperations bool
}

type CommonConfig struct {
	CCompiler string
	Linker    string
}

var ELF = CommonConfig{
	CCompiler: "clang-19",
	Linker:    "ld.lld-19",
}

var EFI_SYSTEM = CommonConfig{
	CCompiler: "clang-19",
	Linker:    "lld-link-19",
}

// If you add a project add it here
const KERNEL = "kernel"
const EFI_TO_KERNEL = "efi-to-kernel"
const OS_LOADER = "os-loader"
const EFI = "efi"
const IMAGE_BUILDER = "image-builder"
const SHARED = "shared"
const POSIX = "posix"
const X86 = "x86"
const X86_EFI = "x86-efi"
const X86_MEMORY_PHYSICAL = "x86-memory-physical"
const X86_MEMORY_VIRTUAL = "x86-memory-virtual"
const X86_MEMORY_POLICY = "x86-memory-policy"
const EFI_UEFI = "efi-uefi"
const FREESTANDING = "freestanding"
const ABSTRACTION = "abstraction"

// and here
var kernelFolder = common.REPO_PROJECTS + "/" + KERNEL
var efiToKernelFolder = common.REPO_PROJECTS + "/" + EFI_TO_KERNEL
var osLoaderFolder = common.REPO_PROJECTS + "/" + OS_LOADER
var efiFolder = common.REPO_PROJECTS + "/" + EFI
var imageBuilderFolder = common.REPO_PROJECTS + "/" + IMAGE_BUILDER
var sharedFolder = common.REPO_PROJECTS + "/" + SHARED
var posixFolder = common.REPO_PROJECTS + "/" + POSIX
var x86Folder = common.REPO_PROJECTS + "/" + X86
var x86EfiFolder = common.REPO_PROJECTS + "/" + "x86/efi"
var x86MemoryPhysicalFolder = common.REPO_PROJECTS + "/" + "x86/memory/physical"
var x86MemoryVirtualFolder = common.REPO_PROJECTS + "/" + "x86/memory/virtual"
var x86MemoryPolicyFolder = common.REPO_PROJECTS + "/" + "x86/memory/policy"
var efiUefiFolder = common.REPO_PROJECTS + "/" + "efi/uefi"
var freestandingFolder = common.REPO_PROJECTS + "/" + FREESTANDING
var abstractionFolder = common.REPO_PROJECTS + "/" + ABSTRACTION

// and here
var PROJECT_STRUCTURES = map[string]*ProjectStructure{
	KERNEL: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          kernelFolder,
		CodeFolder:      kernelFolder + "/code",
		Environment:     string(environment.Freestanding),
		FloatOperations: true,
	},
	EFI_TO_KERNEL: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          efiToKernelFolder,
		CodeFolder:      efiToKernelFolder + "/code",
		Environment:     string(environment.Efi),
		FloatOperations: true,
	},
	OS_LOADER: {
		CCompiler:       EFI_SYSTEM.CCompiler,
		Linker:          EFI_SYSTEM.Linker,
		Folder:          osLoaderFolder,
		CodeFolder:      osLoaderFolder + "/code",
		Environment:     string(environment.Efi),
		FloatOperations: false,
	},
	EFI: {
		CCompiler:       EFI_SYSTEM.CCompiler,
		Linker:          EFI_SYSTEM.Linker,
		Folder:          efiFolder,
		CodeFolder:      efiFolder + "/code",
		Environment:     string(environment.Efi),
		FloatOperations: true,
	},
	IMAGE_BUILDER: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          imageBuilderFolder,
		CodeFolder:      imageBuilderFolder + "/code",
		Environment:     string(environment.Posix),
		FloatOperations: true,
	},
	SHARED: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          sharedFolder,
		CodeFolder:      sharedFolder + "/code",
		Environment:     string(environment.Freestanding),
		FloatOperations: true,
	},
	POSIX: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          posixFolder,
		CodeFolder:      posixFolder + "/code",
		Environment:     string(environment.Posix),
		FloatOperations: true,
	},
	X86: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          x86Folder,
		CodeFolder:      x86Folder + "/code",
		Environment:     string(environment.Freestanding),
		FloatOperations: true,
	},
	X86_MEMORY_PHYSICAL: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          x86MemoryPhysicalFolder,
		CodeFolder:      x86MemoryPhysicalFolder + "/code",
		Environment:     string(environment.Freestanding),
		FloatOperations: true,
	},
	X86_MEMORY_VIRTUAL: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          x86MemoryVirtualFolder,
		CodeFolder:      x86MemoryVirtualFolder + "/code",
		Environment:     string(environment.Freestanding),
		FloatOperations: true,
	},
	X86_MEMORY_POLICY: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          x86MemoryPolicyFolder,
		CodeFolder:      x86MemoryPolicyFolder + "/code",
		Environment:     string(environment.Freestanding),
		FloatOperations: true,
	},
	X86_EFI: {
		CCompiler:       EFI_SYSTEM.CCompiler,
		Linker:          EFI_SYSTEM.Linker,
		Folder:          x86EfiFolder,
		CodeFolder:      x86EfiFolder + "/code",
		Environment:     string(environment.Efi),
		FloatOperations: true,
	},
	EFI_UEFI: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          efiUefiFolder,
		CodeFolder:      efiUefiFolder + "/code",
		Environment:     string(environment.Freestanding),
		FloatOperations: true,
	},
	FREESTANDING: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          freestandingFolder,
		CodeFolder:      freestandingFolder + "/code",
		Environment:     string(environment.Freestanding),
		FloatOperations: true,
	},
	ABSTRACTION: {
		CCompiler:       ELF.CCompiler,
		Linker:          ELF.Linker,
		Folder:          abstractionFolder,
		CodeFolder:      abstractionFolder,
		Environment:     string(environment.Freestanding),
		FloatOperations: true,
	},
}

func getConfiguredProjects() []string {
	var result = make([]string, 0)

	for name := range PROJECT_STRUCTURES {
		result = append(result, name)
	}

	return result
}

var ConfiguredProjects = getConfiguredProjects()

func BuildOutputPath(architecture string, cCompiler string, linker string, environment string, buildMode string, floatOperations string) string {
	configurationPath := strings.Builder{}

	configurationPath.WriteString("build/")
	configurationPath.WriteString(fmt.Sprintf("%s/", architecture))
	configurationPath.WriteString(fmt.Sprintf("%s/", cCompiler))
	configurationPath.WriteString(fmt.Sprintf("%s/", linker))
	configurationPath.WriteString(fmt.Sprintf("%s/", environment))
	configurationPath.WriteString(fmt.Sprintf("%s/", floatOperations))
	configurationPath.WriteString(buildMode)

	return configurationPath.String()
}

func BuildDirectoryRoot(project *ProjectStructure, buildMode string, architecture string) string {
	buildDirectory := strings.Builder{}
	buildDirectory.WriteString(fmt.Sprintf("%s/", project.CodeFolder))

	var floatOperations = "with-floats"
	if !project.FloatOperations {
		floatOperations = "no-floats"
	}
	buildDirectory.WriteString(BuildOutputPath(architecture, project.CCompiler, project.Linker, string(project.Environment), buildMode, floatOperations))

	return buildDirectory.String()
}

func BuildProjectTargetsFile(codeFolder string) string {
	projectTargetsFile := strings.Builder{}
	projectTargetsFile.WriteString(fmt.Sprintf("%s/build/targets.txt", codeFolder))
	return projectTargetsFile.String()
}

func GetAllProjects(selectedProjects []string) map[string]*ProjectStructure {
	if len(selectedProjects) == 0 {
		return PROJECT_STRUCTURES
	}

	result := make(map[string]*ProjectStructure)

	for _, name := range selectedProjects {
		var project = PROJECT_STRUCTURES[name]
		result[name] = project
	}

	return result
}
