/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 3.00.15 */
/* at Wed Jun 26 10:54:36 1996
 */
/* Compiler settings for DigSig.IDL:
    Os, W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef IID_DEFINED
#define IID_DEFINED

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // IID_DEFINED

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_IPublicKeyContainer = {0xE29F6460,0xC53E,0x11cf,{0x89,0xD4,0x00,0xAA,0x00,0x60,0xFA,0x2B}};


const IID IID_IPkcs10 = {0xE29F6462,0xC53E,0x11cf,{0x89,0xD4,0x00,0xAA,0x00,0x60,0xFA,0x2B}};


const IID IID_IPkcs7SignedData = {0x95F87ED1,0xBD63,0x11cf,{0x8A,0x66,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const IID IID_IAmHashed = {0xF9AF3CB1,0x5621,0x11cf,{0x84,0xAA,0x7E,0xEF,0xF0,0x4D,0x00,0x01}};


const IID IID_ISignableDocument = {0xC7DE52F1,0x9A77,0x11cf,{0xAD,0x60,0xAE,0x97,0x56,0x3F,0x00,0x02}};


const IID IID_ISignerInfo = {0xDEA047A5,0x505D,0x11cf,{0xB1,0xCA,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const IID IID_IX509 = {0xE29F6461,0xC53E,0x11cf,{0x89,0xD4,0x00,0xAA,0x00,0x60,0xFA,0x2B}};


const IID IID_ICertificateList = {0x290E0AA0,0x50F7,0x11cf,{0x84,0xA8,0x1E,0x09,0x27,0x4C,0x00,0x01}};


const IID IID_ICertificateStore = {0xBD808DB1,0xA50F,0x11cf,{0x8A,0x57,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const IID IID_ICertificateStoreRegInit = {0x330896B0,0xA529,0x11cf,{0x8A,0x57,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const IID IID_ICertificateStoreAux = {0x1F37F762,0xA467,0x11cf,{0x8A,0x57,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const IID IID_IAmSigned = {0x00E2A642,0xBEF7,0x11cf,{0x8A,0x66,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const IID IID_IPersistMemBlob = {0xAAB149A0,0xCF7B,0x11cf,{0x8A,0x76,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const IID IID_IPersistFileHandle = {0xFE2A3F20,0xC633,0x11cf,{0x89,0xD5,0x00,0xAA,0x00,0x60,0xFA,0x2B}};


const IID IID_IX500Name = {0xBD6BEC66,0x55B9,0x11cf,{0x84,0xAA,0x7E,0xEF,0xF0,0x4D,0x00,0x01}};


const IID IID_ISelectedAttributes = {0x1385FD20,0xCEE3,0x11cf,{0x8A,0x75,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const IID LIBID_DigSig = {0xBD6BEC64,0x55B9,0x11cf,{0x84,0xAA,0x7E,0xEF,0xF0,0x4D,0x00,0x01}};


const CLSID CLSID_Pkcs10 = {0xDEA047A7,0x505D,0x11cf,{0xB1,0xCA,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const CLSID CLSID_Pkcs7SignedData = {0xDEA047A8,0x505D,0x11cf,{0xB1,0xCA,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const CLSID CLSID_Pkcs7SignerInfo = {0x37CF0FB1,0x561F,0x11cf,{0x84,0xAA,0x7E,0xEF,0xF0,0x4D,0x00,0x01}};


const CLSID CLSID_X509 = {0xDEA047A9,0x505D,0x11cf,{0xB1,0xCA,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const CLSID CLSID_X500_Name = {0x3FD911D1,0x5872,0x11cf,{0x84,0xAD,0x72,0x4F,0xB4,0x46,0x00,0x01}};


const CLSID CLSID_CABSigner = {0xB243CC90,0x9C75,0x11cf,{0xAD,0x61,0xC6,0x30,0x5D,0x06,0x00,0x02}};


const CLSID CLSID_SystemCertificateStore = {0x1F37F761,0xA467,0x11cf,{0x8A,0x57,0x00,0xAA,0x00,0x6C,0x37,0x06}};


const CLSID CLSID_MSDefKeyPair = {0xA17635F0,0xC67D,0x11cf,{0x89,0xD5,0x00,0xAA,0x00,0x60,0xFA,0x2B}};


const IID IID_IPublicKeyPair = {0x110C5791,0xC935,0x11cf,{0x8A,0x6F,0x00,0xAA,0x00,0x6C,0x37,0x06}};


#ifdef __cplusplus
}
#endif

