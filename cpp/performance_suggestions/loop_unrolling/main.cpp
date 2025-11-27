

#define quad_loop(start, step, end)\
    for (; (start) + 4 * (step) - 1 < end; (start) += 4 * (step)) {\
        work((start))\
        work((start) + (step))\
        work((start) + 2 * (step))\
        work((start) + 3 * (step))\
    }

#define square_loop(start, step, end)\
    for (; (start) + 2 * (step) - 1 < end; (start) += 2 * (step)) {\
        work((start))\
        work((start) + (step))\
    }

#define unrolling_loop(start, step, end)\
    quad_loop((start), (step), (end))\
    square_loop((start), (step), (end))\
    if ((start) + (step) - 1 < (end)) {\
        work((start))\
        (start) += (step);\
    }




    // #define work(i)\
    // do {\
    //     __m512i va = _mm512_set1_epi64(rule.vlaue);\
    //     __m512i vb = _mm512_set_epi64(a[i][rule.offset], a[i + 1][rule.offset], a[i + 2][rule.offset], a[i + 3][rule.offset],\
    //                 a[i + 4][rule.offset], a[i + 5][rule.offset], a[i + 6][rule.offset], a[i + 7][rule.offset]);\
    //     __mmask64 cmp = _mm512_cmpeq_epi8_mask(va, vb);\
    //     benchmark::DoNotOptimize(cmp);\
    // } while (0);

    // unrolling_loop(i, 8, size);

    // #undef work