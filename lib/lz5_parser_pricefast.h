#define LZ5_PRICEFAST_MIN_OFFSET 8

FORCE_INLINE int LZ5_FindMatchFast(LZ5_stream_t* ctx, intptr_t matchIndex, intptr_t matchIndex3, /* Index table will be updated */
                                               const BYTE* ip, const BYTE* const iLimit,
                                               const BYTE** matchpos)
{
    const BYTE* const base = ctx->base;
    const BYTE* const dictBase = ctx->dictBase;
    const intptr_t dictLimit = ctx->dictLimit;
    const BYTE* const dictEnd = dictBase + dictLimit;
    const intptr_t maxDistance = (1 << ctx->params.windowLog) - 1;
    const intptr_t current = (U32)(ip - base);
    const intptr_t lowLimit = ((intptr_t)ctx->lowLimit + maxDistance >= current) ? (intptr_t)ctx->lowLimit : current - maxDistance;
    const BYTE* const lowPrefixPtr = base + dictLimit;
    const size_t minMatchLongOff = ctx->params.minMatchLongOff;
    const BYTE* match, *matchDict;
    size_t ml=0, mlt;

    if (ctx->last_off >= LZ5_PRICEFAST_MIN_OFFSET) {
        intptr_t matchIndexLO = (ip - ctx->last_off) - base;
        if (matchIndexLO >= lowLimit) {
            if (matchIndexLO >= dictLimit) {
                match = base + matchIndexLO;
                if (MEM_readMINMATCH(match) == MEM_readMINMATCH(ip)) {
                    mlt = LZ5_count(ip+MINMATCH, match+MINMATCH, iLimit) + MINMATCH;
                 //   if ((mlt >= minMatchLongOff) || (ctx->last_off < LZ5_MAX_16BIT_OFFSET)) 
                    {
                        *matchpos = match;
                        return (int)mlt;
                    }
                }
            } else {
                match = dictBase + matchIndexLO;
                if ((U32)((dictLimit-1) - matchIndexLO) >= 3)  /* intentional overflow */
                if (MEM_readMINMATCH(match) == MEM_readMINMATCH(ip)) {
                    mlt = LZ5_count_2segments(ip+MINMATCH, match+MINMATCH, iLimit, dictEnd, lowPrefixPtr) + MINMATCH;
                 //   if ((mlt >= minMatchLongOff) || (ctx->last_off < LZ5_MAX_16BIT_OFFSET)) 
                    {
                        *matchpos = base + matchIndexLO;  /* virtual matchpos */
                        return (int)mlt;
                    }
                }
            }
        }
    }


#if MINMATCH == 3
    if (matchIndex3 < current && matchIndex3 >= lowLimit) {
        intptr_t offset = current - matchIndex3;
        if (offset < LZ5_MAX_8BIT_OFFSET) {
            match = ip - offset;
            if (match > base && MEM_readMINMATCH(ip) == MEM_readMINMATCH(match)) {
                ml = 3;//LZ5_count(ip+MINMATCH, match+MINMATCH, iLimit) + MINMATCH;
                *matchpos = match;
            }
        }
    }
#else
    (void)matchIndex3;
#endif

    if ((matchIndex < current) && (matchIndex >= lowLimit)) {
        match = base + matchIndex;
        if ((U32)(ip - match) >= LZ5_PRICEFAST_MIN_OFFSET) {
            if (matchIndex >= dictLimit) {
                if (*(match+ml) == *(ip+ml) && (MEM_read32(match) == MEM_read32(ip))) {
                    mlt = LZ5_count(ip+MINMATCH, match+MINMATCH, iLimit) + MINMATCH;
                    if ((mlt >= minMatchLongOff) || ((U32)(ip - match) < LZ5_MAX_16BIT_OFFSET))
                    if (!ml || (mlt > ml)) // && LZ5_better_price((ip - *matchpos), ml, (ip - match), mlt, ctx->last_off)))
                    { ml = mlt; *matchpos = match; }
                }
            } else {
                matchDict = dictBase + matchIndex;
    //            fprintf(stderr, "dictBase[%p]+matchIndex[%d]=match[%p] dictLimit=%d base=%p ip=%p iLimit=%p off=%d\n", dictBase, matchIndex, match, dictLimit, base, ip, iLimit, (U32)(ip-match));
                if ((U32)((dictLimit-1) - matchIndex) >= 3)  /* intentional overflow */
                if (MEM_read32(matchDict) == MEM_read32(ip)) {
                    mlt = LZ5_count_2segments(ip+MINMATCH, matchDict+MINMATCH, iLimit, dictEnd, lowPrefixPtr) + MINMATCH;
                    if ((mlt >= minMatchLongOff) || ((U32)(ip - match) < LZ5_MAX_16BIT_OFFSET))
                    if (!ml || (mlt > ml)) // && LZ5_better_price((ip - *matchpos), ml, (U32)(ip - match), mlt, ctx->last_off)))
                    { ml = mlt; *matchpos = match; }   /* virtual matchpos */
                }
            }
        }
    }
    
    return (int)ml;
}


FORCE_INLINE int LZ5_FindMatchFaster (LZ5_stream_t* ctx, U32 matchIndex,  /* Index table will be updated */
                                               const BYTE* ip, const BYTE* const iLimit,
                                               const BYTE** matchpos)
{
    const BYTE* const base = ctx->base;
    const BYTE* const dictBase = ctx->dictBase;
    const U32 dictLimit = ctx->dictLimit;
    const BYTE* const dictEnd = dictBase + dictLimit;
    const U32 maxDistance = (1 << ctx->params.windowLog) - 1;
    const U32 current = (U32)(ip - base);
    const U32 lowLimit = (ctx->lowLimit + maxDistance >= current) ? ctx->lowLimit : current - maxDistance;
    const BYTE* const lowPrefixPtr = base + dictLimit;
    const size_t minMatchLongOff = ctx->params.minMatchLongOff;
    const BYTE* match, *matchDict;
    size_t ml=0, mlt;

    if (matchIndex < current && matchIndex >= lowLimit) {
        match = base + matchIndex;
        if ((U32)(ip - match) >= LZ5_PRICEFAST_MIN_OFFSET) {
            if (matchIndex >= dictLimit) {
                if (MEM_read32(match) == MEM_read32(ip)) {
                    mlt = LZ5_count(ip+MINMATCH, match+MINMATCH, iLimit) + MINMATCH;
                    if ((mlt >= minMatchLongOff) || ((U32)(ip - match) < LZ5_MAX_16BIT_OFFSET))
                    { ml = mlt; *matchpos = match; }
                }
            } else {
                matchDict = dictBase + matchIndex;
                if ((U32)((dictLimit-1) - matchIndex) >= 3)  /* intentional overflow */
                if (MEM_read32(matchDict) == MEM_read32(ip)) {
                    mlt = LZ5_count_2segments(ip+MINMATCH, matchDict+MINMATCH, iLimit, dictEnd, lowPrefixPtr) + MINMATCH;
                    if ((mlt >= minMatchLongOff) || ((U32)(ip - match) < LZ5_MAX_16BIT_OFFSET))
                    { ml = mlt; *matchpos = match; }   /* virtual matchpos */
                }
            }
        }
    }
    
    return (int)ml;
}



FORCE_INLINE int LZ5_compress_priceFast(
        LZ5_stream_t* const ctx,
        const BYTE* ip,
        const BYTE* const iend)
{
    const BYTE* anchor = ip;
    const BYTE* const mflimit = iend - MFLIMIT;
    const BYTE* const matchlimit = (iend - LASTLITERALS);

    size_t ml, ml2=0;
    const BYTE* ref=NULL;
    const BYTE* start2=NULL;
    const BYTE* ref2=NULL;
    const BYTE* lowPrefixPtr = ctx->base + ctx->dictLimit;
    U32* HashTable  = ctx->hashTable;
#if MINMATCH == 3
    U32* HashTable3  = ctx->hashTable3;
#endif 
    const BYTE* const base = ctx->base;
    const size_t minMatchLongOff = ctx->params.minMatchLongOff;
    U32* HashPos;

    /* init */
    ip++;

    /* Main Loop */
    while (ip < mflimit)
    {
        HashPos = &HashTable[LZ5_hashPtr(ip, ctx->params.hashLog, ctx->params.searchLength)];
#if MINMATCH == 3
        {
        U32* HashPos3 = &HashTable3[LZ5_hash3Ptr(ip, ctx->params.hashLog3)];
        ml = LZ5_FindMatchFast (ctx, *HashPos, *HashPos3, ip, matchlimit, (&ref));
        *HashPos3 = (U32)(ip - base);
        }
#else
        ml = LZ5_FindMatchFast (ctx, *HashPos, 0, ip, matchlimit, (&ref));
#endif 
        if ((U32)(ip - base) >= *HashPos + LZ5_PRICEFAST_MIN_OFFSET)
            *HashPos = (U32)(ip - base);

        if (!ml) { ip++; continue; }
        if ((int)(ip - ref) == ctx->last_off) { ml2=0; ref=ip; goto _Encode; }

        {
        int back = 0;
        while ((ip+back>anchor) && (ref+back > lowPrefixPtr) && (ip[back-1] == ref[back-1])) back--;
        ml -= back;
        ip += back;
        ref += back;
        }
        
_Search:
        if (ip+ml >= mflimit) goto _Encode;

        start2 = ip + ml - 2;
        HashPos = &HashTable[LZ5_hashPtr(start2, ctx->params.hashLog, ctx->params.searchLength)];
        ml2 = LZ5_FindMatchFaster(ctx, *HashPos, start2, matchlimit, (&ref2));      
        if ((U32)(start2 - base) >= *HashPos + LZ5_PRICEFAST_MIN_OFFSET)
            *HashPos = (U32)(start2 - base);

        if (!ml2) goto _Encode;

        {
        int back = 0;
        while ((start2+back>ip) && (ref2+back > lowPrefixPtr) && (start2[back-1] == ref2[back-1])) back--;
        ml2 -= back;
        start2 += back;
        ref2 += back;
        }

    //    LZ5_DEBUG("%u: TRY last_off=%d literals=%u off=%u mlen=%u literals2=%u off2=%u mlen2=%u best=%d\n", (U32)(ip - ctx->inputBuffer), ctx->last_off, (U32)(ip - anchor), off0, (U32)ml,  (U32)(start2 - anchor), off1, ml2, (U32)(best_pos - ip));

        if (ml2 <= ml) { ml2 = 0; goto _Encode; }

        if (start2 <= ip)
        {

            ip = start2; ref = ref2; ml = ml2;
            ml2 = 0;
            goto _Encode;
        }

        if (start2 - ip < 3) 
        { 
            ip = start2; ref = ref2; ml = ml2;
            ml2 = 0; 
            goto _Search; 
        }

        if (start2 < ip + ml) 
        {
            size_t correction = ml - (int)(start2 - ip);
            start2 += correction;
            ref2 += correction;
            ml2 -= correction;
            if (ml2 < 3) { ml2 = 0; }
            if ((ml2 < minMatchLongOff) && ((U32)(start2 - ref2) >= LZ5_MAX_16BIT_OFFSET))  { ml2 = 0; }
        }

_Encode:
        if (LZ5_encodeSequence_LZ5v2(ctx, &ip, &anchor, ml, ref)) goto _output_error;

        if (ml2)
        {
            ip = start2; ref = ref2; ml = ml2;
            ml2 = 0;
            goto _Search;
        }
    }

    /* Encode Last Literals */
    ip = iend;
    if (LZ5_encodeLastLiterals_LZ5v2(ctx, &ip, &anchor)) goto _output_error;

    /* End */
    return 1;
_output_error:
    return 0;
}

