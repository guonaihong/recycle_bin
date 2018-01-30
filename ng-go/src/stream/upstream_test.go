package stream

import (
	"core"
	"testing"
)

func TestUpstreamServer(t *testing.T) {
	cf := core.Conf{}

	u := upstreamSrvConf{}
	u.Flags = STREAM_UPSTREAM_FAIL_TIMEOUT
	cf.Args = []string{"server", "127.0.0.1:8001", "fail_timeout=3s"}
	UpstreamServer(&cf, nil, u)
}
