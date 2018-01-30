package stream

import (
	"core"
	"errors"
	"fmt"
	"log"
	"strconv"
	"strings"
	"syscall"
)

const LISTEN_BACKLOG = -1

var coreCommands = []core.Command{
	{
		Name:   "server",
		Type:   STREAM_MAIN_CONF | core.CONF_BLOCK | core.CONF_NOARGS,
		Set:    coreServer,
		Conf:   0,
		Offset: 0,
		Post:   nil,
	},
	{
		Name:   "listen",
		Type:   STREAM_SRV_CONF | core.CONF_1MORE,
		Set:    coreListen,
		Conf:   SRV_CONF_OFFSET(),
		Offset: 0,
		Post:   nil,
	},
}

var coreModuleCtx = streamModuleFunc{
	preconfiguration:  corePreconfiguration,
	postconfiguration: nil,

	createMainConf: coreCreateMainConf,
	initMainConf:   coreInitMainConf,

	createSrvConf: coreCreateSrvConf,
	mergeSrvConf:  coreMergeSrvConf,
}

func corePreconfiguration(cf *core.Conf) int {
	return core.OK
}

func coreCreateMainConf(cf *core.Conf) interface{} {
	var cmcf coreMainConf

	return &cmcf
}

func coreInitMainConf(cf *core.Conf, conf interface{}) interface{} {
	return nil
}

func coreCreateSrvConf(cf *core.Conf) interface{} {
	var cscf coreSrvConf
	return &cscf
}

func coreMergeSrvConf(cf *core.Conf, prev interface{}, conf interface{}) interface{} {
	return nil
}

var coreModule = core.Module{
	Name: "core",
	//Commands: coreCommands,
	MainName: "stream",
	Type:     STREAM_MODULE,
	Ctx:      coreModuleCtx,
}

func genCoreModule() *core.Module {
	coreModule.Commands = coreCommands
	return &coreModule
}

func coreServer(cf *core.Conf, cmd *core.Command, conf interface{}) (int, error) {

	rv := 0
	var err error

	var mconf interface{}
	var pcf core.Conf
	var module streamModuleFunc
	var ctx streamConfCtx
	var streamCtx *streamConfCtx

	var cscf *coreSrvConf
	var cmcf *coreMainConf

	streamCtx = cf.Ctx.(*streamConfCtx)
	ctx.MainConf = streamCtx.MainConf
	ctx.SrvConf = make([]interface{}, len(streamCtx.SrvConf))

	if ctx.SrvConf == nil {
		return core.ERROR, errors.New("oom")
	}

	log.Printf("###core server\n")
	modules := cf.Cycle.GetMainModules("stream")
	for _, v := range modules {
		if v.Type != STREAM_MODULE {
			continue
		}

		if v.Ctx == nil {
			continue
		}

		module = v.Ctx.(streamModuleFunc)

		if module.createSrvConf != nil {
			mconf = module.createSrvConf(cf)
			if mconf == nil {
				return core.ERROR, errors.New("stream core module create srv conf fail")
			}
			ctx.SrvConf[v.CtxIndex] = mconf
		}
	}

	cscf = ctx.SrvConf[coreModule.CtxIndex].(*coreSrvConf)
	cscf.Ctx = &ctx
	cmcf = ctx.MainConf[coreModule.CtxIndex].(*coreMainConf)

	log.Printf("cmcf = %v\n", cmcf)
	/*

	   cscfp = ngx_array_push(&cmcf.servers);
	   if (cscfp == NULL) {
	       return NGX_CONF_ERROR;
	   }

	   *cscfp = cscf;
	*/
	pcf = *cf
	cf.Ctx = &ctx
	cf.CmdType = STREAM_SRV_CONF

	rv, err = core.ConfParse(cf, "")

	*cf = pcf

	if rv == core.OK && !cscf.listen {

		errMsg := fmt.Sprintf("no \"listen\" is defined for server in %s:%ui",
			cscf.file_name, cscf.line)
		log.Printf("%s", errMsg)
		return core.ERROR, errors.New(errMsg)
	}
	return rv, err
}

func coreListen(cf *core.Conf, cmd *core.Command, conf interface{}) (int, error) {
	cscf := conf.(*coreSrvConf)

	//ngx_stream_listen_t          *ls, *als;
	//ngx_stream_core_main_conf_t  *cmcf;

	log.Printf("call coreListen\n")
	var ls streamListen
	var als []*streamListen
	var backlog uint
	var i int
	var cmcf *coreMainConf
	u := core.URL{}
	cscf.listen = true

	value := cf.Args

	u.Url = value[1]
	u.Listen = true

	if core.ParseUrl(&u) != core.OK {
		if len(u.Err) > 0 {
			log.Printf("%s in \"%V\" of the \"listen\" directive",
				u.Err, &u.Url)
		}

		return core.ERROR, errors.New(u.Err)
	}

	cmcf = confGetModuleMainConf(cf, &coreModule).(*coreMainConf)
	//cmcf = ngx_stream_conf_get_module_main_conf(cf, ngx_stream_core_module);

	cmcf.listen = append(cmcf.listen, &ls)

	//ngx_memcpy(&ls.sockaddr.sockaddr, &u.sockaddr, u.socklen);

	//ls.socklen = u.socklen;
	ls.addr = u.Addr
	ls.backlog = LISTEN_BACKLOG
	ls.Type = syscall.SOCK_STREAM
	ls.wildcard = u.Wildcard
	ls.Ctx = cf.Ctx.(*streamConfCtx)

	log.Printf("####:ls(%#v) value[1]:%s:cmcf:%v\n", ls, value[1], cmcf)

	//ipv6
	ls.ipv6only = true

	backlog = 0

	var err error
	for i = 2; i < len(cf.Args); i++ {

		if value[i] == "bind" {
			ls.bind = true
			continue
		}

		if len(value[i]) > 8 && value[i][:8] == "backlog=" {
			ls.backlog, err = strconv.Atoi(value[i][8:])
			ls.bind = true

			if err != nil || ls.backlog == 0 {
				errMsg := fmt.Sprintf("invalid backlog \"%s\"", value[i])
				log.Printf("%s\n", errMsg)
				return core.ERROR, errors.New(errMsg)
			}

			backlog = 1

			continue
		}

		if len(value[i]) > 10 && value[i][:10] == "ipv6only=o" {
			//size_t  len;
			//u_char  buf[NGX_SOCKADDR_STRLEN];

			//if (ls.sockaddr.sockaddr.sa_family == AF_INET6) {

			if value[i][10:] == "n" {
				ls.ipv6only = true

			} else if value[i][10:] == "ff" {
				ls.ipv6only = false

			} else {
				errMsg := fmt.Sprintf("invalid ipv6only flags \"%s\"",
					value[i][9:])
				log.Printf("%s\n", errMsg)
				return core.ERROR, errors.New(errMsg)
			}

			ls.bind = true

			//} else {
			//len = ngx_sock_ntop(&ls.sockaddr.sockaddr, ls.socklen, buf,
			//NGX_SOCKADDR_STRLEN, 1);

			//log.Printf("ipv6only is not supported "
			//                  "on addr \"%*s\", ignored", len, buf);
			//}

			continue
			//log.Printf("bind ipv6only is not supported "
			//"on this platform");
			//return core.ERROR;
		}

		if value[i] == "reuseport" {
			ls.reuseport = true
			ls.bind = true
			//log.Printf("reuseport is not supported "
			//"on this platform, ignored");
			continue
		}

		if value[i] == "ssl" {
			ls.ssl = true
			continue
			//log.Printf("the \"ssl\" parameter requires "
			//"ngx_stream_ssl_module");
			//return core.ERROR;
		}

		if len(value[i]) > 13 && value[i][:13] == "so_keepalive=" {

			if value[i][13:] == "on" {
				ls.soKeepalive = 1

			} else if value[i][13:] == "off" {
				ls.soKeepalive = 2

			} else {

				end := len(value[i])
				spos := 13
				s := value[i][13:]

				p := strings.LastIndex(s, ":")
				if p == -1 {
					p = end
				}

				if p > spos {
					ls.tcpKeepidle = core.ParseTime(value[i][13:p], true)
					if ls.tcpKeepidle == core.ERROR {
						goto invalid_so_keepalive
					}
				}

				if p < end {
					spos = p + 1
					s = value[i][p+1:]
				} else {
					spos = end
					s = value[i][end:]
				}

				p = strings.LastIndex(s, ":")
				if p == -1 {
					p = end
				}

				if p > spos {
					//s.len = p - s.data

					ls.tcpKeepintvl = core.ParseTime(value[i][spos:p], true)
					if ls.tcpKeepintvl == core.ERROR {
						goto invalid_so_keepalive
					}
				}

				if p < end {
					s = value[i][p+1:]
				} else {
					s = value[i][end:]
				}

				if spos < end {
					//s.len = end - s.data

					ls.tcpKeepcnt, err = strconv.Atoi(s)
					if err != nil {
						goto invalid_so_keepalive
					}
				}

				if ls.tcpKeepidle == 0 && ls.tcpKeepintvl == 0 &&
					ls.tcpKeepcnt == 0 {
					goto invalid_so_keepalive
				}

				ls.soKeepalive = 1

				//log.Printf("the \"so_keepalive\" parameter accepts "
				//"only \"on\" or \"off\" on this platform")
				//return core.ERROR;

			}

			ls.bind = false

			continue

		invalid_so_keepalive:

			errMsg := fmt.Sprintf("invalid so_keepalive value: \"%s\"", value[i][13:])
			log.Printf("%s\n", errMsg)
			return core.ERROR, errors.New(errMsg)
		}

		if value[i] == "proxy_protocol" {
			ls.proxyProtocol = true
			continue
		}

		errMsg := fmt.Errorf("the invalid \"%V\" parameter", value[i])
		return core.ERROR, errMsg
	}

	if ls.Type == syscall.SOCK_DGRAM {
		if backlog != 0 {
			return core.ERROR, errors.New("\"backlog\" parameter is incompatible with \"udp\"")
		}

		if ls.ssl {
			return core.ERROR, errors.New("\"ssl\" parameter is incompatible with \"udp\"")
		}

		if ls.soKeepalive != 0 {
			return core.ERROR, errors.New("\"so_keepalive\" parameter is incompatible with \"udp\"")
		}

		if ls.proxyProtocol {
			return core.ERROR, errors.New("\"proxy_protocol\" parameter is incompatible with \"udp\"")
		}
	}

	als = cmcf.listen

	for i = 0; i < len(cmcf.listen)-1; i++ {
		if ls.Type != als[i].Type {
			continue
		}

		//TODO:
		if als[i].addr != ls.addr {
			continue
		}

		/*
		   if (ngx_cmp_sockaddr(&als[i].sockaddr.sockaddr, als[i].socklen,
		                        &ls.sockaddr.sockaddr, ls.socklen, 1)
		       != core.OK)
		   {
		       continue;
		   }
		*/

		//log.Printf("%#v ########### %#v\n", ls, als[i])
		errMsg := fmt.Errorf("duplicate \"%V\" address and port pair", &u.Url)
		return core.ERROR, errMsg
	}

	log.Printf("#current listen address is %#v\n", cmcf)
	return core.OK, nil
}

/*
func coreResolver(cf *core.Conf, cmd *core.Command, conf interface{}) (int, error) {

	cscf := conf.(coreSrvConf)
	if cscf.resolver {
		return core.ERROR, errors.New("is duplicate")
	}

	value := cf.Args

	//cscf.resolver = ngx_resolver_create(cf, &value[1], cf.args.nelts - 1);
	if cscf.resolver == nil {
		return core.ERROR, errors.New("resolver create fail")
	}

	return core.OK, nil
}
*/
