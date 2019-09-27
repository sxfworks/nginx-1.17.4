#!/bin/bash

echo "Begin compile nginx-1.17.4 ......"

if [ $# != 1 ]
then
  echo "Usage: build.sh prefix_path"
  exit -1
fi

if [[ $1 != /* ]]
then
  echo "Prefix should begin with char '/'"
  exit -1
fi

pushd ./deps

tar -xvf nginx_upload_module-2.2.0.tar.gz -C nginx-upload-module-2.3.0
tar -xvf openssl-1.0.0a.tar.gz -C openssl-1.0.0a
tar -xvf zlib-1.1.0.tar.gz -C zlib-1.1.0
tar -xvf pcre-8.00.tar.bz2 -C pcre-8.00

popd

echo ./configure --prefix=$1 \
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
            --with-openssl=../openssl-1.0.0a \
            --with-pcre=../pcre-8.00 \
            --with-zlib=../zlib-1.1.0 \
            --add-module=../nginx-upload-module-2.3.0 \

echo "Complete compile nginx-1.17.4 success."
