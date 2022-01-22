// tosh2450-smcinit.c
// 17-06-2003
// By José Manuel Mariño Mariño
// based on tosh5100-smcinit.c by Rob Miller.
// small compile fixes by Claudiu Costin.

#include <sys/io.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <pci/pci.h>

#include "smcinit.h"

#define INTEL_VID 0x8086

// 2450-101 uses 82801DB chip
#define i82801DB_LPC_DID 0x24C0

// lspci on 2450-101 says
// 00:1f.0 ISA bridge: Intel Corp. 82801DB ISA Bridge (LPC) (rev 02)

// The LPC Bridge function of the ICH3 (82801CAM) and ICH4 (82801DB)
// resides in PCI Device 31:Function 0.
#define BUS_LPC 0x00
#define LPC_DEV 0x1f
#define LPC_FUNC 0x00

// 82801DB registers
#define VID 0x00
#define DID 0x02
#define PIRQA_ROUT 0x60
#define PCI_DMA_C 0x90
#define COM_DEC 0xe0
#define LPC_EN 0xe6
#define GEN2_DEC 0xec

// SMSC 47N227 registers
#define SMC_BASE 0x2e

#define RCV base
#define IER base+1
#define IIR base+2
#define FCR base+2
#define LCR base+3
#define MCR base+4
#define LSR base+5
#define MSR base+6
#define SPR base+7
#define DLSB base
#define DMSB base+1

int main()
{
	int i=0;
	
	/* setpci.c */
	struct pci_access *acc;
	struct pci_dev *dev;
	u16 twobyte;
	
	acc = pci_alloc();
	pci_init(acc);
	
	dev = pci_get_dev(acc, 0, BUS_LPC, LPC_DEV, LPC_FUNC);
	
	twobyte = pci_read_word(dev,VID);
	if (twobyte != INTEL_VID){ 
		fprintf(stderr,"tosh2450-smcinit IO hub vendor %x not Intel (%x)\n",twobyte,INTEL_VID);
		return 1;
	}
	
	twobyte = pci_read_word(dev,DID);
	if (twobyte == i82801DB_LPC_DID){
		fprintf(stderr,"tosh2450-smcinit Found IO hub device 82801DB (%x)\n",i82801DB_LPC_DID);
	}
	else {
		fprintf(stderr,"tosh2450-smcinit IO hub device %x not supported.\n",twobyte);
		return 1;
	}
	
	pci_write_byte(dev, COM_DEC, 0x10);  // comb 2f8-2ff coma 3f8-3ff
	
	twobyte = pci_read_word(dev, LPC_EN);  // LPC_EN register
	twobyte &= 0xfffd;   // wipe bit 1
	twobyte |= 0x0001;   // set bit 0 : COMA addr range enable
	pci_write_word(dev, LPC_EN, twobyte);
	
	twobyte = pci_read_word(dev, PCI_DMA_C);  // PCI_DMA register
	twobyte |= 0xc00c;       // LPC I/F DMA on, channel 1 
	pci_write_word(dev, PCI_DMA_C, twobyte);
	
	pci_write_word(dev, GEN2_DEC, 0x131);  // LPC I/F 2nd decode range
	
	pci_free_dev(dev);
	
	pci_cleanup(acc);
	
	
	/* setsmc.c */
	
	ioperm(SMC_BASE, 2, 1);
	outb(0x55, SMC_BASE);    // enter configuration state
	outb(0x0d, SMC_BASE);    // set for device id
	if ((i = inb(SMC_BASE+1)) == 0x5a)  // if SMC 47N227
	{
		outb(0x24, SMC_BASE);   // select CR24 - UART1 base addr
		outb(0x00, SMC_BASE+1); // disable UART1
		
		outb(0x25, SMC_BASE);   // select CR25 - UART2 base addr
		outb(0xFE, SMC_BASE+1); // bits 2-9 of 0x3f8
		
		outb(0x28, SMC_BASE);   // select CR28 - UART1,2 IRQ select
		i = inb(SMC_BASE+1);    // get current setting for both
      
		outb((i & 0x00) | 0x07, SMC_BASE+1);  // select IRQ 7 
		
		outb(0x2B, SMC_BASE);   // CR2B - SCE (FIR) base addr
		outb(0x26, SMC_BASE+1); // 0x130 bits 2-9
		
		outb(0x2C, SMC_BASE);   // CR2C - SCE (FIR) DMA select
		outb(0x01, SMC_BASE+1); // select DMA 1
		
		outb(0x0C, SMC_BASE);   // CR0C - UART mode
		i = inb(SMC_BASE+1);    // whatever already there
		outb((i & 0xC7) | 0x88, SMC_BASE+1);   // enable IrDA (HPSIR) mode, high speed
		
		outb(0x07, SMC_BASE);   // CR07 - Auto Pwr Mgt/boot drive sel
		i = inb(SMC_BASE+1);    // whatever already there 
		outb(i | 0x20, SMC_BASE+1);  // enable UART2 autopower down
		
		outb(0x0a, SMC_BASE);   // CR0a - ecp fifo / ir mux
		i = inb(SMC_BASE+1);    // whatever already there 
		outb(i | 0x40, SMC_BASE+1);  // send active device to ir port
		
		outb(0x02, SMC_BASE);   // CR02 - UART 1,2 power
		i = inb(SMC_BASE+1);    // whatever already there
		outb(0x80, SMC_BASE+1);  // UART2 power up mode, UART1 power down
		
		outb(0x00, SMC_BASE);   // CR00 - FDC Power/valid config cycle
		i = inb(SMC_BASE + 1);  // whatever already there
		outb(i | 0x80, SMC_BASE + 1);  // valid config cycle done
		
		outb(0xaa, SMC_BASE);   // ?? twiggle config register ??
	} else {
		fprintf(stderr,"tosh2450-smcinit %x not SMC 47N227 (0x5a).\n",i);
		return 1;
	}
	
	return 0;
}

