/********************************************************************************/
/*										*/
/*			     				*/
/*			     Written by Ken Goldman				*/
/*		       IBM Thomas J. Watson Research Center			*/
/*            $Id: CryptHash.c 953 2017-03-06 20:31:40Z kgoldman $		*/
/*										*/
/*  Licenses and Notices							*/
/*										*/
/*  1. Copyright Licenses:							*/
/*										*/
/*  - Trusted Computing Group (TCG) grants to the user of the source code in	*/
/*    this specification (the "Source Code") a worldwide, irrevocable, 		*/
/*    nonexclusive, royalty free, copyright license to reproduce, create 	*/
/*    derivative works, distribute, display and perform the Source Code and	*/
/*    derivative works thereof, and to grant others the rights granted herein.	*/
/*										*/
/*  - The TCG grants to the user of the other parts of the specification 	*/
/*    (other than the Source Code) the rights to reproduce, distribute, 	*/
/*    display, and perform the specification solely for the purpose of 		*/
/*    developing products based on such documents.				*/
/*										*/
/*  2. Source Code Distribution Conditions:					*/
/*										*/
/*  - Redistributions of Source Code must retain the above copyright licenses, 	*/
/*    this list of conditions and the following disclaimers.			*/
/*										*/
/*  - Redistributions in binary form must reproduce the above copyright 	*/
/*    licenses, this list of conditions	and the following disclaimers in the 	*/
/*    documentation and/or other materials provided with the distribution.	*/
/*										*/
/*  3. Disclaimers:								*/
/*										*/
/*  - THE COPYRIGHT LICENSES SET FORTH ABOVE DO NOT REPRESENT ANY FORM OF	*/
/*  LICENSE OR WAIVER, EXPRESS OR IMPLIED, BY ESTOPPEL OR OTHERWISE, WITH	*/
/*  RESPECT TO PATENT RIGHTS HELD BY TCG MEMBERS (OR OTHER THIRD PARTIES)	*/
/*  THAT MAY BE NECESSARY TO IMPLEMENT THIS SPECIFICATION OR OTHERWISE.		*/
/*  Contact TCG Administration (admin@trustedcomputinggroup.org) for 		*/
/*  information on specification licensing rights available through TCG 	*/
/*  membership agreements.							*/
/*										*/
/*  - THIS SPECIFICATION IS PROVIDED "AS IS" WITH NO EXPRESS OR IMPLIED 	*/
/*    WARRANTIES WHATSOEVER, INCLUDING ANY WARRANTY OF MERCHANTABILITY OR 	*/
/*    FITNESS FOR A PARTICULAR PURPOSE, ACCURACY, COMPLETENESS, OR 		*/
/*    NONINFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS, OR ANY WARRANTY 		*/
/*    OTHERWISE ARISING OUT OF ANY PROPOSAL, SPECIFICATION OR SAMPLE.		*/
/*										*/
/*  - Without limitation, TCG and its members and licensors disclaim all 	*/
/*    liability, including liability for infringement of any proprietary 	*/
/*    rights, relating to use of information in this specification and to the	*/
/*    implementation of this specification, and TCG disclaims all liability for	*/
/*    cost of procurement of substitute goods or services, lost profits, loss 	*/
/*    of use, loss of data or any incidental, consequential, direct, indirect, 	*/
/*    or special damages, whether under contract, tort, warranty or otherwise, 	*/
/*    arising in any way out of use or reliance upon this specification or any 	*/
/*    information herein.							*/
/*										*/
/*  (c) Copyright IBM Corp. and others, 2016, 2017				*/
/*										*/
/********************************************************************************/

/* 10.2.14 CryptHash.c */
/* 10.2.14.1 Description */
/* This file contains implementation of cryptographic functions for hashing. */
/* 10.2.14.2 Includes, Defines, and Types */
#define _CRYPT_HASH_C_
#include "Tpm.h"
#define HASH_TABLE_SIZE     (HASH_COUNT + 1)
extern const HASH_INFO   g_hashData[HASH_COUNT + 1];
#ifdef  TPM_ALG_SHA1
HASH_DEF_TEMPLATE(SHA1);
#endif
#ifdef      TPM_ALG_SHA256
HASH_DEF_TEMPLATE(SHA256);
#endif
#ifdef      TPM_ALG_SHA384
HASH_DEF_TEMPLATE(SHA384);
#endif
#ifdef      TPM_ALG_SHA512
HASH_DEF_TEMPLATE(SHA512);
#endif
HASH_DEF nullDef = {{0}};
/* 10.2.14.3 Obligatory Initialization Functions */
BOOL
CryptHashInit(
	      void
	      )
{
    LibHashInit();
    return TRUE;
}
BOOL
CryptHashStartup(
		 void
		 )
{
    return TRUE;
}
/* 10.2.14.4 Hash Information Access Functions */
/* 10.2.14.4.1 Introduction */
/* These functions provide access to the hash algorithm description information. */
/* 10.2.14.4.2 CryptGetHashDef() */
/* This function accesses the hash descriptor associated with a hash a algorithm. The function
   returns NULL for TPM_ALG_NULL and fails if hashAlg is not a hash algorithm. */
PHASH_DEF
CryptGetHashDef(
		TPM_ALG_ID       hashAlg
		)
{
    PHASH_DEF       retVal;
    switch(hashAlg)
	{
#ifdef TPM_ALG_SHA1
	  case TPM_ALG_SHA1:
	    return &SHA1_Def;
	    break;
#endif
#ifdef TPM_ALG_SHA256
	  case TPM_ALG_SHA256:
	    retVal = &SHA256_Def;
	    break;
#endif
#ifdef  TPM_ALG_SHA384
	  case TPM_ALG_SHA384:
	    retVal = &SHA384_Def;
	    break;
#endif
#ifdef  TPM_ALG_SHA512
	  case TPM_ALG_SHA512:
	    retVal = &SHA512_Def;
	    break;
#endif
	  default:
	    retVal = &nullDef;
	    break;
	}
    return retVal;
}
/* 10.2.14.4.3 CryptHashIsImplemented() */
/* This function tests to see if an algorithm ID is a valid hash algorithm. If flag is true, then
   TPM_ALG_NULL is a valid hash. */
/* Return Values Meaning */
/* TRUE hashAlg is a valid, implemented hash on this TPM. */
/* FALSE not valid */
BOOL
CryptHashIsImplemented(
		       TPM_ALG_ID       hashAlg,
		       BOOL             flag
		       )
{
    switch(hashAlg)
	{
#ifdef TPM_ALG_SHA1
	  case TPM_ALG_SHA1:
#endif
#ifdef TPM_ALG_SHA256
	  case TPM_ALG_SHA256:
#endif
#ifdef  TPM_ALG_SHA384
	  case TPM_ALG_SHA384:
#endif
#ifdef  TPM_ALG_SHA512
	  case TPM_ALG_SHA512:
#endif
#ifdef TPM_ALG_SM3_256
	  case TPM_ALG_SHA256:
#endif
	    return TRUE;
	    break;
	  case TPM_ALG_NULL:
	    return flag;
	    break;
	  default:
	    break;
	}
    return FALSE;
}
/* 10.2.14.4.4 GetHashInfoPointer() */
/* This function returns a pointer to the hash info for the algorithm. If the algorithm is not
   supported, function returns a pointer to the data block associated with TPM_ALG_NULL. */
/* NOTE: The data structure must have a digest size of 0 for TPM_ALG_NULL. */
static
const HASH_INFO *
GetHashInfoPointer(
		   TPM_ALG_ID       hashAlg
		   )
{
    UINT32              i;
    //
    // TPM_ALG_NULL is the stop value so search up to it
    for(i = 0; i < HASH_COUNT; i++)
	{
	    if(g_hashData[i].alg == hashAlg)
		return &g_hashData[i];
	}
    // either the input was TPM_ALG_NUL or we didn't find the requested algorithm
    // in either case return a pointer to the TPM_ALG_NULL "hash" descriptor
    return &g_hashData[HASH_COUNT];
}
/* 10.2.14.4.5 CryptHashGetAlgByIndex() */
/* This function is used to iterate through the hashes. TPM_ALG_NULL is returned for all indexes
   that are not valid hashes. If the TPM implements 3 hashes, then an index value of 0 will return
   the first implemented hash and an index of 2 will return the last. All other index values will
   return TPM_ALG_NULL. */
/* Return Values Meaning */
/* TPM_ALG_xxx() a hash algorithm */
/* TPM_ALG_NULL this can be used as a stop value */
LIB_EXPORT TPM_ALG_ID
CryptHashGetAlgByIndex(
		       UINT32           index          // IN: the index
		       )
{
    if(index >= HASH_COUNT)
	return TPM_ALG_NULL;
    return g_hashData[index].alg;
}
/* 10.2.14.4.6 CryptHashGetDigestSize() */
/* Returns the size of the digest produced by the hash. If hashAlg is not a hash algorithm, the TPM
   will FAIL. */
/* Return Values Meaning */
/* 0 TPM_ALG_NULL */
/* > 0 the digest size */
LIB_EXPORT UINT16
CryptHashGetDigestSize(
		       TPM_ALG_ID       hashAlg        // IN: hash algorithm to look up
		       )
{
    return CryptGetHashDef(hashAlg)->digestSize;
}
/* 10.2.14.4.7 CryptHashGetBlockSize() */
/* Returns the size of the block used by the hash. If hashAlg is not a hash algorithm, the TPM will
   FAIL. */
/* Return Values Meaning */
/* 0 TPM_ALG_NULL */
/* > 0 the digest size */
LIB_EXPORT UINT16
CryptHashGetBlockSize(
		      TPM_ALG_ID       hashAlg        // IN: hash algorithm to look up
		      )
{
    return CryptGetHashDef(hashAlg)->blockSize;
}
/* 10.2.14.4.8 CryptHashGetDer */
/* This function returns a pointer to the DER string for the algorithm and indicates its size. */
LIB_EXPORT UINT16
CryptHashGetDer(
		TPM_ALG_ID       hashAlg,       // IN: the algorithm to look up
		const BYTE      **p
		)
{
    const HASH_INFO       *q;
    q = GetHashInfoPointer(hashAlg);
    *p = &q->der[0];
    return q->derSize;
}
/* 10.2.14.4.9 CryptHashGetContextAlg() */
/* This function returns the hash algorithm associated with a hash context. */
TPM_ALG_ID
CryptHashGetContextAlg(
		       PHASH_STATE      state          // IN: the context to check
		       )
{
    return state->hashAlg;
}
/* 10.2.14.5 State Import and Export */
#if 1
/* 10.2.14.5.1 CryptHashCopyState */
/* This function is used to clone a HASH_STATE. */
LIB_EXPORT void
CryptHashCopyState(
		   HASH_STATE          *out,           // OUT: destination of the state
		   const HASH_STATE    *in             // IN: source of the state
		   )
{
    pAssert(out->type == in->type);
    out->hashAlg = in->hashAlg;
    out->def = in->def;
    if(in->hashAlg != TPM_ALG_NULL)
	{
	    // Just verify that the hashAlg makes sense (is implemented)
	    CryptGetHashDef(in->hashAlg);
	    // ... and copy.
	    HASH_STATE_COPY(out, in);
	}
    if(in->type == HASH_STATE_HMAC)
	{
	    const HMAC_STATE    *hIn = (HMAC_STATE *)in;
	    HMAC_STATE          *hOut = (HMAC_STATE *)out;
	    hOut->hmacKey = hIn->hmacKey;
	}
    return;
}
#endif //0
/* 10.2.14.5.2 CryptHashExportState() */
/* This function is used to export a hash or HMAC hash state. This function would be called when
   preparing to context save a sequence object. */
void
CryptHashExportState(
		     PCHASH_STATE         internalFmt,   // IN: the hash state formatted for use by
		     //     library
		     PEXPORT_HASH_STATE   externalFmt    // OUT: the exported hash state
		     )
{
    BYTE                    *outBuf = (BYTE *)externalFmt;
    //
    cAssert(sizeof(HASH_STATE) <= sizeof(EXPORT_HASH_STATE));
#define CopyToOffset(value)						\
    memcpy(&outBuf[offsetof(HASH_STATE,value)], &internalFmt->value,	\
	   sizeof(internalFmt->value))
    CopyToOffset(hashAlg);
    CopyToOffset(type);
    if(internalFmt->type == HASH_STATE_HMAC)
	{
	    HMAC_STATE              *from = (HMAC_STATE *)internalFmt;
	    memcpy(&outBuf[offsetof(HMAC_STATE, hmacKey)], &from->hmacKey,
		   sizeof(from->hmacKey));
	}
    if(internalFmt->hashAlg != TPM_ALG_NULL)
	HASH_STATE_EXPORT(externalFmt, internalFmt);
}
/* 10.2.14.5.3 CryptHashImportState() */
/* This function is used to import the hash state. This function would be called to import a hash
   state when the context of a sequence object was being loaded. */
void
CryptHashImportState(
		     PHASH_STATE          internalFmt,   // OUT: the hash state formatted for use by
		     //     the library
		     PCEXPORT_HASH_STATE  externalFmt    // IN: the exported hash state
		     )
{
    BYTE                    *inBuf = (BYTE *)externalFmt;
    //
#define CopyFromOffset(value)						\
    memcpy(&internalFmt->value, &inBuf[offsetof(HASH_STATE,value)],	\
	   sizeof(internalFmt->value))
    // Copy the hashAlg of the byte-aligned input structure to the structure-aligned
    // internal structure.
    CopyFromOffset(hashAlg);
    CopyFromOffset(type);
    if(internalFmt->hashAlg != TPM_ALG_NULL)
	{
	    internalFmt->def = CryptGetHashDef(internalFmt->hashAlg);
	    HASH_STATE_IMPORT(internalFmt, inBuf);
	    if(internalFmt->type == HASH_STATE_HMAC)
		{
		    HMAC_STATE              *to = (HMAC_STATE *)internalFmt;
		    memcpy(&to->hmacKey, &inBuf[offsetof(HMAC_STATE, hmacKey)],
			   sizeof(to->hmacKey));
		}
	}
}
/* 10.2.14.6 State Modification Functions */
/* 10.2.14.6.1 HashEnd() */
/* Local function to complete a hash that uses the hashDef instead of an algorithm ID. This function
   is used to complete the hash and only return a partial digest. The return value is the size of
   the data copied. */
static UINT16
HashEnd(
	PHASH_STATE      hashState,     // IN: the hash state
	UINT32           dOutSize,      // IN: the size of receive buffer
	PBYTE            dOut           // OUT: the receive buffer
	)
{
    BYTE                temp[MAX_DIGEST_SIZE];
    if(hashState->hashAlg == TPM_ALG_NULL)
	dOutSize = 0;
    if(dOutSize > 0)
	{
	    hashState->def = CryptGetHashDef(hashState->hashAlg);
	    // Set the final size
	    dOutSize = MIN(dOutSize, hashState->def->digestSize);
	    // Complete into the temp buffer and then copy
	    HASH_END(hashState, temp);
	    // Don't want any other functions calling the HASH_END method
	    // directly.
#undef HASH_END
	    memcpy(dOut, &temp, dOutSize);
	}
    hashState->type = HASH_STATE_EMPTY;
    //    hashState->hashAlg = TPM_ALG_ERROR;
    return (UINT16)dOutSize;
}
/* 10.2.14.6.2 CryptHashStart() */
/* Functions starts a hash stack Start a hash stack and returns the digest size. As a side effect,
   the value of stateSize in hashState is updated to indicate the number of bytes of state that were
   saved. This function calls GetHashServer() and that function will put the TPM into failure mode
   if the hash algorithm is not supported. */
/* This function does not use the sequence parameter. If it is necessary to import or export
   context, this will start the sequence in a local state and export the state to the input
   buffer. Will need to add a flag to the state structure to indicate that it needs to be imported
   before it can be used. (BLEH). */
/* Return Values Meaning */
/* 0 hash is TPM_ALG_NULL */
/* >0 digest size */
LIB_EXPORT UINT16
CryptHashStart(
	       PHASH_STATE      hashState,     // OUT: the running hash state
	       TPM_ALG_ID       hashAlg        // IN: hash algorithm
	       )
{
    UINT16               retVal;
    TEST(hashAlg);
    hashState->hashAlg = hashAlg;
    if(hashAlg == TPM_ALG_NULL)
	{
	    retVal = 0;
	}
    else
	{
	    hashState->def = CryptGetHashDef(hashAlg);
	    HASH_START(hashState);
	    retVal = hashState->def->digestSize;
	}
#undef HASH_START
    hashState->type = HASH_STATE_HASH;
    return retVal;
}
/* 10.2.14.6.3 CryptDigestUpdate() */
/* Add data to a hash or HMAC stack. */
LIB_EXPORT void
CryptDigestUpdate(
		  PHASH_STATE      hashState,     // IN: the hash context information
		  UINT32           dataSize,      // IN: the size of data to be added
		  const BYTE      *data           // IN: data to be hashed
		  )
{
    if(hashState->hashAlg != TPM_ALG_NULL)
	{
	    pAssert((hashState->type == HASH_STATE_HASH)
		    || (hashState->type == HASH_STATE_HMAC));
	    hashState->def = CryptGetHashDef(hashState->hashAlg);
	    HASH_DATA(hashState, dataSize, (BYTE *)data);
	}
    return;
}
/* 10.2.14.6.4 CryptHashEnd() */
/* Complete a hash or HMAC computation. This function will place the smaller of digestSize or the
   size of the digest in dOut. The number of bytes in the placed in the buffer is returned. If there
   is a failure, the returned value is <= 0. */
/* Return Values Meaning */
/* 0 no data returned */
/* > 0 the number of bytes in the digest or dOutSize, whichever is smaller */
LIB_EXPORT UINT16
CryptHashEnd(
	     PHASH_STATE      hashState,     // IN: the state of hash stack
	     UINT32           dOutSize,      // IN: size of digest buffer
	     BYTE            *dOut           // OUT: hash digest
	     )
{
    pAssert(hashState->type == HASH_STATE_HASH);
    return HashEnd(hashState, dOutSize, dOut);
}
/* 10.2.14.6.5 CryptHashBlock() */
/* Start a hash, hash a single block, update digest and return the size of the results. */
/* The digestSize parameter can be smaller than the digest. If so, only the more significant bytes
   are returned. */
/* Return Values Meaning */
/* >= 0 number of bytes placed in dOut */
LIB_EXPORT UINT16
CryptHashBlock(
	       TPM_ALG_ID       hashAlg,       // IN: The hash algorithm
	       UINT32           dataSize,      // IN: size of buffer to hash
	       const BYTE      *data,          // IN: the buffer to hash
	       UINT32           dOutSize,      // IN: size of the digest buffer
	       BYTE            *dOut           // OUT: digest buffer
	       )
{
    HASH_STATE          state;
    CryptHashStart(&state, hashAlg);
    CryptDigestUpdate(&state, dataSize, data);
    return HashEnd(&state, dOutSize, dOut);
}
/* 10.2.14.6.6 CryptDigestUpdate2B() */
/* This function updates a digest (hash or HMAC) with a TPM2B. */
/* This function can be used for both HMAC and hash functions so the digestState is void so that
   either state type can be passed. */
LIB_EXPORT void
CryptDigestUpdate2B(
		    PHASH_STATE      state,         // IN: the digest state
		    const TPM2B     *bIn            // IN: 2B containing the data
		    )
{
    // Only compute the digest if a pointer to the 2B is provided.
    // In CryptDigestUpdate(), if size is zero or buffer is NULL, then no change
    // to the digest occurs. This function should not provide a buffer if bIn is
    // not provided.
    pAssert(bIn != NULL);
    CryptDigestUpdate(state, bIn->size, bIn->buffer);
    return;
}
/* 10.2.14.6.7 CryptHashEnd2B() */
/* This function is the same as CypteCompleteHash() but the digest is placed in a TPM2B. This is the
   most common use and this is provided for specification clarity. 'digest.size' should be set to
   indicate the number of bytes to place in the buffer */
/* Return Values Meaning */
/* >=0 the number of bytes placed in 'digest.buffer' */
LIB_EXPORT UINT16
CryptHashEnd2B(
	       PHASH_STATE      state,         // IN: the hash state
	       P2B              digest         // IN: the size of the buffer Out: requested
	       //     number of bytes
	       )
{
    return CryptHashEnd(state, digest->size, digest->buffer);
}
/* 10.2.14.6.8 CryptDigestUpdateInt() */
/* This function is used to include an integer value to a hash stack. The function marshals the
   integer into its canonical form before calling CryptDigestUpdate(). */
LIB_EXPORT void
CryptDigestUpdateInt(
		     void            *state,         // IN: the state of hash stack
		     UINT32           intSize,       // IN: the size of 'intValue' in bytes
		     UINT64           intValue       // IN: integer value to be hashed
		     )
{
#if LITTLE_ENDIAN_TPM == YES
    intValue = REVERSE_ENDIAN_64(intValue);
#endif
    CryptDigestUpdate(state, intSize, &((BYTE *)&intValue)[8 - intSize]);
}
/* 10.2.14.7 HMAC Functions */
/* 10.2.14.7.1 CryptHmacStart */
/* This function is used to start an HMAC using a temp hash context. The function does the
   initialization of the hash with the HMAC key XOR iPad and updates the HMAC key XOR oPad. */
/* The function returns the number of bytes in a digest produced by hashAlg. */
/* Return Values Meaning */
/* >= 0 number of bytes in digest produced by hashAlg (may be zero) */
LIB_EXPORT UINT16
CryptHmacStart(
	       PHMAC_STATE      state,         // IN/OUT: the state buffer
	       TPM_ALG_ID       hashAlg,       // IN: the algorithm to use
	       UINT16           keySize,       // IN: the size of the HMAC key
	       const BYTE      *key            // IN: the HMAC key
	       )
{
    PHASH_DEF                hashDef;
    BYTE *                   pb;
    UINT32                   i;
    //
    hashDef = CryptGetHashDef(hashAlg);
    if(hashDef->digestSize != 0)
	{
	    // If the HMAC key is larger than the hash block size, it has to be reduced
	    // to fit. The reduction is a digest of the hashKey.
	    if(keySize > hashDef->blockSize)
		{
		    // if the key is too big, reduce it to a digest of itself
		    state->hmacKey.t.size = CryptHashBlock(hashAlg, keySize, key,
							   hashDef->digestSize,
							   state->hmacKey.t.buffer);
		}
	    else
		{
		    memcpy(state->hmacKey.t.buffer, key, keySize);
		    state->hmacKey.t.size = keySize;
		}
	    // XOR the key with iPad (0x36)
	    pb = state->hmacKey.t.buffer;
	    for(i = state->hmacKey.t.size; i > 0; i--)
		*pb++ ^= 0x36;
	    // if the keySize is smaller than a block, fill the rest with 0x36
	    for(i = hashDef->blockSize - state->hmacKey.t.size; i > 0; i--)
		*pb++ = 0x36;
	    // Increase the oPadSize to a full block
	    state->hmacKey.t.size = hashDef->blockSize;
	    // Start a new hash with the HMAC key
	    // This will go in the caller's state structure and may be a sequence or not
	    CryptHashStart((PHASH_STATE)state, hashAlg);
	    CryptDigestUpdate((PHASH_STATE)state, state->hmacKey.t.size,
			      state->hmacKey.t.buffer);
	    // XOR the key block with 0x5c ^ 0x36
	    for(pb = state->hmacKey.t.buffer, i = hashDef->blockSize; i > 0; i--)
		*pb++ ^= (0x5c ^ 0x36);
	}
    // Set the hash algorithm
    state->hashState.hashAlg = hashAlg;
    // Set the hash state type
    state->hashState.type = HASH_STATE_HMAC;
    return hashDef->digestSize;
}
/* 10.2.14.7.2 CryptHmacEnd() */
/* This function is called to complete an HMAC. It will finish the current digest, and start a new
   digest. It will then add the oPadKey and the completed digest and return the results in dOut. It
   will not return more than dOutSize bytes. */
/* Return Values Meaning */
/* >= 0 number of bytes in dOut (may be zero) */
LIB_EXPORT UINT16
CryptHmacEnd(
	     PHMAC_STATE      state,         // IN: the hash state buffer
	     UINT32           dOutSize,      // IN: size of digest buffer
	     BYTE            *dOut           // OUT: hash digest
	     )
{
    BYTE                 temp[MAX_DIGEST_SIZE];
    PHASH_STATE          hState = (PHASH_STATE)&state->hashState;
    pAssert(hState->type == HASH_STATE_HMAC);
    hState->def = CryptGetHashDef(hState->hashAlg);
    // Change the state type for completion processing
    hState->type = HASH_STATE_HASH;
    if(hState->hashAlg == TPM_ALG_NULL)
	dOutSize = 0;
    else
	{
	    // Complete the current hash
	    HashEnd(hState, hState->def->digestSize, temp);
	    // Do another hash starting with the oPad
	    CryptHashStart(hState, hState->hashAlg);
	    CryptDigestUpdate(hState, state->hmacKey.t.size, state->hmacKey.t.buffer);
	    CryptDigestUpdate(hState, hState->def->digestSize, temp);
	}
    return HashEnd(hState, dOutSize, dOut);
}
/* 10.2.14.7.3 CryptHmacStart2B() */
/* This function starts an HMAC and returns the size of the digest that will be produced. */
/* This function is provided to support the most common use of starting an HMAC with a TPM2B key. */
/* The caller must provide a block of memory in which the hash sequence state is kept.  The caller
   should not alter the contents of this buffer until the hash sequence is completed or
   abandoned. */
/* Return Values Meaning */
/* > 0 the digest size of the algorithm */
/* = 0 the hashAlg was TPM_ALG_NULL */
LIB_EXPORT UINT16
CryptHmacStart2B(
		 PHMAC_STATE      hmacState,     // OUT: the state of HMAC stack. It will be used
		 //     in HMAC update and completion
		 TPMI_ALG_HASH    hashAlg,       // IN: hash algorithm
		 P2B              key            // IN: HMAC key
		 )
{
    return CryptHmacStart(hmacState, hashAlg, key->size, key->buffer);
}
/*     10.2.14.7.4 CryptHmacEnd2B() */
/* This function is the same as CryptHmacEnd() but the HMAC result is returned in a TPM2B which is
   the most common use. */
/* Return Values Meaning */
/* >=0 the number of bytes placed in digest */
LIB_EXPORT UINT16
CryptHmacEnd2B(
	       PHMAC_STATE      hmacState,     // IN: the state of HMAC stack
	       P2B              digest         // OUT: HMAC
	       )
{
    return CryptHmacEnd(hmacState, digest->size, digest->buffer);
}
/* 10.2.14.8 Mask and Key Generation Functions */
/* 10.2.14.8.1 _crypi_MGF1() */
/* This function performs MGF1 using the selected hash. MGF1 is T(n) = T(n-1) || H(seed ||
   counter). This function returns the length of the mask produced which could be zero if the digest
   algorithm is not supported */
/* Return Values Meaning */
/* 0 hash algorithm was TPM_ALG_NULL */
/* > 0 should be the same as mSize */
LIB_EXPORT UINT16
CryptMGF1(
	  UINT32           mSize,         // IN: length of the mask to be produced
	  BYTE            *mask,          // OUT: buffer to receive the mask
	  TPM_ALG_ID       hashAlg,       // IN: hash to use
	  UINT32           seedSize,      // IN: size of the seed
	  BYTE            *seed           // IN: seed size
	  )
{
    HASH_STATE           hashState;
    PHASH_DEF            hDef = CryptGetHashDef(hashAlg);
    UINT32               remaining;
    UINT32               counter = 0;
    BYTE                 swappedCounter[4];
    // If there is no digest to compute return
    if((hashAlg == TPM_ALG_NULL) || (mSize == 0))
	return 0;
    for(remaining = mSize; ; remaining -= hDef->digestSize)
	{
	    // Because the system may be either Endian...
	    UINT32_TO_BYTE_ARRAY(counter, swappedCounter);
	    // Start the hash and include the seed and counter
	    CryptHashStart(&hashState, hashAlg);
	    CryptDigestUpdate(&hashState, seedSize, seed);
	    CryptDigestUpdate(&hashState, 4, swappedCounter);
	    // Handling the completion depends on how much space remains in the mask
	    // buffer. If it can hold the entire digest, put it there. If not
	    // put the digest in a temp buffer and only copy the amount that
	    // will fit into the mask buffer.
	    HashEnd(&hashState, remaining, mask);
	    if(remaining <= hDef->digestSize)
		break;
	    mask = &mask[hDef->digestSize];
	    counter++;
	}
    return (UINT16)mSize;
}
/*     10.2.14.8.2 CryptKDFa() */
/* This function performs the key generation according to Part 1 of the TPM specification. */
/* This function returns the number of bytes generated which may be zero. */
/* The key and keyStream pointers are not allowed to be NULL. The other pointer values may be
   NULL. The value of sizeInBits must be no larger than (2^18)-1 = 256K bits (32385 bytes). */
/* The once parameter is set to allow incremental generation of a large value. If this flag is TRUE,
   sizeInBits will be used in the HMAC computation but only one iteration of the KDF is
   performed. This would be used for XOR obfuscation so that the mask value can be generated in
   digest-sized chunks rather than having to be generated all at once in an arbitrarily large buffer
   and then XORed() into the result. If once is TRUE, then sizeInBits must be a multiple of 8. */
/* Any error in the processing of this command is considered fatal. */
/* Return Values Meaning */
/* 0 hash algorithm is not supported or is TPM_ALG_NULL */
/* > 0 the number of bytes in the keyStream buffer */
LIB_EXPORT UINT16
CryptKDFa(
	  TPM_ALG_ID       hashAlg,       // IN: hash algorithm used in HMAC
	  const TPM2B     *key,           // IN: HMAC key
	  const TPM2B     *label,         // IN: a label for the KDF
	  const TPM2B     *contextU,      // IN: context U
	  const TPM2B     *contextV,      // IN: context V
	  UINT32           sizeInBits,    // IN: size of generated key in bits
	  BYTE            *keyStream,     // OUT: key buffer
	  UINT32          *counterInOut,  // IN/OUT: caller may provide the iteration
	  //     counter for incremental operations to
	  //     avoid large intermediate buffers.
	  BOOL             once           // IN: TRUE - only 1 iteration is performed
	  //     FALSE if iteration count determined by
	  //     "sizeInBits"
	  )
{
    UINT32                   counter = 0;    // counter value
    INT16                    bytes;          // number of bytes to produce
    BYTE                    *stream = keyStream;
    HMAC_STATE               hState;
    UINT16                   digestSize = CryptHashGetDigestSize(hashAlg);
    pAssert(key != NULL && keyStream != NULL);
    pAssert(once == FALSE || (sizeInBits & 7) == 0);
    if(digestSize == 0)
	return 0;
    if(counterInOut != NULL)
	counter = *counterInOut;
    // If the size of the request is larger than the numbers will handle,
    // it is a fatal error.
    pAssert(((sizeInBits + 7) / 8) <= INT16_MAX);
    bytes = once ? digestSize : (INT16)((sizeInBits + 7) / 8);
    // Generate required bytes
    for(; bytes > 0; bytes -= digestSize)
	{
	    counter++;
	    if(bytes < digestSize)
		digestSize = bytes;
	    // Start HMAC
	    if(CryptHmacStart(&hState, hashAlg, key->size, key->buffer) == 0)
		return 0;
	    // Adding counter
	    CryptDigestUpdateInt(&hState.hashState, 4, counter);
	    // Adding label
	    if(label != NULL)
		HASH_DATA(&hState.hashState, label->size, (BYTE *)label->buffer);
	    // Add a null. SP108 is not very clear about when the 0 is needed but to
	    // make this like the previous version that did not add an 0x00 after
	    // a null-terminated string, this version will only add a null byte
	    // if the label parameter did not end in a null byte, or if no label
	    // is present.
	    if((label == NULL)
	       || (label->size == 0)
	       || (label->buffer[label->size - 1] != 0))
		CryptDigestUpdateInt(&hState.hashState, 1, 0);
	    // Adding contextU
	    if(contextU != NULL)
		HASH_DATA(&hState.hashState, contextU->size, contextU->buffer);
	    // Adding contextV
	    if(contextV != NULL)
		HASH_DATA(&hState.hashState, contextV->size, contextV->buffer);
	    // Adding size in bits
	    CryptDigestUpdateInt(&hState.hashState, 4, sizeInBits);
	    CryptHmacEnd(&hState, digestSize, stream);
	    stream = &stream[digestSize];
	}
    // Mask off bits if the required bits is not a multiple of byte size
    if((sizeInBits % 8) != 0)
	keyStream[0] &= ((1 << (sizeInBits % 8)) - 1);
    if(counterInOut != NULL)
	*counterInOut = counter;
    return (UINT16)((sizeInBits + 7) / 8);
}
/* 	10.2.14.8.3 CryptKDFe() */
/* KDFe() as defined in TPM specification part 1. */
/* This function returns the number of bytes generated which may be zero. */
/* The Z and keyStream pointers are not allowed to be NULL. The other pointer values may be
   NULL. The value of sizeInBits must be no larger than (2^18)-1 = 256K bits (32385 bytes). Any
   error in the processing of this command is considered fatal. */
/* Return Values Meaning */
/* 0 hash algorithm is not supported or is TPM_ALG_NULL */
/* > 0 the number of bytes in the keyStream buffer */
LIB_EXPORT UINT16
CryptKDFe(
	  TPM_ALG_ID       hashAlg,       // IN: hash algorithm used in HMAC
	  TPM2B           *Z,             // IN: Z
	  const TPM2B     *label,         // IN: a label value for the KDF
	  TPM2B           *partyUInfo,    // IN: PartyUInfo
	  TPM2B           *partyVInfo,    // IN: PartyVInfo
	  UINT32           sizeInBits,    // IN: size of generated key in bits
	  BYTE            *keyStream      // OUT: key buffer
	  )
{
    HASH_STATE       hashState;
    PHASH_DEF        hashDef = CryptGetHashDef(hashAlg);
    UINT32           counter = 0;       // counter value
    UINT16           hLen;
    BYTE            *stream = keyStream;
    INT16            bytes;             // number of bytes to generate
    pAssert(keyStream != NULL && Z != NULL && ((sizeInBits + 7) / 8) < INT16_MAX);
    //
    hLen = hashDef->digestSize;
    bytes = (INT16)((sizeInBits + 7) / 8);
    if(hashAlg == TPM_ALG_NULL || bytes == 0)
	return 0;
    // Generate required bytes
    //The inner loop of that KDF uses:
    //  Hash[i] := H(counter | Z | OtherInfo) (5)
    // Where:
    //  Hash[i]   the hash generated on the i-th iteration of the loop.
    //  H()     an approved hash function
    //  counter a 32-bit counter that is initialized to 1 and incremented
    //          on each iteration
    //  Z       the X coordinate of the product of a public ECC key and a
    //          different private ECC key.
    //  OtherInfo   a collection of qualifying data for the KDF defined below.
    //  In this specification, OtherInfo will be constructed by:
    //      OtherInfo := Use | PartyUInfo  | PartyVInfo
    for(; bytes > 0; stream = &stream[hLen], bytes = bytes - hLen)
	{
	    if(bytes < hLen)
		hLen = bytes;
	    counter++;
	    // Do the hash
	    CryptHashStart(&hashState, hashAlg);
	    // Add counter
	    CryptDigestUpdateInt(&hashState, 4, counter);
	    // Add Z
	    if(Z != NULL)
		CryptDigestUpdate2B(&hashState, Z);
	    // Add label
	    if(label != NULL)
		CryptDigestUpdate2B(&hashState, label);
	    // Add a null. SP108 is not very clear about when the 0 is needed but to
	    // make this like the previous version that did not add an 0x00 after
	    // a null-terminated string, this version will only add a null byte
	    // if the label parameter did not end in a null byte, or if no label
	    // is present.
	    if((label == NULL)
	       || (label->size == 0)
	       || (label->buffer[label->size - 1] != 0))
		CryptDigestUpdateInt(&hashState, 1, 0);
	    // Add PartyUInfo
	    if(partyUInfo != NULL)
		CryptDigestUpdate2B(&hashState, partyUInfo);
	    // Add PartyVInfo
	    if(partyVInfo != NULL)
		CryptDigestUpdate2B(&hashState, partyVInfo);
	    // Compute Hash. hLen was changed to be the smaller of bytes or hLen
	    // at the start of each iteration.
	    CryptHashEnd(&hashState, hLen, stream);
	}
    // Mask off bits if the required bits is not a multiple of byte size
    if((sizeInBits % 8) != 0)
	keyStream[0] &= ((1 << (sizeInBits % 8)) - 1);
    return (UINT16)((sizeInBits + 7) / 8);
}
