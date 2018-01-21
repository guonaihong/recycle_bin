package stream

import (
	"core"
	"errors"
	"fmt"
	"log"
	"strconv"
)

const (
	UPSTREAM_NOTIFY_CONNECT = 0x1
)

const (
	UPSTREAM_CREATE       = 0x0001
	UPSTREAM_WEIGHT       = 0x0002
	UPSTREAM_MAX_FAILS    = 0x0004
	UPSTREAM_FAIL_TIMEOUT = 0x0008
	UPSTREAM_DOWN         = 0x0010
	UPSTREAM_BACKUP       = 0x0020
	UPSTREAM_MAX_CONNS    = 0x0100
)

type addr struct {
	name string
}

type upstreamInitPt func(cf *core.Conf, us *upstreamSrvConf) int
type upstreamInitPeerPt func(s *streamSession, us *upstreamSrvConf) int

/*
typedef ngx_int_t (*ngx_stream_upstream_init_pt)(ngx_conf_t *cf,
    ngx_stream_upstream_srv_conf_t *us);
*/

type upstreamMainConf struct {
	upstreams []upstreamSrvConf
}

type upstreamPeer struct {
	initUpstream upstreamInitPt
	init         upstreamInitPeerPt
	data         interface{}
}

type upstreamServer struct {
	name        string
	addrs       []core.Addr
	naddrs      uint
	weight      uint
	maxConns    uint
	maxFails    uint
	failTimeout uint
	slowStart   uint
	down        bool
	backup      bool
}

type upstreamSrvConf struct {
	peer    upstreamPeer
	SrvConf []interface{}

	servers []upstreamServer

	Flags    uint
	Host     string
	FileName string
	Line     uint
	Port     int
	NoPort   bool
}

type upstreamState struct {
	responseTime  int64
	connectTime   int64
	firstByteTime int64
	bytesSent     int64
	bytesReceived int64

	peer string
}

type Resolved struct {
	host   string
	port   int
	NoPort bool

	addrs []core.ResolverAddr
	//struct sockaddr                   *sockaddr;
	//socklen_t                          socklen;
	name string

	ctx *core.ResolverCtx
	//ngx_resolver_ctx_t                *ctx;
}

type Upstream struct {
	peer core.PeerConnection

	downstreamBuf core.Buffer
	upstreamBuf   core.Buffer

	free           *core.Chain
	upstreamOut    *core.Chain
	upstreamBusy   *core.Chain
	downstreamOut  *core.Chain
	downstreamBusy *core.Chain

	received  int
	startSec  int64
	responses uint
	sslName   string

	upstream *upstreamSrvConf
	state    upstreamState
	resolved *Resolved
	//ngx_stream_upstream_resolved_t    *resolved;
	connected     bool
	proxyProtocol bool
}

var upstreamCommands = []core.Command{
	{
		Name:   "upstream",
		Type:   STREAM_MAIN_CONF | core.CONF_BLOCK | core.CONF_TAKE1,
		Set:    UpstreamCmd,
		Conf:   0,
		Offset: 0,
		Post:   nil,
	},
	{
		Name:   "server",
		Type:   STREAM_UPS_CONF | core.CONF_1MORE,
		Set:    UpstreamServer,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		Post:   nil,
	},
}

var upstreamMoudleCtx = streamModuleFunc{
	createMainConf: upstreamCreateMainConf,
	initMainConf:   upstreamInitMainConf,
}

var upstreamModule = core.Module{
	Name: "upstream",
	Ctx:  upstreamMoudleCtx,
	//Commands: upstreamCommands,
	MainName: "stream",
	Type:     STREAM_MODULE,
}

func genUpstreamModule() *core.Module {
	upstreamModule.Commands = upstreamCommands
	return &upstreamModule
}

func upstreamAdd(cf *core.Conf, u *core.URL, flags uint) *upstreamSrvConf {
	var umcf *upstreamMainConf
	var uscf *upstreamSrvConf
	//var uscfp []upstreamSrvConf

	if (flags & UPSTREAM_CREATE) == 0 {

		if core.ParseUrl(u) != core.OK {
			if u.Err != "" {
				log.Printf("%s in upstream \"%v\"", u.Err, u.Url)
			}

			return nil
		}
	}

	umcf = confGetModuleMainConf(cf, &upstreamModule).(*upstreamMainConf)

	//uscfp = umcf.upstreams

	for _, v := range umcf.upstreams {
		if v.Host != u.Host {
			continue
		}

		if (flags&UPSTREAM_CREATE) > 0 && (v.Flags&UPSTREAM_CREATE) > 0 {
			log.Printf("duplicate upstream \"%V\"", &u.Host)
			return nil
		}

		if (v.Flags&UPSTREAM_CREATE) > 0 && !u.NoPort {
			log.Printf("upstream \"%v\" may not have port %d", &u.Host, u.Port)
			return nil
		}

		if (flags&UPSTREAM_CREATE) > 0 && v.NoPort {
			log.Printf("upstream \"%v\" may not have port %d in %s:%ui",
				u.Host, v.Port, v.FileName, v.Line)
			return nil
		}

		if v.Port != u.Port {
			continue
		}

		if (flags & UPSTREAM_CREATE) > 0 {
			v.Flags = flags
		}

		return &v
	}

	uscf = &upstreamSrvConf{
		Flags:    flags,
		Host:     u.Host,
		FileName: cf.ConfFile.File.Name(),
		Line:     cf.ConfFile.Line,
		Port:     u.Port,
		NoPort:   u.NoPort,
	}

	if len(u.Addrs) == 1 {
		log.Printf("u.Addrs = %s\n", u.Addrs)
		uscf.servers = make([]upstreamServer, 1)
		if uscf.servers == nil {
			return nil
		}

		uscf.servers[0].addrs = u.Addrs
	}

	log.Printf("1.uscf = %p\n", uscf)
	umcf.upstreams = append(umcf.upstreams, *uscf)
	return uscf
}

func UpstreamCmd(cf *core.Conf, cmd *core.Command, conf interface{}) (int, error) {
	//var i uint
	var uscf *upstreamSrvConf
	var u core.URL
	var ctx streamConfCtx
	var streamCtx *streamConfCtx
	var module streamModuleFunc
	var mconf interface{}

	value := cf.Args
	u.Host = value[1]
	u.NoResolve = true
	u.NoPort = true

	uscf = upstreamAdd(cf, &u, UPSTREAM_CREATE|
		UPSTREAM_WEIGHT|
		UPSTREAM_MAX_CONNS|
		UPSTREAM_MAX_FAILS|
		UPSTREAM_FAIL_TIMEOUT|
		UPSTREAM_DOWN|
		UPSTREAM_BACKUP)

	if uscf == nil {
		return core.ERROR, errors.New("upstream add fail")
	}

	streamCtx = cf.Ctx.(*streamConfCtx)
	ctx.MainConf = streamCtx.MainConf
	ctx.SrvConf = make([]interface{}, len(streamCtx.SrvConf))

	if ctx.SrvConf == nil {
		return core.ERROR, errors.New("oom")
	}

	ctx.SrvConf[upstreamModule.CtxIndex] = uscf
	log.Printf("xxxxxxx cf.Ctx = %p, ctx.MainConf = %p, ctx.SrvConf = %p\n", &ctx, ctx.MainConf, ctx.SrvConf)
	log.Printf("uscf save:%p\n", ctx.SrvConf[upstreamModule.CtxIndex])
	uscf.SrvConf = ctx.SrvConf

	modules := cf.Cycle.GetMainModules("stream")
	for _, v := range modules {
		//log.Printf("current module %v\n", *v)
		if v.Type != STREAM_MODULE {
			log.Printf("v.type(%x) != stream_module(%x)", v.Type, STREAM_MODULE)
			continue
		}

		//log.Printf(":::v.Ctx = %p\n", v.Ctx)
		if v.Ctx == nil {
			continue
		}

		module = v.Ctx.(streamModuleFunc)

		if module.createSrvConf != nil {
			mconf = module.createSrvConf(cf)
			if mconf == nil {
				return core.ERROR, errors.New("module create srv conf fail")
			}

			ctx.SrvConf[v.CtxIndex] = mconf
		}
	}

	/*
		uscf->servers = ngx_array_create(cf->pool, 4,
		sizeof(ngx_stream_upstream_server_t));
		if (uscf->servers == NULL) {
			return NGX_CONF_ERROR;
		}
	*/

	/* parse inside upstream{} */

	pcf := *cf
	cf.Ctx = &ctx
	cf.CmdType = STREAM_UPS_CONF

	rv, err := core.ConfParse(cf, "")

	*cf = pcf

	if rv != core.OK || err != nil {
		return rv, err
	}

	if len(uscf.servers) == 0 {
		log.Printf("no servers are inside upstream")
		return core.ERROR, errors.New("no servers are inside upstream")
	}

	//ctx := &streamConfCtx{}
	return core.OK, nil
}

func UpstreamServer(cf *core.Conf, cmd *core.Command, conf interface{}) (int, error) {

	var err error
	var errMsg string

	uscf, ok := conf.(*upstreamSrvConf)
	if !ok {

	}

	failTimeout := 0
	u := core.URL{}
	weight, maxConns, maxFails := 0, 0, 0
	us := &upstreamServer{}

	value := cf.Args

	weight = 1
	maxConns = 0
	maxFails = 1
	failTimeout = 10

	//log.Printf("2.conf = %#v, uscf = %v\n", conf, &uscf)
	i := 2
	for l := len(cf.Args); i < l; i++ {

		log.Printf("value = (%s)\n", value[i])

		if len(value[i]) > 7 && value[i][:7] == "weight=" {
			if (uscf.Flags & UPSTREAM_WEIGHT) != UPSTREAM_WEIGHT {
				goto not_supported
			}

			weight, err = strconv.Atoi(value[i][7:])

			if err != nil || weight == 0 {
				goto invalid
			}

			continue
		}

		if len(value[i]) > 10 && value[i][:10] == "max_conns=" {

			if (uscf.Flags & UPSTREAM_MAX_CONNS) != UPSTREAM_MAX_CONNS {
				goto not_supported
			}

			maxConns, err = strconv.Atoi(value[i][10:])

			if err != nil {
				goto invalid
			}

			continue
		}

		if len(value[i]) > 10 && value[i][:10] == "max_fails=" {

			if (uscf.Flags & UPSTREAM_MAX_FAILS) != UPSTREAM_MAX_FAILS {
				goto not_supported
			}
			maxFails, err = strconv.Atoi(value[i][10:])

			if err != nil {
				goto invalid
			}

			continue
		}

		if len(value[i]) > 13 && value[i][:13] == "fail_timeout=" {

			if (uscf.Flags & UPSTREAM_FAIL_TIMEOUT) != UPSTREAM_FAIL_TIMEOUT {
				goto not_supported
			}

			failTimeout = core.ParseTime(value[i][13:], true)

			if failTimeout == core.ERROR {
				goto invalid
			}

			continue
		}

		if value[i] == "backup" {

			if (uscf.Flags & UPSTREAM_BACKUP) != UPSTREAM_BACKUP {
				goto not_supported
			}

			us.backup = true

			continue
		}

		if value[i] == "down" {

			if (uscf.Flags & UPSTREAM_DOWN) != UPSTREAM_DOWN {
				goto not_supported
			}

			us.down = true

			continue
		}

		goto invalid

	}

	u.Url = value[1]

	if core.ParseUrl(&u) != core.OK {
		if u.Err != "" {
			log.Printf("%s in upstream \"%V\"", u.Err, u.Url)
		}

		return core.ERROR, errors.New(u.Err)
	}

	if u.NoPort {
		errMsg := fmt.Sprintf("no port in upstream \"%s\"", value[1])
		log.Printf("%s\n", errMsg)
		return core.ERROR, errors.New(errMsg)
	}

	log.Printf("###:%s\n", u.Addrs)
	us.name = u.Url
	us.addrs = u.Addrs
	us.weight = uint(weight)
	us.maxConns = uint(maxConns)
	us.maxFails = uint(maxFails)
	us.failTimeout = uint(failTimeout)

	uscf.servers = append(uscf.servers, *us)
	log.Printf("us(%#v)\n", us)
	return core.OK, nil

invalid:

	errMsg = fmt.Sprintf("invalid parameter \"%s\"", value[i])

	log.Printf("%s\n", errMsg)
	return core.ERROR, errors.New(errMsg)

not_supported:

	errMsg = fmt.Sprintf("balancing method does not support parameter \"%s\"",
		value[i])
	log.Printf("%s\n", errMsg)

	return core.ERROR, errors.New(errMsg)

}

func upstreamCreateMainConf(cf *core.Conf) interface{} {

	umcf := &upstreamMainConf{}
	if umcf == nil {
		return nil
	}

	return umcf
}

func upstreamInitMainConf(cf *core.Conf, conf interface{}) interface{} {

	umcf := conf.(upstreamMainConf)
	i := 0
	var initFunc upstreamInitPt

	for i = 0; i < len(umcf.upstreams); i++ {

		initFunc = umcf.upstreams[i].peer.initUpstream
		if initFunc == nil {
			initFunc = upstreamInitRoundRobin
		}

		if initFunc(cf, &umcf.upstreams[i]) != core.OK {
			return core.ERROR
		}
	}

	return core.OK
}
