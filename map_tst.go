package main

import (
	"fmt"
	_ "math/rand"
	"strconv"
	"sync"
	"time"
)

const (
	C_STILL  = 0x0
	C_READ   = 0x1
	C_WRITE  = 0x2
	C_DELETE = 0x4
)

type cacheStatus struct {
	sync.RWMutex
	status     int32
	lastModify time.Time
}

type AudioCache struct {
	sync.Mutex
	m map[string]*cacheStatus
}

func CacheNew(size int) *AudioCache {
	a := &AudioCache{}
	a.m = make(map[string]*cacheStatus, 1000)
	return a
}

func (a *AudioCache) Create(k string) *cacheStatus {
	a.Lock()
	v, ok := a.m[k]
	if !ok {
		v = &cacheStatus{status: C_WRITE, lastModify: time.Now()}
		a.m[k] = v
	} else {
		a.m[k].lastModify = time.Now()
	}
	a.Unlock()
	return v
}

func (a *AudioCache) Read(k string) *cacheStatus {
	a.Lock()
	v, ok := a.m[k]
	if !ok {
		v = &cacheStatus{status: C_READ, lastModify: time.Now()}
		a.m[k] = v
	} else {
		a.m[k].lastModify = time.Now()
	}
	a.Unlock()
	return v
}

func (a *AudioCache) Update(k string) *cacheStatus {
	return a.Create(k)
}

func (a *AudioCache) Clear(k string, status int) {
	a.Lock()
	v, ok := a.m[k]
	if ok {
		v.lastModify = time.Now()
		v.status = v.status & int32(^status)
	}
	a.Unlock()
}

func (a *AudioCache) Delete(k string) {
	a.Lock()
	v, ok := a.m[k]
	if ok {
		v.lastModify = time.Now()
		if v.status == 0 {
			v.status = C_DELETE
		}
	}
	a.Unlock()

	if v != nil {
		v.Lock()
		v.status = C_DELETE
		v.Unlock()
	}
}

func (a *AudioCache) GC() {

	i := 1000
	a.Lock()
	for k, v := range a.m {
		if i == 0 {
			break
		}

		if (v.status&C_DELETE == C_DELETE || v.status == 0) &&
			time.Now().Sub(v.lastModify).Seconds() > 60 {
			delete(a.m, k)
		}

		i--
	}
	a.Unlock()
}

func main() {
	c := CacheNew(1)

	var wg sync.WaitGroup

	for i := 0; i < 100000; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			k := strconv.Itoa(i)
			//k := fmt.Sprintf("%d", i)
			v := c.Create(k)
			v.Lock()
			//time.Sleep(time.Millisecond * 100)
			fmt.Printf("key(%d) create ok\n", i)
			v.Unlock()

		}(i)
	}
	for i := 0; i < 100000; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			//k := fmt.Sprintf("%d", i)
			k := strconv.Itoa(i)
			v := c.Read(k)
			v.RLock()
			//time.Sleep(time.Millisecond * 100)
			fmt.Printf("key(%d) read ok\n", i)
			v.RUnlock()
		}(i)
	}

	wg.Wait()
}
