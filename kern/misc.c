/* misc.c - definitions of misc functions */
/*
 *  PUPA  --  Preliminary Universal Programming Architecture for GRUB
 *  Copyright (C) 1999,2000,2001,2002,2003,2004  Free Software Foundation, Inc.
 *
 *  PUPA is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PUPA; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <pupa/misc.h>
#include <pupa/err.h>
#include <pupa/mm.h>
#include <stdarg.h>
#include <pupa/term.h>
#include <pupa/env.h>

void *
pupa_memmove (void *dest, const void *src, pupa_size_t n)
{
  char *d = (char *) dest;
  const char *s = (const char *) src;

  if (d < s)
    while (n--)
      *d++ = *s++;
  else
    {
      d += n;
      s += n;
      
      while (n--)
	*--d = *--s;
    }
  
  return dest;
}
/* GCC emits references to memcpy() for struct copies etc.  */
void *memcpy (void *dest, const void *src, pupa_size_t n)
  __attribute__ ((alias ("pupa_memmove")));

char *
pupa_strcpy (char *dest, const char *src)
{
  char *p = dest;

  while ((*p++ = *src++) != '\0')
    ;

  return dest;
}

char *
pupa_strncpy (char *dest, const char *src, int c)
{
  char *p = dest;
  int pos = 0;

  while ((*p++ = *src++) != '\0' && c > pos)
    pos++;

  return dest;
}

char *
pupa_stpcpy (char *dest, const char *src)
{
  char *d = dest;
  const char *s = src;

  do
    *d++ = *s;
  while (*s++ != '\0');

  return d - 1;
}

char *
pupa_strcat (char *dest, const char *src)
{
  char *p = dest;

  while (*p)
    p++;

  while ((*p++ = *src++) != '\0')
    ;

  return dest;
}

int
pupa_printf (const char *fmt, ...)
{
  va_list ap;
  int ret;
  
  va_start (ap, fmt);
  ret = pupa_vprintf (fmt, ap);
  va_end (ap);

  return ret;
}  

int
pupa_vprintf (const char *fmt, va_list args)
{
  return pupa_vsprintf (0, fmt, args);
}

int
pupa_memcmp (const void *s1, const void *s2, pupa_size_t n)
{
  const char *t1 = s1;
  const char *t2 = s2;
  
  while (n--)
    {
      if (*t1 != *t2)
	return (int) *t1 - (int) *t2;

      t1++;
      t2++;
    }

  return 0;
}

int
pupa_strcmp (const char *s1, const char *s2)
{
  while (*s1 && *s2)
    {
      if (*s1 != *s2)
	return (int) *s1 - (int) *s2;
      
      s1++;
      s2++;
    }

  return (int) *s1 - (int) *s2;
}

int
pupa_strncmp (const char *s1, const char *s2, int c)
{
  int p = 1;

  while (*s1 && *s2 && p < c)
    {
      if (*s1 != *s2)
	return (int) *s1 - (int) *s2;
      
      s1++;
      s2++;
      p++;
    }

  return (int) *s1 - (int) *s2;
}

char *
pupa_strchr (const char *s, int c)
{
  while (*s)
    {
      if (*s == c)
	return (char *) s;
      s++;
    }

  return 0;
}

char *
pupa_strrchr (const char *s, int c)
{
  char *p = 0;

  while (*s)
    {
      if (*s == c)
	p = (char *) s;
      s++;
    }

  return p;
}

int
pupa_isspace (int c)
{
  return (c == '\n' || c == '\r' || c == ' ' || c == '\t');
}

int
pupa_isprint (int c)
{
  return (c >= ' ' && c <= '~');
}

int
pupa_isalpha (int c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int
pupa_isdigit (int c)
{
  return (c >= '0' && c <= '9');
}

int
pupa_isgraph (int c)
{
  return (c >= '!' && c <= '~');
}

int
pupa_tolower (int c)
{
  if (c >= 'A' && c <= 'Z')
    return c - 'A' + 'a';

  return c;
}

unsigned long
pupa_strtoul (const char *str, char **end, int base)
{
  unsigned long num = 0;
  int found = 0;
  
  /* Skip white spaces.  */
  while (*str && pupa_isspace (*str))
    str++;
  
  /* Guess the base, if not specified. The prefix `0x' means 16, and
     the prefix `0' means 8.  */
  if (str[0] == '0')
    {
      if (str[1] == 'x')
	{
	  if (base == 0 || base == 16)
	    {
	      base = 16;
	      str += 2;
	    }
	}
      else if (str[1] >= '0' && str[1] <= '7')
	base = 8;
    }
  
  if (base == 0)
    base = 10;

  while (*str)
    {
      unsigned long digit;

      digit = pupa_tolower (*str) - '0';
      if (digit > 9)
	{
	  digit += '0' - 'a' + 10;
	  if (digit >= (unsigned long) base)
	    break;
	}

      found = 1;
      
      if (num > (~0UL - digit) / base)
	{
	  pupa_error (PUPA_ERR_OUT_OF_RANGE, "overflow is detected");
	  return 0;
	}

      num += num * base + digit;
      str++;
    }

  if (! found)
    {
      pupa_error (PUPA_ERR_BAD_NUMBER, "unrecognized number");
      return 0;
    }
  
  if (end)
    *end = (char *) str;

  return num;
}

char *
pupa_strdup (const char *s)
{
  pupa_size_t len;
  char *p;
  
  len = pupa_strlen (s) + 1;
  p = (char *) pupa_malloc (len);
  if (! p)
    return 0;

  return pupa_memcpy (p, s, len);
}

char *
pupa_strndup (const char *s, pupa_size_t n)
{
  pupa_size_t len = 0;
  char *p = (char *) s;
  
  while (*(p++) && len < n)
    len++;

  len = pupa_strlen (s) + 1;
  p = (char *) pupa_malloc (len);
  if (! p)
    return 0;

  return pupa_memcpy (p, s, len);
}

void *
pupa_memset (void *s, int c, pupa_size_t n)
{
  unsigned char *p = (unsigned char *) s;

  while (n--)
    *p++ = (unsigned char) c;

  return s;
}

pupa_size_t
pupa_strlen (const char *s)
{
  const char *p = s;

  while (*p)
    p++;

  return p - s;
}

static inline void
pupa_reverse (char *str)
{
  char *p = str + pupa_strlen (str) - 1;

  while (str < p)
    {
      char tmp;

      tmp = *str;
      *str = *p;
      *p = tmp;
      str++;
      p--;
    }
}

static char *
pupa_itoa (char *str, int c, unsigned n)
{
  unsigned base = (c == 'x') ? 16 : 10;
  char *p;
  
  if ((int) n < 0 && c == 'd')
    {
      n = (unsigned) (-((int) n));
      *str++ = '-';
    }

  p = str;
  do
    {
      unsigned d = n % base;
      *p++ = (d > 9) ? d + 'a' - 10 : d + '0';
    }
  while (n /= base);
  *p = 0;

  pupa_reverse (str);
  return p;
}

static char *
pupa_ftoa (char *str, double f, int round)
{
  unsigned int intp;
  unsigned int fractp;
  unsigned int power = 1;
  int i;

  for (i = 0; i < round; i++)
    power *= 10;

  intp = f;
  fractp = (f - (float) intp) * power;

  pupa_sprintf (str, "%d.%d", intp, fractp);
  return str;
}

int
pupa_vsprintf (char *str, const char *fmt, va_list args)
{
  char c;
  int count = 0;
  auto void write_char (unsigned char ch);
  auto void write_str (const char *s);
  auto void write_fill (const char ch, int n);
  
  void write_char (unsigned char ch)
    {
      if (str)
	*str++ = ch;
      else
	pupa_putchar (ch);

      count++;
    }

  void write_str (const char *s)
    {
      while (*s)
	write_char (*s++);
    }

  void write_fill (const char ch, int n)
    {
      int i;
      for (i = 0; i < n; i++)
	write_char (ch);
    }
  
  while ((c = *fmt++) != 0)
    {
      if (c != '%')
	write_char (c);
      else
	{
	  char tmp[16];
	  char *p;
	  unsigned int format1 = 0;
	  unsigned int format2 = 3;
	  char zerofill = ' ';
	  int rightfill = 0;
	  int n;
	  
	  if (*fmt && *fmt =='-')
	    {
	      rightfill = 1;
	      fmt++;
	    }
	  
	  p = (char *) fmt;
	  /* Read formatting parameters.  */
	  while (*p && pupa_isdigit (*p))
	    p++;

	  if (p > fmt)
	    {
	      char s[p - fmt];
	      pupa_strncpy (s, fmt, p - fmt);
	      if (s[0] == '0')
		zerofill = '0';
	      format1 = pupa_strtoul (s, 0, 10);
	      fmt = p;
	      if (*p && *p == '.')
		{
		  p++;
		  fmt++;
		  while (*p && pupa_isdigit (*p))
		    p++;
		  
		  if (p > fmt)
		    {
		      char fstr[p - fmt];
		      pupa_strncpy (fstr, fmt, p - fmt);
		      format2 = pupa_strtoul (fstr, 0, 10);
		      fmt = p;
		    }
		}
	    }

	  c = *fmt++;

	  switch (c)
	    {
	    case 'p':
	      write_str ("0x");
	      c = 'x';
	      /* fall through */
	    case 'x':
	    case 'u':
	    case 'd':
	      n = va_arg (args, int);
	      pupa_itoa (tmp, c, n);
	      if (!rightfill && pupa_strlen (tmp) < format1)
		write_fill (zerofill, format1 - pupa_strlen (tmp));
	      write_str (tmp);
	      if (rightfill && pupa_strlen (tmp) < format1)
		write_fill (zerofill, format1 - pupa_strlen (tmp));
	      break;
	      
	    case 'c':
	      n = va_arg (args, int);
	      write_char (n & 0xff);
	      break;

	    case 'f':
	      {
		float f;
		f = va_arg (args, double);
		pupa_ftoa (tmp, f, format2);
		if (!rightfill && pupa_strlen (tmp) < format1)
		  write_fill (zerofill, format1 - pupa_strlen (tmp));
		write_str (tmp);
		if (rightfill && pupa_strlen (tmp) < format1)
		  write_fill (zerofill, format1 - pupa_strlen (tmp));
		break;
	      }
	      
	    case 'C':
	      {
		pupa_uint32_t code = va_arg (args, pupa_uint32_t);
		int shift;
		unsigned mask;
		
		if (code <= 0x7f)
		  {
		    shift = 0;
		    mask = 0;
		  }
		else if (code <= 0x7ff)
		  {
		    shift = 6;
		    mask = 0xc0;
		  }
		else if (code <= 0xffff)
		  {
		    shift = 12;
		    mask = 0xe0;
		  }
		else if (code <= 0x1fffff)
		  {
		    shift = 18;
		    mask = 0xf0;
		  }
		else if (code <= 0x3ffffff)
		  {
		    shift = 24;
		    mask = 0xf8;
		  }
		else if (code <= 0x7fffffff)
		  {
		    shift = 30;
		    mask = 0xfc;
		  }
		else
		  {
		    code = '?';
		    shift = 0;
		    mask = 0;
		  }

		write_char (mask | (code >> shift));
		
		for (shift -= 6; shift >= 0; shift -= 6)
		  write_char (0x80 | (0x3f & (code >> shift)));
	      }
	      break;

	    case 's':
	      p = va_arg (args, char *);
	      if (p)
		{
		  if (!rightfill && pupa_strlen (p) < format1)
		    write_fill (zerofill, format1 - pupa_strlen (p));
		  
		  write_str (p);
		  
		  if (rightfill && pupa_strlen (p) < format1)
		    write_fill (zerofill, format1 - pupa_strlen (p));
		}
	      else
		write_str ("(null)");
	      
	      break;

	    default:
	      write_char (c);
	      break;
	    }
	}
    }

  if (str)
    *str = '\0';

  if (count && !str)
    pupa_refresh ();
  
  return count;
}

int
pupa_sprintf (char *str, const char *fmt, ...)
{
  va_list ap;
  int ret;
  
  va_start (ap, fmt);
  ret = pupa_vsprintf (str, fmt, ap);
  va_end (ap);

  return ret;
}

pupa_err_t
pupa_split_cmdline (const char *cmdline, pupa_err_t (* getline) (char **), int *argc, char ***argv)
{
  /* XXX: Fixed size buffer, perhaps this buffer should be dynamically
     allocated.  */
  char buffer[1024];
  char *bp = buffer;
  char *rd = (char *) cmdline;
  char unputbuf;
  int unput = 0;
  char *args;
  int i;

  auto char getchar (void);
  auto void unputc (char c);
  auto void getenvvar (void);
  auto int getarg (void);

  /* Get one character from the commandline.  If the caller reads
     beyond the end of the string a new line will be read.  This
     function will not chech for errors, the caller has to check for
     pupa_errno.  */
  char getchar (void)
    {
      int c;
      if (unput)
	{
	  unput = 0;
	  return unputbuf;
	}

      if (! rd)
	{
	  getline (&rd);
	  /* Error is ignored here, the caller will check for this
	     when it reads beyond the EOL.  */
	  c = *(rd)++;
	  return c;
	}

      c = *(rd)++;
      if (! c)
	{
	  rd = 0;
	  return '\n';
	}

      return c;
    }

  void unputc (char c)
    {
      unputbuf = c;
      unput = 1;
    }

  /* Read a variable name from the commandline and insert its content
     into the buffer.  */
  void getenvvar (void)
    {
      char varname[100];
      char *p = varname;
      char *val;
      char c;

      c = getchar ();
      if (c == '{')
	while ((c = getchar ()) != '}')
	  *(p++) = c;
      else
	{
	  /* XXX: An env. variable can have characters and digits in
	     its name, are more characters allowed here?  */
	  while (c && (pupa_isalpha (c) || pupa_isdigit (c)))
	    {
	      *(p++) = c;
	      c = getchar ();
	    }
	  unputc (c);
	}
      *p = '\0';

      /* The variable does not exist.  */
      val = pupa_env_get (varname);
      if (! val)
	return;

      /* Copy the contents of the variable into the buffer.  */
      for (p = val; *p; p++)
	*(bp++) = *p;
    }

  /* Read one argument.  Return 1 if no variables can be read anymore,
     otherwise return 0.  If there is an error, return 1, the caller
     has to check pupa_errno.  */
  int getarg (void)
    {
      char c;

      /* Skip all whitespaces before an argument.  */
      do {
	c = getchar ();
      } while (c == ' ' || c == '\t');

      do {
	switch (c)
	  {
	  case '"':
	    /* Double quote.  */
	    while ((c = getchar ()))
	      {
		if (pupa_errno)
		  return 1;
		/* Read in an escaped character.  */
		if (c == '\\')
		  {
		    c = getchar ();
		    *(bp++) = c;
		    continue;
		  }
		else if (c == '"')
		  break;
		/* Read a variable.  */
		if (c == '$')
		  {
		    getenvvar ();
		    continue;
		  }
		*(bp++) = c;
	      }
	    break;

	  case '\'':
	    /* Single quote.  */
	    while ((c = getchar ()) != '\'')
	      {
		if (pupa_errno)
		  return 1;

		*(bp++) = c;
	      }
	    break;

	  case '\n':
	    /* This was not a argument afterall.  */
	    return 1;

	  default:
	    /* A normal option.  */
	    while (c && (pupa_isalpha (c)
			 || pupa_isdigit (c) || pupa_isgraph (c)))
	      {
		/* Read in an escaped character.  */
		if (c == '\\')
		  {
		    c = getchar ();
		    *(bp++) = c;
		    c = getchar ();
		    continue;
		  }
		/* Read a variable.  */
		if (c == '$')
		  {
		    getenvvar ();
		    c = getchar ();
		    continue;
		  }
		*(bp++) = c;
		c = getchar ();
	      }
	    unputc (c);

	    break;
	  }
      } while (! pupa_isspace (c) && c != '\'' && c != '"');

      return 0;
    }

  /* Read in all arguments and count them.  */
  *argc = 0;
  while (1)
    {
      if (getarg ())
	break;
      *(bp++) = '\0';
      (*argc)++;
    }

  /* Check if there were no errors.  */
  if (pupa_errno)
    return pupa_errno;

  /* Reserve memory for the return values.  */
  args = pupa_malloc (bp - buffer);
  if (! args)
    return pupa_errno;
  pupa_memcpy (args, buffer, bp - buffer);
  
  *argv = pupa_malloc (sizeof (char *) * (*argc + 1));
  if (! *argv)
    {
      pupa_free (args);
      return pupa_errno;
    }

  /* The arguments are separated with 0's, setup argv so it points to
     the right values.  */
  bp = args;
  for (i = 0; i < *argc; i++)
    {
      (*argv)[i] = bp;
      while (*bp)
	bp++;
      bp++;
    }

  (*argc)--;
  return 0;
}
