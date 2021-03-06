/*	$NetBSD: xdr_mem.c,v 1.14 1998/11/15 17:32:47 christos Exp $	*/

/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * xdr_mem.h, XDR implementation using memory buffers.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * If you have some data to be interpreted as external data representation
 * or to be converted to external data representation in a memory buffer,
 * then this is the package for you.
 *
 */

#include "portable.h"

#ifdef DECVAX
#include "multinet_root:[multinet.include.sys]types.h"
#else /* DECVAX */
#include <sys/types.h>
#endif /* DECVAX */

#include <string.h>

#ifdef DECVAX
#include "multinet_root:[multinet.include.netinet]in.h"
#else /* DECVAX */
#if defined(unix)||defined(amiga)
#include <netinet/in.h>
#endif /* defined(unix)||defined(amiga) */
#endif /* DECVAX */

#ifdef DECVAX
#include "multinet_root:[multinet.include.sys]socket.h"
#else /* DECVAX */
#ifdef _WIN32
#include <winsock.h>
#else /* _WIN32 */
#include <sys/socket.h>
#endif /* _WIN32 */
#ifdef __MWERKS__
#include <machine/endian.h>
#endif /* __MWERKS__ */
#endif /* DECVAX */

#if defined(unix)||defined(amiga)
#include <rpc/types.h>
#include <rpc/xdr.h>
#else /* defined(unix)||defined(amiga) */
#include "xdr.h"
#endif /* defined(unix)||defined(amiga) */

static void xdrmem_destroy P_((XDR *));
static bool_t xdrmem_getlong_aligned P_((XDR *, long *));
static bool_t xdrmem_putlong_aligned P_((XDR *, const long *));
static bool_t xdrmem_getlong_unaligned P_((XDR *, long *));
static bool_t xdrmem_putlong_unaligned P_((XDR *, const long *));
static bool_t xdrmem_getbytes P_((XDR *, char *, u_int));
static bool_t xdrmem_putbytes P_((XDR *, const char *, u_int));
/* XXX: w/64-bit pointers, u_int not enough! */
#ifdef _WIN32
static u_int xdrmem_getpos P_((XDR *));
#else
static u_int xdrmem_getpos P_((__const XDR *));
#endif
static bool_t xdrmem_setpos P_((XDR *, u_int));
static int32_t *xdrmem_inline_aligned P_((XDR *, u_int));
static int32_t *xdrmem_inline_unaligned P_((XDR *, u_int));

static const struct	xdr_ops xdrmem_ops_aligned = {
	xdrmem_getlong_aligned,
	xdrmem_putlong_aligned,
	xdrmem_getbytes,
	xdrmem_putbytes,
	xdrmem_getpos,
	xdrmem_setpos,
	xdrmem_inline_aligned,
	xdrmem_destroy
};

static const struct	xdr_ops xdrmem_ops_unaligned = {
	xdrmem_getlong_unaligned,
	xdrmem_putlong_unaligned,
	xdrmem_getbytes,
	xdrmem_putbytes,
	xdrmem_getpos,
	xdrmem_setpos,
	xdrmem_inline_unaligned,
	xdrmem_destroy
};

/*
 * The procedure xdrmem_create initializes a stream descriptor for a
 * memory buffer.  
 */
void
xdrmem_create(xdrs, addr, size, op)
	XDR *xdrs;
	char *addr;
	u_int size;
	enum xdr_op op;
{

	xdrs->x_op = op;
	xdrs->x_ops = ((unsigned long)addr & (sizeof(int32_t) - 1))
	    ? &xdrmem_ops_unaligned : &xdrmem_ops_aligned;
	xdrs->x_private = xdrs->x_base = addr;
	xdrs->x_handy = size;
}

/*ARGSUSED*/
static void
xdrmem_destroy(xdrs)
	XDR *xdrs;
{

}

static bool_t
xdrmem_getlong_aligned(xdrs, lp)
	XDR *xdrs;
	long *lp;
{

	if ((xdrs->x_handy -= sizeof(int32_t)) < 0)
		return (FALSE);
	*lp = ntohl(*(u_int32_t *)xdrs->x_private);
	xdrs->x_private = (char *)xdrs->x_private + sizeof(int32_t);
	return (TRUE);
}

static bool_t
xdrmem_putlong_aligned(xdrs, lp)
	XDR *xdrs;
	const long *lp;
{

	if ((xdrs->x_handy -= sizeof(int32_t)) < 0)
		return (FALSE);
	*(u_int32_t *)xdrs->x_private = htonl((u_int32_t)*lp);
	xdrs->x_private = (char *)xdrs->x_private + sizeof(int32_t);
	return (TRUE);
}

static bool_t
xdrmem_getlong_unaligned(xdrs, lp)
	XDR *xdrs;
	long *lp;
{
	u_int32_t l;

	if ((xdrs->x_handy -= sizeof(int32_t)) < 0)
		return (FALSE);
	memmove(&l, xdrs->x_private, sizeof(int32_t));
	*lp = ntohl(l);
	xdrs->x_private = (char *)xdrs->x_private + sizeof(int32_t);
	return (TRUE);
}

static bool_t
xdrmem_putlong_unaligned(xdrs, lp)
	XDR *xdrs;
	const long *lp;
{
	u_int32_t l;

	if ((xdrs->x_handy -= sizeof(int32_t)) < 0)
		return (FALSE);
	l = htonl((u_int32_t)*lp);
	memmove(xdrs->x_private, &l, sizeof(int32_t));
	xdrs->x_private = (char *)xdrs->x_private + sizeof(int32_t);
	return (TRUE);
}

static bool_t
xdrmem_getbytes(xdrs, addr, len)
	XDR *xdrs;
	char *addr;
	u_int len;
{

	if ((xdrs->x_handy -= len) < 0)
		return (FALSE);
	memmove(addr, xdrs->x_private, len);
	xdrs->x_private = (char *)xdrs->x_private + len;
	return (TRUE);
}

static bool_t
xdrmem_putbytes(xdrs, addr, len)
	XDR *xdrs;
	const char *addr;
	u_int len;
{

	if ((xdrs->x_handy -= len) < 0)
		return (FALSE);
	memmove(xdrs->x_private, addr, len);
	xdrs->x_private = (char *)xdrs->x_private + len;
	return (TRUE);
}

static u_int
xdrmem_getpos(xdrs)
#ifdef _WIN32
XDR *xdrs;
#else
__const XDR *xdrs;
#endif
{

	/* XXX w/64-bit pointers, u_int not enough! */
	return (u_int)((u_long)xdrs->x_private - (u_long)xdrs->x_base);
}

static bool_t
xdrmem_setpos(xdrs, pos)
	XDR *xdrs;
	u_int pos;
{
	char *newaddr = xdrs->x_base + pos;
	char *lastaddr = (char *)xdrs->x_private + xdrs->x_handy;

	if ((long)newaddr > (long)lastaddr)
		return (FALSE);
	xdrs->x_private = newaddr;
	xdrs->x_handy = (int)((long)lastaddr - (long)newaddr);
	return (TRUE);
}

static int32_t *
xdrmem_inline_aligned(xdrs, len)
	XDR *xdrs;
	u_int len;
{
	int32_t *buf = 0;

	if ((u_int)(xdrs->x_handy) >= len) {
		xdrs->x_handy -= len;
		buf = (int32_t *)xdrs->x_private;
		xdrs->x_private = (char *)xdrs->x_private + len;
	}
	return (buf);
}

/* ARGSUSED */
static int32_t *
xdrmem_inline_unaligned(xdrs, len)
	XDR *xdrs;
	u_int len;
{

	return (0);
}
