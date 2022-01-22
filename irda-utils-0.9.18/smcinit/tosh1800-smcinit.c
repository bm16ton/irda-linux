/*
 * tosh1800-smcinit.c
 *
 * Based on toshsat1800-irdasetup, version 0.2 2002/04/07 15:42
 *
 * 
 *
 * IrDA configurator for laptops with ALI1533 bridge (47n227 SuperIO),
 * smc-ircc and not initializing BIOS (tested on Toshiba Satellite 1800-514)
 * to be used with Linux kernel.
 * Copyright (C) 2002, Daniele Peri <peri@csai.unipa.it>
 *
 * http://lancelot.csai.unipa.it/~peri/toshsat1800-irdasetup.tgz 
 *
 *
 * Cleanups, small fixes by Claudiu Costin <claudiuc@kde.org>
 *
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

#include <sys/io.h>
#include <sys/types.h>
#include <errno.h>
#include <getopt.h>
#include <pci/pci.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "smcinit.h"

#define PROGNAME "tosh1800-smcinit"
#define AUTHOR "Daniele Peri"
#define AUTHOR_EMAIL "<peri@csai.unipa.it>"

/*
 * ALI 1533 bitmapped decode addresses
 */
struct port_decoding_access_info {
    int port;
    u8 reg;
    u8 or_mask;
};

struct port_decoding_access_info ali1533_ports[] = {
    {0x130, 0xb0, 0x80},        /* this sounds confusing: */
    {0x178, 0xb0, 0x80},        /* both ports are decoded */

    {0x3f8, 0xb4, 0x80},
    {0x2f8, 0xb4, 0x30},        /* bit 4 enables decoding, bit 5 enables something unknown it can't stand without... */
    {0x2e8, 0xb4, 0x08},
    /* the followings are all the active bits in the default bitmap of
       Satellite1800-514:

       {0x3f8, 0xb4, 0x80},
       {0x???, 0xb6, 0x80},
       {0x???, 0xb6, 0x80},
       {0x???, 0xb7, 0x40},
       {0x???, 0xb7, 0x20},
       {0x???, 0xb7, 0x10},
       {0x???, 0xb7, 0x02},
       {0x???, 0xb8, 0x80},
       {0x???, 0xb8, 0x40},
       {0x???, 0xb8, 0x02},
       {0x???, 0xb9, 0x04},
       {0x???, 0xb9, 0x02},

       According to Paul Hampson decoded ISA ports are thus:
       2e, 2f, 62, 66, 60, 64, 378-37f, 778-77f, 2f0-2f5, 3f7, 3f8-3ff

       Unfortunately I don't have any other information about the ALI1533. 
     */
    {0, 0, 0}
};

/* SMC-IRCC
 *
 *
 */

/* Chip specific config function(s) */
static int
configure_47N227_smc_ircc(int revision, int cfgbase, int sirbase,
                          int firbase, int dma, int irq);

struct smc_chip_model {
    char *name;
    u8 version_id;
    int (*config_function) (int revision, int cfgbase, int sirbase,
                            int firbase, int dma, int irq);
};

static struct smc_chip_model smc_chips[] = {
    {"LPC47N227", 0x5a, configure_47N227_smc_ircc},
    {NULL, 0, NULL}
};

/*
 * LPC47N227
 */

#define LPC47N227_CFGACCESSKEY		0x55
#define LPC47N227_CFGEXITKEY		0xaa

/* Register 0x00 */
#define LPC47N227_FDCPOWERVALIDCONF_REG		0x00
#define 	LPC47N227_FDCPOWER_MASK			0x08
#define 	LPC47N227_VALID_MASK				0x80

/* Register 0x02 */
#define LPC47N227_UART12POWER_REG				0x02
#define 	LPC47N227_UART1POWERDOWN_MASK		0x08
#define 	LPC47N227_UART2POWERDOWN_MASK		0x80

/* Register 0x07 */
#define LPC47N227_APMBOOTDRIVE_REG				0x07
#define 	LPC47N227_PARPORT2AUTOPWRDOWN_MASK	0x10 /* auto power down on if set */
#define 	LPC47N227_UART2AUTOPWRDOWN_MASK	0x20 /* auto power down on if set */
#define 	LPC47N227_UART1AUTOPWRDOWN_MASK	0x40 /* auto power down on if set */

/* Register 0x0c */
#define LPC47N227_UARTMODE0C_REG				0x0c
#define 	LPC47N227_UART2MODE_MASK			0x38
#define 	LPC47N227_UART2MODE_VAL_COM		0x00
#define 	LPC47N227_UART2MODE_VAL_IRDA		0x08
#define 	LPC47N227_UART2MODE_VAL_ASKIR		0x10

/* Register 0x0d */
#define LPC47N227_DEVICEID_REG					0x0d
#define 	LPC47N227_DEVICEID_DEFVAL			0x5a

/* Register 0x0e */
#define LPC47N227_REVISIONID_REG				0x0e

/* Register 0x25 */
#define LPC47N227_UART2BASEADDR_REG			0x25

/* Register 0x28 */
#define LPC47N227_UARTIRQSELECT_REG			0x28
#define 	LPC47N227_UART2IRQSELECT_MASK		0x0f
#define 	LPC47N227_UART1IRQSELECT_MASK		0xf0
#define 	LPC47N227_UARTIRQSELECT_VAL_NONE	0x00

/* Register 0x2b */
#define LPC47N227_FIRBASEADDR_REG				0x2b

/* Register 0x2c */
#define LPC47N227_FIRDMASELECT_REG				0x2c
#define 	LPC47N227_FIRDMASELECT_MASK		0x0f
#define 	LPC47N227_FIRDMASELECT_VAL_DMA1	0x01 /* 47n227 has three dma channels */
#define 	LPC47N227_FIRDMASELECT_VAL_DMA2	0x02
#define 	LPC47N227_FIRDMASELECT_VAL_DMA3	0x03
#define 	LPC47N227_FIRDMASELECT_VAL_NONE	0x0f


/*
 * Defaults values for Toshiba Satellite 1800-514
 */
#define DEF_CFGBASE 0x2e
#define DEF_SIRBASE 0x2e8
#define DEF_FIRBASE 0x2f8
#define DEF_DMA 0x03
#define DEF_IRQ 0x07
#define DEF_ISABRIDGE_VENDOR_ID 0x10b9
#define DEF_ISABRIDGE_DEVICE_ID 0x1533

/*
 * Actions
 */
#define ACTION_CONFIG_ALI1533_PORTS_DECODING 1
#define ACTION_INIT_SMC_IRCC 2
#define ACTION_PRINT_ALI1533_CONFIGURATION 4
#define DEF_ACTIONS ACTION_CONFIG_ALI1533_PORTS_DECODING | ACTION_INIT_SMC_IRCC

struct option options[] = {
    {"skip-decoding-cfg", no_argument, NULL, 'a'},
    {"skip-smc-ircc-cfg", no_argument, NULL, 'b'},
    {"vendor", required_argument, NULL, 'v'},
    {"device", required_argument, NULL, 'x'},
    {"cfgbase", required_argument, NULL, 'c'},
    {"sirbase", required_argument, NULL, 's'},
    {"firbase", required_argument, NULL, 'f'},
    {"dma", required_argument, NULL, 'm'},
    {"irq", required_argument, NULL, 'i'},
    {"print", no_argument, NULL, 'p'},
    {"version", no_argument, NULL, 'V'},
    {"help", no_argument, NULL, 'h'},
#ifdef DEBUG_ON
    {"debug", no_argument, NULL, 'd'},
#endif
    {NULL, 0, NULL, 0}
};

static char *options_explications[] = {
    "\tskip ISA bridge decoding configuration",
    "\tskip SMC-IRCC configuration",
    "\tlook for the specified ISA bridge PCI vendor id",
    "\tlook for the specified ISA bridge PCI device id",
    "\tset SMC-IRCC IO cfgbase address",
    "\tset SMC-IRCC IO sirbase address",
    "\tset SMC-IRCC IO firbase address",
    "\t\tset SMC-IRCC DMA channel",
    "\t\tset SMC-IRCC IRQ",
    "\t\tprint ISA bridge configuration",
    "\t\tshow program version",
    "\t\tshow this help",
#ifdef DEBUG_ON
    NULL,
#endif
    NULL
};


#ifdef DEBUG_ON
static char *short_options = "abpv:x:c:s:f:m:i:dVh";
#endif
#ifndef DEBUG_ON
static char *short_options = "abpv:x:c:s:f:m:i:Vh";
#endif

/* ALI 1533 */

static int find_ali1533_port_access_info(long port, struct port_decoding_access_info *info)
{
    struct port_decoding_access_info *p;
    int i;

    p = &ali1533_ports[0];
    i = 0;
    while (p->port != 0) {
        DEBUG_VAL("port", p->port);
        if (p->port == port) {
            memcpy(info, p, sizeof(struct port_decoding_access_info));
            return 1;
        }
        i++;
        p = &ali1533_ports[i];
    }
    return 0;
}

static int print_ali1533_port_status(struct pci_dev *dev)
{
    struct port_decoding_access_info *p;
    u8 onebyte;
    int i;

    DEBUG("printing port status");
    p = &ali1533_ports[0];
    i = 0;
    puts("ALi 1533 Decoding status:");
    while (p->port != 0) {
        DEBUG_VAL("port", p->port);
        onebyte = pci_read_byte(dev, p->reg);
        printf("port 0x%x %s\n", p->port,
               (onebyte & p->or_mask ? "enabled" : "disabled"));
        i++;
        p = &ali1533_ports[i];
    }
    return 0;
}

/* Sets ALi1533 port decoding mode and
 * returns previous port decoding mode
 * (0=disabled, 1=enabled, -1 port unknown)
 */

static int set_ali1533_port(struct pci_dev *dev, int port, int decode)
{
    struct port_decoding_access_info info;
    u8 previousval, onebyte, and_mask;
    int retval;

    DEBUG_VAL("looking for port", port);
    retval = find_ali1533_port_access_info(port, &info);
    if (retval) {
        previousval = pci_read_byte(dev, info.reg);
        DEBUG_VAL("current register value", previousval);
        and_mask = ~info.or_mask;
        DEBUG_VAL("and with mask", and_mask);
        onebyte = previousval & and_mask;
        if (decode) {
            DEBUG_VAL("or with mask", info.or_mask);
            onebyte |= info.or_mask;
        }
        DEBUG_VAL("writing value", onebyte);
        pci_write_byte(dev, info.reg, onebyte);
        return (previousval & info.or_mask) ? 1 : 0;
    } else {
        DEBUG_VAL("unknown port", port);
    }
    return -1;
}


/* PCI */

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

/* Configure ali1533 PCI-ISA bridge to decode
 * SMC-IRCC ports
 */
static int configure_ali1533_smc_ports_decoding(int vendor_id, int device_id, int cfgbase, int firbase, int sirbase, int print)
{
    struct pci_access *access;
    struct pci_dev *dev = NULL;
    struct pci_filter filter;
    int ports[2], port_previous_mode, i;

    /* claudiuc: cfgbase is not used yet */
    int dummy;
    dummy = cfgbase;

    access = pci_alloc();
    if (access == NULL) {
        DEBUG("impossible to obtain PCI access");
        return 0;
    }
    DEBUG("initializing filter");
    pci_filter_init(access, &filter);
    filter.vendor = vendor_id;
    filter.device = device_id;

    pci_init(access);

    dev = find_pci_device(access, &filter);
    if (dev) {
        if (print) {
            print_ali1533_port_status(dev);
        } else {

            syslog(LOG_NOTICE,
                   "PCI device 0x%x:0x%x found at %02x:%02x:%02x. Configuring:",
                   vendor_id, device_id, dev->bus, dev->dev, dev->func);
            ports[0] = sirbase;
            ports[1] = firbase;
            for (i = 0; i < 2; i++) {
                port_previous_mode = set_ali1533_port(dev, ports[i], 1);
                if (port_previous_mode == 1) {
                    syslog(LOG_ERR,
                           "port 0x%x was already enabled", ports[i]
                        );
                } else {
                    syslog(LOG_NOTICE,
                           "enabled decoding of port 0x%x", ports[i]
                        );
                }
            }
        }
    }
    if (dev != NULL) {
        syslog(LOG_NOTICE,
               "PCI device 0x%x:0x%x. Configuration ended",
               vendor_id, device_id);

    } else {
        syslog(LOG_ERR, "PCI device 0x%x:0x%x not found", vendor_id,
               device_id);
    }
    pci_cleanup(access);
    return dev != NULL;
}

/* SMC IRCC
 *
 *
 */

/* Configure 47N227 SuperIO IrDA subsystem  
 *
 */

static int configure_47N227_smc_ircc(int revision, int cfgbase, int sirbase, int firbase, int dma, int irq)
{
    u8 onebyte;
    int retval;

    /* claudiuc: revision is not used yet */
    int dummy;
    dummy = revision;

    DEBUG_VAL("trying to init SMC with cfgbase", cfgbase);
    retval = ioperm(cfgbase, 2, 1);
    if (retval == -1) {
        syslog(LOG_ERR, "failed to obtain ioperm for cfgbase %m");

        ERROR("failed to obtain ioperm for cfgbase");
        return 0;
    }


    outb(LPC47N227_CFGACCESSKEY, cfgbase);
    /*
       outb(LPC47N227_DEVICEID_REG, cfgbase);
       if (inb(cfgbase+1) == LPC47N227_DEVICEID_DEFVAL)
       {
     */
    /*syslog(LOG_NOTICE, "LPC47N227 present. Configuring:"); */

    /*SIR Base */
    DEBUG_VAL("setting sirbase to", sirbase);
    outb(LPC47N227_UART2BASEADDR_REG, cfgbase);
    onebyte = sirbase >> 2;
    outb(onebyte, cfgbase + 1);

    /* FIR Base */
    DEBUG_VAL("setting firbase to", firbase);
    outb(LPC47N227_FIRBASEADDR_REG, cfgbase);
    onebyte = firbase >> 3;
    outb(onebyte, cfgbase + 1);

    /* DMA */
    DEBUG_VAL("setting dma to", dma);
    outb(LPC47N227_FIRDMASELECT_REG, cfgbase);
    outb(dma & LPC47N227_FIRDMASELECT_MASK, cfgbase + 1);

    /* IRQ */
    DEBUG_VAL("setting irq to", irq);
    outb(LPC47N227_UARTIRQSELECT_REG, cfgbase);
    onebyte = inb(cfgbase + 1);
    onebyte &= LPC47N227_UART1IRQSELECT_MASK;
    onebyte |= (irq & LPC47N227_UART2IRQSELECT_MASK);
    outb(onebyte, cfgbase + 1);

    syslog(LOG_NOTICE,
           "set sirbase=0x%x, firbase=0x%x, dma=%d, irq=%d",
           sirbase, firbase, dma, irq);

    DEBUG("setting other registers");

    /* Set UART2 mode to IrDA */
    outb(LPC47N227_UARTMODE0C_REG, cfgbase);
    onebyte = inb(cfgbase + 1);
    onebyte &= ~LPC47N227_UART2MODE_MASK | LPC47N227_UART2MODE_VAL_IRDA;
    outb(onebyte, cfgbase + 1);

    /* Set UART2 AUTO POWER DOWN */
    outb(LPC47N227_APMBOOTDRIVE_REG, cfgbase);
    onebyte = inb(cfgbase + 1);
    outb(onebyte | LPC47N227_UART2AUTOPWRDOWN_MASK, cfgbase + 1);

    /* Set UART2 POWER ON */
    outb(LPC47N227_UART12POWER_REG, cfgbase);
    onebyte = inb(cfgbase + 1);
    outb(onebyte | LPC47N227_UART2POWERDOWN_MASK, cfgbase + 1);

    syslog(LOG_NOTICE,
           "set UART 2 IR mode to IrDA, auto powerdown on and powered up");

    /* Validate configuration */
    outb(LPC47N227_FDCPOWERVALIDCONF_REG, cfgbase);
    onebyte = inb(cfgbase + 1);
    outb(onebyte | LPC47N227_VALID_MASK, cfgbase + 1);

    outb(LPC47N227_CFGEXITKEY, cfgbase);
    DEBUG("configuration ended");
    return 1;
}

static struct smc_chip_model *find_smc_chip_model(struct smc_chip_model
                                                  *chips, int version_id)
{
    int i;
    for (i = 0; chips[i].name != NULL; i++) {
        if (chips[i].version_id == version_id)
            return &chips[i];
    }
    return NULL;
}

static int
configure_smc_ircc(int cfgbase, int sirbase, int firbase, int dma, int irq)
{
    u8 version_id, revision_id;
    struct smc_chip_model *chip_model;
    int retval;

    DEBUG_VAL("trying to init SMC with cfgbase", cfgbase);
    retval = ioperm(cfgbase, 2, 1);
    if (retval == -1) {
        syslog(LOG_ERR, "failed to obtain ioperm for cfgbase %m");
        ERROR("failed to obtain ioperm for cfgbase");
        return 0;
    }

    outb(LPC47N227_CFGACCESSKEY, cfgbase);
    outb(LPC47N227_DEVICEID_REG, cfgbase);
    version_id = inb(cfgbase + 1);
    outb(LPC47N227_REVISIONID_REG, cfgbase);
    revision_id = inb(cfgbase + 1);
    outb(LPC47N227_CFGEXITKEY, cfgbase);

    chip_model = find_smc_chip_model(smc_chips, version_id);
    if (chip_model != NULL) {
        syslog(LOG_NOTICE,
               "%s chip (ver 0x%x, rev 0x%x) found. Configuring:",
               chip_model->name, version_id, revision_id);
        retval =
            (*(chip_model->config_function)) (revision_id, cfgbase,
                                              sirbase, firbase, dma, irq);
        syslog(LOG_NOTICE,
               "%s chip (ver 0x%x, rev 0x%x) configuration %ssuccessfully ended",
               chip_model->name, version_id, revision_id,
               (retval ? "" : "un"));
        return 1;
    } else {
        syslog(LOG_ERR, "Unknown chip version 0x%x", version_id);
        return 0;
    }
}

void print_usage(struct option *options_array, char **options_explications_array)
{
    struct option *option;
    char *option_explication = NULL;
    int i;

    printf("%s %s %s\nUsage: %s [options]\nOptions:\n", PROGNAME, VERSION,
           AUTHOR, PROGNAME);
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

int main(int argc, char **argv)
{
    int cfgbase = DEF_CFGBASE;
    int sirbase = DEF_SIRBASE;
    int firbase = DEF_FIRBASE;
    int dma = DEF_DMA;
    int irq = DEF_IRQ;
    int isabridge_vendor_id = DEF_ISABRIDGE_VENDOR_ID;
    int isabridge_device_id = DEF_ISABRIDGE_DEVICE_ID;
    int actions = DEF_ACTIONS;
    int retval = 0, opt;


    while ((opt =
            getopt_long(argc, argv, short_options, options, NULL)) != -1) {
        switch (opt) {
        case 'a':
            {
                actions &= ~ACTION_CONFIG_ALI1533_PORTS_DECODING;
                break;
            }
        case 'b':
            {
                actions &= ~ACTION_INIT_SMC_IRCC;
                break;
            }
        case 'p':
            {
                actions |= ACTION_PRINT_ALI1533_CONFIGURATION;
                break;
            }
        case 'v':
            {
                isabridge_vendor_id = strtoul(optarg, NULL, 0);
                break;
            }
        case 'x':
            {
                isabridge_device_id = strtoul(optarg, NULL, 0);
                break;
            }
        case 'c':
            {
                cfgbase = strtoul(optarg, NULL, 0);
                break;
            }
        case 's':
            {
                sirbase = strtoul(optarg, NULL, 0);
                break;
            }
        case 'f':
            {
                firbase = strtoul(optarg, NULL, 0);
                break;
            }
        case 'm':
            {
                dma = strtoul(optarg, NULL, 0);
                break;
            }
        case 'i':
            {
                irq = strtoul(optarg, NULL, 0);
                break;
            }
#ifdef DEBUG_ON
        case 'd':
            {
                DEBUG_LEVEL(1);
                break;
            }
#endif
        case 'V':
            {
                print_version();
                exit(0);
                break;
            }
        case 'h':
            {
                print_usage(options, options_explications);
                exit(0);
                break;
            }

        default:
            break;
        }
    }

    if (getuid() != 0) {
        fprintf(stderr, "%s can only be used by root\n", PROGNAME);
        exit(1);
    }

    if (actions &
        (ACTION_INIT_SMC_IRCC | ACTION_CONFIG_ALI1533_PORTS_DECODING)) {
        syslog(LOG_INFO, "%s %s %s", PROGNAME, VERSION, AUTHOR);
    }

    if (actions & ACTION_INIT_SMC_IRCC) {
        retval = configure_smc_ircc(cfgbase, sirbase, firbase, dma, irq);
    }

    if (actions & ACTION_CONFIG_ALI1533_PORTS_DECODING && retval) {
        retval = configure_ali1533_smc_ports_decoding(isabridge_vendor_id,
                                                      isabridge_device_id,
                                                      cfgbase,
                                                      firbase,
                                                      sirbase,
                                                      actions &
                                                      ACTION_PRINT_ALI1533_CONFIGURATION);
    }

    exit(!retval);
}
