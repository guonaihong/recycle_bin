package core

type ResolverAddr struct {
	//struct sockaddr          *sockaddr;
	//socklen_t                 socklen;
	Name     string
	Priority uint16
	weight   uint16
}

type Resolver struct {
	/*
		ngx_event_t              *event;
		void                     *dummy;
		ngx_log_t                *log;

		ngx_int_t                 ident;

		ngx_array_t               connections;
		ngx_uint_t                last_connection;

		ngx_rbtree_t              name_rbtree;
		ngx_rbtree_node_t         name_sentinel;

		ngx_rbtree_t              srv_rbtree;
		ngx_rbtree_node_t         srv_sentinel;

		ngx_rbtree_t              addr_rbtree;
		ngx_rbtree_node_t         addr_sentinel;

		ngx_queue_t               name_resend_queue;
		ngx_queue_t               srv_resend_queue;
		ngx_queue_t               addr_resend_queue;

		ngx_queue_t               name_expire_queue;
		ngx_queue_t               srv_expire_queue;
		ngx_queue_t               addr_expire_queue;

		ngx_uint_t                ipv6;
		ngx_rbtree_t              addr6_rbtree;
		ngx_rbtree_node_t         addr6_sentinel;
		ngx_queue_t               addr6_resend_queue;
		ngx_queue_t               addr6_expire_queue;

		time_t                    resend_timeout;
		time_t                    tcp_timeout;
		time_t                    expire;
		time_t                    valid;

		ngx_uint_t                log_level;
	*/
}

type ResolverCtx struct {
	Name    string
	Data    interface{}
	Timeout uint
}
