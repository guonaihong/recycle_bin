package redis

import (
	"container/list"
	"time"
)

type TimeEvent struct {
	id            int
	when          time.Time
	timeProc      TimeProc
	finalizerProc EventFinalizerProc
	clientData    interface{}
	refcount      int
}

func createTimeEvent() *list.List {
	return list.New()
}

func addTimeEvent(head *list.List, id int, when time.Duration, proc TimeProc, clientData interface{}, finalizerProc EventFinalizerProc) {
	te := &TimeEvent{
		id:            id,
		when:          time.Now().Add(when),
		timeProc:      proc,
		finalizerProc: finalizerProc,
		clientData:    clientData,
	}

	head.PushFront(te)
}
