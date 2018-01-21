package stream

import (
	"core"
	"testing"
)

func Test_upstreamInitRoundRobin(t *testing.T) {

	cf := core.Conf{}
	us := upstreamSrvConf{}
	upstreamInitRoundRobin(&cf, &us)
}
