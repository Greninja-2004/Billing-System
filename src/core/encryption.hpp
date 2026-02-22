#pragma once
// =============================================================================
// encryption.hpp — XOR Cipher + AES-128 CBC Stub
// Used for: Encrypting sensitive fields (card numbers, SSN, passwords)
// =============================================================================
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace billing::core {

// ---------------------------------------------------------------------------
// XOR Cipher — simple reversible encryption for demo
// Complexity: O(n)
// ---------------------------------------------------------------------------
class XORCipher {
public:
  explicit XORCipher(const std::string &key) : key_(key) {
    if (key.empty())
      throw std::invalid_argument("Encryption key must not be empty");
  }

  std::string encrypt(const std::string &plaintext) const {
    std::string result = plaintext;
    for (std::size_t i = 0; i < result.size(); ++i)
      result[i] ^= key_[i % key_.size()];
    return result;
  }

  std::string decrypt(const std::string &ciphertext) const {
    return encrypt(ciphertext); // XOR is self-inverse
  }

  // Hex encode for storage
  static std::string to_hex(const std::string &data) {
    std::ostringstream oss;
    for (unsigned char c : data)
      oss << std::hex << std::setw(2) << std::setfill('0')
          << static_cast<int>(c);
    return oss.str();
  }

  static std::string from_hex(const std::string &hex) {
    std::string result;
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
      result.push_back(
          static_cast<char>(std::stoi(hex.substr(i, 2), nullptr, 16)));
    }
    return result;
  }

  std::string encrypt_hex(const std::string &plaintext) const {
    return to_hex(encrypt(plaintext));
  }

  std::string decrypt_hex(const std::string &hex_cipher) const {
    return decrypt(from_hex(hex_cipher));
  }

private:
  std::string key_;
};

// ---------------------------------------------------------------------------
// Caesar Cipher — for obfuscating non-binary data
// ---------------------------------------------------------------------------
class CaesarCipher {
public:
  explicit CaesarCipher(int shift = 13) : shift_(shift % 26) {}

  std::string encrypt(const std::string &text) const {
    std::string result = text;
    for (char &c : result) {
      if (std::isupper(c))
        c = 'A' + (c - 'A' + shift_ + 26) % 26;
      else if (std::islower(c))
        c = 'a' + (c - 'a' + shift_ + 26) % 26;
    }
    return result;
  }

  std::string decrypt(const std::string &text) const {
    CaesarCipher rev(-shift_);
    return rev.encrypt(text);
  }

private:
  int shift_;
};

// ---------------------------------------------------------------------------
// AES-128 Stub Interface (production replacement point)
// ---------------------------------------------------------------------------
class AES128Stub {
public:
  // In production: replace with OpenSSL EVP_aes_128_cbc or libsodium
  static std::string encrypt(const std::string &plaintext,
                             const std::string & /*key*/) {
    // Stub: just XOR with a fixed demo key
    XORCipher cipher("AES_STUB_KEY_2024");
    return cipher.encrypt_hex(plaintext);
  }

  static std::string decrypt(const std::string &ciphertext,
                             const std::string & /*key*/) {
    XORCipher cipher("AES_STUB_KEY_2024");
    return cipher.decrypt_hex(ciphertext);
  }
};

// ---------------------------------------------------------------------------
// Password hashing stub (for RBAC)
// ---------------------------------------------------------------------------
inline uint32_t simple_hash(const std::string &input) {
  uint32_t hash = 5381;
  for (char c : input)
    hash = ((hash << 5) + hash) + static_cast<uint32_t>(c);
  return hash;
}

// Global cipher singleton
inline XORCipher &get_cipher() {
  static XORCipher cipher("BillingSystemKey2024!");
  return cipher;
}

} // namespace billing::core
