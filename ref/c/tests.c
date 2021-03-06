#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "segwit_addr.h"
#include "txref_code.h"

#define MAGIC_BTC_MAINNET 0x03

struct test_vector {
    const char magic;
    const char* txref_encoded;
    int blockheight;
    int pos;
    int enc_fail; //0 == must not fail, 1 == can fail, 2 == can fail and continue with next test, 3 == skip
    int dec_fail;
};

static struct test_vector test_vectors[] = {
    {
        MAGIC_BTC_MAINNET,
        "tx1-rjk0-u5ng-4jsf-mc",
        466793,
        2205,
        0,0
    },
    {
        MAGIC_BTC_MAINNET,
        "tx1-rjk0-u5n1-2jsi-mc", /* error correct test >2tsi< instead of >4jsf<*/
        466793,
        2205,
        1,0
    },
    {
        MAGIC_BTC_MAINNET,
        "tx1-rqqq-qqqq-qmhu-qk",
        0,
        0,
        0,0
    },
    {
        MAGIC_BTC_MAINNET,
        "tx1-rzqq-qqqq-uvlj-ez",
        1,
        0,
        0,0
    },
    {
        MAGIC_BTC_MAINNET,
        "tx1-rjk0-u5ng-4jsf-mc", /* complete invalid */
        0,
        0,
        1,1 /* enc & dec must fail */
    },
    {
        MAGIC_BTC_MAINNET,
        "tx1-r7ll-lrar-a27h-kt",
        2097151, /* last valid block height with current enc/dec version is 0x1FFFFF*/
        1000,
        0,0
    },
    {
        MAGIC_BTC_MAINNET,
        "", /* encoding must fail, no txref to chain against */
        2097152, /* invalid height */
        1000,
        2,1
    },
    {
        MAGIC_BTC_MAINNET,
        "tx1-r7ll-llll-khym-tq",
        2097151, /* last valid block height with current enc/dec version is 0x1FFFFF*/
        8191, /* last valid tx pos is 0x1FFF */
        0,0
    },
    {
        MAGIC_BTC_MAINNET,
        "", /* encoding must fail, no txref to chain against */
        2097151, /* last valid block height with current enc/dec version is 0x1FFFFF*/
        8192, /* invalid tx pos */
        2,1
    },
    {
        MAGIC_BTC_MAINNET,
        "tx1-r7ll-lrqq-vq5e-gg",
        2097151, /* last valid block height with current enc/dec version is 0x1FFFFF*/
        0,
        0,0
    },
    {
        MAGIC_BTC_MAINNET,
        "tx1-rqqq-qull-6v87-r7",
        0,
        8191, /* last valid tx pos is 0x1FFF */
        0,0
    },
    {
        MAGIC_BTC_MAINNET,
        "tx1-rjk0-u5ng-gghq-fkg7", /* valid Bech32, but 10x5bit packages instead of 8 */
        0,
        0,
        3,2 /* ignore encoding */
    },
    {
        MAGIC_BTC_MAINNET,
        "tx1-rjk0-u5qd-s43z", /* valid Bech32, but 6x5bit packages instead of 8 */
        0,
        0,
        3,2 /* ignore encoding */
    },

};

int main(void) {
    int fail = 0;
    size_t i;
    char magic;
    int bh_check;
    int pos_check;
    int res;
    char encoded_txref[22] = {}; /* 22 bytes is required with 1 char magic, use max 6 bytes */

    for (i = 0; i < sizeof(test_vectors) / sizeof(test_vectors[0]); ++i) {
        memset(encoded_txref, 0, sizeof(encoded_txref));
        if (test_vectors[i].enc_fail != 3) {
            res = btc_txref_encode(encoded_txref, test_vectors[i].magic, test_vectors[i].blockheight, test_vectors[i].pos);
            if (strcmp(test_vectors[i].txref_encoded, encoded_txref) != 0 && !test_vectors[i].enc_fail) {
                fail++;
            }
            if (test_vectors[i].enc_fail == 2) {
                continue;
            }

            res = btc_txref_decode(encoded_txref, &magic, &bh_check, &pos_check);
        }
        else {
            res = btc_txref_decode(test_vectors[i].txref_encoded, &magic, &bh_check, &pos_check);
        }

        if (res!=1 && !test_vectors[i].dec_fail) {
            fail++;
        }
        if (test_vectors[i].dec_fail == 2) {
            continue;
        }
        if (test_vectors[i].pos != pos_check) {
            fail++;
        }
        if (test_vectors[i].blockheight != bh_check) {
            fail++;
        }
        if (magic != test_vectors[i].magic) {
            fail++;
        }
    }

    printf("%i failures\n", fail);
    return fail != 0;
}
