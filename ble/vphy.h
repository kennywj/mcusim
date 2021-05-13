#ifndef _VPHY_H_
#define _VPHY_H_

extern unsigned int vphy_tx_pcnt, vphy_tx_bcnt, vphy_rx_pcnt, vphy_rx_bcnt;
extern char devname[DEVICE_NAME_LEN+1];
extern int baudid;

extern int vphy_output(unsigned char *data, int len);
extern int vphy_init(void (*recv)(char *, int len));
extern int vphy_exit();
extern int vphy_ctrl(int opt, int len, void *data);

#endif
// _VPHY_H_