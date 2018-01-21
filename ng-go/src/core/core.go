package core

const (
	OK       = 0
	ERROR    = -1
	AGAIN    = -2
	BUSY     = -3
	DONE     = -4
	DECLINED = -5
	ABORT    = -6
)

type Command struct {
	Name     string
	Type     uint
	Set      func(c *Conf, cmd *Command, conf interface{}) (int, error)
	Conf     uint
	Offset   uint
	Post     interface{}
	CtxIndex uint
}
