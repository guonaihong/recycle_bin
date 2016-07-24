# nginx-upload-module 安装文档

* 下载nginx-upload-module模块(这个不是原作者的,原作者的模块不能在nginx 1.9.2上编译)
git clone https://github.com/hongzhidao/nginx-upload-module

* install ./configure --add-module={module_dir} && make && make install

* conf
```
server {
    client_max_body_size 100m;
    listen       81;

    # Upload form should be submitted to this location
    location /upload {
        # Pass altered request body to this location
        upload_pass   @test;

        # Store files to this directory
        # The directory is hashed, subdirectories 0 1 2 3 4 5 6 7 8 9 should exist
        upload_store /tmp 1;
        
        # Allow uploaded files to be read only by user
        upload_store_access user:r;

        # Set specified fields in request body
        upload_set_form_field $upload_field_name.name "$upload_file_name";
        upload_set_form_field $upload_field_name.content_type "$upload_content_type";
        upload_set_form_field $upload_field_name.path "$upload_tmp_path";

        # Inform backend about hash and size of a file
        upload_aggregate_form_field "$upload_field_name.md5" "$upload_file_md5";
        upload_aggregate_form_field "$upload_field_name.size" "$upload_file_size";

        upload_pass_form_field "^.*$";

        upload_cleanup 400 404 499 500-505;
    }

    # Pass altered request body to a backend
    location @test {
        proxy_pass   http://localhost:8080;
    }
}
```

* 后面代理的服务,主要打印nginx传过来的信息，主要就是文件名,md5,size,文件保存在哪个路径下等一些信息
```go

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
```
