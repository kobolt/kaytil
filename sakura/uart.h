#ifndef _UART_H
#define _UART_H

void uart0_setup(void);
void uart0_send(char *s);
char uart0_recv(void);
int uart0_pending(void);

#endif /* _UART_H */
