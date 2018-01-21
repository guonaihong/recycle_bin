package stream

import (
	"core"
	"fmt"
	"log"
)

func init() {
	log.Printf("::::::::::init")
	core.Register(&streamModule)
	core.Register(&proxyModule)
	core.Register(genUpstreamModule())
	core.Register(genCoreModule())
}

const (
	STREAM_MODULE    = 0x4d525453 /* "STRM" */
	STREAM_MAIN_CONF = 0x02000000
	STREAM_SRV_CONF  = 0x04000000
	STREAM_UPS_CONF  = 0x08000000
)

const (
	OK                    = 200
	BAD_REQUEST           = 400
	FORBIDDEN             = 403
	INTERNAL_SERVER_ERROR = 500
	BAD_GATEWAY           = 502
	SERVICE_UNAVAILABLE   = 503
)

func MAIN_CONF_OFFSET() uint {
	return 0
}

func SRV_CONF_OFFSET() uint {
	return 1
}

const (
	CORE_MODULE = 0x45524F43
	CONF_MODULE = 0x464E4F43
)

type streamAddrConf struct {
	Ctx           *streamConfCtx
	addrText      string
	ssl           bool
	proxyProtocol bool
}

type streamInAddr struct {
	conf streamAddrConf
}

type streamPort struct {
	addrs []streamInAddr
}

type streamListen struct {
	addr string
	//ngx_sockaddr_t                 sockaddr
	//socklen_t                      socklen

	/* server ctx */
	//ngx_stream_conf_ctx_t         *ctx

	Ctx           *streamConfCtx
	bind          bool
	wildcard      bool
	ssl           bool
	ipv6only      bool
	reuseport     bool
	soKeepalive   uint16
	proxyProtocol bool
	tcpKeepidle   int
	tcpKeepintvl  int
	tcpKeepcnt    int
	backlog       int
	Type          int
}

type coreMainConf struct {
	servers []coreSrvConf
	listen  []*streamListen

	//ngx_stream_phase_engine_t phase_engine

	//ngx_hash_t variables_hash

	//ngx_array_t variables        /* ngx_stream_variable_t */
	//ngx_array_t prefix_variables /* ngx_stream_variable_t */
	ncaptures uint

	variablesHashMaxSize    uint
	variablesHashBucketSize uint

	//ngx_hash_keys_arrays_t        *variables_keys

	//ngx_stream_phase_t             phases[NGX_STREAM_LOG_PHASE + 1]
}

type coreSrvConf struct {
	//ngx_stream_content_handler_pt handler

	Ctx *streamConfCtx

	file_name string
	line      uint

	tcpNodelay        bool
	prereadBufferSize uint
	prereadTimeout    uint

	//ngx_log_t *error_log

	resolverTimeout uint
	resolver        core.Resolver
	//ngx_resolver_t *resolver

	proxyProtocolTimeout uint

	listen bool
}

type streamSession struct {
	signature uint32 /* "STRM" */

	connection core.Connection
	received   uint
	startSec   uint
	startMsec  uint

	//ngx_log_handler_pt             log_handler

	//void                         **ctx

	MainConf []interface{}
	SrvConf  []interface{}

	upstream *Upstream
	//ngx_stream_upstream_t         *upstream
	upstreamStates []upstreamState
	//ngx_array_t                   *upstream_states
	/* of ngx_stream_upstream_state_t */
	//ngx_stream_variable_value_t *variables

	//phaseHandler

	status uint

	ssl            bool
	statProcessing bool

	healthCheck bool
}

type confPort struct {
	family int
	Type   int
	Port   uint16
	addrs  []confAddr
}

type confAddr struct {
	opt streamListen
}

type streamConfCtx struct {
	MainConf []interface{}
	SrvConf  []interface{}
}

type streamModuleFunc struct {
	preconfiguration  func(cf *core.Conf) int
	postconfiguration func(cf *core.Conf) int

	createMainConf func(cf *core.Conf) interface{}
	initMainConf   func(cf *core.Conf, conf interface{}) interface{}

	createSrvConf func(cf *core.Conf) interface{}
	mergeSrvConf  func(cf *core.Conf, prev interface{}, conf interface{}) interface{}
}

var streamCommand = []core.Command{
	{
		Name: "stream",
		Type: core.MAIN_CONF | core.CONF_BLOCK | core.CONF_NOARGS,
		Set:  streamBlock,
	},
}

var streamModule = core.Module{
	Name:     "stream",
	Commands: streamCommand,
	MainName: "stream",
	Type:     CORE_MODULE,
}

func getModuleMainConf(s *streamSession, module *core.Module) interface{} {
	return s.MainConf[module.CtxIndex]
}

func getModuleSrvConf(s *streamSession, module *core.Module) interface{} {
	return s.SrvConf[module.CtxIndex]
}

func confGetModuleMainConf(cf *core.Conf, module *core.Module) interface{} {
	var ctx *streamConfCtx

	ctx = cf.Ctx.(*streamConfCtx)

	return ctx.MainConf[module.CtxIndex]
}

func confGetModuleSrvConf(cf *core.Conf, module *core.Module) interface{} {
	var ctx *streamConfCtx

	ctx = cf.Ctx.(*streamConfCtx)

	return ctx.SrvConf[module.CtxIndex]

}

func streamBlock(cf *core.Conf, cmd *core.Command, conf interface{}) (int, error) {

	var ctx2 *interface{}
	var ctx streamConfCtx
	var module streamModuleFunc
	var err error
	var rv int
	var ok bool

	ctx2 = conf.(*interface{})
	*ctx2 = &ctx

	modules := cf.Cycle.GetMainModules("stream")
	ctx.MainConf = make([]interface{}, len(modules))
	ctx.SrvConf = make([]interface{}, len(modules))
	log.Printf("streamConfCtx address is %p: MainConf = %p, SrvConf = %p, &MainConf = %p, &SrvConf = %p\n",
		&ctx, ctx.MainConf, ctx.SrvConf, &ctx.MainConf, &ctx.SrvConf)

	for k, v := range modules {
		if v.Ctx == nil {
			continue
		}

		log.Printf("v.Ctx = %p\n", v.Ctx)
		module, ok = v.Ctx.(streamModuleFunc)
		if !ok {
			continue
		}
		if module.createMainConf != nil {
			ctx.MainConf[k] = module.createMainConf(cf)
			if ctx.MainConf[k] == nil {
				return core.ERROR, fmt.Errorf("create main conf fail, in %s", v.Name)
			}
		}

		if module.createSrvConf != nil {
			ctx.SrvConf[k] = module.createSrvConf(cf)
			if ctx.SrvConf[k] == nil {
				return core.ERROR, fmt.Errorf("create srv conf fail, in %s", v.Name)
			}
		}
	}

	pcf := *cf
	cf.Ctx = &ctx

	for _, v := range modules {
		if v.Ctx == nil {
			continue
		}

		module, ok = v.Ctx.(streamModuleFunc)
		if !ok {
			continue
		}
		if module.preconfiguration != nil {
			if module.preconfiguration(cf) != core.OK {
				return core.ERROR, fmt.Errorf("preconfiguration fail, in %s", v.Name)
			}
		}

	}

	cf.ModuleType = STREAM_MODULE
	cf.CmdType = STREAM_MAIN_CONF
	rv, err = core.ConfParse(cf, "")

	if rv != core.OK {
		*cf = pcf
		return rv, err
	}

	cmcf := ctx.MainConf[coreModule.CtxIndex].(*coreMainConf)

	cscfp := cmcf.servers

	for _, v := range modules {
		if v.Ctx == nil {
			continue
		}

		module, ok = v.Ctx.(streamModuleFunc)
		if !ok {
			continue
		}

		mi := v.CtxIndex

		/* init stream{} main_conf's */

		cf.Ctx = ctx

		if module.initMainConf != nil {

			//TODO
			rvc := module.initMainConf(cf, ctx.MainConf[mi])
			if rvc != nil {
				*cf = pcf
				return core.ERROR, fmt.Errorf("init main conf fail")
			}
		}

		for s := 0; s < len(cmcf.servers); s++ {

			/* merge the server{}s' SrvConf's */

			cf.Ctx = cscfp[s].Ctx

			if module.mergeSrvConf != nil {
				rvc := module.mergeSrvConf(cf,
					ctx.SrvConf[mi],
					cscfp[s].Ctx.SrvConf[mi])
				if rvc != nil {
					*cf = pcf
					return core.ERROR, fmt.Errorf("merge srv conf fail")
				}
			}
		}
	}

	/*
		if ngx_stream_init_phases(cf, cmcf) != core.OK {
			return core.ERROR
		}
	*/

	for _, v := range modules {
		if v.Ctx == nil {
			continue
		}

		module, ok = v.Ctx.(streamModuleFunc)
		if !ok {
			continue
		}

		if module.postconfiguration != nil {
			if module.postconfiguration(cf) != core.OK {
				return core.ERROR, fmt.Errorf("post configuration fail")
			}
		}
	}

	/*
		if (ngx_stream_variables_init_vars(cf) != core.OK) {
			return core.ERROR
		}
	*/

	*cf = pcf

	/*
		if (ngx_stream_init_phase_handlers(cf, cmcf) != core.OK) {
			return core.ERROR
		}
	*/

	ports := make([]confPort, 0, 4)

	for i := 0; i < len(cmcf.listen); i++ {
		if streamAddPorts(cf, &ports, cmcf.listen[i]) != core.OK {
			return core.ERROR, fmt.Errorf("stream add port fail")
		}
	}

	//return ngx_stream_optimize_servers(cf, &ports)

	return core.OK, nil
}

func streamOptimizeServers(cf *core.Conf, ports *[]confPort) int {

	//ngx_listening_t * ls
	//ngx_stream_port_t * stport
	//ngx_stream_conf_port_t      *port
	//ngx_stream_conf_addr_t      *addr
	//ngx_stream_coreSrvConf * cscf

	var bindWildcard bool
	var i, p, last int
	var ls *core.Listening
	var stport *streamPort
	var port []confPort
	var addr []confAddr
	var cscf *coreSrvConf

	port = *ports
	for p = 0; p < len(port); p++ {

		// todo sort
		/*
		   ngx_sort(port[p].addrs.elts, (size_t) port[p].addrs.nelts,
		            sizeof(ngx_stream_conf_addr_t), ngx_stream_cmp_conf_addrs)
		*/

		addr = port[p].addrs
		last = len(port[p].addrs)

		/*
		 * if there is the binding to the "*:port" then we need to bind()
		 * to the "*:port" only and ignore the other bindings
		 */

		if addr[last-1].opt.wildcard {
			addr[last-1].opt.bind = true
			bindWildcard = true

		} else {
			bindWildcard = false
		}

		i = 0

		for i < last {

			if bindWildcard && !addr[i].opt.bind {
				i++
				continue
			}

			ls = core.CreateListening(cf, "tcp", addr[i].opt.addr)
			if ls == nil {
				return core.ERROR
			}

			//ls.addrNtop = 1
			ls.Handler = streamInitConnection
			//ls.Type = addr[i].opt.Type

			cscf = addr[0].opt.Ctx.SrvConf[coreModule.CtxIndex].(*coreSrvConf)

			//TODO
			//ls.logp = cscf.error_log
			//ls.log.data = &ls.addrText
			//ls.log.handler = ngx_accept_log_error

			//ls.backlog = addr[i].opt.backlog

			ls.Wildcard = addr[i].opt.wildcard

			ls.Keepalive = addr[i].opt.soKeepalive

			ls.Keepidle = addr[i].opt.tcpKeepidle
			ls.Keepintvl = addr[i].opt.tcpKeepintvl
			ls.Keepcnt = addr[i].opt.tcpKeepcnt

			ls.Ipv6only = addr[i].opt.ipv6only

			ls.Reuseport = addr[i].opt.reuseport

			stport = &streamPort{}
			if stport == nil {
				return core.ERROR
			}

			ls.servers = stport

			stport.naddrs = i + 1

			/*
				switch ls.sockaddr.sa_family {
				case syscall.AF_INET6:
					if ngx_stream_add_addrs6(cf, stport, addr) != core.OK {
						return core.ERROR
					}
					break
				default:
					if ngx_stream_add_addrs(cf, stport, addr) != core.OK {
						return core.ERROR
					}
					break
				}
			*/

			/*
				if ngx_clone_listening(cf, ls) != core.OK {
					return core.ERROR
				}
			*/

			addr = addr[1:]
			last--
		}
	}

	return core.OK
}

func streamAddPorts(cf *core.Conf, ports *[]confPort, listen *streamListen) int {
	var p uint16
	//var i uint
	//struct sockaddr         *sa
	var port *confPort
	var addr confAddr

	//sa = &listen.sockaddr.sockaddr
	//p = ngx_inet_get_port(sa)

	for _, v := range *ports {
		//for (i = 0; i < ports.nelts; i++) {

		/*
			if (p == v.port
			&& listen.Type == v.Type
			&& sa.sa_family == v.family)
		*/

		if p == v.Port {
			/* a port is already in the port list */

			port = &v
			goto found
		}
	}

	/* add a port to the port list */

	port = &confPort{}
	//port.family = sa.sa_family
	port.Type = listen.Type
	port.Port = p

found:

	addr.opt = *listen
	port.addrs = append(port.addrs, addr)

	*ports = append(*ports, *port)
	return core.OK
}
