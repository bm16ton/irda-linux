/*
 * smcinit.c
 *
 * modified by Thomas Pinz <tom_p (at) gmx.de>
 *
 * IrDA configurator for laptops with  SMSC 47N227 SuperIO,
 * smc-ircc and not initializing BIOS (tested on Toshiba Satellite 5100
 * and Toshiba Tecra 9100).
 * Copyright (C) 2003, Rob Miller <rob@janerob.com>
 *
 * http://www.janerob.com/rob/ts5100/
 *
 *
 * Cleanups, small fixes by Claudiu Costin <claudiuc@kde.org>
 * Expansions for several modes by Thomas Pinz <tom_p@gmx.de>
 * Generalization (entry point 2e OR 4e), code cleanup, HP nc6000
 * 	by Jérôme Fenal <jfenal@free.fr>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pci/pci.h>

#include <sys/io.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/limits.h>
#include <linux/serial.h>

#include "smcinit.h"

#define PROGNAME     "smcinit"
#define AUTHOR       "Thomas Pinz"
#define AUTHOR_EMAIL "<tom_p (at) gmx.net>"


/*
 * default values 
 */
#define MACHINE "Satellite 5200"
#define SIR_IO 0x3f8
#define FIR_IO 0x130
#define FIR_IRQ 3
#define FIR_DMA 3

/*
 * lspci on 5100 says 
 */
/*
 * 00:1f.0 ISA bridge: Intel Corp. 82801CAM ISA Bridge (LPC) (rev 02) 
 */

#define BUS_LPC 0x00
#define LPC_DEV 0x1f
#define LPC_FUNC 0x00

/*
 * (*) 
 */
/*
 * Bitshifters 
 */
#define SMC_SIR_IO ( (SIR_IO & 0xFF8) >> 2 )	/* CR25, p158: shift 2 down */
#define	SMC_FIR_IO ( (FIR_IO & 0xFF8) >> 3 )	/* CR2B, p162: shift 3  down */
#define SMC_FIR_IRQ FIR_IRQ	/* CR28, p160: equal */
#define	SMC_FIR_DMA FIR_DMA	/* CR2C, p162: equal */


/*
 * 82801 registers 
 */
#define VID 0x00
#define DID 0x02
#define PIRQA_ROUT 0x60
#define PCI_DMA_C 0x90
#define COM_DEC 0xe0
#define LPC_EN 0xe6
#define GEN2_DEC 0xec

/*
 * SMSC 47N227 registers 
 */
// #define SMC_BASE 0x4e

/*
 * 47N227 UART base offsets xxx enable fifo ? 
 */
#define RCV  (base)
#define IER  ((base)+1)
#define IIR  ((base)+2)
#define FCR  ((base)+2)
#define LCR  ((base)+3)
#define MCR  ((base)+4)
#define LSR  ((base)+5)
#define MSR  ((base)+6)
#define SPR  ((base)+7)
#define DLSB (base)
#define DMSB ((base)+1)




struct option options[] = {
	{"version", no_argument, NULL, 'V'},
	{"help",    no_argument, NULL, 'h'},
	{"verbose", no_argument, NULL, 'v'},
	{"sirio",   required_argument, NULL, 's'},
	{"firio",   required_argument, NULL, 'f'},
	{"firirq",  required_argument, NULL, 'i'},
	{"firdma",  required_argument, NULL, 'd'},
	{"tty",  required_argument, NULL, 't'},
	{NULL, 0, NULL, 0}
};

static char *options_explications[] = {
	"\t\t\tshow program version",
	"\t\t\tshow this help",
	"\t\t\tbe verbose, show the compiled-in settings.",
	"\t\tset the SIR I/O port",
	"\t\tset the FIR I/O port",
	"\t\tset the FIR IRQ line",
	"\t\tset the FIR DMA channel",
	"\t\tspecify the serial tty to unbind from serial driver",
	NULL
};

static char *short_options = "Vhvs:f:i:d:t:";


void
print_usage(struct option *options_array,
	    char **options_explications_array)
{
	struct option *option;
	char *option_explication = NULL;
	int i;

	printf("%s %s %s\nUsage: %s [options]\nOptions:\n", PROGNAME,
	       VERSION, AUTHOR, PROGNAME);
	i = 0;
	option = &options_array[0];
	option_explication = options_explications_array[0];
	while (option->name != NULL) {
		printf("-%c, --%s%s", option->val, option->name,
		       (option->has_arg ? "=VALUE" : ""));
		if (options_explications_array[i] != NULL)
			printf("%s\n", options_explications_array[i]);
		else
			puts("\n");
		i++;
		option = &options[i];
		option_explication = options_explications_array[i];
	}
	printf("Report suggestions and bugs to: %s\n",
	       AUTHOR " " AUTHOR_EMAIL);
}

void print_version()
{
	printf("%s %s\n", PROGNAME, VERSION);
}

void print_settings(int sir_io, int fir_io, int fir_irq, int fir_dma)
{
	print_version();
	printf("\n");
	// printf("Default-Values for %s:\n", MACHINE);
	printf("SIR ioport: 0x%x\n", sir_io);
	printf("FIR ioport: 0x%x\n", fir_io);
	printf("FIR interupt: %d\n", fir_irq);
	printf("FIR DMA: %d\n", fir_dma);
	printf("\n");
}

void die(const char *message)
{
	fprintf(stderr, "%s: %s\n", PROGNAME, message);
	exit(1);
}

/*
 * Shamelessly taken from setserial.c by Ty T'so
 *
 * Unbind the specified device from kernel stock serial driver.
 */
int unbind_from_serial(char *device)
{
	int fd;
	struct serial_struct old_serinfo, new_serinfo;

	if ((fd = open(device, O_RDWR|O_NONBLOCK)) < 0) {
		if (/*verbosity==0 && */errno==ENOENT)
			exit(201);
		perror(device);
		exit(201);
	}
	if (ioctl(fd, TIOCGSERIAL, &old_serinfo) < 0) {
		perror("Cannot get serial info");
		exit(1);
	}
	new_serinfo = old_serinfo;
	new_serinfo.type = PORT_UNKNOWN;

	if (ioctl(fd, TIOCSSERIAL, &new_serinfo) < 0) {
		perror("Cannot set serial info");
		exit(1);
	}
	close(fd);

	return 0;
}

#if 0		 
static struct pci_dev *find_pci_device(struct pci_access *access, struct pci_filter *filter)
{
	struct pci_dev *dev;

	DEBUG("scanning PCI busses");
	pci_scan_bus(access);

	DEBUG("looking for PCI device");
	for (dev = access->devices; dev; dev = dev->next) {
		if (pci_filter_match(filter, dev)) {
			DEBUG("device found");
			DEBUG_VAL("bus", dev->bus);
			DEBUG_VAL("dev", dev->dev);
			DEBUG_VAL("func", dev->func);
			return dev;
		}
	}
	return NULL;
}
#endif


int set_smc(int * smc_base, int sir_io, int fir_io, int fir_irq, int fir_dma, int verbose)
{
	/*
	 * setpci.c 
	 */
	struct pci_access *acc;
	struct pci_dev *dev;
	u16 twobyte;
	int i = 0, chip = 0;
	int local_sir_io, local_fir_io, local_fir_irq, local_fir_dma;
	unsigned int *address, SMC_BASE = 0;
	static unsigned int possible_cfgbase[] = { 0x2e, 0x4e, 0 };

	local_sir_io = ((sir_io & 0xFF8) >> 2);	/* CR25, p158: shift 2 down */
	local_fir_io = ((fir_io & 0xFF8) >> 3);	/* CR2B, p162: shift 3 down */
	local_fir_irq = fir_irq;	/* CR28, p160: equal */
	local_fir_dma = fir_dma;	/* CR2C, p162: equal */

	if (getuid() != 0) {
		fprintf(stderr, "%s can only be used by root\n", PROGNAME);
		exit(1);
	}
	acc = pci_alloc();
	pci_init(acc);
	dev = pci_get_dev(acc, 0, BUS_LPC, LPC_DEV, LPC_FUNC);	/* 5100 also dev
								 * 1f */
	twobyte = pci_read_word(dev, VID);
	if (twobyte != 0x8086) {
		fprintf(stderr, "%s IO hub vendor %x not intel (0x8086)\n",
			PROGNAME, twobyte);
		return 1;
	}
	if (verbose) {
		printf("Detected IO hub vendor id: 0x%x\n", twobyte);
	}

	twobyte = pci_read_word(dev, DID);
	if ((twobyte != 0x24cc) & (twobyte != 0x248c)) {
		fprintf(stderr,
			"%s IO hub device %x not 82801CAM (0x248c or 0x24cc)\n",
			PROGNAME, twobyte);
		return 1;
	}
	if (verbose) {
		printf("Detected IO hub device id: 0x%x\n", twobyte);
	}

	pci_write_byte(dev, COM_DEC, 0x10);	/* comb 2f8-2ff coma 3f8-3ff */
	twobyte = pci_read_word(dev, LPC_EN);	/* LPC_EN register */
	twobyte &= 0xfffd;	/* wipe bit 1 */

	twobyte |= 0x0001;	/* set bit 0 : COMA addr range enable */
	pci_write_word(dev, LPC_EN, twobyte);
	twobyte = pci_read_word(dev, PCI_DMA_C);	/* PCI_DMA register */
	twobyte &= 0xfff3;	/* wipe bits 2,3 */

	twobyte = 0xc0c0;	/* LPC I/F DMA on, channel 3 -- rtm (??
				 * PCI DMA ?) */
	pci_write_word(dev, PCI_DMA_C, twobyte);
	pci_write_word(dev, GEN2_DEC, 0x131);	/* LPC I/F 2nd decode
						 * range */

	pci_free_dev(dev);
	pci_cleanup(acc);

	/*
	 * setsmc.c 
	 */

	/*
	 * test all possible SMC_BASE
	 */
	address = possible_cfgbase ;
	while ( (SMC_BASE = *address) != 0 ) {
		ioperm(SMC_BASE, 2, 1);
		outb(0x55, SMC_BASE);	// enter configuration state
		outb(0x0d, SMC_BASE);	// set for device id
		i = inb(SMC_BASE + 1);
		if ((i == 0x5a) || (i == 0x7a)) {
			chip = i;
			break;
		}
		address++;
	}

	if (chip != 0) {
		// if SMC 47N227 or other
		if (verbose) {
			printf("Detected smc_base: 0x%x \n \n", SMC_BASE);
			printf("Detected Chip id: 0x%x \n \n", i);
		}

		/*
		 * ! sir-io ! 
		 */
		outb(0x24, SMC_BASE);	// select CR24 - UART1 base addr
		outb(0x00, SMC_BASE + 1);	// disable UART1
		outb(0x25, SMC_BASE);	// select CR25 - UART2 base addr
		outb(local_sir_io, SMC_BASE + 1);	// bits 2-9 of 0x3f8
		i = inb(SMC_BASE + 1);
		if (verbose) {
			printf
			    ("SIR ioport register \t write: 0x%x \t read: 0x%x \n",
			     local_sir_io, i);
		}
		if (i != local_sir_io) {
			fprintf(stderr,
				"Sorry, SIR ioport could not be written. \n");
			return 1;
		}

		/*
		 * ! fir-irq ! 
		 */
		outb(0x28, SMC_BASE);	// select CR28 - UART1,2 IRQ select
		i = inb(SMC_BASE + 1);	// get current setting for both
		outb((i & 0x00) | local_fir_irq, SMC_BASE + 1);	// low order bits 
		i = inb(SMC_BASE + 1);
		if (verbose) {
			printf
			    ("FIR interrupt register\t write: 0x%d \t read: 0x%d \n",
			     local_fir_irq, i);
		}
		if (i != local_fir_irq) {
			fprintf(stderr,
				"Sorry, FIR interrupt could not be written. \n");
			return 1;
		}


		/*
		 * ! fir-io ! 
		 */
		outb(0x2B, SMC_BASE);	// CR2B - SCE (FIR) base addr
		outb(local_fir_io, SMC_BASE + 1);	// io 
		i = inb(SMC_BASE + 1);
		if (verbose) {
			printf
			    ("FIR ioport register\t write: 0x%x \t read: 0x%x \n",
			     local_fir_io, i);
		}
		if (i != local_fir_io) {
			fprintf(stderr,
				"Sorry, FIR ioport could not be written. \n");
			return 1;
		}

		/*
		 * ! fir-dma ! 
		 */
		outb(0x2C, SMC_BASE);	// CR2C - SCE (FIR) DMA select
		outb(local_fir_dma, SMC_BASE + 1);	// DMA 
		i = inb(SMC_BASE + 1);
		if (verbose) {
			printf
			    ("FIR dma register\t write: 0x%x \t read: 0x%x \n",
			     local_fir_dma, i);
		}
		if (i != local_fir_dma) {
			fprintf(stderr,
				"Sorry, FIR dma could not be written. \n");
			return 1;
		}


		outb(0x0C, SMC_BASE);	// CR0C - UART mode
		i = inb(SMC_BASE + 1);	// whatever already there
		outb((i & 0xC7) | 0x88, SMC_BASE + 1);	// enable IrDA (HPSIR)
		// mode, high speed
		outb(0x07, SMC_BASE);	// CR07 - Auto Pwr Mgt/boot drive sel
		i = inb(SMC_BASE + 1);	// whatever already there 
		outb(i | 0x20, SMC_BASE + 1);	// enable UART2 autopower down
		outb(0x0a, SMC_BASE);	// CR0a - ecp fifo / ir mux
		i = inb(SMC_BASE + 1);	// whatever already there 
		outb(i | 0x40, SMC_BASE + 1);	// send active device to ir port
		outb(0x02, SMC_BASE);	// CR02 - UART 1,2 power
		i = inb(SMC_BASE + 1);	// whatever already there
		outb(0x80, SMC_BASE + 1);	// UART2 power up mode, UART1
		// power down
		outb(0x00, SMC_BASE);	// CR00 - FDC Power/valid config cycle
		i = inb(SMC_BASE + 1);	// whatever already there
		outb(i | 0x80, SMC_BASE + 1);	// valid config cycle done
		outb(0xaa, SMC_BASE);	// ?? twiggle config register ??

		if (verbose) {
			printf("\n");
			printf
			    ("Initialization of the SMC 47Nxxx succeeded\n");
		}

	} else {
		*smc_base = 0;
		fprintf(stderr,
			"%s id:%x not SMC 47Nxxx (should be 0x5a or 0x7a)\n",
			PROGNAME, i);
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int opt, ret_val = 0;
	int verbose = 0;

	/*
	 * set default values 
	 */
	unsigned int sir_io = SIR_IO;
	unsigned int fir_io = FIR_IO;
	unsigned int fir_irq = FIR_IRQ;
	unsigned int fir_dma = FIR_DMA;
	const char *nptr;
	char *endptr;
	int smc_base;
	char tty_device[PATH_MAX];


	tty_device[0]='\0';
	tty_device[PATH_MAX-1]='\0';

	while ((opt =
		getopt_long(argc, argv, short_options, options,
			    NULL)) != -1) {
		switch (opt) {
		case 'V':
			print_version();
			exit(0);
			break;

		case 'h':
			print_usage(options, options_explications);
			exit(0);
			break;

		case 'v':
			verbose = 1;
			break;

		case 't':
			nptr = optarg;
			strncpy(tty_device, nptr, PATH_MAX);
			/*if (! strncmp(tty_device, "/dev/tty", 8) ) {
				die("Specified device not a path to a device");
			}*/
			break;
		case 's':
			nptr = optarg;
			endptr = NULL;
			sir_io = strtoul(nptr, &endptr, 0);
			if (!nptr || !*nptr || !endptr || *endptr) {
				die("Cannot convert SIR I/O to number");
			}
			break;
		case 'f':
			nptr = optarg;
			endptr = NULL;
			fir_io = strtoul(nptr, &endptr, 0);
			if (!nptr || !*nptr || !endptr || *endptr) {
				die("Cannot convert FIR I/O to number");
			}
			break;

		case 'i':
			nptr = optarg;
			endptr = NULL;
			fir_irq = strtoul(nptr, &endptr, 0);
			if (!nptr || !*nptr || !endptr || *endptr) {
				die("Cannot convert FIR IRQ to number");
			}
			break;
		case 'd':
			nptr = optarg;
			endptr = NULL;
			fir_dma = strtoul(nptr, &endptr, 0);
			if (!nptr || !*nptr || !endptr || *endptr) {
				die("Cannot convert FIR DMA to number");
			}
			break;

		default:
			break;
		}		// end switch

	}			// end while

	if (verbose) {
		print_settings(sir_io, fir_io, fir_irq, fir_dma);
	}

	if (*tty_device) {
		ret_val = unbind_from_serial(tty_device);
		/* ret_val should always be 0 set_serial exits on error */
	}
	ret_val += set_smc(&smc_base, sir_io, fir_io, fir_irq, fir_dma, verbose);

	return ret_val;
}				// end main
