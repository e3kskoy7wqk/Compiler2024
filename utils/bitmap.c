/* $OpenBSD: bitmap.c,v 1.9 2017/10/20 01:56:39 djm Exp $ */
/*
 * Copyright (c) 2015 Damien Miller <djm@mindrot.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bitmap.h"

/* Global data */
bitmap_element bitmap_zero_bits;        /* An element of all zero bits.  */
static bitmap_element *bitmap_free;        /* Freelist of bitmap elements.  */
static bitmap_element *bitmap_ggc_free;

static void bitmap_elem_to_freelist (bitmap, bitmap_element *);
static void bitmap_element_free (bitmap, bitmap_element *);
static bitmap_element *bitmap_element_allocate (bitmap);
static int bitmap_element_zerop (bitmap_element *);
static void bitmap_element_link (bitmap, bitmap_element *);
static bitmap_element *bitmap_elt_insert_after (bitmap, bitmap_element *);
static void bitmap_elt_clear_from (bitmap, bitmap_element *);
static bitmap_element *bitmap_find_bit (bitmap, unsigned int);


static void
xmalloc_failed (size_t size)
{
  fprintf (stderr,
       "\n%sout of memory allocating %lu bytes\n",
       "",
       (unsigned long) size);
  exit (1);
}  

static void *
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

/* Add ELEM to the appropriate freelist.  */
static void
bitmap_elem_to_freelist (bitmap head, bitmap_element *elt)
{
  elt->next = bitmap_ggc_free;
  bitmap_ggc_free = elt;
}

void
bitmap_terminate (void)
{
  bitmap_element *element;

  while (bitmap_ggc_free != NULL)
    {
      element = bitmap_ggc_free;
      bitmap_ggc_free = element->next;
      free (element);
    }
}

/* Free a bitmap element.  Since these are allocated off the
   bitmap_obstack, "free" actually means "put onto the freelist".  */

static void
bitmap_element_free (bitmap head, bitmap_element *elt)
{
  bitmap_element *next = elt->next;
  bitmap_element *prev = elt->prev;

  if (prev)
    prev->next = next;

  if (next)
    next->prev = prev;

  if (head->first == elt)
    head->first = next;

  /* Since the first thing we try is to insert before current,
     make current the next entry in preference to the previous.  */
  if (head->current == elt)
    {
      head->current = next != 0 ? next : prev;
      if (head->current)
        head->indx = head->current->indx;
    }
  bitmap_elem_to_freelist (head, elt);
}

/* Allocate a bitmap element.  The bits are cleared, but nothing else is.  */

static bitmap_element *
bitmap_element_allocate (bitmap head)
{
  bitmap_element *element;

  if (bitmap_ggc_free != NULL)
    {
      element = bitmap_ggc_free;
      bitmap_ggc_free = element->next;
    }
  else
    element = (bitmap_element *) xmalloc (sizeof (bitmap_element));
  

  memset (element->bits, 0, sizeof (element->bits));

  return element;
}

/* Release any memory allocated by bitmaps.  */

void
bitmap_release_memory (void)
{
  bitmap_free = 0;
}

/* Return nonzero if all bits in an element are zero.  */

static int
bitmap_element_zerop (bitmap_element *element)
{
#if BITMAP_ELEMENT_WORDS == 2
  return (element->bits[0] | element->bits[1]) == 0;
#else
  unsigned i;

  for (i = 0; i < BITMAP_ELEMENT_WORDS; i++)
    if (element->bits[i] != 0)
      return 0;

  return 1;
#endif
}

/* Link the bitmap element into the current bitmap linked list.  */

static void
bitmap_element_link (bitmap head, bitmap_element *element)
{
  unsigned int indx = element->indx;
  bitmap_element *ptr;

  /* If this is the first and only element, set it in.  */
  if (head->first == 0)
    {
      element->next = element->prev = 0;
      head->first = element;
    }

  /* If this index is less than that of the current element, it goes someplace
     before the current element.  */
  else if (indx < head->indx)
    {
      for (ptr = head->current;
           ptr->prev != 0 && ptr->prev->indx > indx;
           ptr = ptr->prev)
        ;

      if (ptr->prev)
        ptr->prev->next = element;
      else
        head->first = element;

      element->prev = ptr->prev;
      element->next = ptr;
      ptr->prev = element;
    }

  /* Otherwise, it must go someplace after the current element.  */
  else
    {
      for (ptr = head->current;
           ptr->next != 0 && ptr->next->indx < indx;
           ptr = ptr->next)
        ;

      if (ptr->next)
        ptr->next->prev = element;

      element->next = ptr->next;
      element->prev = ptr;
      ptr->next = element;
    }

  /* Set up so this is the first element searched.  */
  head->current = element;
  head->indx = indx;
}

/* Insert a new uninitialized element into bitmap HEAD after element
   ELT.  If ELT is NULL, insert the element at the start.  Return the
   new element.  */

static bitmap_element *
bitmap_elt_insert_after (bitmap head, bitmap_element *elt)
{
  bitmap_element *node = bitmap_element_allocate (head);

  if (!elt)
    {
      if (!head->current)
        head->current = node;
      node->next = head->first;
      if (node->next)
        node->next->prev = node;
      head->first = node;
      node->prev = NULL;
    }
  else
    {
      node->next = elt->next;
      if (node->next)
        node->next->prev = node;
      elt->next = node;
      node->prev = elt;
    }
  return node;
}

/* Remove ELT and all following elements from bitmap HEAD.  */

void
bitmap_elt_clear_from (bitmap head, bitmap_element *elt)
{
  bitmap_element *next;

  while (elt)
    {
      next = elt->next;
      bitmap_element_free (head, elt);
      elt = next;
    }
}

/* Clear a bitmap by freeing the linked list.  */

void
bitmap_clear (bitmap head)
{
  bitmap_element *element, *next;

  for (element = head->first; element != 0; element = next)
    {
      next = element->next;
      bitmap_elem_to_freelist (head, element);
    }

  head->first = head->current = 0;
}

/* Copy a bitmap to another bitmap.  */

void
bitmap_copy (bitmap to, bitmap from)
{
  bitmap_element *from_ptr, *to_ptr = 0;
#if BITMAP_ELEMENT_WORDS != 2
  unsigned i;
#endif

  bitmap_clear (to);

  /* Copy elements in forward direction one at a time.  */
  for (from_ptr = from->first; from_ptr; from_ptr = from_ptr->next)
    {
      bitmap_element *to_elt = bitmap_element_allocate (to);

      to_elt->indx = from_ptr->indx;

#if BITMAP_ELEMENT_WORDS == 2
      to_elt->bits[0] = from_ptr->bits[0];
      to_elt->bits[1] = from_ptr->bits[1];
#else
      for (i = 0; i < BITMAP_ELEMENT_WORDS; i++)
        to_elt->bits[i] = from_ptr->bits[i];
#endif

      /* Here we have a special case of bitmap_element_link, for the case
         where we know the links are being entered in sequence.  */
      if (to_ptr == 0)
        {
          to->first = to->current = to_elt;
          to->indx = from_ptr->indx;
          to_elt->next = to_elt->prev = 0;
        }
      else
        {
          to_elt->prev = to_ptr;
          to_elt->next = 0;
          to_ptr->next = to_elt;
        }

      to_ptr = to_elt;
    }
}

/* Find a bitmap element that would hold a bitmap's bit.
   Update the `current' field even if we can't find an element that
   would hold the bitmap's bit to make eventual allocation
   faster.  */

static bitmap_element *
bitmap_find_bit (bitmap head, unsigned int bit)
{
  bitmap_element *element;
  unsigned int indx = bit / BITMAP_ELEMENT_ALL_BITS;

  if (head->current == 0
      || head->indx == indx)
    return head->current;

  if (head->indx > indx)
    for (element = head->current;
         element->prev != 0 && element->indx > indx;
         element = element->prev)
      ;

  else
    for (element = head->current;
         element->next != 0 && element->indx < indx;
         element = element->next)
      ;

  /* `element' is the nearest to the one we want.  If it's not the one we
     want, the one we want doesn't exist.  */
  head->current = element;
  head->indx = element->indx;
  if (element != 0 && element->indx != indx)
    element = 0;

  return element;
}

/* Clear a single bit in a bitmap.  */

void
bitmap_clear_bit (bitmap head, int bit)
{
  bitmap_element *ptr = bitmap_find_bit (head, bit);

  if (ptr != 0)
    {
      unsigned bit_num  = bit % BITMAP_WORD_BITS;
      unsigned word_num = bit / BITMAP_WORD_BITS % BITMAP_ELEMENT_WORDS;
      ptr->bits[word_num] &= ~ (((BITMAP_WORD) 1) << bit_num);

      /* If we cleared the entire word, free up the element.  */
      if (bitmap_element_zerop (ptr))
        bitmap_element_free (head, ptr);
    }
}

/* Set a single bit in a bitmap.  Return true if the bit changed.  */

int
bitmap_set_bit (bitmap head, int bit)
{
  bitmap_element *ptr = bitmap_find_bit (head, bit);
  unsigned word_num = bit / BITMAP_WORD_BITS % BITMAP_ELEMENT_WORDS;
  unsigned bit_num  = bit % BITMAP_WORD_BITS;
  BITMAP_WORD bit_val = ((BITMAP_WORD) 1) << bit_num;

  if (ptr == 0)
    {
      ptr = bitmap_element_allocate (head);
      ptr->indx = bit / BITMAP_ELEMENT_ALL_BITS;
      ptr->bits[word_num] = bit_val;
      bitmap_element_link (head, ptr);
      return 1;
    }
  else
    {
      int res = (ptr->bits[word_num] & bit_val) == 0;
      if (res)
        ptr->bits[word_num] |= bit_val;
      return res;
    }
}

/* Return whether a bit is set within a bitmap.  */

int
bitmap_bit_p (bitmap head, int bit)
{
  bitmap_element *ptr;
  unsigned bit_num;
  unsigned word_num;

  ptr = bitmap_find_bit (head, bit);
  if (ptr == 0)
    return 0;

  bit_num = bit % BITMAP_WORD_BITS;
  word_num = bit / BITMAP_WORD_BITS % BITMAP_ELEMENT_WORDS;

  return (ptr->bits[word_num] >> bit_num) & 1;
}

/* Table of number of set bits in a character, indexed by value of char.  */
static const unsigned char popcount_table[] =
{
    0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
};

static unsigned long
bitmap_popcount (BITMAP_WORD a)
{
  unsigned long ret = 0;
  unsigned i;

  /* Just do this the table way for now  */
  for (i = 0; i < BITMAP_WORD_BITS; i+= 8)
    ret += popcount_table[(a >> i) & 0xff];
  return ret;
}

/* Count the number of bits set in the bitmap, and return it.  */
unsigned long
bitmap_count_bits (bitmap a)
{
  unsigned long count = 0;
  const bitmap_element *elt;
  unsigned ix;

  for (elt = a->first; elt; elt = elt->next)
    {
      for (ix = 0; ix != BITMAP_ELEMENT_WORDS; ix++)
        {
          count += bitmap_popcount (elt->bits[ix]);
        }
    }
  return count;
}

/* Return the bit number of the first set bit in the bitmap.  The
   bitmap must be non-empty.  */

unsigned
bitmap_first_set_bit (bitmap a)
{
  bitmap_element *elt = a->first;
  unsigned bit_no;
  BITMAP_WORD word;
  unsigned ix;
  
  bit_no = elt->indx * BITMAP_ELEMENT_ALL_BITS;
  for (ix = 0; ix != BITMAP_ELEMENT_WORDS; ix++)
    {
      word = elt->bits[ix];
      if (word)
        goto found_bit;
    }
 found_bit:
  bit_no += ix * BITMAP_WORD_BITS;

  /* Binary search for the first set bit.  */
#if BITMAP_WORD_BITS > 64
#error "Fill out the table."
#endif
#if BITMAP_WORD_BITS > 32
  if (!(word & 0xffffffff))
    word >>= 32, bit_no += 32;
#endif
  if (!(word & 0xffff))
    word >>= 16, bit_no += 16;
  if (!(word & 0xff))
    word >>= 8, bit_no += 8;
  if (!(word & 0xf))
    word >>= 4, bit_no += 4;
  if (!(word & 0x3))
    word >>= 2, bit_no += 2;
  if (!(word & 0x1))
    word >>= 1, bit_no += 1;
 return bit_no;
}

/* DST = A & B.  */

void
bitmap_and (bitmap dst, bitmap a, bitmap b)
{
  bitmap_element *dst_elt = dst->first;
  bitmap_element *a_elt = a->first;
  bitmap_element *b_elt = b->first;
  bitmap_element *dst_prev = NULL;

  while (a_elt && b_elt)
    {
      if (a_elt->indx < b_elt->indx)
        a_elt = a_elt->next;
      else if (b_elt->indx < a_elt->indx)
        b_elt = b_elt->next;
      else
        {
          /* Matching elts, generate A & B.  */
          unsigned ix;
          BITMAP_WORD ior = 0;

          if (!dst_elt)
            dst_elt = bitmap_elt_insert_after (dst, dst_prev);
          
          dst_elt->indx = a_elt->indx;
          for (ix = BITMAP_ELEMENT_WORDS; ix--;)
            {
              BITMAP_WORD r = a_elt->bits[ix] & b_elt->bits[ix];

              dst_elt->bits[ix] = r;
              ior |= r;
            }
          if (ior)
            {
              dst_prev = dst_elt;
              dst_elt = dst_elt->next;
            }
          a_elt = a_elt->next;
          b_elt = b_elt->next;
        }
    }
  bitmap_elt_clear_from (dst, dst_elt);
  if (dst->current)
    dst->indx = dst->current->indx;
}

/* A &= B.  */

void
bitmap_and_into (bitmap a, bitmap b)
{
  bitmap_element *a_elt = a->first;
  bitmap_element *b_elt = b->first;
  bitmap_element *next;

  while (a_elt && b_elt)
    {
      if (a_elt->indx < b_elt->indx)
        {
          next = a_elt->next;
          bitmap_element_free (a, a_elt);
          a_elt = next;
        }
      else if (b_elt->indx < a_elt->indx)
        b_elt = b_elt->next;
      else
        {
          /* Matching elts, generate A &= B.  */
          unsigned ix;
          BITMAP_WORD ior = 0;

          for (ix = BITMAP_ELEMENT_WORDS; ix--;)
            {
              BITMAP_WORD r = a_elt->bits[ix] & b_elt->bits[ix];

              a_elt->bits[ix] = r;
              ior |= r;
            }
          next = a_elt->next;
          if (!ior)
            bitmap_element_free (a, a_elt);
          a_elt = next;
          b_elt = b_elt->next;
        }
    }
  bitmap_elt_clear_from (a, a_elt);
}

/* DST = A & ~B  */

void
bitmap_and_compl (bitmap dst, bitmap a, bitmap b)
{
  bitmap_element *dst_elt = dst->first;
  bitmap_element *a_elt = a->first;
  bitmap_element *b_elt = b->first;
  bitmap_element *dst_prev = NULL;
  
  while (a_elt)
    {
      if (!b_elt || a_elt->indx < b_elt->indx)
        {
          /* Copy a_elt.  */
          if (!dst_elt)
            dst_elt = bitmap_elt_insert_after (dst, dst_prev);
          
          dst_elt->indx = a_elt->indx;
          memcpy (dst_elt->bits, a_elt->bits, sizeof (dst_elt->bits));
          dst_prev = dst_elt;
          dst_elt = dst_elt->next;
          a_elt = a_elt->next;
        }
      else if (b_elt->indx < a_elt->indx)
        b_elt = b_elt->next;
      else
        {
          /* Matching elts, generate A & ~B.  */
          unsigned ix;
          BITMAP_WORD ior = 0;

          if (!dst_elt)
            dst_elt = bitmap_elt_insert_after (dst, dst_prev);
          
          dst_elt->indx = a_elt->indx;
          for (ix = BITMAP_ELEMENT_WORDS; ix--;)
            {
              BITMAP_WORD r = a_elt->bits[ix] & ~b_elt->bits[ix];

              dst_elt->bits[ix] = r;
              ior |= r;
            }
          if (ior)
            {
              dst_prev = dst_elt;
              dst_elt = dst_elt->next;
            }
          a_elt = a_elt->next;
          b_elt = b_elt->next;
        }
    }
  bitmap_elt_clear_from (dst, dst_elt);
  if (dst->current)
    dst->indx = dst->current->indx;
}

/* A &= ~B */

void
bitmap_and_compl_into (bitmap a, bitmap b)
{
  bitmap_element *a_elt = a->first;
  bitmap_element *b_elt = b->first;
  bitmap_element *next;

  while (a_elt && b_elt)
    {
      if (a_elt->indx < b_elt->indx)
        a_elt = a_elt->next;
      else if (b_elt->indx < a_elt->indx)
        b_elt = b_elt->next;
      else
        {
          /* Matching elts, generate A &= ~B.  */
          unsigned ix;
          BITMAP_WORD ior = 0;

          for (ix = BITMAP_ELEMENT_WORDS; ix--;)
            {
              BITMAP_WORD r = a_elt->bits[ix] & ~b_elt->bits[ix];

              a_elt->bits[ix] = r;
              ior |= r;
            }
          next = a_elt->next;
          if (!ior)
            bitmap_element_free (a, a_elt);
          a_elt = next;
          b_elt = b_elt->next;
        }
    }
}

/* DST = A | B.  Return true if DST changes.  */

int
bitmap_ior (bitmap dst, bitmap a, bitmap b)
{
  bitmap_element *dst_elt = dst->first;
  bitmap_element *a_elt = a->first;
  bitmap_element *b_elt = b->first;
  bitmap_element *dst_prev = NULL;
  int changed = 0;  

  while (a_elt || b_elt)
    {
      if (a_elt && b_elt && a_elt->indx == b_elt->indx)
        {
          /* Matching elts, generate A | B.  */
          unsigned ix;
              
          if (!changed && dst_elt && dst_elt->indx == a_elt->indx)
            {
              for (ix = BITMAP_ELEMENT_WORDS; ix--;)
                {
                  BITMAP_WORD r = a_elt->bits[ix] | b_elt->bits[ix];

                  if (r != dst_elt->bits[ix])
                    {
                      dst_elt->bits[ix] = r;
                      changed = 1;
                    }
                }
            }
          else
            {
              changed = 1;
              if (!dst_elt)
                dst_elt = bitmap_elt_insert_after (dst, dst_prev);
              dst_elt->indx = a_elt->indx;
              for (ix = BITMAP_ELEMENT_WORDS; ix--;)
                {
                  BITMAP_WORD r = a_elt->bits[ix] | b_elt->bits[ix];
                  
                  dst_elt->bits[ix] = r;
                }
            }
          a_elt = a_elt->next;
          b_elt = b_elt->next;
          dst_prev = dst_elt;
          dst_elt = dst_elt->next;
        }
      else
        {
          /* Copy a single element.  */
          bitmap_element *src;

          if (!b_elt || (a_elt && a_elt->indx < b_elt->indx))
            {
              src = a_elt;
              a_elt = a_elt->next;
            }
          else
            {
              src = b_elt;
              b_elt = b_elt->next;
            }

          if (!changed && dst_elt && dst_elt->indx == src->indx)
            {
              unsigned ix;
              
              for (ix = BITMAP_ELEMENT_WORDS; ix--;)
                if (src->bits[ix] != dst_elt->bits[ix])
                  {
                    dst_elt->bits[ix] = src->bits[ix];
                    changed = 1;
                  }
            }
          else
            {
              changed = 1;
              if (!dst_elt)
                dst_elt = bitmap_elt_insert_after (dst, dst_prev);
              dst_elt->indx = src->indx;
              memcpy (dst_elt->bits, src->bits, sizeof (dst_elt->bits));
            }
          
          dst_prev = dst_elt;
          dst_elt = dst_elt->next;
        }
    }

  if (dst_elt)
    {
      changed = 1;
      bitmap_elt_clear_from (dst, dst_elt);
    }
  if (dst->current)
    dst->indx = dst->current->indx;
  return changed;
}

/* A |= B.  Return true if A changes.  */

int
bitmap_ior_into (bitmap a, bitmap b)
{
  bitmap_element *a_elt = a->first;
  bitmap_element *b_elt = b->first;
  bitmap_element *a_prev = NULL;
  int changed = 0;

  while (b_elt)
    {
      if (!a_elt || b_elt->indx < a_elt->indx)
        {
          /* Copy b_elt.  */
          bitmap_element *dst = bitmap_elt_insert_after (a, a_prev);
          
          dst->indx = b_elt->indx;
          memcpy (dst->bits, b_elt->bits, sizeof (dst->bits));
          a_prev = dst;
          b_elt = b_elt->next;
          changed = 1;
        }
      else if (a_elt->indx < b_elt->indx)
        {
          a_prev = a_elt;
          a_elt = a_elt->next;
        }
      else
        {
          /* Matching elts, generate A |= B.  */
          unsigned ix;

          if (changed)
            for (ix = BITMAP_ELEMENT_WORDS; ix--;)
              {
                BITMAP_WORD r = a_elt->bits[ix] | b_elt->bits[ix];
                
                a_elt->bits[ix] = r;
              }
          else
            for (ix = BITMAP_ELEMENT_WORDS; ix--;)
              {
                BITMAP_WORD r = a_elt->bits[ix] | b_elt->bits[ix];

                if (a_elt->bits[ix] != r)
                  {
                    changed = 1;
                    a_elt->bits[ix] = r;
                  }
              }
          b_elt = b_elt->next;
          a_prev = a_elt;
          a_elt = a_elt->next;
        }
    }
  if (a->current)
    a->indx = a->current->indx;
  return changed;
}

/* DST = A ^ B  */

void
bitmap_xor (bitmap dst, bitmap a, bitmap b)
{
  bitmap_element *dst_elt = dst->first;
  bitmap_element *a_elt = a->first;
  bitmap_element *b_elt = b->first;
  bitmap_element *dst_prev = NULL;

  while (a_elt || b_elt)
    {
      if (a_elt && b_elt && a_elt->indx == b_elt->indx)
        {
          /* Matching elts, generate A ^ B.  */
          unsigned ix;
          BITMAP_WORD ior = 0;

          if (!dst_elt)
            dst_elt = bitmap_elt_insert_after (dst, dst_prev);
          
          dst_elt->indx = a_elt->indx;
          for (ix = BITMAP_ELEMENT_WORDS; ix--;)
            {
              BITMAP_WORD r = a_elt->bits[ix] ^ b_elt->bits[ix];

              ior |= r;
              dst_elt->bits[ix] = r;
            }
          a_elt = a_elt->next;
          b_elt = b_elt->next;
          if (ior)
            {
              dst_prev = dst_elt;
              dst_elt = dst_elt->next;
            }
        }
      else
        {
          /* Copy a single element.  */
          bitmap_element *src;

          if (!b_elt || (a_elt && a_elt->indx < b_elt->indx))
            {
              src = a_elt;
              a_elt = a_elt->next;
            }
          else
            {
              src = b_elt;
              b_elt = b_elt->next;
            }

          if (!dst_elt)
            dst_elt = bitmap_elt_insert_after (dst, dst_prev);
          
          dst_elt->indx = src->indx;
          memcpy (dst_elt->bits, src->bits, sizeof (dst_elt->bits));
          dst_prev = dst_elt;
          dst_elt = dst_elt->next;
        }
    }
  bitmap_elt_clear_from (dst, dst_elt);
  if (dst->current)
    dst->indx = dst->current->indx;
}

/* A ^= B */

void
bitmap_xor_into (bitmap a, bitmap b)
{
  bitmap_element *a_elt = a->first;
  bitmap_element *b_elt = b->first;
  bitmap_element *a_prev = NULL;

  while (b_elt)
    {
      if (!a_elt || b_elt->indx < a_elt->indx)
        {
          /* Copy b_elt.  */
          bitmap_element *dst = bitmap_elt_insert_after (a, a_prev);
          
          dst->indx = b_elt->indx;
          memcpy (dst->bits, b_elt->bits, sizeof (dst->bits));
          a_prev = dst;
          b_elt = b_elt->next;
        }
      else if (a_elt->indx < b_elt->indx)
        {
          a_prev = a_elt;
          a_elt = a_elt->next;
        }
      else
        {
          /* Matching elts, generate A ^= B.  */
          unsigned ix;
          BITMAP_WORD ior = 0;
          bitmap_element *next = a_elt->next;

          for (ix = BITMAP_ELEMENT_WORDS; ix--;)
            {
              BITMAP_WORD r = a_elt->bits[ix] ^ b_elt->bits[ix];

              ior |= r;
              a_elt->bits[ix] = r;
            }
          b_elt = b_elt->next;
          if (ior)
            a_prev = a_elt;
          else
            bitmap_element_free (a, a_elt);
          a_elt = next;
        }
    }
  if (a->current)
    a->indx = a->current->indx;
}

/* Return true if two bitmaps are identical.
   We do not bother with a check for pointer equality, as that never
   occurs in practice.  */

int
bitmap_equal_p (bitmap a, bitmap b)
{
  bitmap_element *a_elt;
  bitmap_element *b_elt;
  unsigned ix;
  
  for (a_elt = a->first, b_elt = b->first;
       a_elt && b_elt;
       a_elt = a_elt->next, b_elt = b_elt->next)
    {
      if (a_elt->indx != b_elt->indx)
        return 0;
      for (ix = BITMAP_ELEMENT_WORDS; ix--;)
        if (a_elt->bits[ix] != b_elt->bits[ix])
          return 0;
    }
  return !a_elt && !b_elt;
}

/* Return true if A AND B is not empty.  */

int
bitmap_intersect_p (bitmap a, bitmap b)
{
  bitmap_element *a_elt;
  bitmap_element *b_elt;
  unsigned ix;
  
  for (a_elt = a->first, b_elt = b->first;
       a_elt && b_elt;)
    {
      if (a_elt->indx < b_elt->indx)
        a_elt = a_elt->next;
      else if (b_elt->indx < a_elt->indx)
        b_elt = b_elt->next;
      else
        {
          for (ix = BITMAP_ELEMENT_WORDS; ix--;)
            if (a_elt->bits[ix] & b_elt->bits[ix])
              return 1;
          a_elt = a_elt->next;
          b_elt = b_elt->next;
        }
    }
  return 0;
}

/* Return true if A AND NOT B is not empty.  */

int
bitmap_intersect_compl_p (bitmap a, bitmap b)
{
  bitmap_element *a_elt;
  bitmap_element *b_elt;
  unsigned ix;
  for (a_elt = a->first, b_elt = b->first;
       a_elt && b_elt;)
    {
      if (a_elt->indx < b_elt->indx)
        return 1;
      else if (b_elt->indx < a_elt->indx)
        b_elt = b_elt->next;
      else
        {
          for (ix = BITMAP_ELEMENT_WORDS; ix--;)
            if (a_elt->bits[ix] & ~b_elt->bits[ix])
              return 1;
          a_elt = a_elt->next;
          b_elt = b_elt->next;
        }
    }
  return a_elt != NULL;
}


/* DST = A | (FROM1 & ~FROM2).  Return true if DST changes.  */

int
bitmap_ior_and_compl (bitmap dst, bitmap a, bitmap from1, bitmap from2)
{
  bitmap_head tmp;
  int changed;
  
  tmp.first = tmp.current = 0;
  bitmap_and_compl (&tmp, from1, from2);
  changed = bitmap_ior (dst, a, &tmp);
  bitmap_clear (&tmp);

  return changed;
}

/* A |= (FROM1 & ~FROM2).  Return true if A changes.  */

int
bitmap_ior_and_compl_into (bitmap a, bitmap from1, bitmap from2)
{
  bitmap_head tmp;
  int changed;
  
  tmp.first = tmp.current = 0;
  bitmap_and_compl (&tmp, from1, from2);
  changed = bitmap_ior_into (a, &tmp);
  bitmap_clear (&tmp);

  return changed;
}

/* Initialize a bitmap header.  */

bitmap
bitmap_initialize (bitmap head, int using_obstack)
{
  if (head == NULL && ! using_obstack)
    head = (bitmap) xmalloc (sizeof (struct bitmap_head_def));

  head->first = head->current = 0;

  return head;
}

/* Debugging function to print out the contents of a bitmap.  */

void
debug_bitmap_file (FILE *file, bitmap head)
{
  bitmap_element *ptr;

  fprintf (file, "\nfirst = " "%p"
           " current = " "%p" " indx = %u\n",
           (void *) head->first, (void *) head->current, head->indx);

  for (ptr = head->first; ptr; ptr = ptr->next)
    {
      unsigned int i, j, col = 26;

      fprintf (file, "\t" "%p" " next = " "%p"
               " prev = " "%p" " indx = %u\n\t\tbits = {",
               (void*) ptr, (void*) ptr->next, (void*) ptr->prev, ptr->indx);

      for (i = 0; i < BITMAP_ELEMENT_WORDS; i++)
        for (j = 0; j < BITMAP_WORD_BITS; j++)
          if ((ptr->bits[i] >> j) & 1)
            {
              if (col > 70)
                {
                  fprintf (file, "\n\t\t\t");
                  col = 24;
                }

              fprintf (file, " %u", (ptr->indx * BITMAP_ELEMENT_ALL_BITS
                                     + i * BITMAP_WORD_BITS + j));
              col += 4;
            }

      fprintf (file, " }\n");
    }
}

/* Function to be called from the debugger to print the contents
   of a bitmap.  */

void
debug_bitmap (bitmap head)
{
  debug_bitmap_file (stdout, head);
}

/* Function to print out the contents of a bitmap.  Unlike debug_bitmap_file,
   it does not print anything but the bits.  */

void
bitmap_print (FILE *file, bitmap head, const char *prefix, const char *suffix)
{
  const char *comma = "";
  unsigned i;
  bitmap_iterator bi;

  fputs (prefix, file);
  EXECUTE_IF_SET_IN_BITMAP (head, 0, i, bi)
    {
      fprintf (file, "%s%d", comma, i);
      comma = ", ";
    }
  fputs (suffix, file);
}
