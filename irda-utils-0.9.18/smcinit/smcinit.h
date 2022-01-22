/*
 * smcinit.h
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

/*
 * Debug Macros
 */

#undef DEBUG_ON

#ifdef DEBUG_ON
static int _debug = 0;
#define DEBUG_LEVEL(level) _debug=level
#define DEBUG(label) if(_debug){fprintf(stderr, "%s: %s\n", PROGNAME, label);}
#define DEBUG_VAL(label, val) if(_debug){fprintf(stderr, "%s: %s: 0x%x\n", PROGNAME, label, val);}
#define DEBUG_STRING(label, val) if(_debug){fprintf(stderr, "%s: %s: %s\n", PROGNAME, label, val);}
#else
#define DEBUG_LEVEL(level)
#define DEBUG(label)
#define DEBUG_VAL(label, val)
#define DEBUG_STRING(label, val)
#endif

#define ERROR(label) fprintf(stderr, "%s: %s: %s\n", PROGNAME, label, strerror(errno));

#define VERSION      "0.5cvs"
