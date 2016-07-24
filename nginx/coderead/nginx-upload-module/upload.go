package main

import (
	"fmt"
	"io"
	"net/http"
	"os"
)

func main() {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {

		/*
		   Content-Disposition: form-data; name="file.name"

		   http_mp3.go
		   --------------------------d1e4b746e77824dd
		   Content-Disposition: form-data; name="file.content_type"

		   application/octet-stream
		   --------------------------d1e4b746e77824dd
		   Content-Disposition: form-data; name="file.path"

		   /tmp/4/0000000024
		   --------------------------d1e4b746e77824dd
		   Content-Disposition: form-data; name="file.md5"

		   108307089dfa8cc718e97349b7412b9c
		   --------------------------d1e4b746e77824dd
		   Content-Disposition: form-data; name="file.size"
		*/

		reader, err := r.MultipartReader()

		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		/*
		   file.name:http_mp3.go
		   file.content_type:application/octet-stream
		   file.path:/tmp/6/0000000046
		   file.md5:108307089dfa8cc718e97349b7412b9c
		   file.size:619
		*/

		for {
			part, err := reader.NextPart()
			if err == io.EOF {
				break
			}

			fmt.Printf("%s:", part.FormName())
			io.Copy(os.Stdout, part)
			fmt.Printf("\n")
			if part.FileName() == "" {
				continue
			}
		}

	})
	http.ListenAndServe(":8080", nil)
}
