package core

import (
	"fmt"
	"log"
	"net"
	"strconv"
	"strings"
)

type Addr struct {
	Name string
}

type URL struct {
	Url      string
	Host     string
	PortText string
	Uri      string

	Port        int
	DefaultPort int
	Family      string

	Listen    bool
	UriPart   bool
	NoResolve bool

	NoPort   bool
	Wildcard bool

	Addr string
	/*
		socklen_t                 socklen;
		ngx_sockaddr_t            sockaddr;
	*/

	Addrs []Addr
	Err   string
}

func parseUnixDomainUrl(u *URL) int {
	path := 5
	length := len(u.Url) - 5
	uri := -1

	//TODO,
	if u.UriPart {

		uri = strings.Index(u.Url[path:], ":")

		if uri != -1 {
			length = uri - path
			uri++
			u.Uri = u.Uri[uri:]
		}
	}

	if length == 0 {
		u.Err = "no path in the unix domain socket"
		return ERROR
	}

	u.Host = u.Url[path:]

	u.Addrs = make([]Addr, 1)
	if u.Addrs == nil {
		return ERROR
	}

	u.Family = "unix"

	u.Addrs[0].Name = u.Url

	return OK

}

func ParseUrl(u *URL) int {

	if len(u.Url) >= 5 && strings.EqualFold(u.Url[:5], "unix:") {
		return parseUnixDomainUrl(u)
	}

	if len(u.Url) > 0 && u.Url[0] == '[' {
		return parseInet6Url(u)
	}

	return parseInetUrl(u)
}

func parseInet6Url(u *URL) int {
	//u_char               *p, *host, *port, *last, *uri;
	//size_t                length;
	//ngx_int_t             n;
	//struct sockaddr_in6  *sin6;

	//u.socklen = sizeof(struct sockaddr_in6);
	//sin6 = (struct sockaddr_in6 *) &u.sockaddr;
	//sin6.sin6_family = AF_INET6;

	var err error
	host := 1
	port := 0
	n := 0
	last := len(u.Url)

	p := strings.Index(u.Url[host:], "]")

	if p == -1 {
		u.Err = "invalid host"
		return ERROR
	}

	port = p + 1

	uri := strings.Index(u.Url[port:], "/")

	if uri != -1 {
		if u.Listen || !u.UriPart {
			u.Err = "invalid host"
			return ERROR
		}

		u.Uri = u.Url[uri:]

		last = uri
	}

	if port < last {
		if u.Url[port] != ':' {
			u.Err = "invalid host"
			return ERROR
		}

		port++

		n, err = strconv.Atoi(u.Url[port:])

		if n < 1 || n > 65535 || err != nil {
			u.Err = "invalid port"
			return ERROR
		}

		u.Port = n

		u.PortText = u.Url[port:]

	} else {
		u.NoPort = true
		u.Port = u.DefaultPort
	}

	length := p - host

	if length == 0 {
		u.Err = "no host"
		return ERROR
	}

	u.Host = u.Url[host-1:]

	if net.ParseIP(u.Url[host:]) == nil {
		u.Err = "invalid IPv6 address"
		return ERROR
	}

	/*
		if (IN6_IS_ADDR_UNSPECIFIED(&sin6.sin6_addr)) {
			u.Wildcard = true;
		}
	*/

	u.Family = "tcp6"

	u.Addrs = make([]Addr, 1)
	if u.Addrs == nil {
		return ERROR
	}

	u.Addrs[0].Name = fmt.Sprintf("%s:%d", u.Host, u.Port)

	return OK

}

func parseInetUrl(u *URL) int {

	var err error

	n := 0

	length := 0

	u.Family = "tcp"

	host := 0

	last := host + len(u.Url)

	port := strings.Index(u.Url[host:], ":")

	uri := strings.Index(u.Url[host:], "/")

	args := strings.Index(u.Url[host:], "?")

	if args != -1 {
		if uri == -1 || args < uri {
			uri = args
		}
	}

	if uri != -1 {
		if u.Listen || !u.UriPart {
			u.Err = "invalid host"
			return ERROR
		}

		u.Uri = u.Url[uri:]

		last = uri

		if uri < port {
			port = -1
		}
	}

	if port != -1 {
		port++

		length = last - port

		n, err = strconv.Atoi(u.Url[port:])
		if n < 1 || n > 65535 || err != nil {
			u.Err = "invalid port"
			return ERROR
		}

		u.Port = n
		u.PortText = u.Url[port:]
		last = port - 1

	} else {
		if uri == -1 {

			if u.Listen {

				/* test value as port only */

				//log.Printf("<%s>###%d\n", u.Url, host)
				n, err = strconv.Atoi(u.Url[host:])
				if n != ERROR {

					if n < 1 || n > 65535 {
						u.Err = "invalid port"
						return ERROR
					}

					u.Port = n

					u.Addr = ":" + u.Url[host:]

					u.PortText = u.Url[host:]

					u.Wildcard = true

					return OK
				}
			}
		}

		u.NoPort = true
		u.Port = u.DefaultPort
	}

	length = last - host

	if length == 0 {
		u.Err = "no host"
		return ERROR
	}

	u.Host = u.Url[:last]

	if u.Listen && length == 1 && u.Url[host:] == "*" {
		//sin.sin_addr.s_addr = INADDR_ANY
		u.Wildcard = true
		return OK
	}

	/*
		sin.sin_addr.s_addr = ngx_inet_addr(host, length)
	*/

	if ip := net.ParseIP(u.Host); ip != nil {

		/*
			if sin.sin_addr.s_addr == INADDR_ANY {
				u.wildcard = 1
			}
		*/

		u.Addrs = make([]Addr, 1)
		if u.Addrs == nil {
			return ERROR
		}

		u.Addrs[0].Name = fmt.Sprintf("%s:%d", u.Host, u.Port)

		log.Printf("u.Addrs=%s\n", u.Addrs)
		return OK
	}

	if u.NoResolve {
		return OK
	}

	/*
		if (ngx_inet_resolve_host(pool, u) != core.OK) {
			return core.ERROR;
		}
	*/

	u.Family = "tcp"
	switch u.Family {

	case "tcp6":
	/*
		sin6 = (struct sockaddr_in6 *) &u.sockaddr;

		if (IN6_IS_ADDR_UNSPECIFIED(&sin6.sin6_addr)) {
			u.wildcard = 1;
		}
	*/

	default: /* AF_INET */
		/*
			sin = (struct sockaddr_in *) &u.sockaddr;

			if (sin.sin_addr.s_addr == INADDR_ANY) {
				u.wildcard = 1;
			}
		*/

	}

	return OK
}

func ParseAddrPort(addr *Addr, text string) int {
	/*
		u_char     *p, *last;
		size_t      plen;
		ngx_int_t   rc, port;

		rc = ngx_parse_addr(pool, addr, text, len);

		if (rc != NGX_DECLINED) {
			return rc;
		}

		last = text + len;

		#if (NGX_HAVE_INET6)
		if (len && text[0] == '[') {

			p = ngx_strlchr(text, last, ']');

			if (p == NULL || p == last - 1 || *++p != ':') {
				return NGX_DECLINED;
			}

			text++;
			len -= 2;

		} else
		#endif

		{
			p = ngx_strlchr(text, last, ':');

			if (p == NULL) {
				return NGX_DECLINED;
			}
		}

		p++;
		plen = last - p;

		port = ngx_atoi(p, plen);

		if (port < 1 || port > 65535) {
			return NGX_DECLINED;
		}

		len -= plen + 1;

		rc = ngx_parse_addr(pool, addr, text, len);

		if (rc != NGX_OK) {
			return rc;
		}

		ngx_inet_set_port(addr->sockaddr, (in_port_t) port);

	*/
	return OK
}
