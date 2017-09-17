/* $Id: record_util.c,v 1.10 2005/07/11 20:06:27 oohara Exp $ */

#include <stdio.h>
/* strtol */
#include <stdlib.h>
/* errno */
#include <errno.h>
/* INT_MIN, INT_MAX */
#include <limits.h>
/* strlen, strcpy */
#include <string.h>
#include <stdarg.h>

#include "record_util.h"

/* convert string (arg 2) to a number and store it to value (arg 1)
 * return 0 on success, 1 on error
 */
int
convert_to_number(int *value, const char *string)
{
  long int temp;
  char *tail;

  /* to detect overflow */
  errno = 0;
  temp = strtol(string, &tail, 10);
  if (tail[0] != '\0')
  {
    fprintf(stderr, "convert_to_number: string is not a number\n");
    return 1;
  }
  if (errno != 0)
  {
    fprintf(stderr, "convert_to_number: number overflowed\n");
    return 1;
  }
  if ((temp <= INT_MIN) || (temp >= INT_MAX))
  {
    fprintf(stderr, "convert_to_number: too big or small integer\n");
    return 1;
  }
  *value = (int) temp;

  return 0;
}

/* add ... (arg 3--) at the end of result (arg 1)
 * ... must be exactly n (arg 2) pointers to const char
 * if result (arg 1) is not NULL, its memory must be allocated
 * with malloc(); it is resized as necessary
 * ... (arg 3--) are copied
 * return NULL on error
 */
char *
concatenate_string(char *result, int n, ...)
{
  va_list ap;
  int i;
  int length;
  int head;
  const char *s = NULL;
  char *temp = NULL;

  /* sanity check */
  if (n < 0)
  {
    fprintf(stderr, "concatenate_string: n is negative (%d)\n", n);
    return NULL;
  }

  if (n == 0)
    return result;

  va_start(ap, n);
  length = 0;
  for (i = 0; i < n; i++)
  {
    s = va_arg(ap, const char *);
    if (s == NULL)
    {
      fprintf(stderr, "concatenate_string: string %d is NULL\n", i + 1);
      return NULL;
    }
    length += (int) strlen(s);
  }
  va_end(ap);

  if (length < 0)
  {
    fprintf(stderr, "concatenate_string: length is negative (%d)\n", length);
    return NULL;
  }
  if (length == 0)
    return result;

  /* +1 is for the trailing '\0' */
  if (result == NULL)
  {  
    temp = (char *) malloc(sizeof(char) * (length + 1));
  }
  else
  {
    length += (int) strlen(result);
    temp = (char *) realloc(result, sizeof(char) * (length + 1));
  }
  if (temp == NULL)
  {
    fprintf(stderr, "concatenate_string: can't allocate enough memory\n");
    return NULL;
  }
  result = temp;

  va_start(ap, n);
  head = 0;
  for (i = 0; i < n; i++)
  {
    s = va_arg(ap, const char *);
    strcpy(result + head, s);
    head += (int) strlen(s);
  }
  va_end(ap);

  result[length] = '\0';
  return result;
}
