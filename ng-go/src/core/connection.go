package core

import (
	"log"
	"net"
)

const (
	TCP_NODELAY_UNSET = iota
	TCP_NODELAY_SET
	TCP_NODELAY_DISABLED
)

const (
	PEER_KEEPALIVE = 1
	PEER_NEXT      = 2
	PEER_FAILED    = 4
)

type getPeerPt func(pc *PeerConnection, data interface{}) int
type freePeerPt func(pc *PeerConnection, data interface{}, state uint)
type notifyPeerPt func(pc *PeerConnection, data interface{}, tp uint)

type Listening struct {
	Handler   func(c *Connection)
	Ln        net.Listener
	Type      string //tcp
	Wildcard  bool
	Keepalive uint16
	Keepidle  int
	Keepintvl int
	Keepcnt   int
	Ipv6only  bool
	Reuseport bool
}

type Connection struct {
	Data interface{}
	//ngx_event_t        *read;
	//ngx_event_t        *write;

	conn net.Conn
	fd   int

	//ngx_recv_pt         recv;
	//ngx_send_pt         send;
	//ngx_recv_chain_pt   recv_chain;
	//ngx_send_chain_pt   send_chain;

	//ngx_listening_t    *listening;

	Sent uint

	//ngx_log_t          *log;

	Type int

	//struct sockaddr    *sockaddr;
	//socklen_t socklen
	//ngx_str_t addr_text

	//ngx_str_t proxy_protocol_addr
	//in_port_t proxy_protocol_port

	//ngx_ssl_connection_t  *ssl;

	//struct sockaddr    *local_sockaddr;
	//socklen_t           local_socklen;

	Buffer Buffer
	//ngx_buf_t          *buffer;

	//ngx_queue_t         queue;

	//ngx_atomic_uint_t   number;

	//ngx_uint_t          requests;

	//unsigned            buffered:8;

	//unsigned            log_error:3;     /* ngx_connection_log_error_e */

	//unsigned            timedout:1;
	//unsigned            error:1;
	//unsigned            destroyed:1;

	//unsigned            idle:1;
	//unsigned            reusable:1;
	//unsigned            close:1;
	//unsigned            shared:1;

	//unsigned            sendfile:1;
	//unsigned            sndlowat:1;
	TcpNodelay uint16
	//unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

	//unsigned            need_last_buf:1;

	//unsigned            busy_count:2;

	//ngx_thread_task_t  *sendfile_task;
}

func CreateListening(cf *Conf, nType string, addr string) *Listening {
	l := &Listening{}

	var err error
	if nType == "" {
		nType = "tcp"
	}

	l.Ln, err = net.Listen(nType, addr)
	if err != nil {
		log.Printf("create listen fail:%v\n", err)
	}

	return l
}

func (l *Listening) TcpLoop() {

	for {
		conn, err := l.Ln.Accept()
		if err != nil {
			log.Printf("accpet socket fail:%s\n", err)
		}

		c := &Connection{conn: conn}
		go l.Handler(c)
	}
}

func (c *Connection) Read(last int, size int) int {
	return 0
}

type PeerConnection struct {
	Connection Connection

	//struct sockaddr                 *sockaddr;
	//socklen_t                        socklen;
	Name string

	Tries     uint
	StartTime int64

	Get    getPeerPt
	Free   freePeerPt
	Notify notifyPeerPt

	Data interface{}

	//ngx_addr_t                      *local;

	Type   int
	Rcvbuf int

	//*log;

	Cached      bool
	Transparent bool

	//unsigned                         log_error:2;

}
