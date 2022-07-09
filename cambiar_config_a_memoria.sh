rm memoria/cfg/memoria.config
rm cpu/cfg/cpu.config
rm kernel/cfg/kernel.config

echo "PUERTO_ESCUCHA=8002
TAM_MEMORIA=2048
TAM_PAGINA=256
ENTRADAS_POR_TABLA=4
RETARDO_MEMORIA=1000
ALGORITMO_REEMPLAZO=CLOCK
MARCOS_POR_PROCESO=3
RETARDO_SWAP=5000
PATH_SWAP=/home/utnso/tp-2022-1c-system32/memoria/swap" > memoria/cfg/memoria.config

echo "ENTRADAS_TLB=1
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
ESTIMACION_INICIAL=20000
ALFA=0.5
GRADO_MULTIPROGRAMACION=2
TIEMPO_MAXIMO_BLOQUEADO=5000" > kernel/cfg/kernel.config
