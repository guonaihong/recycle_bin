package core

import (
	"fmt"
	"net"
)

const PROXY_PROTOCOL_MAX_HEADER = 107

func ProxyProtocolWrite(c *Connection, old []byte) []byte {
	//ngx_uint_t  port, lport;

	var buf string
	var remoteAddr, localAddr *net.TCPAddr
	var ok bool
	/*
		if (last - buf < NGX_PROXY_PROTOCOL_MAX_HEADER) {
			return NULL;
		}
	*/

	/*
		if (ngx_connection_local_sockaddr(c, NULL, 0) != NGX_OK) {
			return NULL;
		}
	*/

	//var addr net.Addr
	if remoteAddr, ok = c.conn.RemoteAddr().(*net.TCPAddr); ok {
		if len(remoteAddr.IP) == net.IPv4len {

			buf = "PROXY TCP4 "
		} else if len(remoteAddr.IP) == net.IPv6len {

			buf = "PROXY TCP6 "
		} else {

			buf = "PROXY UNKNOWN\r\n"
		}
	}

	localAddr, _ = c.conn.LocalAddr().(*net.TCPAddr)
	/*
		switch (c->sockaddr->sa_family) {

		case AF_INET:
			buf = ngx_cpymem(buf, "PROXY TCP4 ", sizeof("PROXY TCP4 ") - 1);
			break;

		case AF_INET6:
			buf = ngx_cpymem(buf, "PROXY TCP6 ", sizeof("PROXY TCP6 ") - 1);
			break;

		default:
			return ngx_cpymem(buf, "PROXY UNKNOWN" CRLF,
			sizeof("PROXY UNKNOWN" CRLF) - 1);
		}
	*/

	buf = fmt.Sprintf("%s%s %s %u %u\r\n",
		buf,
		c.conn.RemoteAddr().String(),
		c.conn.LocalAddr().String(),
		remoteAddr.Port,
		localAddr)

	return []byte(string(old) + buf)

	/*
		buf += ngx_sock_ntop(c->sockaddr, c->socklen, buf, last - buf, 0);

		*buf++ = ' ';

		buf += ngx_sock_ntop(c->local_sockaddr, c->local_socklen, buf, last - buf,
		0);

		port = ngx_inet_get_port(c->sockaddr);
		lport = ngx_inet_get_port(c->local_sockaddr);
	*/
	//return ngx_slprintf(buf, last, " %ui %ui" CRLF, port, lport);
}
