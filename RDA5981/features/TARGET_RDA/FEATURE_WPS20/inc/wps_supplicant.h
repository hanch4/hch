/*
 * wpa_supplicant / WPS integration
 * Copyright (c) 2008-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef _WPS_SUPPLICANT_H
#define _WPS_SUPPLICANT_H

#include "wps.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wps_sm {
    u8 last_identifier;
    u8 *identity;
    size_t identity_len;
    u8 own_mac_addr[6];
    struct wps_context *wps_ctx;
};

struct eap_hdr {
    u8 code;
    u8 identifier;
    u16 length; /* including code and identifier; network byte order */
    /* followed by length-4 octets of data */
};
enum { EAP_CODE_REQUEST = 1, EAP_CODE_RESPONSE = 2, EAP_CODE_SUCCESS = 3,
       EAP_CODE_FAILURE = 4 };

typedef enum {
    EAP_TYPE_NONE = 0,
    EAP_TYPE_IDENTITY = 1 /* RFC 3748 */,
    EAP_TYPE_NOTIFICATION = 2 /* RFC 3748 */,
    EAP_TYPE_NAK = 3 /* Response only, RFC 3748 */,
    EAP_TYPE_MD5 = 4, /* RFC 3748 */
    EAP_TYPE_OTP = 5 /* RFC 3748 */,
    EAP_TYPE_GTC = 6, /* RFC 3748 */
    EAP_TYPE_TLS = 13 /* RFC 2716 */,
    EAP_TYPE_LEAP = 17 /* Cisco proprietary */,
    EAP_TYPE_SIM = 18 /* RFC 4186 */,
    EAP_TYPE_TTLS = 21 /* RFC 5281 */,
    EAP_TYPE_AKA = 23 /* RFC 4187 */,
    EAP_TYPE_PEAP = 25 /* draft-josefsson-pppext-eap-tls-eap-06.txt */,
    EAP_TYPE_MSCHAPV2 = 26 /* draft-kamath-pppext-eap-mschapv2-00.txt */,
    EAP_TYPE_TLV = 33 /* draft-josefsson-pppext-eap-tls-eap-07.txt */,
    EAP_TYPE_TNC = 38 /* TNC IF-T v1.0-r3; note: tentative assignment;
               * type 38 has previously been allocated for
               * EAP-HTTP Digest, (funk.com) */,
    EAP_TYPE_FAST = 43 /* RFC 4851 */,
    EAP_TYPE_PAX = 46 /* RFC 4746 */,
    EAP_TYPE_PSK = 47 /* RFC 4764 */,
    EAP_TYPE_SAKE = 48 /* RFC 4763 */,
    EAP_TYPE_IKEV2 = 49 /* RFC 5106 */,
    EAP_TYPE_AKA_PRIME = 50 /* RFC 5448 */,
    EAP_TYPE_GPSK = 51 /* RFC 5433 */,
    EAP_TYPE_PWD = 52 /* RFC 5931 */,
    EAP_TYPE_EKE = 53 /* RFC 6124 */,
    EAP_TYPE_EXPANDED = 254 /* RFC 3748 */
} EapType;

/* SMI Network Management Private Enterprise Code for vendor specific types */
enum {
    EAP_VENDOR_IETF = 0,
    EAP_VENDOR_MICROSOFT = 0x000137 /* Microsoft */,
    EAP_VENDOR_WFA = 0x00372A /* Wi-Fi Alliance */,
    EAP_VENDOR_HOSTAP = 39068 /* hostapd/wpa_supplicant project */
};

extern struct wps_context *wpas_wps_init(u8 *own_addr);
extern struct wpabuf *wpas_wps_start_pbc(struct wps_context *wps);
extern int wpas_wps_ssid_wildcard_ok(u8 *ie, u32 ie_len);
extern int wps_connect(struct wps_context *wps);
extern int wps_start(struct wps_context *wps);
extern void wps_sm_start(struct wps_context *wps);
extern s32 set_wps_ie(struct wpabuf *ie);
extern struct wpabuf * eap_sm_buildIdentity(void);
extern void wland_wifi_wps_end(void);
extern void wland_wifi_wps_fail(void);
extern s32 wland_set_wps_ie(void *ie, u32 len);

#ifdef __cplusplus
}
#endif

#endif /* WPS_SUPPLICANT_H */
