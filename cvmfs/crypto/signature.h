/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_CRYPTO_SIGNATURE_H_
#define CVMFS_CRYPTO_SIGNATURE_H_

#include <pthread.h>

#include <openssl/bio.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "crypto/hash.h"
#include "util/export.h"

namespace signature {

class CVMFS_EXPORT SignatureManager {
 public:
  enum ESignMethod { kSignManifest, kSignWhitelist };

  SignatureManager();

  void Init();
  void Fini();
  std::string GetCryptoError();

  void UnloadPrivateKey();
  void UnloadPublicRsaKeys();
  void UnloadPrivateMasterKey();
  void UnloadCertificate();

  bool LoadPrivateMasterKeyPath(const std::string &file_pem);
  bool LoadPrivateKeyPath(const std::string &file_pem,
                          const std::string &password);
  bool LoadPrivateMasterKeyMem(const std::string &key);
  bool LoadPrivateKeyMem(const std::string &key);
  bool LoadCertificatePath(const std::string &file_pem);
  bool LoadCertificateMem(const unsigned char *buffer,
                          const unsigned buffer_size);
  bool WriteCertificateMem(unsigned char **buffer, unsigned *buffer_size);
  bool KeysMatch();
  bool VerifyCaChain();
  std::string Whois();
  shash::Any HashCertificate(const shash::Algorithms hash_algorithm);
  std::string FingerprintCertificate(const shash::Algorithms hash_algorithm);
  static shash::Any MkFromFingerprint(const std::string &fingerprint);

  bool LoadPublicRsaKeys(const std::string &path_list);
  bool LoadBlacklist(const std::string &path_blacklist, bool append);
  std::vector<std::string> GetBlacklist();

  bool LoadTrustedCaCrl(const std::string &path_list);

  bool Sign(const unsigned char *buffer, const unsigned buffer_size,
            unsigned char **signature, unsigned *signature_size);
  bool SignRsa(const unsigned char *buffer, const unsigned buffer_size,
                unsigned char **signature, unsigned *signature_size);
  bool Verify(const unsigned char *buffer, const unsigned buffer_size,
              const unsigned char *signature, unsigned signature_size);
  bool VerifyRsa(const unsigned char *buffer, const unsigned buffer_size,
                 const unsigned char *signature, unsigned signature_size);
  bool VerifyLetter(const unsigned char *buffer, const unsigned buffer_size,
                    const bool by_rsa);
  bool VerifyPkcs7(const unsigned char *buffer, const unsigned buffer_size,
                   unsigned char **content, unsigned *content_size,
                   std::vector<std::string> *alt_uris);
  static void CutLetter(const unsigned char *buffer,
                        const unsigned buffer_size,
                        const char separator,
                        unsigned *letter_length,
                        unsigned *pos_after_mark);

  // Returns the PEM-encoded text of all loaded RSA pubkeys
  std::string GetActivePubkeys() const;
  // The PEM-encoded private key matching the public master key
  std::string GetPrivateMasterKey();
  // The PEM-encoded certificate without private key
  std::string GetCertificate() const;
  // The PEM-encoded private key matching the certificate
  std::string GetPrivateKey();

  void GenerateMasterKeyPair();
  void GenerateCertificate(const std::string &cn);

 private:
  RSA *GenerateRsaKeyPair();
  std::string GenerateKeyText(RSA *pubkey) const;

  void InitX509Store();

  EVP_PKEY *private_key_;
  RSA *private_master_key_;
  X509 *certificate_;
  std::vector<RSA *> public_keys_;  /**< Contains cvmfs public master keys */
  pthread_mutex_t lock_blacklist_;
  std::vector<std::string> blacklist_;
  X509_STORE *x509_store_;
  X509_LOOKUP *x509_lookup_;
};  // class SignatureManager

}  // namespace signature

#endif  // CVMFS_CRYPTO_SIGNATURE_H_
