// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
/*****************************************************************************/

#if defined (_WIN32) && !defined(NDEBUG)
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>

static void
xmalloc_failed (size_t size)
{
  fprintf (stderr,
       "\n%sout of memory allocating %lu bytes\n",
       "",
       (unsigned long) size);
  exit (1);
}  

void *
xmalloc (size_t size)
{
  void * newmem;

  if (size == 0)
    size = 1;
  newmem = malloc (size);
  if (!newmem)
    xmalloc_failed (size);

  return (newmem);
}

void *
xcalloc (size_t nelem, size_t elsize)
{
  void * newmem;

  if (nelem == 0 || elsize == 0)
    nelem = elsize = 1;

  newmem = calloc (nelem, elsize);
  if (!newmem)
    xmalloc_failed (nelem * elsize);

  return (newmem);
}

void *
xrealloc (void *oldmem, size_t size)
{
  void * newmem;

  if (size == 0)
    size = 1;
  if (!oldmem)
    newmem = malloc (size);
  else
    newmem = realloc (oldmem, size);
  if (!newmem)
    xmalloc_failed (size);

  return (newmem);
}
