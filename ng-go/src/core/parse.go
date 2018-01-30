package core

import (
	"strconv"
)

const MAX_SIZE_T_VALUE = 9223372036854775807

func ParseSize(line string) int {
	var unit byte
	var err error
	var l int
	var size, scale, max int
	l = len(line)

	if l == 0 {
		return ERROR
	}

	unit = line[l-1]

	switch unit {
	case 'K', 'k':
		l--
		max = MAX_SIZE_T_VALUE / 1024
		scale = 1024

	case 'M', 'm':
		l--
		max = MAX_SIZE_T_VALUE / (1024 * 1024)
		scale = 1024 * 1024

	default:
		max = MAX_SIZE_T_VALUE
		scale = 1
	}

	size, err = strconv.Atoi(line)
	if err != nil || size > max {
		return ERROR
	}

	size *= scale

	return size
}

func ParseTime(line string, isSec bool) int {
	type mytime int
	const (
		st_start mytime = iota
		st_year
		st_month
		st_week
		st_day
		st_hour
		st_min
		st_sec
		st_msec
		st_last
	)

	valid := false
	value := 0
	total := 0
	scale := 0

	max := 0
	cutoff := 0
	cutlim := 0
	cutoff = MAX_INT_T_VALUE / 10
	cutlim = MAX_INT_T_VALUE % 10

	step := st_month
	if isSec {
		step = st_start
	}

	p := 0
	last := len(line)
	ch := byte(0)

	for ; p < last; p++ {

		ch = line[p]
		if ch >= '0' && ch <= '9' {
			if value >= cutoff && (value > cutoff || int(ch)-int('0') > cutlim) {
				return ERROR
			}

			value = value*10 + (int(ch) - int('0'))
			p++
			valid = true
			continue
		}

		p++
		switch ch {

		case 'y':
			if step > st_start {
				return ERROR
			}
			step = st_year
			max = MAX_INT_T_VALUE / (60 * 60 * 24 * 365)
			scale = 60 * 60 * 24 * 365

		case 'M':
			if step >= st_month {
				return ERROR
			}
			step = st_month
			max = MAX_INT_T_VALUE / (60 * 60 * 24 * 30)
			scale = 60 * 60 * 24 * 30

		case 'w':
			if step >= st_week {
				return ERROR
			}
			step = st_week
			max = MAX_INT_T_VALUE / (60 * 60 * 24 * 7)
			scale = 60 * 60 * 24 * 7

		case 'd':
			if step >= st_day {
				return ERROR
			}
			step = st_day
			max = MAX_INT_T_VALUE / (60 * 60 * 24)
			scale = 60 * 60 * 24

		case 'h':
			if step >= st_hour {
				return ERROR
			}
			step = st_hour
			max = MAX_INT_T_VALUE / (60 * 60)
			scale = 60 * 60

		case 'm':
			if p < last && ch == 's' {
				if isSec || step >= st_msec {
					return ERROR
				}

				p++
				step = st_msec
				max = MAX_INT_T_VALUE
				scale = 1
			}

			if step >= st_min {
				return ERROR
			}
			step = st_min
			max = MAX_INT_T_VALUE / 60
			scale = 60

		case 's':
			if step >= st_sec {
				return ERROR
			}
			step = st_sec
			max = MAX_INT_T_VALUE
			scale = 1

		case ' ':
			if step >= st_sec {
				return ERROR
			}
			step = st_last
			max = MAX_INT_T_VALUE
			scale = 1

		default:
			return ERROR
		}

		if step != st_msec && !isSec {
			scale *= 1000
			max /= 1000
		}

		if value > max {
			return ERROR
		}

		value *= scale

		if total > MAX_INT_T_VALUE-value {
			return ERROR
		}

		total += value

		value = 0

		for p < last && line[p] == ' ' {
			p++
		}
	}

	if !valid {
		return ERROR
	}

	if !isSec {
		if value > MAX_INT_T_VALUE/1000 {
			return ERROR
		}

		value *= 1000
	}

	if total > MAX_INT_T_VALUE-value {
		return ERROR
	}

	return total + value
}
