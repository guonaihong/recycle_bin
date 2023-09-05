package libevent

const (
	EV_CHANGE_ADD = 1 << iota
	EV_CHANGE_DEL
	EV_CHANGE_SIGNAL  = EV_SIGNAL
	EV_CHANGE_PERSIST = EV_PERSIST
	EV_CHANGE_ET      = EV_ET
)

type EventChange struct {
	fd          int
	oldEvents   uint16
	readChange  uint8
	writeChange uint8
	closeChange uint8
}
