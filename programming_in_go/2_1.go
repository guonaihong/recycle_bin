package main

import (
	 "errors"
	"fmt"
	"log"
    "sort"
	"net/http"
	"strconv"
	"strings"
)

type stats struct {
	numbers []float64
	avg     float64
	mid  float64
}

func parseRequest(r *http.Request) ([]float64, error) {
	var numbers []float64

	strs, found := r.Form["numbers"]
	if !found || len(strs) <= 0 {
		return numbers, errors.New("r.Form error")
	}

	text := strings.Replace(strs[0], ",", " ", -1)
	for _, v := range strings.Fields(text) {
		if x, err := strconv.ParseFloat(v, 64); err != nil {
			return numbers, err
        } else {
            numbers = append(numbers, x)
        }
	}

    if len(numbers) == 0 {
        return numbers, errors.New("numbers len 0")
    }
	return numbers, nil
}

func float64_sum(numbers []float64) float64{
    sum := float64(0.0)
    for _, v := range numbers {
        sum += v
    }
    return sum
}

func float64_median(numbers []float64) float64{
    mid := len(numbers) / 2
    result := numbers[mid]
    
    if len(numbers) >1 && len(numbers) % 2 == 0 {
        result = (result + numbers[mid - 1]) / 2
    }
    return result
}

func getStats(numbers []float64) stats{
    var s stats
    s.numbers = numbers
    sort.Float64s(s.numbers)
    s.avg = float64_sum(numbers) / float64(len(numbers))
    s.mid = float64_median(numbers)
    return s
}

func fmtStats(s *stats) string{
    return fmt.Sprintf(`<table border="1">
    <tr> <th colspan = "2">result </th></tr>
    <tr> <td>numbers</td><td>%v</td></tr>
    <tr> <td>count</td><td>%d</td></tr>
    <tr> <td>avg</td><td>%f</td></tr>
    <tr> <td>mid</td><td>%f</td></tr>
    </table>
`, s.numbers, len(s.numbers), s.avg, s.mid)
}

func handle(w http.ResponseWriter, r *http.Request) {
	forms := `<form action="/" method = "post">
        number:<input type = "text" name = "numbers" /> </br>
        <input type = "submit" value = "submit">
    </form>
    `
	error_info := `<p class = "error"> %s </p>`

	w.Header().Add("Content-Type", "text/html")
	w.WriteHeader(200)
	fmt.Fprintf(w, forms)

    var s stats
    var n []float64
	err := r.ParseForm()
	if err != nil {
		goto fail
	}

	n, err = parseRequest(r)
	if err != nil {
		goto fail
	}

    s = getStats(n)
    fmt.Fprintf(w, fmtStats(&s))
    return
fail:
	log.Printf("error:%s\n", err.Error())
	fmt.Fprintf(w, error_info, err.Error())
}

func main() {
	http.HandleFunc("/", handle)
	err := http.ListenAndServe(":1234", nil)
	if err != nil {
		log.Fatal("listenAndServe: ", err)
	}
}
