rm memoria/cfg/memoria.config
rm cpu/cfg/cpu.config
rm kernel/cfg/kernel.config

echo "PUERTO_ESCUCHA=8002
TAM_MEMORIA=4096
TAM_PAGINA=64
ENTRADAS_POR_TABLA=4
RETARDO_MEMORIA=1000
ALGORITMO_REEMPLAZO=CLOCK-M
MARCOS_POR_PROCESO=4
RETARDO_SWAP=2000
PATH_SWAP=/home/utnso/tp-2022-1c-system32/memoria/swap" > memoria/cfg/memoria.config

echo "ENTRADAS_TLB=4
REEMPLAZO_TLB=LRU
RETARDO_NOOP=1000
IP_MEMORIA=127.0.0.1
PUERTO_MEMORIA=8002
PUERTO_ESCUCHA_DISPATCH=8001
PUERTO_ESCUCHA_INTERRUPT=8005" > cpu/cfg/cpu.config

echo "IP_MEMORIA=127.0.0.1
PUERTO_MEMORIA=8002
IP_CPU=127.0.0.1
PUERTO_CPU_DISPATCH=8001
PUERTO_CPU_INTERRUPT=8005
PUERTO_ESCUCHA=8000
ALGORITMO_PLANIFICACION=FIFO
ESTIMACION_INICIAL=10000
ALFA=0.5
GRADO_MULTIPROGRAMACION=4
TIEMPO_MAXIMO_BLOQUEADO=100000" > kernel/cfg/kernel.config