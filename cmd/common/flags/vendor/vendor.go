package vendor

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/converter"
	"cmd/common/flags"
	"flag"
	"fmt"
	"slices"
)

const VENDOR_LONG_FLAG = "vendor"
const VENDOR_SHORT_FLAG = "vn"

type BuildMode string

const (
	Intel BuildMode = "INTEL"
	Amd   BuildMode = "AMD"
)

var PossibleVendors = []string{
	string(Intel),
	string(Amd),
}

func IsValidVendor(mode string) bool {
	return slices.Contains(PossibleVendors, mode)
}

func DefaultVendor() string {
	return string(Amd)
}

func DisplayVendor() {
	// Not sure why go doesnt understand string lengths of this one, but whatever
	var vendorDescription = fmt.Sprintf("Set the vendor (%s%s%s)                            ", common.WHITE,
		converter.ArrayIntoPrintableString(PossibleVendors[:]), common.RESET)
	flags.DisplayArgumentInput(VENDOR_SHORT_FLAG, VENDOR_LONG_FLAG, vendorDescription, DefaultVendor())
}

func AddVendorAsFlag(vendor *string) {
	flag.StringVar(vendor, VENDOR_LONG_FLAG, *vendor, "")
	flag.StringVar(vendor, VENDOR_SHORT_FLAG, *vendor, "")
}

func DisplayVendorConfiguration(vendor string) {
	configuration.DisplayStringArgument(VENDOR_LONG_FLAG, vendor)
}
