package stream

import (
	"core"
	"log"
)

type upstreamRRPeer struct {
	//struct sockaddr                 *sockaddr;
	//socklen_t                        socklen;
	name   string
	server string

	currentWeight   int
	effectiveWeight int
	weight          int

	conns    int
	maxConns int

	fails    int
	accessed int
	checked  int

	maxFails    int
	failTimeout int
	slowStart   int
	startTime   int

	down bool

	//void                            *sslSession;
	//int                              sslSessionLen;

	//next *streamUpstreamRrPeer

}

type upstreamRRPeers struct {
	number uint

	totalWeight uint

	single   bool
	weighted bool

	name string

	//stream_upstream_rr_peers_t *next

	next []upstreamRRPeers
	peer []upstreamRRPeer
}

type upstreamRRPeerData struct {
	config  uint
	peers   *upstreamRRPeers
	current *upstreamRRPeer
	tried   []uint64
	data    uint64
}

func upstreamInitRoundRobin(cf *core.Conf, us *upstreamSrvConf) int {
	u := core.URL{}
	var i, j, n, w int
	var server []upstreamServer

	//stream_upstream_rr_peer_t   *peer, **peerp;
	//stream_upstream_rr_peers_t  *peers, *backup;

	//us.peer.init = stream_upstream_init_round_robin_peer

	peers := &upstreamRRPeers{}
	if len(us.servers) > 0 {
		server = us.servers

		n = 0
		w = 0

		for i = 0; i < len(us.servers); i++ {
			if server[i].backup {
				continue
			}

			n += len(server[i].addrs)
			w += len(server[i].addrs) * int(server[i].weight)
		}

		if n == 0 {
			log.Printf("no servers in upstream \"%s\" in %s:%ui",
				us.Host, us.FileName, us.Line)
			return core.ERROR
		}

		peer := make([]upstreamRRPeer, n)
		if peer == nil {
			return core.ERROR
		}

		peers.single = (n == 1)
		peers.number = uint(n)
		peers.weighted = (w != n)
		peers.totalWeight = uint(w)
		peers.name = us.Host

		n = 0

		for i = 0; i < len(us.servers); i++ {
			if server[i].backup {
				continue
			}

			for j = 0; j < len(server[i].addrs); j++ {
				//peer[n].sockaddr = server[i].addrs[j].sockaddr
				//peer[n].socklen = server[i].addrs[j].socklen
				peer[n].name = server[i].addrs[j].Name
				peer[n].weight = int(server[i].weight)
				peer[n].effectiveWeight = int(server[i].weight)
				peer[n].currentWeight = 0
				peer[n].maxConns = int(server[i].maxConns)
				peer[n].maxFails = int(server[i].maxFails)
				peer[n].failTimeout = int(server[i].failTimeout)
				peer[n].down = server[i].down
				peer[n].server = server[i].name

				//*peerp = &peer[n]
				//peerp = &peer[n].next
				n++
			}
		}

		us.peer.data = peers

		/* backup servers */

		n = 0
		w = 0

		for i = 0; i < len(us.servers); i++ {
			if !server[i].backup {
				continue
			}

			n += len(server[i].addrs)
			w += len(server[i].addrs) * int(server[i].weight)
		}

		if n == 0 {
			return core.OK
		}

		backup := &upstreamRRPeers{}

		peer = make([]upstreamRRPeer, n)
		if peer == nil {
			return core.ERROR
		}

		peers.single = false
		backup.single = false
		backup.number = uint(n)
		backup.weighted = (w != n)
		backup.totalWeight = uint(w)
		backup.name = us.Host

		n = 0
		//peerp = &backup.peer

		for i = 0; i < len(us.servers); i++ {
			if !server[i].backup {
				continue
			}

			for j = 0; j < len(server[i].addrs); j++ {
				//peer[n].sockaddr = server[i].addrs[j].sockaddr
				//peer[n].socklen = server[i].addrs[j].socklen
				peer[n].name = server[i].addrs[j].Name
				peer[n].weight = int(server[i].weight)
				peer[n].effectiveWeight = int(server[i].weight)
				peer[n].currentWeight = 0
				peer[n].maxConns = int(server[i].maxConns)
				peer[n].maxFails = int(server[i].maxFails)
				peer[n].failTimeout = int(server[i].failTimeout)
				peer[n].down = server[i].down
				peer[n].server = server[i].name

				//*peerp = &peer[n]
				//peerp = &peer[n].next
				n++
			}
		}

		//peers.next = backup

		return core.OK
	}

	/* an upstream implicitly defined by proxy_pass, etc. */

	if us.Port == 0 {
		log.Printf("no port in upstream \"%V\" in %s:%ui",
			us.Host, us.FileName, us.Line)
		return core.ERROR
	}

	u.Host = us.Host
	u.Port = us.Port

	/*
		if inet_resolve_host(cf.pool, &u) != core.OK {
			if u.Err {
				log.Printf("%s in upstream \"%V\" in %s:%ui",
					u.Err, us.Host, us.FileName, us.Line)
			}

			return core.ERROR
		}
	*/

	n = len(u.Addrs)

	/*
		peers = pcalloc(cf.pool, sizeof(stream_upstream_rr_peers_t))
		if peers == nil {
			return core.ERROR
		}

		peer = pcalloc(cf.pool, sizeof(stream_upstream_rr_peer_t)*n)
		if peer == nil {
			return core.ERROR
		}
	*/

	peers.single = (n == 1)
	peers.number = uint(n)
	peers.weighted = false
	peers.totalWeight = uint(n)
	peers.name = us.Host

	//peerp = &peers.peer

	for i = 0; i < len(u.Addrs); i++ {
		/*
			peer[i].sockaddr = u.addrs[i].sockaddr
			peer[i].socklen = u.addrs[i].socklen
			peer[i].name = u.addrs[i].name
			peer[i].weight = 1
			peer[i].effectiveWeight = 1
			peer[i].currentWeight = 0
			peer[i].maxConns = 0
			peer[i].maxFails = 1
			peer[i].failTimeout = 10
			*peerp = &peer[i]
			peerp = &peer[i].next
		*/
	}

	//us.peer.data = peers

	/* implicitly defined upstream has no backup servers */

	return core.OK
}

func upstreamInitRoundRobinPeer(s *streamSession, us *upstreamSrvConf) int {

	var rrp *upstreamRRPeerData
	n := uint(0)
	rrp = s.upstream.peer.Data.(*upstreamRRPeerData)

	if rrp == nil {
		/*
			rrp = ngx_palloc(s.connection.pool,
				sizeof(ngx_stream_upstream_rr_peer_data_t))
		*/
		if rrp == nil {
			return core.ERROR
		}

		s.upstream.peer.Data = rrp
	}

	rrp.peers = us.peer.data.(*upstreamRRPeers)
	rrp.current = nil
	rrp.config = 0

	n = rrp.peers.number

	if len(rrp.peers.next) > 0 && rrp.peers.next[0].number > n {
		n = rrp.peers.next[0].number
	}

	if n <= 8*8 {
		rrp.tried[0] = rrp.data
		rrp.data = 0

	} else {
		n = (n + (8*8 - 1)) / (8 * 8)

		rrp.tried = make([]uint64, n)
		if rrp.tried == nil {
			return core.ERROR
		}
	}

	s.upstream.peer.Get = upstreamGetRoundRobinPeer
	s.upstream.peer.Free = upstreamFreeRoundRobinPeer
	s.upstream.peer.Notify = upstreamNotifyRoundRobinPeer
	//s.upstream.peer.tries = upstreamTries(rrp.peers)

	return core.OK
}

func upstreamCreateRoundRobinPeer(s *streamSession, ur *Resolved) int {
	//u_char                              *p;
	//size_t                               len;
	//socklen_t                            socklen;
	var n uint
	//struct sockaddr                     *sockaddr;
	//ngx_stream_upstream_rr_peer_t       *peer, **peerp;
	//ngx_stream_upstream_rr_peers_t      *peers;
	var rrp *upstreamRRPeerData
	peers := &upstreamRRPeers{}

	rrp = s.upstream.peer.Data.(*upstreamRRPeerData)

	if rrp == nil {
		/*
		   rrp = ngx_palloc(s.connection.pool,
		                    sizeof(ngx_stream_upstream_rr_peer_data_t));
		*/
		if rrp == nil {
			return core.ERROR
		}

		s.upstream.peer.Data = rrp
	}

	/*
	   peers = ngx_pcalloc(s.connection.pool,
	                       sizeof(ngx_stream_upstream_rr_peers_t));
	   if (peers == NULL) {
	       return core.ERROR;
	   }
	*/

	naddrs := len(ur.addrs)
	peer := make([]upstreamRRPeer, naddrs)
	if peer == nil {
		return core.ERROR
	}

	peers.single = (naddrs == 1)
	peers.number = uint(naddrs)
	peers.name = ur.host

	//TODO
	if false {
		//if ur.sockaddr
		//peer[0].sockaddr = ur.sockaddr
		//peer[0].socklen = ur.socklen
		peer[0].name = ur.name
		peer[0].weight = 1
		peer[0].effectiveWeight = 1
		peer[0].currentWeight = 0
		peer[0].maxConns = 0
		peer[0].maxFails = 1
		peer[0].failTimeout = 10
		peers.peer = peer

	} else {
		//peerp = &peers.peer

		for i := 0; i < len(ur.addrs); i++ {

			/*
				socklen = ur.addrs[i].socklen

				sockaddr = ngx_palloc(s.connection.pool, socklen)
				if sockaddr == NULL {
					return core.ERROR
				}

				ngx_memcpy(sockaddr, ur.addrs[i].sockaddr, socklen)
				ngx_inet_set_port(sockaddr, ur.port)

				p = ngx_pnalloc(s.connection.pool, NGX_SOCKADDR_STRLEN)
				if p == NULL {
					return core.ERROR
				}

				len = ngx_sock_ntop(sockaddr, socklen, p, NGX_SOCKADDR_STRLEN, 1)

				peer[i].sockaddr = sockaddr
				peer[i].socklen = socklen
				peer[i].name.len = len
				peer[i].name.data = p
				peer[i].weight = 1
				peer[i].effectiveWeight = 1
				peer[i].currentWeight = 0
				peer[i].maxConns = 0
				peer[i].maxFails = 1
				peer[i].failTimeout = 10
				*peerp = &peer[i]
				peerp = &peer[i].next
			*/
		}
	}

	rrp.peers = peers
	rrp.current = nil
	rrp.config = 0

	if rrp.peers.number <= 8*8 {
		rrp.tried[0] = rrp.data
		rrp.data = 0

	} else {
		n = (rrp.peers.number + (8*8 - 1)) / (8 * 8)

		rrp.tried = make([]uint64, n)
		if rrp.tried == nil {
			return core.ERROR
		}
	}

	s.upstream.peer.Get = upstreamGetRoundRobinPeer
	s.upstream.peer.Free = upstreamFreeRoundRobinPeer
	//s.upstream.peer.tries = upstreamTries(rrp.peers)

	//ssl
	/*
	   s.upstream.peer.set_session = ngx_stream_upstream_empty_set_session;
	   s.upstream.peer.save_session = ngx_stream_upstream_empty_save_session;
	*/

	return core.OK
}

func upstreamGetRoundRobinPeer(pc *core.PeerConnection, data interface{}) int {
	//ngx_stream_upstream_rr_peer_data_t * rrp = data

	rc := 0
	var i, n uint64
	//ngx_stream_upstream_rr_peer_t   *peer;
	//ngx_stream_upstream_rr_peers_t  *peers;

	rrp := data.(*upstreamRRPeerData)
	log.Printf("get rr peer, try: %u", pc.Tries)

	//pc.connection = nil;

	peers := rrp.peers

	var peer *upstreamRRPeer

	if peers.single {
		peer = &peers.peer[0]

		if peer.down {
			goto failed
		}

		if peer.maxConns > 0 && peer.conns >= peer.maxConns {
			goto failed
		}

		rrp.current = peer

	} else {

		/* there are several peers */

		peer = upstreamGetPeer(rrp)

		if peer == nil {
			goto failed
		}

		log.Printf("get rr peer, current: %p %i",
			peer, peer.currentWeight)
	}

	//pc.sockaddr = peer.sockaddr;
	//pc.socklen = peer.socklen;
	pc.Name = peer.name

	peer.conns++

	//ngx_stream_upstream_rr_peers_unlock(peers);

	return core.OK

failed:

	if len(peers.next) > 0 {

		log.Printf("backup servers")

		rrp.peers = &peers.next[0]

		n = uint64(rrp.peers.number) + uint64((8*8-1)/(8*8))

		for i = 0; i < n; i++ {
			rrp.tried[i] = 0
		}

		//ngx_stream_upstream_rr_peers_unlock(peers);

		rc = upstreamGetRoundRobinPeer(pc, rrp)

		if rc != core.BUSY {
			return rc
		}

		//ngx_stream_upstream_rr_peers_wlock(peers)
	}

	//ngx_stream_upstream_rr_peers_unlock(peers)

	pc.Name = peers.name

	return core.BUSY
}

func upstreamGetPeer(rrp *upstreamRRPeerData) *upstreamRRPeer {
	//ngx_stream_upstream_rr_peer_t  *peer, *best;

	//now = ngx_time()

	now := 0
	total := 0

	var peer, best *upstreamRRPeer

	m := uint64(0)
	var n, p uint

	for i, _ := range rrp.peers.peer {

		peer = &rrp.peers.peer[i]

		n = uint(i) / (8 * 8)
		m = uint64(1) << uint64(i) % uint64(8*8)
		if (rrp.tried[n] & m) == m {
			continue
		}

		if peer.down {
			continue
		}

		if peer.maxFails > 0 &&
			peer.fails >= int(peer.maxFails) &&
			now-peer.checked <= peer.failTimeout {
			continue
		}

		if peer.maxConns > 0 && peer.conns >= peer.maxConns {
			continue
		}

		peer.currentWeight += peer.effectiveWeight
		total += peer.effectiveWeight

		if peer.effectiveWeight < peer.weight {
			peer.effectiveWeight++
		}

		if best == nil || peer.currentWeight > best.currentWeight {
			best = peer
			p = uint(i)
		}
	}

	if best == nil {
		return nil
	}

	rrp.current = best

	n = p / (8 * 8)
	m = 1 << p % (8 * 8)

	rrp.tried[n] |= m

	best.currentWeight -= total

	if now-best.checked > best.failTimeout {
		best.checked = now
	}

	return best
}

func upstreamFreeRoundRobinPeer(pc *core.PeerConnection, data interface{}, state uint) {
	//ngx_stream_upstream_rr_peer_data_t * rrp = data

	rrp := data.(upstreamRRPeerData)
	//time_t                          now;
	//ngx_stream_upstream_rr_peer_t * peer

	var peer *upstreamRRPeer
	log.Printf("free rr peer %ui %ui", pc.Tries, state)

	peer = rrp.current

	now := 0
	//ngx_stream_upstream_rr_peers_rlock(rrp.peers)
	//ngx_stream_upstream_rr_peer_lock(rrp.peers, peer)

	if rrp.peers.single {
		peer.conns--

		//ngx_stream_upstream_rr_peer_unlock(rrp.peers, peer)
		//ngx_stream_upstream_rr_peers_unlock(rrp.peers)

		pc.Tries = 0
		return
	}

	if (state & core.PEER_FAILED) == core.PEER_FAILED {
		//now = ngx_time()

		peer.fails++
		peer.accessed = now
		peer.checked = now

		if peer.maxFails > 0 {
			peer.effectiveWeight -= peer.weight / int(peer.maxFails)

			if peer.fails >= int(peer.maxFails) {
				log.Printf("upstream server temporarily disabled")
			}
		}

		log.Printf("free rr peer failed: %p %i",
			peer, peer.effectiveWeight)

		if peer.effectiveWeight < 0 {
			peer.effectiveWeight = 0
		}

	} else {

		/* mark peer live if check passed */

		if peer.accessed < peer.checked {
			peer.fails = 0
		}
	}

	peer.conns--

	//ngx_stream_upstream_rr_peer_unlock(rrp.peers, peer);
	//ngx_stream_upstream_rr_peers_unlock(rrp.peers);

	if pc.Tries > 0 {
		pc.Tries--
	}
}

func upstreamNotifyRoundRobinPeer(pc *core.PeerConnection,
	data interface{}, Type uint) {
	//ngx_stream_upstream_rr_peer_data_t * rrp = data

	rrp := data.(upstreamRRPeerData)
	//ngx_stream_upstream_rr_peer_t * peer

	peer := rrp.current

	if Type == UPSTREAM_NOTIFY_CONNECT {
		//pc.connection.Type == SOCK_STREAM
		//ngx_stream_upstream_rr_peers_rlock(rrp.peers);
		//ngx_stream_upstream_rr_peer_lock(rrp.peers, peer);

		if peer.accessed < peer.checked {
			peer.fails = 0
		}

		//ngx_stream_upstream_rr_peer_unlock(rrp.peers, peer);
		//ngx_stream_upstream_rr_peers_unlock(rrp.peers);
	}
}
