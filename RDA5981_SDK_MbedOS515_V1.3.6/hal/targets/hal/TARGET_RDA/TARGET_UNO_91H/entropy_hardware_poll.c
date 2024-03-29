/*
 *  Hardware entropy collector for the RDA5981 family, using RDA's TRNG
 *
 *  Copyright (C) 2006-2016, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)

#include "trng_api.h"

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    trng_t trng_obj;
    trng_init(&trng_obj);
    int ret = trng_get_bytes(&trng_obj, output, len, olen);
    trng_free(&trng_obj);
    return ret;
}

#endif

