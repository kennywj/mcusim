/* TODO:
 * - stop dispatching packets directly from interrupt handler context,
 * - stop disabling interrupts during packet transmission.
 *
 * Both features will need either packet queues + dynamic allocation or
 * two big static atomic ringbuffers.
 */
#include "c_types.h"
#include "comm.h"
#include "cobs.h"
#include "crc16.h"
#include "esp8266_if.h"

#include "FreeRTOS.h"
#include "task.h"

// Shift beginnig of the buffer so payload is aligned
#define BUF_ALIGN_OFFSET (__BIGGEST_ALIGNMENT__ - 1)

struct decoder {
	struct cobs_decoder cobs;
	uint8_t buf[BUF_ALIGN_OFFSET + COBS_ENCODED_SIZE(MAX_MESSAGE_SIZE)];

	uint32_t proto_errors;
	uint32_t crc_errors;
	comm_callback_t cb;
};

struct encoder {
	uint8_t buf[COBS_ENCODED_SIZE(MAX_MESSAGE_SIZE)];
	size_t idx;
};


struct decoder dec_uart0;
struct encoder enc_uart0;


/* ------------------------------------------------------------------ send */
/* TX implementation here is quite hacky and is inherited from times when
   simple HDLC-like framing was used. It was possible to transfer several
   chunks of data prepared on stack directly to the UART FIFO, and then
   send end of frame.

   Currently used COBS encoder has to see all frame at once, so we copy
   chunks of data into a static buffer. When frame is complete, it's encoded
   and sent to UART. Since I don't use any OS-level locks, interrupt has
   to be disabled since beginning of this process to the end to provide
   exclusive access to encoder. That's not good and code should be
   refactored later (hopefully).
*/

//static uint32_t irq_level;

void ICACHE_FLASH_ATTR
encoder_init(struct encoder *e)
{
	e->idx = 0;
}

bool ICACHE_FLASH_ATTR
encoder_put_data(struct encoder *e, void *data, size_t len)
{
	if (e->idx + len > sizeof(e->buf)) {
		return FALSE;
	}

	memcpy(e->buf + e->idx, data, len);
	e->idx += len;

	return TRUE;
}

bool ICACHE_FLASH_ATTR
encoder_finalize(struct encoder *e)
{
	if (COBS_ENCODED_SIZE(e->idx + 2) > sizeof(e->buf)) {
		return FALSE;
	}

	uint16_t crc = crc16_block(e->buf, e->idx);
	encoder_put_data(e, &crc, sizeof(crc)); /* Caution: assule LE here */

	ssize_t final_size = cobs_encode(e->buf, e->idx, sizeof(e->buf));
	if (final_size < 0) {
		e->idx = 0;
		return FALSE;
	} else {
		e->idx = final_size;
		return TRUE;
	}
}

void ICACHE_FLASH_ATTR
comm_send_begin(uint8_t c) {
	//irq_level = irq_save();
	encoder_put_data(&enc_uart0, &c, 1);
}

void ICACHE_FLASH_ATTR
comm_send_u8(uint8_t c) {
	encoder_put_data(&enc_uart0, &c, 1);
}

void ICACHE_FLASH_ATTR
comm_send_data(uint8_t *data, size_t n)
{
	encoder_put_data(&enc_uart0, data, n);
}

void ICACHE_FLASH_ATTR
comm_send_status(uint8_t s)
{
	comm_send_begin(MSG_STATUS);
	comm_send_u8(s);
	comm_send_end();
}

void ICACHE_FLASH_ATTR
comm_send_end()
{
	size_t i;
	
	encoder_finalize(&enc_uart0);
	// out put to console port
	esp8266_output(enc_uart0.buf, enc_uart0.idx);
	
	//__disable_irq();
	//for (i = 0; i < enc_uart0.idx; i++)
	//	uart_tx_one_char(enc_uart0.buf[i]);
	//__enable_irq();
	enc_uart0.idx = 0;

	//irq_restore(irq_level);
	/* ets_intr_unlock(); */
}


/* ------------------------------------------------------------------ receive */

static inline void ICACHE_FLASH_ATTR
decoder_check_and_dispatch_cb(void *decoder, uint8_t *data, size_t len)
{
	struct decoder *dec = decoder;
	uint16_t crc_msg;
	uint16_t crc_calc;

	if (len < 3) {
		dec->proto_errors ++;
		return;
	}

	crc_calc = crc16_block(data, len - 2);
	memcpy(&crc_msg, data + len - 2, 2);
	if (crc_calc != crc_msg) {
		dec->crc_errors++;
		return;
	}

	if (dec->cb)
		dec->cb(data[0], data + 1, len - 3);
}

static inline void ICACHE_FLASH_ATTR
decoder_init(struct decoder *dec, comm_callback_t cb)
{
	cobs_decoder_init(
		&dec->cobs,
		dec->buf + BUF_ALIGN_OFFSET, sizeof(dec->buf) - BUF_ALIGN_OFFSET,
		decoder_check_and_dispatch_cb, dec);
	dec->proto_errors = 0;
	dec->crc_errors = 0;
	dec->cb = cb;
}

static inline void ICACHE_FLASH_ATTR
decoder_put_data(struct decoder *dec, void *data, size_t len)
{
	cobs_decoder_put(&dec->cobs, data, len);
}

#if 0
void uart0_rx_intr_handler(void *para)
{
	uint8_t buf[64];
	uint32_t i, n;
	uint8 c;

	if (UART_RXFIFO_FULL_INT_ST !=
		(READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST)) {
		return;
	}
	WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);

	while (1) {
		n = READ_PERI_REG(UART_STATUS(UART0)) &
			(UART_RXFIFO_CNT << UART_RXFIFO_CNT_S);
		if (!n)
			break;

		n = MIN(n, sizeof(buf));
		for (i = 0; i < n; i++) {
		    buf[i] = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
		}
		decoder_put_data(&dec_uart0, buf, n);
	}
}
#endif

static unsigned char packet_buf[2048];
static unsigned short packet_buf_index;


//
// uart receive task
//
void tsk_uart_reader(void * pvParameters)
{
//	int i = 0;
	int recv_byte;
	printf("uart reader task start...\n");

	for ( ;; ) {
		recv_byte = esp8266_input();
		if (recv_byte<0)
			break;
		packet_buf[packet_buf_index++] = (unsigned char) recv_byte;
		if (recv_byte == 0) {
			//dump flame...
			dump_frame(packet_buf, packet_buf_index, "Recv");
			decoder_put_data(&dec_uart0, packet_buf, packet_buf_index);
			packet_buf_index = 0;
		}

		if (packet_buf_index >= 2048) {
			printf("msg size is too big, something wrong...\n");
			packet_buf_index = 0;
		}
	}
	// end rx task
	vTaskDelete( NULL );
}


/* ------------------------------------------------------------------ misc */

uint8_t comm_loglevel = 0;
void ICACHE_FLASH_ATTR
comm_set_loglevel(uint8_t level)
{
	comm_loglevel = level;
}

void ICACHE_FLASH_ATTR
comm_get_stats(uint32_t *rx_errors, uint32_t *rx_crc_errors) {
	*rx_errors = dec_uart0.proto_errors + dec_uart0.crc_errors;
	*rx_crc_errors = dec_uart0.crc_errors;
}

void ICACHE_FLASH_ATTR
comm_init(comm_callback_t cb) {
	
	unsigned char ch=COBS_BYTE_EOF;
	encoder_init(&enc_uart0);
	decoder_init(&dec_uart0, cb);
	esp8266_output(&ch, 1);
	//uart_tx_one_char(COBS_BYTE_EOF);
}
