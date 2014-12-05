#include <kern/e1000.h>
#include <kern/pci.h>
#include <kern/pmap.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>


// LAB 6: Your driver code here

int i; 

volatile uint32_t *e1000;

struct tx_desc tx_array[E1000_TXD] __attribute__ ((aligned (16)));
struct tx_pkt tx_pkt_bufs[E1000_TXD];


struct rcv_desc rcv_array[E1000_RCVD] __attribute__ ((aligned (16)));
struct rcv_pkt rcv_pkt_bufs[E1000_RCVD];

int 
e1000_init(struct pci_func *pci)
{
	pci_func_enable(pci);

	e1000 = mmio_map_region(pci->reg_base[0], pci->reg_size[0]);
		
	memset(tx_array, 0, sizeof(struct tx_desc) * E1000_TXD);
	memset(tx_pkt_bufs, 0, sizeof(struct tx_pkt) * E1000_TXD);
	for (i = 0; i < E1000_TXD; i++) {
		tx_array[i].addr = PADDR(tx_pkt_bufs[i].buf);
		tx_array[i].status |= E1000_TXD_STAT_DD;
	}

	// Initialize rcv desc buffer array
	memset(rcv_array, 0x0, sizeof(struct rcv_desc) * E1000_RCVD);
	memset(rcv_pkt_bufs, 0x0, sizeof(struct rcv_pkt) * E1000_RCVD);
	for (i = 0; i < E1000_RCVD; i++) {
		rcv_array[i].addr = PADDR(rcv_pkt_bufs[i].buf);
	}

	/* Transmit initialization */
	// Transmit Descriptor Base Address Registers
	e1000[E1000_TDBAL] = PADDR(tx_array);
	e1000[E1000_TDBAH] = 0x0;

	// Set Transmit Descriptor Length Register
	e1000[E1000_TDLEN] = sizeof(struct tx_desc) * E1000_TXD;

	// Set Transmit Descriptor Head and Tail Registers

	e1000[E1000_TDT] = 0x0;
	e1000[E1000_TDH] = 0x0;
	
	// Initialize the Transmit Control Register 
	e1000[E1000_TCTL] |= E1000_TCTL_EN;
	e1000[E1000_TCTL] |= E1000_TCTL_PSP;
	e1000[E1000_TCTL] &= ~E1000_TCTL_CT;
	//E1000_TCTL_CT to binary 0b111111110000
	e1000[E1000_TCTL] |= (0x10) << 4;
	e1000[E1000_TCTL] &= ~E1000_TCTL_COLD;
	//E1000_TCTL_COLD to binary 0b1111111111000000000000
	e1000[E1000_TCTL] |= (0x40) << 12;


	// Program the Transmit IPG Register
	e1000[E1000_TIPG] = 0x0;	//reserve
	e1000[E1000_TIPG] |= (0x6) << 20; // IPGR2 
	e1000[E1000_TIPG] |= (0x8) << 10; // IPGR1
	e1000[E1000_TIPG] |= 0xA; // IPGT


	/* Recieve Initialization */
	// Recieve Address Registers
	//MAC address 52:54:00:12:(34:56) -> Low (High) bits.  
	// 00110100 : 00110110 : 00000000 : 00001100 : 00100010 : 00111000
	
	e1000[E1000_RAL] = 0x12005452; 

	e1000[E1000_RAH] |= 0x5634;
	e1000[E1000_RAH] |= 0x1 << 31;

	// Program the Receive Descriptor Base Address Registers
	e1000[E1000_RDBAL] = PADDR(rcv_array);
        e1000[E1000_RDBAH] = 0x0;

	// Set the Receive Descriptor Length Register
	e1000[E1000_RDLEN] = sizeof(struct rcv_desc)  * E1000_RCVD;
	//e1000[E1000_RDLEN] = 16 * 128;
	

        // Set the Receive Descriptor Head and Tail Registers
	
	//e1000[E1000_RDT] = 0x0;
	//e1000[E1000_RDH] = E1000_RCVD - 1;

	e1000[E1000_RDT] = 127;
	e1000[E1000_RDH] = 0x0;
	
	// Initialize the Receive Control Register
	e1000[E1000_RCTL] |= E1000_RCTL_EN;
	e1000[E1000_RCTL] &= ~E1000_RCTL_LPE;
	e1000[E1000_RCTL] &= ~E1000_RCTL_LBM;
	e1000[E1000_RCTL] &= ~E1000_RCTL_RDMTS;
	e1000[E1000_RCTL] &= ~E1000_RCTL_MO;
	e1000[E1000_RCTL] |= E1000_RCTL_BAM;
	e1000[E1000_RCTL] &= ~E1000_RCTL_SZ; // 2048 byte size
	e1000[E1000_RCTL] |= E1000_RCTL_SECRC;


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

		tx_array[tdt].status &= ~E1000_TXD_STAT_DD;
		tx_array[tdt].cmd |= E1000_TXD_CMD_RS;
		tx_array[tdt].cmd |= E1000_TXD_CMD_EOP;

		e1000[E1000_TDT] = (tdt + 1) % E1000_TXD;
	}
	else { // queue full
		return -E_TX_FULL;
	}
	
	return 0;
}

int
e1000_receive(char *data)
{
	uint32_t rdt, len;
	//rdt = e1000[E1000_RDT];
	rdt = (e1000[E1000_RDT] + 1) % E1000_RCVD;


	if (rcv_array[rdt].status & E1000_RXD_STAT_DD) {
		if (!(rcv_array[rdt].status & E1000_RXD_STAT_EOP)) {
			panic("e1000_receive: end of packet \n");
		}
		len = rcv_array[rdt].length;
		
		memcpy(data, rcv_pkt_bufs[rdt].buf, len);
		rcv_array[rdt].status &= ~E1000_RXD_STAT_DD;
		rcv_array[rdt].status &= ~E1000_RXD_STAT_EOP;
		//e1000[E1000_RDT] = (rdt + 1) % E1000_RCVD;
		e1000[E1000_RDT] = rdt;
	
		return len;
	}

	//queue empty
	return -E_RCV_EMPTY;
}
