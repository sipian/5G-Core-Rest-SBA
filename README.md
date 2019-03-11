# 5G-Core-Rest-SBA

## How to run

### Create Docker containers

```
docker network create vepc

docker run -ti --rm --name ran --cap-add NET_ADMIN --network vepc -v `pwd`:/code -w /code/src codeme0720/5g_sba:v1 /bin/bash

docker run -ti --rm --name ausf --cap-add NET_ADMIN --network vepc -v `pwd`:/code -w /code/src codeme0720/5g_sba:v1 /bin/bash

docker run -ti --rm --name amf --cap-add NET_ADMIN --network vepc -v `pwd`:/code -w /code/src codeme0720/5g_sba:v1 /bin/bash

docker run -ti --rm --name smf --cap-add NET_ADMIN --network vepc -v `pwd`:/code -w /code/src codeme0720/5g_sba:v1 /bin/bash

docker run -ti --rm --name upf --cap-add NET_ADMIN --network vepc -v `pwd`:/code -w /code/src codeme0720/5g_sba:v1 /bin/bash

docker run -ti --rm --name udm --cap-add NET_ADMIN --network vepc -v `pwd`:/code -w /code/src codeme0720/5g_sba:v1 /bin/bash

docker run -ti --rm --name sink --cap-add NET_ADMIN --network vepc -v `pwd`:/code -w /code/src codeme0720/5g_sba:v1 /bin/bash
```

### Start Consul Server for NRF

From host machine get the IP address of your interface and add this value in [src/discover.h#L10](src/discover.h#L10)
```
consul agent -server -bootstrap -bind=<Interface-IP-Address> -client=<Interface-IP-Address> -ui -data-dir=/tmp/consul -node=consul-node
```

### Start the 5G Network Function modules

Run the follow commands in every docker container shell

```
export LD_LIBRARY_PATH=/usr/local/lib/
```

> Start the following modules in the order specified below.

#### Run UDM
```
sudo service mysql restart
make udm.out
./udm.out 10
# 10 denotes the num_of_udm_threads for N_udm interface
```

#### Run AUSF
```
make ausf.out
./ausf.out 10
# 10 denotes the num_of_ausf_threads for N_ausf interface
```

#### Run SMF
```
make smf.out
./smf.out 10
# 10 denotes the num_of_smf_threads for N_smf and N4 interfaces
```

#### Run AMF
```
make amf.out
./amf.out 10
# 10 denotes the num_of_amf_threads for N_amf interface
```

#### Run UPF
```
make upf.out
./upf.out 10 10 10
# 10 denotes the num_of_upf_threads for N3, N4 and N6 interface
```

#### Run SINK
```
make sink.out
./settings.sh
./sink.out 10
# 10 denotes the num_of_sink_server threads
```

#### Run RAN
```
make ransim.out
./settings.sh
./ransim.out 10 10
# first 10 denotes the num_of_ue_threads at RAN
# second 10 denotes the time duration of each UE thread
```

To disable logging, change the DEBUG macro present [here](https://github.com/sipian/5G-Core-Rest-SBA/blob/master/src/utils.h#L51) to 0.

