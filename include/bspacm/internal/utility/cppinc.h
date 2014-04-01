/* Copyright 2014, Peter A. Bigot
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the software nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** @file
 *
 * @brief C preprocessor macros used inside code-generating inc files.
 *
 * These allow generation of unique variables and functions,
 * e.g. IRQHandler instances for specific peripheral.  You don't need
 * to be using these yourself.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_INTERNAL_UTILITY_CPPINC_H
#define BSPACM_INTERNAL_UTILITY_CPPINC_H

/* @cond DOXYGEN_EXCLUDE */
#define BSPACM_INC_CATENATE_(a_,b_) a_##b_
#define BSPACM_INC_CATENATE3_(a_,b_,c_) a_##b_##c_
/* @endcond */

/** Form a preprocessor token by catenating the values of @p a_ and @p
 * b_. */
#define BSPACM_INC_CATENATE(a_,b_) BSPACM_INC_CATENATE_(a_,b_)

/** Form a preprocessor token by catenating the values of @p a_, @p
 * b_, and @p c_. */
#define BSPACM_INC_CATENATE3(a_,b_,c_) BSPACM_INC_CATENATE3_(a_,b_,c_)


#endif /* BSPACM_INTERNAL_UTILITY_CPPINC_H */
