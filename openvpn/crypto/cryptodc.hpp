//    OpenVPN -- An application to securely tunnel IP networks
//               over a single port, with support for SSL/TLS-based
//               session authentication and key exchange,
//               packet encryption, packet authentication, and
//               packet compression.
//
//    Copyright (C) 2013-2014 OpenVPN Technologies, Inc.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License Version 3
//    as published by the Free Software Foundation.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program in the COPYING file.
//    If not, see <http://www.gnu.org/licenses/>.

// Base class for OpenVPN data channel encryption/decryption

#ifndef OPENVPN_CRYPTO_CRYPTODC_H
#define OPENVPN_CRYPTO_CRYPTODC_H

#include <openvpn/buffer/buffer.hpp>
#include <openvpn/error/error.hpp>
#include <openvpn/common/rc.hpp>
#include <openvpn/frame/frame.hpp>
#include <openvpn/crypto/static_key.hpp>
#include <openvpn/crypto/packet_id.hpp>
#include <openvpn/crypto/cryptoalgs.hpp>

namespace openvpn {

  // Base class for encryption/decryption of data channel
  class CryptoDCInstance : public RC<thread_unsafe_refcount>
  {
  public:
    typedef boost::intrusive_ptr<CryptoDCInstance> Ptr;

    // Encrypt/Decrypt

    // returns true if packet ID is close to wrapping
    virtual bool encrypt(BufferAllocated& buf, const PacketID::time_t now) = 0;

    virtual Error::Type decrypt(BufferAllocated& buf, const PacketID::time_t now) = 0;

    // Initialization

    // return value of defined()
    enum {
      CIPHER_DEFINED=(1<<0),
      HMAC_DEFINED=(1<<1)
    };
    virtual unsigned int defined() const = 0;

    virtual void init_cipher(StaticKey&& encrypt_key,
			     StaticKey&& decrypt_key) = 0;

    virtual void init_hmac(StaticKey&& encrypt_key,
			   StaticKey&& decrypt_key) = 0;

    virtual void init_pid(const int send_form,
			  const int recv_mode,
			  const int recv_form,
			  const int recv_seq_backtrack,
			  const int recv_time_backtrack,
			  const char *recv_name,
			  const int recv_unit,
			  const SessionStats::Ptr& recv_stats_arg) = 0;

    // Rekeying

    enum RekeyType {
      ACTIVATE_PRIMARY,
      DEACTIVATE_SECONDARY,
      PROMOTE_SECONDARY_TO_PRIMARY,
      DEACTIVATE_ALL,
    };

    virtual void rekey(const RekeyType type) = 0;
  };

  // Factory for CryptoDCInstance objects
  class CryptoDCContext : public RC<thread_unsafe_refcount>
  {
  public:
    typedef boost::intrusive_ptr<CryptoDCContext> Ptr;

    virtual CryptoDCInstance::Ptr new_obj(const unsigned int key_id) = 0;

    // cipher/HMAC/key info
    struct Info {
      CryptoAlgs::Type cipher_alg;
      CryptoAlgs::Type hmac_alg;
      unsigned int cipher_key_size;
      unsigned int hmac_key_size;
    };
    virtual Info crypto_info() = 0;

    // Info for ProtoContext::link_mtu_adjust
    virtual size_t encap_overhead() const = 0;
  };

  // Factory for CryptoDCContext objects
  class CryptoDCFactory : public RC<thread_unsafe_refcount>
  {
  public:
    typedef boost::intrusive_ptr<CryptoDCFactory> Ptr;

    virtual CryptoDCContext::Ptr new_obj(const CryptoAlgs::Type cipher,
					 const CryptoAlgs::Type digest) = 0;
  };
}

#endif
