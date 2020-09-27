//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       wincrypt.h
//
//  Contents:   Cryptographic API Prototypes and Definitions
//
//----------------------------------------------------------------------------

#ifndef __WINCRYPT_H__
#define __WINCRYPT_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// Algorithm IDs and Flags
//

// ALG_ID crackers
#define GET_ALG_CLASS(x)                (x & (3 << 14))
#define GET_ALG_TYPE(x)                 (x & (15 << 10))
#define GET_ALG_SID(x)                  (x & (511))

// Algorithm classes
#define ALG_CLASS_ANY                   (0)
#define ALG_CLASS_SIGNATURE             (1 << 14)
#define ALG_CLASS_MSG_ENCRYPT           (2 << 14)
#define ALG_CLASS_DATA_ENCRYPT          (3 << 14)
#define ALG_CLASS_HASH                  (4 << 14)
#define ALG_CLASS_KEY_EXCHANGE          (5 << 14)

// Algorithm types
#define ALG_TYPE_ANY                    (0)
#define ALG_TYPE_DSS                    (1 << 10)
#define ALG_TYPE_RSA                    (2 << 10)
#define ALG_TYPE_BLOCK                  (3 << 10)
#define ALG_TYPE_STREAM                 (4 << 10)

// Generic sub-ids
#define ALG_SID_ANY                     (0)

// Some RSA sub-ids
#define ALG_SID_RSA_ANY                 0
#define ALG_SID_RSA_PKCS                1
#define ALG_SID_RSA_MSATWORK            2
#define ALG_SID_RSA_ENTRUST             3
#define ALG_SID_RSA_PGP                 4

// Some DSS sub-ids
//
#define ALG_SID_DSS_ANY                 0
#define ALG_SID_DSS_PKCS                1
#define ALG_SID_DSS_DMS                 2

// Block cipher sub ids
// DES sub_ids
#define ALG_SID_DES                     1

// RC2 sub-ids
#define ALG_SID_RC2                     2

// Stream cipher sub-ids
#define ALG_SID_RC4                     1
#define ALG_SID_SEAL                    2

// Hash sub ids
#define ALG_SID_MD2                     1
#define ALG_SID_MD4                     2
#define ALG_SID_MD5                     3
#define ALG_SID_SHA                     4
#define ALG_SID_MAC                     5

// Our silly example sub-id
#define ALG_SID_EXAMPLE                 80

#ifndef ALGIDDEF
#define ALGIDDEF
typedef unsigned int ALG_ID;
#endif

// algorithm identifier definitions
#define CALG_MD2        (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MD2)
#define CALG_MD4        (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MD4)
#define CALG_MD5        (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MD5)
#define CALG_SHA        (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SHA)
#define CALG_MAC        (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MAC)
#define CALG_RSA_SIGN   (ALG_CLASS_SIGNATURE | ALG_TYPE_RSA | ALG_SID_RSA_ANY)
#define CALG_DSS_SIGN   (ALG_CLASS_SIGNATURE | ALG_TYPE_DSS | ALG_SID_DSS_ANY)
#define CALG_RSA_KEYX   (ALG_CLASS_KEY_EXCHANGE|ALG_TYPE_RSA|ALG_SID_RSA_ANY)
#define CALG_DES        (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_DES)
#define CALG_RC2        (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_RC2)
#define CALG_RC4        (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_STREAM|ALG_SID_RC4)
#define CALG_SEAL       (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_STREAM|ALG_SID_SEAL)

typedef struct _VTableProvStruc {
    FARPROC FuncVerifyImage;
} VTableProvStruc, *PVTableProvStruc;

typedef unsigned long HCRYPTPROV;
typedef unsigned long HCRYPTKEY;
typedef unsigned long HCRYPTHASH;

// dwFlags definitions for CryptAquireContext
#define CRYPT_NEWKEYSET         0x8
#define CRYPT_DELETEKEYSET      0x10

// dwFlag definitions for CryptGenKey
#define CRYPT_EXPORTABLE        0x00000001
#define CRYPT_USER_PROTECTED    0x00000002
#define CRYPT_CREATE_SALT       0x00000004
#define CRYPT_UPDATE_KEY        0x00000008

// exported key blob definitions
#define SIMPLEBLOB              0x1
#define PUBLICKEYBLOB           0x6

#define AT_KEYEXCHANGE          1
#define AT_SIGNATURE            2

#define CRYPT_USERDATA          1

// dwParam
#define KP_IV                   1       // Initialization vector
#define KP_SALT                 2       // Salt value
#define KP_PADDING              3       // Padding values
#define KP_MODE                 4       // Mode of the cipher
#define KP_MODE_BITS            5       // Number of bits to feedback
#define KP_PERMISSIONS          6       // Key permissions DWORD
#define KP_ALGID                7       // Key algorithm
#define KP_BLOCKLEN             8       // Block size of the cipher

// KP_PADDING
#define PKCS5_PADDING           1       // PKCS 5 (sec 6.2) padding method

// KP_MODE
#define CRYPT_MODE_CBC          1       // Cipher block chaining
#define CRYPT_MODE_ECB          2       // Electronic code book
#define CRYPT_MODE_OFB          3       // Output feedback mode
#define CRYPT_MODE_CFB          4       // Cipher feedback mode
#define CRYPT_MODE_CTS          5       // Ciphertext stealing mode

// KP_PERMISSIONS
#define CRYPT_ENCRYPT           1       // Allow encryption
#define CRYPT_DECRYPT           2       // Allow decryption
#define CRYPT_EXPORT            4       // Allow key to be exported
#define CRYPT_READ              8       // Allow parameters to be read
#define CRYPT_WRITE             16      // Allow parameters to be set
#define CRYPT_MAC               32      // Allow MACs to be used with key


#define CRYPT_FAILED            FALSE
#define CRYPT_SUCCEED           TRUE

#define RCRYPT_SUCCEEDED(rt)     ((rt) == CRYPT_SUCCEED)
#define RCRYPT_FAILED(rt)        ((rt) == CRYPT_FAILED)

#define PROV_RSA_FULL           1
#define PROV_RSA_SIG            2
#define PROV_DSS                3
#define PROV_FORTEZZA           4
#define PROV_MS_MAIL            5
#define PROV_SSL                6

#define MS_DEF_PROV         TEXT("Microsoft Base Cryptographic Provider v1.0")

#define MAXUIDLEN               64

#define CUR_BLOB_VERSION        2

typedef struct _PUBLICKEYSTRUC {
        BYTE    bType;
        BYTE    bVersion;
        WORD    reserved;
        ALG_ID  aiKeyAlg;
} PUBLICKEYSTRUC;

typedef struct _RSAPUBKEY {
        DWORD   magic;                  // Has to be RSA1
        DWORD   bitlen;                 // # of bits in modulus
        DWORD   pubexp;                 // public exponent
                                        // Modulus data follows
} RSAPUBKEY;



WINADVAPI
BOOL
WINAPI
CryptAcquireContextA(
    HCRYPTPROV *phProv,
    LPCSTR pszContainer,
    LPCSTR pszProvider,
    DWORD dwProvType,
    DWORD dwFlags);
WINADVAPI
BOOL
WINAPI
CryptAcquireContextW(
    HCRYPTPROV *phProv,
    LPCWSTR pszContainer,
    LPCWSTR pszProvider,
    DWORD dwProvType,
    DWORD dwFlags);
#ifdef UNICODE
#define CryptAcquireContext  CryptAcquireContextW
#else
#define CryptAcquireContext  CryptAcquireContextA
#endif // !UNICODE


WINADVAPI
BOOL
WINAPI
CryptReleaseContext(
    HCRYPTPROV hProv,
    DWORD dwFlags);


WINADVAPI
BOOL
WINAPI
CryptGenKey(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    DWORD dwFlags,
    HCRYPTKEY *phKey);

WINADVAPI
BOOL
WINAPI
CryptDeriveKey(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    HCRYPTHASH hBaseData,
    DWORD dwFlags,
    HCRYPTKEY *phKey);


WINADVAPI
BOOL
WINAPI
CryptDestroyKey(
    HCRYPTKEY hKey);

WINADVAPI
BOOL
WINAPI
CryptSetKeyParam(
    HCRYPTKEY hKey,
    DWORD dwParam,
    BYTE *pbData,
    DWORD dwFlags);

WINADVAPI
BOOL
WINAPI
CryptGetKeyParam(
    HCRYPTKEY hKey,
    DWORD dwParam,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags);

WINADVAPI
BOOL
WINAPI
CryptGenRandom(
    HCRYPTPROV hProv,
    DWORD dwLen,
    BYTE *pbBuffer);

WINADVAPI
BOOL
WINAPI
CryptGetUserKey(
    HCRYPTPROV hProv,
    DWORD dwKeySpec,
    HCRYPTKEY *phUserKey);

WINADVAPI
BOOL
WINAPI
CryptExportKey(
    HCRYPTKEY hKey,
    HCRYPTKEY hExpKey,
    DWORD dwBlobType,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen);

WINADVAPI
BOOL
WINAPI
CryptImportKey(
    HCRYPTPROV hProv,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    HCRYPTKEY hPubKey,
    DWORD dwFlags,
    HCRYPTKEY *phKey);

WINADVAPI
BOOL
WINAPI
CryptEncrypt(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwBufLen);

WINADVAPI
BOOL
WINAPI
CryptDecrypt(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen);

WINADVAPI
BOOL
WINAPI
CryptCreateHash(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    HCRYPTKEY hKey,
    DWORD dwFlags,
    HCRYPTHASH *phHash);

WINADVAPI
BOOL
WINAPI
CryptHashData(
    HCRYPTHASH hHash,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    DWORD dwFlags);

WINADVAPI
BOOL
WINAPI
CryptHashSessionKey(
    HCRYPTHASH hHash,
    HCRYPTKEY hKey,
    DWORD dwFlags);

WINADVAPI
BOOL
WINAPI
CryptGetHashValue(
    HCRYPTHASH hHash,
    DWORD dwFlags,
    BYTE *pbHash,
    DWORD *pdwHashLen);

WINADVAPI
BOOL
WINAPI
CryptDestroyHash(
    HCRYPTHASH hHash);

WINADVAPI
BOOL
WINAPI
CryptSignHashA(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCSTR sDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen);

WINADVAPI
BOOL
WINAPI
CryptSignHashW(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCWSTR sDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen);

#ifdef UNICODE
#define CryptSignHash  CryptSignHashW
#else
#define CryptSignHash  CryptSignHashA
#endif // !UNICODE

WINADVAPI
BOOL
WINAPI
CryptVerifySignatureA(
    HCRYPTHASH hHash,
    CONST BYTE *pbSignature,
    DWORD dwSigLen,
    HCRYPTKEY hPubKey,
    LPCSTR sDescription,
    DWORD dwFlags);

WINADVAPI
BOOL
WINAPI
CryptVerifySignatureW(
    HCRYPTHASH hHash,
    CONST BYTE *pbSignature,
    DWORD dwSigLen,
    HCRYPTKEY hPubKey,
    LPCWSTR sDescription,
    DWORD dwFlags);

#ifdef UNICODE
#define CryptVerifySignature  CryptVerifySignatureW
#else
#define CryptVerifySignature  CryptVerifySignatureA
#endif // !UNICODE

WINADVAPI
BOOL
WINAPI
CryptSetProviderA(
    LPCSTR pszProvName,
    DWORD dwProvType);
WINADVAPI
BOOL
WINAPI
CryptSetProviderW(
    LPCWSTR pszProvName,
    DWORD dwProvType);
#ifdef UNICODE
#define CryptSetProvider  CryptSetProviderW
#else
#define CryptSetProvider  CryptSetProviderA
#endif // !UNICODE





#ifdef __cplusplus
}       // Balance extern "C" above
#endif

#endif // __WINCRYPT_H__
