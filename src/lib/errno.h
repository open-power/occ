#ifndef __ERRNO_H__
#define __ERRNO_H__

// $Id: errno.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/errno.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file errno.h
/// \brief Replacement for <errno.h>
///
/// SSX does not support a per-thread or global 'errno'.  The standard Unix
/// errno values returned by library functions are defined here. The prefix
/// code is the 'telephone code' for "errn".

#define EINVAL 0x00377601
#define EBADF  0x00377602
#define EAGAIN 0x00377603
#define ENXIO  0x00377604
#define ENOMEM 0x00377605

#endif  /* __ERRNO_H__ */
