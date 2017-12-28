#pragma once

#include <algorithm>
#include <map>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <cstring>

#include <DataElement.hpp>
#include <Utils.hpp>

namespace isolib
{
  enum class BitmapType : char { Binary, Hex };

  template <typename DataElementFactory>
  class IsoMessage
  {
    public:
    IsoMessage(const std::string& messageType, BitmapType binBitmap = BitmapType::Hex) :
      _bitmapType(binBitmap)
    {
      validateMessageType(messageType);
      strncpy(_messageType, messageType.data(), 4);
    }

    IsoMessage(const IsoMessage& other) = delete;
    IsoMessage(IsoMessage&& other) = default;

    IsoMessage& operator=(const IsoMessage& other) = delete;
    IsoMessage& operator=(IsoMessage&& other) = default;

    void setField(size_t pos, std::unique_ptr<DataElementBase>&& de)
    {
      if (pos < 2 || pos > 128)
        throw std::invalid_argument("Field index must be between 2 and 128");

      if (pos > 64 && !_bitmaps[1])
        _bitmaps[0] = set(1, _bitmaps[0]);

      auto normalizedPos = pos > 64 ? pos - 64 : pos;
      _fields[pos] = std::move(de);
      _bitmaps[pos > 64] = set(normalizedPos, _bitmaps[pos > 64]);
    }

    std::string getField(size_t pos) const
    {
      if (pos < 2 || pos > 128)
      {
        throw std::invalid_argument("Field index must be between 2 and 128");
      }

      const auto it = _fields.find(pos);
      if (it == std::end(_fields))
        throw std::invalid_argument("Field is not present " + std::to_string(pos));

      return it->second->toString();
    }

    std::string write() const
    {
      std::ostringstream oss;
      auto writeBitmap = [&, this](int pos) -> void {
        if (_bitmapType == BitmapType::Binary)
          oss << toBinary(_bitmaps[pos]);
        else
          oss << toHex(_bitmaps[pos]);
      };

      oss.write(_messageType, 4);
      writeBitmap(0);
      if (_bitmaps[1])
        writeBitmap(1);
      for (const auto& kv : _fields)
      {
        oss << kv.second->toString();
      }

      return oss.str();
    }

    // Right now it only works with at most 2 bitmaps
    void read(const std::string& in)
    {
      std::istringstream iss{in};
      auto readBitmap = [&, this](size_t bitmapNum) -> void {
        if (_bitmapType == BitmapType::Binary)
          _bitmaps[bitmapNum] = fromBinary<uint64_t>(readFixedField(iss, 8));
        else
          _bitmaps[bitmapNum] = fromHex<uint64_t>(readFixedField(iss, 16));
      };

      auto createFromBitmap = [&, this](size_t bitmapNum) -> void {
        auto offset = bitmapNum ? 64 : 0;
        size_t i = bitmapNum ? 1 : 2;

        for (; i <= 64; i++)
        {
          if (!get(i, _bitmaps[bitmapNum]))
            continue;
          auto debPtr = DataElementFactory::create("DE" + std::to_string(i + offset));
          debPtr->parse(iss);
          _fields[i + offset] = std::move(debPtr);
        }
      };

      strncpy(_messageType, readFixedField(iss, 4).data(), 4);
      readBitmap(0);
      if (get(1, _bitmaps[0]))
        readBitmap(1);
      createFromBitmap(0);
      if (get(1, _bitmaps[0]))
        createFromBitmap(1);
    }

    private:
    char _messageType[4];
    BitmapType _bitmapType;
    std::array<uint64_t, 2> _bitmaps{{0, 0}};
    std::map<int, std::unique_ptr<DataElementBase>> _fields;
  };
}
