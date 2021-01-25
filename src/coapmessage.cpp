#include "coapmessage.hpp"
#include <algorithm>
#include <cstdint>

CoapOption::CoapOption(Option name, QObject *parent) : QObject(parent)
{
    this->name_ = name;
}

CoapOption::~CoapOption()
{

}

void CoapOption::setName(CoapOption::Option name)
{
    name_ = name;
}

void CoapOption::setValue(QByteArray &value)
{
    value_ = value;
}

CoapOption::Option CoapOption::name() const
{
    return name_;
}

QByteArray& CoapOption::value()
{
    return value_;
}

CoapMessage::CoapMessage(Type type, Code code, QObject *parent) : QObject(parent)
{
    version_ = 1;
    type_ = type;
    code_ = code;
    messageId_ = 0;
}

CoapMessage::~CoapMessage()
{
    foreach (CoapOption *option, options_) {
        delete option;
        option = nullptr;
    }
    options_.clear();
}

void CoapMessage::setVersion(uint8_t version)
{
    version_ = version;
}

void CoapMessage::setType(CoapMessage::Type type)
{
    type_ = type;
}

void CoapMessage::setCode(CoapMessage::Code code)
{
    code_ = code;
}

void CoapMessage::setMessageId(uint16_t messageId)
{
    messageId_ = messageId;
}

bool CoapMessage::setToken(QByteArray &token)
{
    if (token.length() > 8) {
        return false;
    }
    token_ = token;
    return true;
}

void CoapMessage::addOption(CoapOption *option)
{
    Q_ASSERT(option != nullptr);
    options_.append(option);
}

void CoapMessage::setPayload(QByteArray &payload)
{
    payload_ = payload;
}

uint8_t CoapMessage::version()
{
    return version_;
}

CoapMessage::Type CoapMessage::type()
{
    return type_;
}

CoapMessage::Code CoapMessage::code()
{
    return code_;
}

uint16_t CoapMessage::messageId()
{
    return messageId_;
}

QByteArray& CoapMessage::token()
{
    return token_;
}

QList<CoapOption*>& CoapMessage::options()
{
    return options_;
}

bool sortCoapOption (CoapOption* a, CoapOption* b) 
{ 
    return a->name() < b->name();
}

QByteArray& CoapMessage::payload()
{
    return payload_;
}

//! 0                   1                   2                   3
//! 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |Ver| T |  TKL  |      Code     |          Message ID           |
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |   Token (if any, TKL bytes) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |   Options (if any) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |1 1 1 1 1 1 1 1|    Payload (if any) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool CoapMessage::toByteArray(QByteArray &target)
{
    target.append(version_ << 6 | 
                static_cast<uint8_t>(type_) << 4 | 
                static_cast<uint8_t>(token_.length() & 0x0F));
    target.append(static_cast<uint8_t>(code_));
    target.append(static_cast<uint8_t>((messageId_ >> 8) & 0xFF));
    target.append(static_cast<uint8_t>(messageId_ & 0xFF));
    target.append(token_);
    std::sort(options_.begin(), options_.end(), sortCoapOption);

    CoapOption::Option last_option = CoapOption::Option::COAP_OPTION_INVALID;
    foreach (CoapOption *option, options_) {

        uint16_t optionDelta = static_cast<uint16_t>(option->name()) - static_cast<uint16_t>(last_option);
        bool isOptionDeltaExtended = false;
        uint8_t optionDeltaExtended = 0;

        // Delta value > 12 : special values
        if (optionDelta > 268) {
            optionDeltaExtended = static_cast<uint8_t>(optionDelta - 269);
            optionDelta = 14;
            isOptionDeltaExtended = true;
        } else if (optionDelta > 12) {
            optionDeltaExtended = static_cast<uint8_t>(optionDelta - 13);
            optionDelta = 13;
            isOptionDeltaExtended = true;
        }
        
        uint16_t optionLength = static_cast<uint16_t>(option->value().length());
        bool isOptionLengthExtended = false;
        uint8_t optionLengthExtended = 0;

        // Length > 12 : special values
        if (optionLength > 268) {
            optionLengthExtended = static_cast<uint8_t>(optionLength - 269);
            optionLength = 14;
            isOptionLengthExtended = true;
        } else if (optionLength > 12) {
            optionLengthExtended = static_cast<uint8_t>(optionLength - 13);
            optionLength = 13;
            isOptionLengthExtended = true;
        }

        target.append((optionDelta << 4) | (optionLength & 0x0F));

        if (isOptionDeltaExtended) {
            target.append(optionDeltaExtended);
        }

        if (isOptionLengthExtended) {
            target.append(optionLengthExtended);
        }

        target.append(option->value());
        last_option = option->name();
    }

    if (!payload_.isEmpty()) {
        target.append(0xFF);
        target.append(payload_);
    }

    return true;
}

//! 0                   1                   2                   3
//! 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |Ver| T |  TKL  |      Code     |          Message ID           |
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |   Token (if any, TKL bytes) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |   Options (if any) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |1 1 1 1 1 1 1 1|    Payload (if any) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
CoapMessage* CoapMessage::createFromByteArray(const QByteArray &from, QObject *parent)
{
    if (from.length() < 4) {
        return nullptr;
    }

    const uint8_t *data = reinterpret_cast<const uint8_t *>(from.data());

    uint8_t version = (data[0] >> 6) & 0x03;
    Type type = static_cast<Type>((data[0] >> 4) & 0x03);
    uint8_t tokenLen = data[0] & 0x0F;
    Code code = static_cast<Code>(data[1]);
    uint16_t messageId = static_cast<uint16_t>(data[2]) << 8 | static_cast<uint16_t>(data[3]);

    if (from.length() < 4 + tokenLen) {
        return nullptr;
    }

    CoapMessage *message = new CoapMessage(type, code, parent);
    message->setVersion(version);
    QByteArray token = from.mid(4, tokenLen);
    message->setToken(token);

    uint16_t idx = 4 + tokenLen;
    uint16_t lastOptionNumber = 0;
    while (idx != from.length() && data[idx] != 0xFF) {
        uint16_t optionDelta = ((data[idx] >> 4) & 0x0F);
        uint16_t optionLength = (data[idx] & 0x0F);

        // Delta value > 12 : special values
        if (optionDelta == 13) {
            idx++;
            optionDelta = data[idx] + 13;
        } else if (optionDelta == 14) {
            idx++;
            optionDelta = data[idx] + 269;
        }

        // Delta length > 12 : special values
        if (optionLength == 13) {
            idx++;
            optionLength = data[idx] + 13;
        } else if (optionLength == 14) {
            idx++;
            optionLength = data[idx] + 269;
        }

        uint16_t optionNumber = lastOptionNumber + optionDelta;
        lastOptionNumber = optionNumber;

        CoapOption *option = new CoapOption(static_cast<CoapOption::Option>(optionNumber), message);
        QByteArray value = from.mid(idx + 1, optionLength);
        option->setValue(value);
        message->addOption(option);

        idx += 1 + optionLength;
    }

    if (idx < from.length() && data[idx] == 0xFF) {
        QByteArray payload = from.mid(idx + 1);
        message->setPayload(payload);
    } else {
        delete message;
        return nullptr;
    }

    return message;
}
