package graphic

import (
	"cmd/common/configuration"
	"cmd/common/flags"
	"flag"
	"fmt"
)

const GRAPHIC_LONG_FLAG = "graphic"
const GRAPHIC_SHORT_FLAG = "g"

func DefaultGraphic() bool {
	return false
}

func DisplayGraphic() {
	flags.DisplayArgumentInput(GRAPHIC_SHORT_FLAG, GRAPHIC_LONG_FLAG, "Enable graphical display", fmt.Sprint(DefaultGraphic()))
}

func AddGraphicAsFlag(graphic *bool) {
	flag.BoolVar(graphic, GRAPHIC_LONG_FLAG, *graphic, "")
	flag.BoolVar(graphic, GRAPHIC_SHORT_FLAG, *graphic, "")
}

func DisplayGraphicConfiguration(graphic bool) {
	configuration.DisplayBoolArgument(GRAPHIC_LONG_FLAG, graphic)
}
