#!/bin/bash
echo "Begin compile nginx-1.17.4 ......"

if [ $# != 1 ]
then
  echo "Usage: compile.sh debug/release/example"
  exit -1
fi

if [ $1 != "debug" -a $1 != "release" -a $1 != "example" ]
then
  echo "$1 not in debug/release/example"
  exit -1
fi

nginx_root=`pwd`
mkdir -p $nginx_root/output/deps
mkdir -p $nginx_root/output/nginx

osname=`uname`

if [ $1 == 'release' ]
then
  pushd ./deps
  if [ osname == 'Linux' ]
  then
    tar -xvf nginx_upload_module-2.2.0.tar.gz -C $nginx_root/output/deps/
  fi
  tar -xvf openssl-1.0.0a.tar.gz -C $nginx_root/output/deps/
  tar -xvf zlib-1.1.0.tar.gz -C $nginx_root/output/deps/
  tar -xvf pcre-8.00.tar.bz2 -C $nginx_root/output/deps/
  popd
fi


compile_cmd="./configure --prefix=$nginx_root/output/nginx"
if [ $1 == 'release' ]
then
  compile_cmd=$compile_cmd" --with-threads"
  compile_cmd=$compile_cmd" --with-http_ssl_module"
  compile_cmd=$compile_cmd" --with-http_v2_module"
  compile_cmd=$compile_cmd" --with-http_realip_module"
  compile_cmd=$compile_cmd" --with-http_addition_module"
  compile_cmd=$compile_cmd" --with-http_dav_module"
  compile_cmd=$compile_cmd" --with-http_flv_module"
  compile_cmd=$compile_cmd" --with-http_mp4_module"
  compile_cmd=$compile_cmd" --with-http_gunzip_module"
  compile_cmd=$compile_cmd" --with-http_gzip_static_module"
  compile_cmd=$compile_cmd" --with-stream"
  compile_cmd=$compile_cmd" --with-stream_ssl_module"
  compile_cmd=$compile_cmd" --with-openssl=${nginx_root}/output/deps/openssl-1.0.0a"
  compile_cmd=$compile_cmd" --with-pcre=$nginx_root/output/deps/pcre-8.00"
  compile_cmd=$compile_cmd" --with-zlib=$nginx_root/output/deps/zlib-1.1.0"

  if [ osname == 'Linux' ]
  then
    compile_cmd=$compile_cmd" --with-file-aio"

    compile_cmd=$compile_cmd" --add-module=$nginx_root/output/deps/nginx_upload_module-2.2.0"
  fi
else
  compile_cmd=$compile_cmd" --with-cc-opt='-g'"
  compile_cmd=$compile_cmd" --with-debug"
fi

if [ $1 == "example" ]
then
  for dir in `ls $nginx_root/example/`
  do
    compile_cmd=$compile_cmd" --add-module=$nginx_root/example/$dir"
  done
fi
echo $compile_cmd
sh $compile_cmd && make && make install && echo "Complete compile nginx-1.17.4 success."