// +build linux

package libevent

import (
	"golang.org/x/sys/unix"
)

const (
	INITIAL_NEVENT = 32
)

type op struct {
	epfd   int
	events []unix.EpollEvent
	// TODO timerfd
}

func (e *EventLoop) init() (op, error) {
	e := &epollop{}
	epfd, err := unix.EpollCreate1(0)
	if err != nil {
		return nil, err
	}

	e.events = make(unix.EpollEvent, INITIAL_NEVENT)
	e.epfd = epfd
	return e, nil
}

func (e *EventLoop) noChangelistAdd(fd int, old uint16, events uint16) {
	var ch EventChange
	ch.fd = fd
	ch.oldEvents = old

	if events & EV_WRITE {
		ch.writeChange = EV_CHANGE_ADD | (events & EV_ET)
	}

	if events & EV_READ {
		ch.readChange = EV_CHANGE_ADD | (events & EV_ET)
	}

	if events & EV_CLOSED {
		ch.closeChange = EV_CHANGE_ADD | (events & EV_ET)
	}
}

func (e *EventLoop) applyOneChange() {
}
