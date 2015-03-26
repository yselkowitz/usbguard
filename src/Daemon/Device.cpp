#include "Device.hpp"
#include <mutex>
#include <sodium.h>

namespace usbguard {
  Device::Device()
  {
    _seqn = Rule::SeqnDefault;
    _target = Rule::Target::Unknown;
    return;
  }

  std::mutex& Device::refDeviceMutex()
  {
    return _mutex;
  }

  Pointer<Rule> Device::getDeviceRule(bool include_port)
  {
    Pointer<Rule> device_rule = makePointer<Rule>();
    std::unique_lock<std::mutex> device_lock(refDeviceMutex());

    device_rule->setSeqn(_seqn);
    device_rule->setTarget(_target);
    device_rule->setVendorID(_vendor_id);
    device_rule->setProductID(_product_id);
    device_rule->setSerialNumber(_serial_number);
    //if (include_port) {
    // ... TODO ...
    //}
    device_rule->setDeviceName(_name);
    device_rule->setDeviceHash(getDeviceHash(/*include_port=*/false));
    device_rule->setInterfaceTypes(_interface_types);
    
    return std::move(device_rule);
  }

  uint32_t Device::getSeqn() const
  {
    return _seqn;
  }

  String Device::getDeviceHash(bool include_port) const
  {
    unsigned char hash[crypto_generichash_BYTES_MIN];
    crypto_generichash_state state;
    std::string hash_string;

    if (_vendor_id.empty() || _product_id.empty()) {
      throw std::runtime_error("Cannot compute device hash value. Vendor ID and/or Product ID empty.");
    }

    crypto_generichash_init(&state, NULL, 0, sizeof hash);

    for (auto field : {
	&_name, &_vendor_id, &_product_id, &_serial_number }) {
      /* Update the hash value */
      crypto_generichash_update(&state, (const uint8_t *)field->c_str(), field->size());
    }

    const uint32_t types = _interface_types.size();
    crypto_generichash_update(&state, (const uint8_t *)&types, sizeof types);

    for (auto const& type : _interface_types) {
      crypto_generichash_update(&state, (const uint8_t *)type.c_str(), type.size());
    }

    /* Finalize the hash value */
    crypto_generichash_final(&state, hash, sizeof hash);

    /* Binary => Hex string conversion */
    const size_t hexlen = crypto_generichash_BYTES_MIN * 2 + 1;
    char hexval[hexlen];
    sodium_bin2hex(hexval, hexlen, hash, sizeof hash);

    return String(hexval, hexlen - 1);
  }

  void Device::setSeqn(uint32_t seqn)
  {
    _seqn = seqn;
    return;
  }

  void Device::setTarget(Rule::Target target)
  {
    _target = target;
    return;
  }

  void Device::setDeviceName(const String& name)
  {
    _name = name;
    return;
  }

  void Device::setVendorID(const String& vendor_id)
  {
    _vendor_id = vendor_id;
    return;
  }

  void Device::setProductID(const String& product_id)
  {
    _product_id = product_id;
    return;
  }

  void Device::setSerialNumber(const String& serial_number)
  {
    _serial_number = serial_number;
    return;
  }

  void Device::setInterfaceTypes(const StringVector& types)
  {
    _interface_types = types;
    return;
  }

  StringVector& Device::refInterfaceTypes()
  {
    return _interface_types;
  }

} /* namespace usbguard */
