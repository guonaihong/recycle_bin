package stream

import (
	"core"
	"errors"
	"fmt"
	"log"
	"strings"
	"syscall"
	"time"
)

type upstreamLocal struct {
	addr core.Addr
	//ngx_stream_complex_value_t *value
	transparent bool
}

type proxySrvConf struct {
	connectTimeout      uint
	timeout             uint
	nextUpstreamTimeout uint
	bufferSize          uint
	uploadRate          uint
	downloadRate        uint
	responses           uint
	nextUpstreamTries   uint
	nextUpstream        uint
	proxyProtocol       bool
	local               *upstreamLocal

	sslEnable       bool
	sslSessionReuse bool
	sslProtocols    uint
	sslCiphers      string
	//ngx_stream_complex_value_t *ssl_name
	sslServerName uint

	sslVerify             uint
	sslVerifyDepth        uint
	sslTrustedCertificate string
	sslCrl                string
	sslCertificate        string
	sslCertificateKey     string
	//ngx_array_t             *ssl_passwords

	//ngx_ssl_t *ssl

	upstream *upstreamSrvConf
	//ngx_stream_complex_value_t     *upstream_value
}

var proxyCommands = []core.Command{
	{
		Name:   "proxy_pass",
		Type:   STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    proxyPass,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		Post:   nil,
	},
	{
		Name:   "proxy_bind",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE12,
		Set:    proxyBind,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		Post:   nil,
	},
	{
		Name:   "proxy_connect_timeout",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    core.ConfSetMsecSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		Post:   nil,
	},
	{
		Name:   "proxy_timeout",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    core.ConfSetMsecSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		//offsetof(ngx_stream_proxy_srv_conf_t, timeout),
		Post: nil,
	},
	{
		Name:   "proxy_buffer_size",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    core.ConfSetSizeSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		Post:   nil,
		//offsetof(ngx_stream_proxy_srv_conf_t, buffer_size),
	},

	{
		Name:   "proxy_downstream_buffer",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    core.ConfSetSizeSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		//Offset: offsetof(ngx_stream_proxy_srv_conf_t, buffer_size),
		Post: nil,
		//&ngx_conf_deprecated_proxy_downstream_buffer,
	},
	{
		Name:   "proxy_upstream_buffer",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    core.ConfSetSizeSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		Post:   nil,
		//offsetof(ngx_stream_proxy_srv_conf_t, buffer_size),
		//&ngx_conf_deprecated_proxy_upstream_buffer,
	},
	{
		Name:   "proxy_upload_rate",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    core.ConfSetSizeSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		//Offset:offsetof(ngx_stream_proxy_srv_conf_t, upload_rate),
		Post: nil,
	},
	{
		Name:   "proxy_download_rate",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    core.ConfSetSizeSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		//offsetof(ngx_stream_proxy_srv_conf_t, download_rate),
		Post: nil,
	},
	{
		Name:   "proxy_responses",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    core.ConfSetNumSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		//Offset:offsetof(ngx_stream_proxy_srv_conf_t, responses),
		Post: nil,
	},
	{
		Name:   "proxy_next_upstream",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_FLAG,
		Set:    core.ConfSetFlagSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		//Offset: offsetof(ngx_stream_proxy_srv_conf_t, next_upstream),
		Post: nil,
	},
	{
		Name:   "proxy_next_upstream_tries",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    core.ConfSetNumSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		//Offset: offsetof(ngx_stream_proxy_srv_conf_t, next_upstream_tries),
		Post: nil,
	},
	{Name: "proxy_next_upstream_timeout",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_TAKE1,
		Set:    core.ConfSetMsecSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		//Offset: offsetof(ngx_stream_proxy_srv_conf_t, next_upstream_timeout),
		Post: nil,
	},
	{
		Name:   "proxy_protocol",
		Type:   STREAM_MAIN_CONF | STREAM_SRV_CONF | core.CONF_FLAG,
		Set:    core.ConfSetFlagSlot,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		//offsetof(ngx_stream_proxy_srv_conf_t, proxy_protocol),
		Post: nil,
	},
}

var proxyModuleCtx = streamModuleFunc{
	createSrvConf: proxyCreateSrvConf,
	mergeSrvConf:  proxyMergeSrvConf,
}

var proxyModule = core.Module{
	Name:     "proxy",
	Ctx:      proxyModuleCtx,
	Commands: proxyCommands,
	MainName: "stream",
	Type:     STREAM_MODULE,
}

func proxyHandler(s *streamSession) {
	//ngx_resolver_ctx_t               *ctx, temp;
	//ngx_stream_upstream_srv_conf_t   *uscf, **uscfp;

	var temp core.ResolverCtx
	var ctx *core.ResolverCtx
	var cscf *coreSrvConf
	var uscf *upstreamSrvConf
	var umcf *upstreamMainConf
	var uscfp []upstreamSrvConf
	var pscf *proxySrvConf
	var p []byte
	u := &Upstream{}
	host := ""
	i := 0
	//c := s.connection

	pscf = getModuleSrvConf(s, &proxyModule).(*proxySrvConf)

	log.Printf("proxy connection handler")

	//u := make(streamUpstream)
	//u = ngx_pcalloc(c.pool, sizeof(ngx_stream_upstream_t));
	if u == nil {
		proxyFinalize(s, INTERNAL_SERVER_ERROR)
		//proxyFinalize(s, INTERNAL_SERVER_ERROR);
		return
	}

	s.upstream = u

	//s.log_handler = ngx_stream_proxy_log_error;

	//u.peer.log = c.log;
	//u.peer.log_error = core.ERROR_ERR;

	if proxySetLocal(s, u, pscf.local) != core.OK {
		proxyFinalize(s, INTERNAL_SERVER_ERROR)
		return
	}

	//u.peer.Type = c.Type
	u.startSec = time.Now().UnixNano()

	//c.write.handler = proxyDownstreamHandler;
	//c.read.handler = proxyDownstreamHandler;

	s.upstreamStates = []upstreamState{upstreamState{}}
	if s.upstreamStates == nil {
		proxyFinalize(s, INTERNAL_SERVER_ERROR)
		return
	}

	//TODO socket判断是tcp
	if true {
		//if c.Type == syscall.SOCK_STREAM {
		p = make([]byte, pscf.bufferSize)
		if p == nil {
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}

		u.downstreamBuf.Start = 0
		u.downstreamBuf.End = len(p)
		u.downstreamBuf.Pos = 0
		u.downstreamBuf.Last = 0

		/*
			if (c.read.ready) {
				ngx_post_event(c.read, &ngx_posted_events);
			}
		*/
	}

	/*
		if (pscf.upstream_value) {
			if (ngx_stream_proxy_eval(s, pscf) != core.OK) {
				proxyFinalize(s, INTERNAL_SERVER_ERROR);
				return;
			}
		}
	*/

	if u.resolved == nil {

		uscf = pscf.upstream

	} else {

		u.sslName = u.resolved.host

		host = u.resolved.host

		umcf = getModuleMainConf(s, &proxyModule).(*upstreamMainConf)
		//umcf = getModuleMainConf(s, ngx_stream_upstream_module);

		uscfp = umcf.upstreams

		for i = 0; i < len(umcf.upstreams); i++ {

			uscf = &uscfp[i]

			if len(uscf.Host) == len(host) &&
				((uscf.Port == 0 && u.resolved.NoPort) ||
					uscf.Port == u.resolved.port) &&
				strings.ToLower(uscf.Host) == strings.ToLower(host) {
				goto found
			}
		}

		//TODO:检查socket地址的合法性
		if false {

			if u.resolved.port == 0 {
				//TODO:检查地址的family
				//u.resolved.sockaddr.sa_family != syscall.AF_UNIX
				log.Printf("no port in upstream \"%V\"", host)
				proxyFinalize(s, INTERNAL_SERVER_ERROR)
				return
			}

			if upstreamCreateRoundRobinPeer(s, u.resolved) != core.OK {
				proxyFinalize(s, INTERNAL_SERVER_ERROR)
				return
			}

			proxyConnect(s)

			return
		}

		if u.resolved.port == 0 {
			log.Printf("no port in upstream \"%V\"", host)
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}

		temp.Name = host

		cscf = getModuleSrvConf(s, &coreModule).(*coreSrvConf)

		ctx = resolveStart(&cscf.resolver, &temp)
		if ctx == nil {
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}

		if ctx == nil {
			log.Printf("no resolver defined to resolve %V", host)
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}

		ctx.Name = host
		//ctx.handler = ngx_stream_proxy_resolve_handler
		ctx.Data = s
		ctx.Timeout = cscf.resolverTimeout

		u.resolved.ctx = ctx

		if resolveName(ctx) != core.OK {
			u.resolved.ctx = nil
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}

		return
	}

found:

	if uscf == nil {
		log.Printf("no upstream configuration")
		proxyFinalize(s, INTERNAL_SERVER_ERROR)
		return
	}

	u.upstream = uscf

	u.sslName = uscf.Host

	if uscf.peer.init(s, uscf) != core.OK {
		proxyFinalize(s, INTERNAL_SERVER_ERROR)
		return
	}

	u.peer.StartTime = time.Now().UnixNano()

	if pscf.nextUpstreamTries > 0 &&
		u.peer.Tries > pscf.nextUpstreamTries {
		u.peer.Tries = pscf.nextUpstreamTries
	}

	proxyConnect(s)
}

func resolveStart(r *core.Resolver, temp *core.ResolverCtx) *core.ResolverCtx {
	var ctx core.ResolverCtx
	/*
		in_addr_t            addr;
		ngx_resolver_ctx_t  *ctx;

		if (temp) {
			addr = ngx_inet_addr(temp->name.data, temp->name.len);

			if (addr != INADDR_NONE) {
				temp->resolver = r;
				temp->state = NGX_OK;
				temp->naddrs = 1;
				temp->addrs = &temp->addr;
				temp->addr.sockaddr = (struct sockaddr *) &temp->sin;
				temp->addr.socklen = sizeof(struct sockaddr_in);
				ngx_memzero(&temp->sin, sizeof(struct sockaddr_in));
				temp->sin.sin_family = AF_INET;
				temp->sin.sin_addr.s_addr = addr;
				temp->quick = 1;

				return temp;
			}
		}

		if (r->connections.nelts == 0) {
			return NGX_NO_RESOLVER;
		}

		ctx = ngx_resolver_calloc(r, sizeof(ngx_resolver_ctx_t));

		if (ctx) {
			ctx->resolver = r;
		}

	*/
	return &ctx
}

func resolveName(ctx *core.ResolverCtx) int {
	/*
	   size_t           slen;
	   ngx_int_t        rc;
	   ngx_str_t        name;
	   ngx_resolver_t  *r;

	   r = ctx->resolver;

	   if (ctx->name.len > 0 && ctx->name.data[ctx->name.len - 1] == '.') {
	       ctx->name.len--;
	   }

	   ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->log, 0,
	                  "resolve: \"%V\"", &ctx->name);

	   if (ctx->quick) {
	       ctx->handler(ctx);
	       return NGX_OK;
	   }

	   if (ctx->service.len) {
	       slen = ctx->service.len;

	       if (ngx_strlchr(ctx->service.data,
	                       ctx->service.data + ctx->service.len, '.')
	           == NULL)
	       {
	           slen += sizeof("_._tcp") - 1;
	       }

	       name.len = slen + 1 + ctx->name.len;

	       name.data = ngx_resolver_alloc(r, name.len);
	       if (name.data == NULL) {
	           return NGX_ERROR;
	       }

	       if (slen == ctx->service.len) {
	           ngx_sprintf(name.data, "%V.%V", &ctx->service, &ctx->name);

	       } else {
	           ngx_sprintf(name.data, "_%V._tcp.%V", &ctx->service, &ctx->name);
	       }


	       rc = ngx_resolve_name_locked(r, ctx, &name);

	       ngx_resolver_free(r, name.data);

	   } else {

	       rc = ngx_resolve_name_locked(r, ctx, &ctx->name);
	   }

	   if (rc == NGX_OK) {
	       return NGX_OK;
	   }

	   if (rc == NGX_AGAIN) {
	       return NGX_OK;
	   }

	   if (ctx->event) {
	       ngx_resolver_free(r, ctx->event);
	   }

	   ngx_resolver_free(r, ctx);

	*/
	return core.ERROR
}

func proxySetLocal(s *streamSession, u *Upstream, local *upstreamLocal) int {
	/*
		ngx_int_t    rc;
		ngx_str_t    val;
		ngx_addr_t  *addr;

		if (local == NULL) {
			u->peer.local = NULL;
			return NGX_OK;
		}

		#if (NGX_HAVE_TRANSPARENT_PROXY)
		u->peer.transparent = local->transparent;
		#endif

		if (local->value == NULL) {
			u->peer.local = local->addr;
			return NGX_OK;
		}

		if (ngx_stream_complex_value(s, local->value, &val) != NGX_OK) {
			return NGX_ERROR;
		}

		if (val.len == 0) {
			return NGX_OK;
		}

		addr = ngx_palloc(s->connection->pool, sizeof(ngx_addr_t));
		if (addr == NULL) {
			return NGX_ERROR;
		}

		rc = ngx_parse_addr_port(s->connection->pool, addr, val.data, val.len);
		if (rc == NGX_ERROR) {
			return NGX_ERROR;
		}

		if (rc != NGX_OK) {
			ngx_log_error(NGX_LOG_ERR, s->connection->log, 0,
			"invalid local address \"%V\"", &val);
			return NGX_OK;
		}

		addr->name = val;
		u->peer.local = addr;

	*/
	return core.OK
}

func proxyConnect(s *streamSession) {
	//ngx_int_t                     rc;
	//ngx_connection_t             *c, *pc;
	//ngx_stream_upstream_t        *u;
	//ngx_stream_proxy_srv_conf_t  *pscf;

	var rc int
	var c core.Connection
	var pc core.Connection
	var u *Upstream
	var pscf *proxySrvConf
	c = s.connection

	log.Printf("%p\n", c)
	//c.log.action = "connecting to upstream";

	pscf = getModuleSrvConf(s, &proxyModule).(*proxySrvConf)

	u = s.upstream

	u.connected = false
	u.proxyProtocol = pscf.proxyProtocol

	//if u.state {
	u.state.responseTime = time.Now().UnixNano() - int64(u.state.responseTime)
	//u.state.responseTime = ngx_current_msec - u.state.responseTime
	//}

	/*
		u.state = ngx_array_push(s.upstream_states)
		if u.state == nil {
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}
	*/

	//ngx_memzero(u.state, sizeof(ngx_stream_upstream_state_t))

	u.state.connectTime = -1
	u.state.firstByteTime = -1
	//u.state.responseTime = current_msec

	//rc = ngx_event_connect_peer(&u.peer)

	log.Printf("proxy connect: %i", rc)

	if rc == core.ERROR {
		proxyFinalize(s, INTERNAL_SERVER_ERROR)
		return
	}

	u.state.peer = u.peer.Name

	if rc == core.BUSY {
		log.Printf("no live upstreams")
		proxyFinalize(s, BAD_GATEWAY)
		return
	}

	if rc == core.DECLINED {
		proxyNextUpstream(s)
		return
	}

	/* rc == core.OK || rc == NGX_AGAIN || rc == NGX_DONE */

	pc = u.peer.Connection

	pc.Data = s
	//pc.log = c.log
	//pc.read.log = c.log
	//pc.write.log = c.log

	if rc != core.AGAIN {
		proxyInitUpstream(s)
		return
	}

	//pc.read.handler = ngx_stream_proxy_connect_handler
	//pc.write.handler = ngx_stream_proxy_connect_handler

	//ngx_add_timer(pc.write, pscf.connect_timeout)
}

func proxyInitUpstream(s *streamSession) {
	//var tcpNodelay int
	var p []byte
	//u_char                       *p;
	//ngx_chain_t                  *cl;
	var cl *core.Chain
	var c, pc *core.Connection
	//ngx_log_handler_pt            handler;
	var u *Upstream
	//ngx_stream_upstream_t        *u;
	var cscf *coreSrvConf
	var pscf *proxySrvConf
	//ngx_stream_core_srv_conf_t   *cscf;
	//ngx_stream_proxy_srv_conf_t  *pscf;

	u = s.upstream
	pc = &u.peer.Connection

	cscf = getModuleSrvConf(s, &coreModule).(*coreSrvConf)

	if pc.Type == syscall.SOCK_STREAM &&
		cscf.tcpNodelay &&
		pc.TcpNodelay == core.TCP_NODELAY_UNSET {
		log.Printf("tcp_nodelay")

		//tcpNodelay = 1
		/*
			if (setsockopt(pc.fd, IPPROTO_TCP, TCP_NODELAY,
			(const void *) &tcpNodelay, sizeof(int)) == -1)
			{
				ngx_connection_error(pc, ngx_socket_errno,
				"setsockopt(TCP_NODELAY) failed");
				proxyNextUpstream(s);
				return;
			}
		*/

		pc.TcpNodelay = core.TCP_NODELAY_SET
	}

	pscf = getModuleSrvConf(s, &proxyModule).(*proxySrvConf)

	if pc.Type == syscall.SOCK_STREAM {
		// if pc.Type == syscall.SOCK_STREAM && pscf.ssl {

		if u.proxyProtocol {
			if proxySendProxyProtocol(s) != core.OK {
				return
			}

			u.proxyProtocol = false
		}

		/*
			if pc.ssl == nil {
				proxySslInitConnection(s)
				return
			}
		*/
	}

	c = &s.connection

	/*
		if (c.log.log_level >= NGX_LOG_INFO) {
			ngx_str_t  str;
			u_char     addr[NGX_SOCKADDR_STRLEN];

			str.len = NGX_SOCKADDR_STRLEN;
			str.data = addr;

			if (ngx_connection_local_sockaddr(pc, &str, 1) == core.OK) {
				handler = c.log.handler;
				c.log.handler = nil;

				ngx_log_error(NGX_LOG_INFO, c.log, 0,
				"%sproxy %V connected to %V",
				pc.type == syscall.SOCK_DGRAM ? "udp " : "",
				&str, u.peer.name);

				c.log.handler = handler;
			}
		}
	*/

	u.state.connectTime = time.Now().UnixNano() - u.state.responseTime
	//u.state.connectTime = current_msec - u.state.responseTime

	if u.peer.Notify != nil {
		u.peer.Notify(&u.peer, u.peer.Data,
			UPSTREAM_NOTIFY_CONNECT)
	}

	//c.log.action = "proxying connection"

	if u.upstreamBuf.Start == 0 {
		p = make([]byte, pscf.bufferSize)
		if p == nil {
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}

		u.upstreamBuf.Start = 0
		u.upstreamBuf.End = 0 + int(pscf.bufferSize)
		u.upstreamBuf.Pos = 0
		u.upstreamBuf.Last = int(pscf.bufferSize)
	}

	if c.Buffer.Pos < c.Buffer.Last {
		//if c.buffer && c.buffer.pos < c.buffer.last
		log.Printf("stream proxy add preread buffer: %uz",
			c.Buffer.Last-c.Buffer.Pos)

		cl = &core.Chain{}
		//cl = ngx_chain_get_free_buf(c.pool, &u.free)
		if cl == nil {
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}

		cl.Buf = c.Buffer

		//cl.Buf.tag = (ngx_buf_tag_t) & ngx_stream_proxy_module
		cl.Buf.Flush = true
		cl.Buf.LastBuf = (c.Type == syscall.SOCK_DGRAM)

		cl.Next = u.upstreamOut
		u.upstreamOut = cl
	}

	if u.proxyProtocol {
		log.Printf("stream proxy add PROXY protocol header")

		cl = &core.Chain{}
		//cl = ngx_chain_get_free_buf(c.pool, &u.free)
		if cl == nil {
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}

		p = make([]byte, core.PROXY_PROTOCOL_MAX_HEADER)
		if p == nil {
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}

		cl.Buf.Buf = p
		cl.Buf.Pos = 0

		p = core.ProxyProtocolWrite(c, p)
		if p == nil {
			proxyFinalize(s, INTERNAL_SERVER_ERROR)
			return
		}

		cl.Buf.Last = len(p)
		//cl.Buf.temporary = 1
		cl.Buf.Flush = false
		cl.Buf.LastBuf = false
		//cl.buf.tag = (ngx_buf_tag_t) & ngx_stream_proxy_module

		cl.Next = u.upstreamOut
		u.upstreamOut = cl

		u.proxyProtocol = false
	}

	if c.Type == syscall.SOCK_DGRAM && pscf.responses == 0 {
		//pc.read.ready = 0
		//pc.read.eof = 1
	}

	u.connected = true

	//pc.read.handler = ngx_stream_proxy_upstream_handler
	//pc.write.handler = ngx_stream_proxy_upstream_handler

	/*
		if pc.read.ready || pc.read.eof {
			ngx_post_event(pc.read, &ngx_posted_events)
		}
	*/

	proxyProcess(s, false, false)
}

/*
func proxyProcessConnection(ngx_event_t *ev, fromUpstream uint) {
	var c, pc *core.Connection
	var s *streamSession
	var u *Upstream
	var pscf *proxySrvConf

	c = ev.data
	s = c.data
	u = s.upstream

	c = s.connection
	pc = u.peer.Connection

	pscf = getModuleSrvConf(s, "stream_proxy")

	if ev.timedout {
		ev.timedout = 0

		if ev.delayed {
			ev.delayed = 0

			if !ev.ready {
				if ngx_handle_read_event(ev, 0) != core.OK {
					proxyFinalize(s,
					NGX_STREAM_INTERNAL_SERVER_ERROR)
					return
				}

				if u.connected && !c.read.delayed && !pc.read.delayed {
					ngx_add_timer(c.write, pscf.timeout)
				}

				return
			}

		} else {
			if s.connection.Type == syscall.SOCK_DGRAM {
				if pscf.responses == NGX_MAX_INT32_VALUE {


					pc.read.ready = 0
					pc.read.eof = 1

					proxyProcess(s, 1, 0)
					return
				}

				if u.received == 0 {
					proxyNextUpstream(s)
					return
				}
			}

			ngx_connection_error(c, NGX_ETIMEDOUT, "connection timed out")
			proxyFinalize(s, OK)
			return
		}

	} else if ev.delayed {

		log.Printf(NGX_LOG_DEBUG_STREAM, c.log, 0,
		"stream connection delayed")

		if ngx_handle_read_event(ev, 0) != core.OK {
			proxyFinalize(s, NGX_STREAM_INTERNAL_SERVER_ERROR)
		}

		return
	}

	if fromUpstream && !u.connected {
		return
	}

	proxyProcess(s, fromUpstream, ev.write)
}
*/

func proxyConnectHandler(ev core.Event) {
	var c *core.Connection
	var s *streamSession

	c = ev.Data.(*core.Connection)
	s = c.Data.(*streamSession)

	if ev.Timedout {
		log.Printf("upstream timed out")
		proxyNextUpstream(s)
		return
	}

	//ngx_del_timer(c.write);

	log.Printf("stream proxy connect upstream")

	if proxyTestConnect(c) != core.OK {
		proxyNextUpstream(s)
		return
	}

	proxyInitUpstream(s)
}

func proxyProcess(s *streamSession, fromUpstream bool, doWrite bool) {
	var received, limit int
	var size, limitRate int
	var n int
	var b core.Buffer
	var rc int
	//var flags uint
	var delay uint
	//ngx_chain_t                  *cl, **ll, **out, **busy;
	var cl, out, busy *core.Chain
	var c, pc, src, dst *core.Connection
	var u *Upstream
	var pscf *proxySrvConf

	u = s.upstream

	c = &s.connection
	if u.connected {
		pc = &u.peer.Connection
	}

	if c.Type == syscall.SOCK_DGRAM {
		//if (c.Type == syscall.SOCK_DGRAM && (ngx_terminate || ngx_exiting)) {

		/* socket is already closed on worker shutdown */

		//handler = c.log.handler;
		//c.log.handler = nil;

		log.Printf("disconnected on shutdown")

		//c.log.handler = handler;

		proxyFinalize(s, OK)
		return
	}

	pscf = getModuleSrvConf(s, &proxyModule).(*proxySrvConf)

	if fromUpstream {
		src = pc
		dst = c
		b = u.upstreamBuf
		limitRate = int(pscf.downloadRate)
		received = u.received
		out = u.downstreamOut
		busy = u.downstreamBusy

	} else {
		src = c
		dst = pc
		b = u.downstreamBuf
		limitRate = int(pscf.uploadRate)
		received = int(s.received)
		out = u.upstreamOut
		busy = u.upstreamBusy
	}

	for {

		if doWrite && dst != nil {

			//if *out || *busy || dst.buffered {
			if out != nil || busy != nil {
				//rc = ngx_stream_top_filter(s, *out, fromUpstream)
				rc = core.OK //todo remove
				if rc == core.ERROR {
					if c.Type == syscall.SOCK_DGRAM && !fromUpstream {
						proxyNextUpstream(s)
						return
					}

					proxyFinalize(s, OK)
					return
				}

				/*
					ngx_chain_update_chains(&u.free, busy, out,
					(ngx_buf_tag_t)&ngx_stream_proxy_module)
				*/

				if busy == nil {
					b.Pos = b.Start
					b.Last = b.Start
				}
			}
		}

		size = b.End - b.Last

		if size > 0 {
			//if size && src.read.ready && !src.read.delayed && !src.read.error
			if limitRate > 0 {
				limit = limitRate*int(time.Now().UnixNano()-u.startSec+1) - received

				if limit <= 0 {
					//src.read.delayed = 1
					delay = uint(-limit*1000/limitRate + 1)
					//ngx_add_timer(src.read, delay)
					break
				}

				if size > limit {
					size = limit
				}
			}

			n = src.Read(b.Last, size)

			if n == core.ERROR {
				if c.Type == syscall.SOCK_DGRAM && u.received == 0 {
					proxyNextUpstream(s)
					return
				}

				n = 0
			}

			if n >= 0 {
				if limitRate > 0 {
					delay = uint(n * 1000 / limitRate)

					if delay > 0 {
						//ngx_add_timer(src.read, delay)
					}
				}

				if fromUpstream {
					if u.state.firstByteTime == -1 {
						u.state.firstByteTime = time.Now().UnixNano() - u.state.responseTime
						//u.state.firstByteTime = ngx_current_msec -u.state.responseTime
					}
				}

				if u.responses++; c.Type == syscall.SOCK_DGRAM && u.responses == pscf.responses {
					//src.read.ready = 0
				}

				//for (ll = out; *ll; ll = &(*ll).next) { /* void */ }

				//TODO
				//cl = ngx_chain_get_free_buf(&u.free)
				if cl == nil {
					proxyFinalize(s, INTERNAL_SERVER_ERROR)
					return
				}

				cl.Buf.Pos = b.Last
				cl.Buf.Last = b.Last + n
				//cl.Buf.tag = &ngx_stream_proxy_module

				if n > 0 {
					//cl.Buf.temporary = 1
				}
				//cl.buf.last_buf = src.read.eof
				//cl.Buf.flush = 1

				received += n
				b.Last += n
				doWrite = true

				continue
			}
		}

		break
	}

	if false {
		//if src.read.eof && dst && (dst.read.eof || !dst.buffered)
		//handler = c.log.handler
		//c.log.handler = nil

		typeName := ""
		from := "client"
		sent := 0
		if src.Type == syscall.SOCK_DGRAM {
			typeName = "upd"
		}
		if fromUpstream {
			from = "upstream"
		}
		if pc != nil {
			sent = int(pc.Sent)
		}

		log.Printf("%s%s disconnected %d\n",
			typeName, from, sent)
		/*
			log.Printf("%s%s disconnected"+
				", bytes from/to client:%O/%O"+
				", bytes from/to upstream:%O/%O",
				typeName,
				from,
				s.Received, c.Sent, u.Received, sent)
		*/

		//c.log.handler = handler;

		proxyFinalize(s, OK)
		return
	}

	/*
		if src.read.eof {
			flags = CLOSE_EVENT
		}
	*/

	/*
		if !src.shared && ngx_handle_read_event(src.read, flags) != NGX_OK {
			proxyFinalize(s, NGX_STREAM_INTERNAL_SERVER_ERROR)
			return
		}
	*/

	if dst != nil {
		/*
			if !dst.shared && ngx_handle_write_event(dst.write, 0) != NGX_OK {
				proxyFinalize(s, NGX_STREAM_INTERNAL_SERVER_ERROR)
				return
			}

			if !c.read.delayed && !pc.read.delayed {
				ngx_add_timer(c.write, pscf.timeout)

			} else if c.write.timer_set {
				ngx_del_timer(c.write)
			}
		*/
	}
}

func ngx_stream_proxy_connect_handler(ev *core.Event) {
	var c *core.Connection
	var s *streamSession

	c = ev.Data.(*core.Connection)
	s = c.Data.(*streamSession)

	if ev.Timedout {
		log.Printf("upstream timed out")
		proxyNextUpstream(s)
		return
	}

	//ngx_del_timer(c.write)

	log.Printf("stream proxy connect upstream")

	if proxyTestConnect(c) != core.OK {
		proxyNextUpstream(s)
		return
	}

	proxyInitUpstream(s)
}

func proxyTestConnect(c *core.Connection) int {
	/*
		if (getsockopt(c.fd, SOL_SOCKET, SO_ERROR, (void *) &err, &len)
		== -1)
		{
			err = ngx_socket_errno;
		}

		if (err) {
			log.Printf("connect() failed");
			return core.ERROR;
		}
	*/

	return core.OK
}

func proxyNextUpstream(s *streamSession) {
	var timeout uint
	var pc *core.Connection
	var u *Upstream
	var pscf *proxySrvConf

	log.Printf("stream proxy next upstream")

	u = s.upstream
	pc = &u.peer.Connection

	if u.upstreamOut != nil || u.upstreamBusy != nil || pc != nil {
		//if u.upstreamOut != nil || u.upstreamBusy != nil || (pc != nil && pc.buffered)
		log.Printf("pending buffers on next upstream")

		u.peer.Free(&u.peer, u.peer.Data, core.PEER_FAILED)
	}

	pscf = getModuleSrvConf(s, &proxyModule).(*proxySrvConf)

	timeout = pscf.nextUpstreamTimeout

	/*
		if u.peer.tries == 0 ||
			!pscf.nextUpstream ||
			(timeout && ngx_current_msec-u.peer.startTime >= timeout) {
			proxyFinalize(s, BAD_GATEWAY)
			return
		}
	*/

	if u.peer.Tries == 0 ||
		pscf.nextUpstream == 0 ||
		(timeout > 0 && uint(time.Now().UnixNano()-u.peer.StartTime) >= timeout) {
		proxyFinalize(s, BAD_GATEWAY)
		return
	}

	if pc != nil {
		log.Printf("close proxy upstream connection")

		//ssl todo
		//if pc.ssl {
		//	pc.ssl.no_wait_shutdown = 1
		//	pc.ssl.no_send_shutdown = 1

		//(void) ngx_ssl_shutdown(pc);
		//}

		u.state.bytesReceived = int64(u.received)
		u.state.bytesSent = int64(pc.Sent)

		//ngx_close_connection(pc);
	}
}

func proxyFinalize(s *streamSession, rc uint) {

	log.Printf("finalize stream proxy: %i", rc)

	u := s.upstream
	pc := u.peer.Connection

	if u == nil {
		goto noupstream
	}

	if u.resolved != nil && u.resolved.ctx != nil {
		//TODO
		//ngx_resolve_name_done(u.resolved.ctx)
		u.resolved.ctx = nil
	}

	u.state.responseTime = time.Now().UnixNano() - u.state.responseTime
	//u.state.responseTime = ngx_current_msec - u.state.responseTime

	u.state.bytesReceived = int64(u.received)
	u.state.bytesSent = int64(pc.Sent)

	if u.peer.Free != nil {
		//if u.peer.Free != nil && u.peer.sockaddr
		u.peer.Free(&u.peer, u.peer.Data, 0)
		//u.peer.sockaddr = nil;
	}

	/*
		if pc {
			log.Printf("close stream proxy upstream connection: %d", pc.fd)

			if pc.ssl {
				//pc.ssl.no_wait_shutdown = 1;
				//ngx_ssl_shutdown(pc);
			}

			//ngx_close_connection(pc);
			u.peer.Connection = nil
		}
	*/

noupstream:

	finalizeSession(s, rc)
}

/*
static u_char *
ngx_stream_proxy_log_error(ngx_log_t *log, u_char *buf, size_t len)
{
	u_char                 *p;
	ngx_connection_t       *pc;
	ngx_stream_session_t   *s;
	ngx_stream_upstream_t  *u;

	s = log.data;

	u = s.upstream;

	p = buf;

	if (u.peer.name) {
		p = ngx_snprintf(p, len, ", upstream: \"%V\"", u.peer.name);
		len -= p - buf;
	}

	pc = u.peer.Connection;

	p = ngx_snprintf(p, len,
	", bytes from/to client:%O/%O"
	", bytes from/to upstream:%O/%O",
	s.received, s.connection.sent,
	u.received, pc ? pc.sent : 0);

	return p;
}
*/

func proxyCreateSrvConf(cf *core.Conf) interface{} {
	var conf *proxySrvConf
	//ngx_stream_proxy_srv_conf_t  *conf;

	conf = &proxySrvConf{}
	if conf == nil {
		return nil
	}

	conf.connectTimeout = core.CONF_UNSET_MSEC
	conf.timeout = core.CONF_UNSET_MSEC
	conf.nextUpstreamTimeout = core.CONF_UNSET_MSEC
	conf.bufferSize = core.CONF_UNSET_SIZE
	conf.uploadRate = core.CONF_UNSET_SIZE
	conf.downloadRate = core.CONF_UNSET_SIZE
	conf.responses = core.CONF_UNSET_UINT
	conf.nextUpstreamTries = core.CONF_UNSET_UINT
	conf.nextUpstream = core.CONF_UNSET
	conf.proxyProtocol = false
	//conf.local = core.CONF_UNSET_PTR

	// ssl
	/*
		conf.ssl_enable = NGX_CONF_UNSET;
		conf.ssl_session_reuse = NGX_CONF_UNSET;
		conf.ssl_server_name = NGX_CONF_UNSET;
		conf.ssl_verify = NGX_CONF_UNSET;
		conf.ssl_verify_depth = NGX_CONF_UNSET_UINT;
		conf.ssl_passwords = NGX_CONF_UNSET_PTR;
	*/

	return conf
}

func proxyMergeSrvConf(cf *core.Conf, parent interface{}, child interface{}) interface{} {
	/*
		var prev *proxySrvConf
		var conf *proxySrvConf

		prev = parent.(*proxySrvConf)
		conf = parent.(*proxySrvConf)
	*/

	/*
			ngx_conf_merge_msec_value(conf.connect_timeout,
				prev.connect_timeout, 60000)

			ngx_conf_merge_msec_value(conf.timeout,
				prev.timeout, 10*60000)

			ngx_conf_merge_msec_value(conf.next_upstream_timeout,
				prev.next_upstream_timeout, 0)

			ngx_conf_merge_size_value(conf.buffer_size,
				prev.buffer_size, 16384)

			ngx_conf_merge_size_value(conf.upload_rate,
				prev.upload_rate, 0)

			ngx_conf_merge_size_value(conf.download_rate,
				prev.download_rate, 0)

			ngx_conf_merge_uint_value(conf.responses,
				prev.responses, NGX_MAX_INT32_VALUE)

		ngx_conf_merge_uint_value(conf.next_upstream_tries,
			prev.next_upstream_tries, 0)

		ngx_conf_merge_value(conf.next_upstream, prev.next_upstream, 1)

		ngx_conf_merge_value(conf.proxy_protocol, prev.proxy_protocol, 0)

		ngx_conf_merge_ptr_value(conf.local, prev.local, nil)
	*/

	/*

		ngx_conf_merge_value(conf.ssl_enable, prev.ssl_enable, 0);

		ngx_conf_merge_value(conf.ssl_session_reuse,
		prev.ssl_session_reuse, 1);

		ngx_conf_merge_bitmask_value(conf.ssl_protocols, prev.ssl_protocols,
		(NGX_CONF_BITMASK_SET|NGX_SSL_TLSv1
		|NGX_SSL_TLSv1_1|NGX_SSL_TLSv1_2));

		ngx_conf_merge_str_value(conf.ssl_ciphers, prev.ssl_ciphers, "DEFAULT");

		if (conf.ssl_name == nil) {
			conf.ssl_name = prev.ssl_name;
		}

		ngx_conf_merge_value(conf.ssl_server_name, prev.ssl_server_name, 0);

		ngx_conf_merge_value(conf.ssl_verify, prev.ssl_verify, 0);

		ngx_conf_merge_uint_value(conf.ssl_verify_depth,
		prev.ssl_verify_depth, 1);

		ngx_conf_merge_str_value(conf.ssl_trusted_certificate,
		prev.ssl_trusted_certificate, "");

		ngx_conf_merge_str_value(conf.ssl_crl, prev.ssl_crl, "");

		ngx_conf_merge_str_value(conf.ssl_certificate,
		prev.ssl_certificate, "");

		ngx_conf_merge_str_value(conf.ssl_certificate_key,
		prev.ssl_certificate_key, "");

		ngx_conf_merge_ptr_value(conf.ssl_passwords, prev.ssl_passwords, nil);

		if (conf.ssl_enable && ngx_stream_proxy_set_ssl(cf, conf) != NGX_OK) {
			return core.ERROR;
		}

	*/

	return core.OK
}

/*

static ngx_int_t
ngx_stream_proxy_set_ssl(ngx_conf_t *cf, ngx_stream_proxy_srv_conf_t *pscf)
{
	ngx_pool_cleanup_t  *cln;

	pscf.ssl = ngx_pcalloc(cf.pool, sizeof(ngx_ssl_t));
	if (pscf.ssl == nil) {
		return NGX_ERROR;
	}

	pscf.ssl.log = cf.log;

	if (ngx_ssl_create(pscf.ssl, pscf.ssl_protocols, nil) != NGX_OK) {
		return NGX_ERROR;
	}

	cln = ngx_pool_cleanup_add(cf.pool, 0);
	if (cln == nil) {
		return NGX_ERROR;
	}

	cln.handler = ngx_ssl_cleanup_ctx;
	cln.data = pscf.ssl;

	if (pscf.ssl_certificate.len) {

		if (pscf.ssl_certificate_key.len == 0) {
			ngx_log_error(NGX_LOG_EMERG, cf.log, 0,
			"no \"proxy_ssl_certificate_key\" is defined "
			"for certificate \"%V\"", &pscf.ssl_certificate);
			return NGX_ERROR;
		}

		if (ngx_ssl_certificate(cf, pscf.ssl, &pscf.ssl_certificate,
		&pscf.ssl_certificate_key, pscf.ssl_passwords)
		!= NGX_OK)
		{
			return NGX_ERROR;
		}
	}

	if (ngx_ssl_ciphers(cf, pscf.ssl, &pscf.ssl_ciphers, 0) != NGX_OK) {
		return NGX_ERROR;
	}

	if (pscf.ssl_verify) {
		if (pscf.ssl_trusted_certificate.len == 0) {
			ngx_log_error(NGX_LOG_EMERG, cf.log, 0,
			"no proxy_ssl_trusted_certificate for proxy_ssl_verify");
			return NGX_ERROR;
		}

		if (ngx_ssl_trusted_certificate(cf, pscf.ssl,
		&pscf.ssl_trusted_certificate,
		pscf.ssl_verify_depth)
		!= NGX_OK)
		{
			return NGX_ERROR;
		}

		if (ngx_ssl_crl(cf, pscf.ssl, &pscf.ssl_crl) != NGX_OK) {
			return NGX_ERROR;
		}
	}

	return NGX_OK;
}

*/

func proxyPass(cf *core.Conf, cmd *core.Command, conf interface{}) (int, error) {

	log.Printf("call proxy pass\n")
	pscf := conf.(*proxySrvConf)
	var u core.URL
	//ngx_stream_complex_value_t           cv;
	//ngx_stream_core_srv_conf_t          *cscf;
	//ngx_stream_compile_complex_value_t   ccv;

	if pscf.upstream != nil {
		//if (pscf.upstream != nil|| pscf.upstreamValue)
		return core.ERROR, errors.New("is duplicate")
	}

	//cscf = ngx_stream_conf_get_module_srv_conf(cf, ngx_stream_core_module);

	//cscf.handler = ngx_stream_proxy_handler;

	value := cf.Args

	url := value[1]

	//ngx_memzero(&ccv, sizeof(ngx_stream_compile_complex_value_t));

	/*
		ccv.cf = cf;
		ccv.value = url;
		ccv.complex_value = &cv;

		if (ngx_stream_compile_complex_value(&ccv) != core.OK) {
			return core.ERROR;
		}
	*/

	/*
		if (cv.lengths) {
			pscf.upstream_value = ngx_palloc(cf.pool,
			sizeof(ngx_stream_complex_value_t));
			if (pscf.upstream_value == nil) {
				return core.ERROR;
			}

			*pscf.upstream_value = cv;

			return core.OK;
		}
	*/

	u.Url = url
	u.NoResolve = true

	pscf.upstream = upstreamAdd(cf, &u, 0)
	if pscf.upstream == nil {
		return core.ERROR, errors.New("upstream add fail")
	}

	return core.OK, nil
}

func proxyBind(cf *core.Conf, cmd *core.Command, conf interface{}) (int, error) {
	var rc int
	var pscf *proxySrvConf
	var value []string
	pscf = conf.(*proxySrvConf)
	//ngx_stream_complex_value_t           cv;
	//ngx_stream_upstream_local_t         *local;

	//ngx_stream_compile_complex_value_t   ccv;

	if pscf.local != nil {
		return core.ERROR, errors.New("is duplicate")
	}

	value = cf.Args

	if len(cf.Args) == 2 && value[1] == "off" {
		pscf.local = nil
		return core.OK, nil
	}

	//ngx_memzero(&ccv, sizeof(ngx_stream_compile_complex_value_t))

	//ccv.cf = cf
	//ccv.value = value[1]
	//ccv.complex_value = &cv

	/*
		if ngx_stream_compile_complex_value(&ccv) != NGX_OK {
			return core.ERROR
		}
	*/

	var local upstreamLocal

	pscf.local = &local

	//todo
	if false {
		//if cv.lengths
		/*
			local.value = ngx_palloc(cf.pool, sizeof(ngx_stream_complex_value_t))
			if local.value == nil {
				return core.ERROR
			}

			*local.value = cv
		*/

	} else {

		rc = core.ParseAddrPort(&local.addr, value[1])

		switch rc {
		case core.OK:
			local.addr.Name = value[1]
			break

		case core.DECLINED:
			log.Printf("invalid address \"%V\"", &value[1])
			/* fall through */

		default:
			return core.ERROR, errors.New("unkown error")
		}
	}

	if len(cf.Args) > 2 {
		if value[2] == "transparent" {
			local.transparent = true
			/*
				ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
				"transparent proxying is not supported "
				"on this platform, ignored");
			*/
		} else {
			log.Printf("invalid parameter \"%V\"", value[2])
			return core.ERROR, fmt.Errorf("invalid parameter \"%s\"", value[2])
		}
	}

	return core.OK, nil
}

func proxySendProxyProtocol(s *streamSession) int {
	return core.OK
}
