#!/bin/bash
echo "Begin compile nginx-1.17.4 ......"

nginx_root=`pwd`
mkdir -p $nginx_root/output/deps
mkdir -p $nginx_root/output/nginx

# create tar target dir
mkdir -p $nginx_root/output/deps/nginx-upload-module-2.3.0
mkdir -p $nginx_root/output/deps/openssl-1.0.0a
mkdir -p $nginx_root/output/deps/zlib-1.1.0
mkdir -p $nginx_root/output/deps/pcre-8.00

pushd ./deps
tar -xvf nginx_upload_module-2.2.0.tar.gz -C $nginx_root/output/deps/nginx-upload-module-2.3.0
tar -xvf openssl-1.0.0a.tar.gz -C $nginx_root/output/deps/openssl-1.0.0a
tar -xvf zlib-1.1.0.tar.gz -C $nginx_root/output/deps/zlib-1.1.0
tar -xvf pcre-8.00.tar.bz2 -C $nginx_root/output/deps/pcre-8.00
popd

echo ./configure --prefix=$nginx_root/output/nginx \
            --with-file-aio \
            --with-threads \
            --with-http_ssl_module \
            --with-http_v2_module \
            --with-http_realip_module \
            --with-http_addition_module \
            --with-http_dav_module \
            --with-http_flv_module \ 
            --with-http_mp4_module \
            --with-http_gunzip_module \
            --with-http_gzip_static_module \
            --with-stream \
            --with-stream_ssl_module \
            --with-openssl=$nginx_root/output/deps/openssl-1.0.0a \
            --with-pcre=$nginx_root/output/deps/pcre-8.00 \
            --with-zlib=$nginx_root/output/deps/zlib-1.1.0 \
            --add-module=$nginx_root/output/deps/nginx-upload-module-2.3.0 \

echo "Complete compile nginx-1.17.4 success."
