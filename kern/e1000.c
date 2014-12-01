#include <kern/e1000.h>
#include <kern/pci.h>
#include <kern/pmap.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>


// LAB 6: Your driver code here

int i; 

volatile uint32_t *e1000;

struct tx_desc tx_array[E1000_TXDESC] __attribute__ ((aligned (16)));
struct tx_pkt tx_pkt_bufs[E1000_TXDESC];


int 
e1000_init(struct pci_func *pci)
{
	pci_func_enable(pci);

	e1000 = mmio_map_region(pci->reg_base[0], pci->reg_size[0]);
		
	memset(tx_array, 0, sizeof(struct tx_desc) * E1000_TXDESC);
	memset(tx_pkt_bufs, 0, sizeof(struct tx_pkt) * E1000_TXDESC);
	for (i = 0; i < E1000_TXDESC; i++) {
		tx_array[i].addr = PADDR(tx_pkt_bufs[i].buf);
		tx_array[i].status |= E1000_TXD_STAT_DD;
	}


	/* Transmit initialization */
	// Program the Transmit Descriptor Base Address Registers
	e1000[E1000_TDBAL] = PADDR(tx_array);
	e1000[E1000_TDBAH] = 0x0;

	// Set the Transmit Descriptor Length Register
	e1000[E1000_TDLEN] = sizeof(struct tx_desc) * E1000_TXDESC;

	// Set the Transmit Descriptor Head and Tail Registers
	e1000[E1000_TDH] = 0x0;
	e1000[E1000_TDT] = 0x0;

	// Initialize the Transmit Control Register 
	e1000[E1000_TCTL] |= E1000_TCTL_EN;
	e1000[E1000_TCTL] |= E1000_TCTL_PSP;
	e1000[E1000_TCTL] &= ~E1000_TCTL_CT;
	e1000[E1000_TCTL] |= (0x10) << 4;
	e1000[E1000_TCTL] &= ~E1000_TCTL_COLD;
	e1000[E1000_TCTL] |= (0x40) << 12;

	// Program the Transmit IPG Register
	e1000[E1000_TIPG] = 0x0;
	e1000[E1000_TIPG] |= (0x6) << 20; // IPGR2 
	e1000[E1000_TIPG] |= (0x4) << 10; // IPGR1
	e1000[E1000_TIPG] |= 0xA; // IPGR
	
	cprintf("addr is %08x \n", e1000[E1000_STATUS]);		
	return 0;

}

int
e1000_transmit(char *data, int len)
{
	if (len > TX_PKT_SIZE) {
		return -E_PKT_TOO_BIG;
	}

	uint32_t tdt = e1000[E1000_TDT];

	// Check if next tx desc is free
	if (tx_array[tdt].status & E1000_TXD_STAT_DD) {
		memcpy(tx_pkt_bufs[tdt].buf, data, len);
		tx_array[tdt].length = len;

		//cprintf("E1000 tx len: %0dx\n", len);
		//HEXDUMP("tx dump:", data, len);

		tx_array[tdt].status &= ~E1000_TXD_STAT_DD;
		tx_array[tdt].cmd |= E1000_TXD_CMD_RS;
		tx_array[tdt].cmd |= E1000_TXD_CMD_EOP;

		e1000[E1000_TDT] = (tdt + 1) % E1000_TXDESC;
	}
	else { // queue full!
		return -E_TX_FULL;
	}
	
	return 0;
}



