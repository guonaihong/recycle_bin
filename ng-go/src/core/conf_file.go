package core

import (
	"errors"
	"fmt"
	"math"
	"strconv"
	"strings"
)

const (
	CONF_UNSET      = math.MaxUint64
	CONF_UNSET_UINT = math.MaxUint64
	CONF_UNSET_PTR  = 0
	CONF_UNSET_SIZE = math.MaxUint64
	CONF_UNSET_MSEC = math.MaxUint64
)

type ConfPostHandler func(cf *Conf, data interface{}, conf interface{}) (int, error)

type ConfPost struct {
	PostHandler ConfPostHandler
}

func ConfSetFlagSlot(cf *Conf, cmd *Command, conf interface{}) (int, error) {

	var post ConfPost
	var fp *int
	/*
		ngx_flag_t * fp

		fp = (p + cmd.Offset)

		if *fp != NGX_CONF_UNSET {
			return "is duplicate"
		}
	*/

	value := cf.Args

	if strings.ToLower(value[1]) == "on" {
		*fp = 1

	} else if strings.ToLower(value[1]) == "off" {
		*fp = 0

	} else {
		errMsg := fmt.Errorf("invalid value \"%s\" in \"%s\" directive, "+
			"it must be \"on\" or \"off\"",
			value[1], cmd.Name)
		return ERROR, errMsg
	}

	if cmd.Post != nil {
		post = cmd.Post.(ConfPost)
		return post.PostHandler(cf, post, fp)
	}

	return OK, nil
}

func ConfSetMsecSlot(cf *Conf, cmd *Command, conf interface{}) (int, error) {
	//char * p = conf

	var msp *int
	var post ConfPost

	/*
		msp = (ngx_msec_t *) (p + cmd.offset);
		if (*msp != NGX_CONF_UNSET_MSEC) {
			return "is duplicate";
		}
	*/

	value := cf.Args

	*msp = ParseTime(value[1], false)
	if *msp == ERROR {
		return ERROR, errors.New("invalid value")
	}

	if cmd.Post != nil {
		post = cmd.Post.(ConfPost)
		return post.PostHandler(cf, post, msp)
	}

	return OK, nil
}

func ConfSetSizeSlot(cf *Conf, cmd *Command, conf interface{}) (int, error) {

	var sp *int
	var post ConfPost

	/*
		sp =  (p + cmd.offset);
		if (*sp != NGX_CONF_UNSET_SIZE) {
			return "is duplicate";
		}
	*/

	value := cf.Args

	*sp = ParseSize(value[1])
	if *sp == ERROR {
		return ERROR, errors.New("invalid value")
	}

	if cmd.Post != nil {
		post = cmd.Post.(ConfPost)
		return post.PostHandler(cf, post, sp)
	}

	return OK, nil
}

func ConfSetNumSlot(cf *Conf, cmd *Command, conf interface{}) (int, error) {
	//char * p = conf

	var np *int
	var post ConfPost
	var err error

	/*
		np = (ngx_int_t *) (p + cmd.offset);

		if (*np != NGX_CONF_UNSET) {
			return "is duplicate";
		}
	*/

	value := cf.Args
	*np, err = strconv.Atoi(value[1])
	if err != nil {
		return ERROR, errors.New("invalid number")
	}

	if cmd.Post != nil {
		post = cmd.Post.(ConfPost)
		return post.PostHandler(cf, post, np)
	}

	return OK, nil
}
