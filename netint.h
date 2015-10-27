
#ifndef NETINT_H
#define NETINT_H

#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>

/*
 * Преобразует первые size байт буфера data с сетевым порядком байт
 * в 32-битное целое без знака с порядком байт хоста.
 */
uint32_t data_ntohl(const void *data, size_t size);

int data_htonl(void *buf, uint32_t val);

/*
 * Определяет минимальное количество октетов, необходимых для хранения значения value.
 * Пустые (нулевые) старшие октеты не учитываются.
 */
int int_max_octet(uint64_t value);

#define BOBIG		1
#define BOLITTLE	2
#define BONETWORK	BOBIG
#define BOERROR		-1
/*
 * Определяет порядок байтов системы (используемый в стандартных типах данных). 
 * Возвращаемое значение - одна из констант:
 * 	BOBIG - обратный порядок байтов.
 * 	BOLITTLE - прямой порядок байтов.
 * 	BONETWORK - сетевой порядок байтов - соответствует обратному порядку байтов.
 * 	BOERROR - порядок байтов определить не удалось.
 */
int byteorder(void);


#endif 

