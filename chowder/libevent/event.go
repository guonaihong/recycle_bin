package libevent

const (
	EV_TIMEOUT = 1 << iota
	EV_READ
	EV_WRITE
	EV_SIGNAL
	EV_PERSIST
	EV_ET
	EV_FINALIZE
	EV_CLOSED
)
