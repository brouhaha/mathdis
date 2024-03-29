/*
 * mathdis.c: Atari math box microcode disassembler for Red Baron,
 *            Battlezone, and Tempest
 *
 * To have any hope of understanding the disassembly, you'd better have the
 * math box schematics, a 2901 data sheet, and a lot of patience.
 *
 * Copyright 1992, 1993, 1997, 2019 Eric Smith
 * SPDX-License-Identifier: GPL-3.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define I_MAX 256

uint32_t instruction [I_MAX];
int entry [I_MAX];

char *progname;

void print_usage (FILE *f)
{
  fprintf (f, "Usage: %s [objectfile [listingfile]]\n", progname);
}

void fatal (int retval, char *fmt, ...)
{
  va_list ap;

  if (retval == 1)
    print_usage (stderr);
  else
    {
      fprintf (stderr, "%s: ", progname);
      va_start (ap, fmt);
      vfprintf (stderr, fmt, ap);
      va_end (ap);
    }
  exit (retval);
}

#define BUFSIZE 80


const char *dispatch_rom_fn = "036174-01.b1";

const char *ucode_rom_fn[] =
{
  "036175-01.m1",
  "036176-01.l1",
  "036177-01.k1",
  "036178-01.j1",
  "036179-01.h1",
  "036180-01.f1",
};


void read_inst (void)
{
  FILE *f;

  for (int addr = 0; addr < I_MAX; addr++)
  {
    entry [addr] = -1;
    instruction [addr] = 0;
  }

  f = fopen(dispatch_rom_fn, "rb");
  if (! f)
    fatal(2, "error opening dispatch ROM\n");
  for (int i = 0; i < 32; i++)
  {
    uint8_t d;
    if (1 != fread(& d, sizeof(d), 1, f))
      fatal(3, "error reading dispath ROM\n");
    entry[d] = i;
  }
  fclose(f);

  for (int i = 0; i < sizeof(ucode_rom_fn) / sizeof(char *); i++)
  {
    f = fopen(ucode_rom_fn[i], "rb");
    if (! f)
      fatal(2, "error opening microcode ROM %s\n", ucode_rom_fn[i]);
    for (int j = 0; j < 256; j++)
    {
      uint8_t d;
      if (1 != fread(& d, sizeof(d), 1, f))
        fatal(3, "error reading dispath ROM\n");
      instruction[j] |= d << (i * 4);
    }
    fclose(f);
  }
}

void dis_fcn (FILE *file, int f, int s, int a, int b, int c)
{
  static char *pc [2] = { "", " + 1" };
  static char *mc [2] = { " - 1", "" };

  switch ((f << 3) | s)
    {
    case 000: fprintf (file, "R%x + q%s", a, pc [c]);      break;
    case 001: fprintf (file, "R%x + R%x%s", a, b, pc [c]); break;
    case 002: fprintf (file, "q%s", pc [c]);               break;
    case 003: fprintf (file, "R%x%s", b, pc [c]);          break;
    case 004: fprintf (file, "R%x%s", a, pc [c]);          break;
    case 005: fprintf (file, "d + R%x%s", a, pc [c]);      break;
    case 006: fprintf (file, "d + q%s", pc [c]);           break;
    case 007: fprintf (file, "d%s", pc [c]);               break;

    case 010: fprintf (file, "q - R%x%s", a, mc [c]);      break;
    case 011: fprintf (file, "R%x - R%x%s", b, a, mc [c]); break;
    case 012: fprintf (file, "q%s", mc [c]);               break;
    case 013: fprintf (file, "R%x%s", b, mc [c]);          break;
    case 014: fprintf (file, "R%x%s", a, mc [c]);          break;
    case 015: fprintf (file, "R%x - d%s", a, mc [c]);      break;
    case 016: fprintf (file, "q - d%s", mc [c]);           break;
    case 017: fprintf (file, "-d%s", mc [c]);              break;

    case 020: fprintf (file, "R%x - q%s", a, mc [c]);      break;
    case 021: fprintf (file, "R%x - R%x%s", a, b, mc [c]); break;
    case 022: fprintf (file, "-q%s", mc [c]);              break;
    case 023: fprintf (file, "-R%x%s", b, mc [c]);         break;
    case 024: fprintf (file, "-R%x%s", a, mc [c]);         break;
    case 025: fprintf (file, "d - R%x%s", a, mc [c]);      break;
    case 026: fprintf (file, "d - q%s", mc [c]);           break;
    case 027: fprintf (file, "d%s", mc [c]);               break;

    case 030: fprintf (file, "R%x | q", a);           break;
    case 031: fprintf (file, "R%x | R%x", a, b);      break;
    case 032: fprintf (file, "q");                    break;
    case 033: fprintf (file, "R%x", b);               break;
    case 034: fprintf (file, "R%x", a);               break;
    case 035: fprintf (file, "d | R%x", a);           break;
    case 036: fprintf (file, "d | q");                break;
    case 037: fprintf (file, "d");                    break;

    case 040: fprintf (file, "R%x & q", a);           break;
    case 041: fprintf (file, "R%x & R%x", a, b);      break;
    case 042: fprintf (file, "0");                    break;
    case 043: fprintf (file, "0");                    break;
    case 044: fprintf (file, "0");                    break;
    case 045: fprintf (file, "d & R%x", a);           break;
    case 046: fprintf (file, "d & q");                break;
    case 047: fprintf (file, "0");                    break;

    case 050: fprintf (file, "!R%x & q", a);          break;
    case 051: fprintf (file, "!R%x & R%x", a, b);     break;
    case 052: fprintf (file, "q");                    break;
    case 053: fprintf (file, "R%x", b);               break;
    case 054: fprintf (file, "R%x", a);               break;
    case 055: fprintf (file, "!d & R%x", a);          break;
    case 056: fprintf (file, "!d & q");               break;
    case 057: fprintf (file, "0");                    break;

    case 060: fprintf (file, "R%x ^ q", a);           break;
    case 061: fprintf (file, "R%x ^ R%x", a, b);      break;
    case 062: fprintf (file, "q");                    break;
    case 063: fprintf (file, "R%x", b);               break;
    case 064: fprintf (file, "R%x", a);               break;
    case 065: fprintf (file, "d ^ R%x", a);           break;
    case 066: fprintf (file, "d ^ q");                break;
    case 067: fprintf (file, "d");                    break;

    case 070: fprintf (file, "!R%x ^ q", a);          break;
    case 071: fprintf (file, "!R%x ^ R%x", a, b);     break;
    case 072: fprintf (file, "!q");                   break;
    case 073: fprintf (file, "!R%x", b);              break;
    case 074: fprintf (file, "!R%x", a);              break;
    case 075: fprintf (file, "!d & R%x", a);          break;
    case 076: fprintf (file, "!d & q");               break;
    case 077: fprintf (file, "!d");                   break;
    }
}

void disassemble_inst (FILE *file, unsigned long inst, int *eos)
{
  int f, sh, sl, d, a, b, c, m, l, s, h, j, jmp_addr, show_y;

  a = (inst >> 20) & 0x0f;
  b = (inst >> 16) & 0x0f;

  f = (inst >> 8) & 0x07;
  
  sh = (inst >> 12) & 0x0b;
  if (sh > 7)
    sh -= 4;
  sl = (inst >> 12) & 0x07;

  d = (inst >> 4) & 0x07;

  h = inst & 0x000800;
  l = inst & 0x000080;
  s = inst & 0x000008;
  j = inst & 0x000004;
  m = inst & 0x000002;
  c = inst & 0x000001;

  *eos = h || (j && !s);

  show_y = h || (j && s);

  fprintf (file, h ? "H" : " ");
  fprintf (file, s ? "S" : " ");
  fprintf (file, l ? "L" : " ");

  if (l)
    jmp_addr = inst >> 16;

  if (j)
    {
      if (s)
	fprintf (file, " (->%03x) ", jmp_addr);
      else
	fprintf (file, "  ->%03x  ", jmp_addr);
    }
  else
    fprintf (file, "         ");

  if ((inst & 0xfffb) == 0x0090)
    fprintf (file, "--load addr--");
  else
    {
      if (show_y)
	switch (d)
	  {
	  case 0: fprintf (file, "y = q = ("); break;
	  case 1: fprintf (file, "y = ("); break;
	  case 2: fprintf (file, "y = R%x, R%x = (", a, b); break;
	  case 3: fprintf (file, "y = R%x = (", b); break;
	  case 4: fprintf (file, "q = 1/2 * q, R%x = 1/2 * (y = ", b); break;
	  case 5: fprintf (file, "R%x = 1/2 * (y = ", b); break;
	  case 6: fprintf (file, "q = 2 * q, R%x = 2 * (y = ", b); break;
	  case 7: fprintf (file, "R%x = 2 * (y = ", b); break;
	  }
      else
	switch (d)
	  {
	  case 0: fprintf (file, "q = ("); break;
	  case 1: fprintf (file, "y = ("); break;
	  case 2: fprintf (file, "R%x = (", b); break;
	  case 3: fprintf (file, "R%x = (", b); break;
	  case 4: fprintf (file, "q = 1/2 * q, R%x = 1/2 * (", b); break;
	  case 5: fprintf (file, "R%x = 1/2 * (", b); break;
	  case 6: fprintf (file, "q = 2 * q, R%x = 2 * (", b); break;
	  case 7: fprintf (file, "R%x = 2 * (", b); break;
	  }

      dis_fcn (file, f, sh, a, b, c);

      if (sh != sl)
	{
	  fprintf (file, ", ");
	  dis_fcn (file, f, sl, a, b, c);
	}

      if (m)
	{
	  fprintf (file, " ? ");
	  dis_fcn (file, f, sh ^ 2, a, b, c);
	  if (sh != sl)
	    {
	      fprintf (file, ", ");
	      dis_fcn (file, f, sl ^ 2, a, b, c);
	    }
	}

      fprintf (file, ") ");
    }
}

void disassemble_all (FILE *f)
{
  int addr;
  int eos;

  fprintf (f, "                          jump\n"
              "entry  addr   hex    hsl  addr   function\n"
              "-----  ----  ------  ---  -----  ---------------------------------\n");

  for (addr = 0; addr < I_MAX; addr++)
    {
      if (entry [addr] >= 0)
	fprintf (f, "%03x->  ", entry [addr]);
      else
	fprintf (f, "       ");

      fprintf (f, "%03x:  %06x  ", addr, instruction [addr]);
      disassemble_inst (f, instruction [addr], & eos);
      fprintf (f, "\n");
      if (eos)
	fprintf (f, "\n");
    }
}


int main (int argc, char *argv[])
{
  FILE *outfile = stdout;
  char *fn;
  progname = argv [0];
  read_inst ();
  disassemble_all (outfile);
  return (0);
}
