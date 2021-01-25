#ifndef __COAP_MESSAGE_H__
#define __COAP_MESSAGE_H__

#include <cstdint>
#include <QByteArray>
#include <QList>
#include <QObject>

class CoapOption : public QObject {
public:
    /// CoAP option numbers.
    enum class Option : uint8_t {
        COAP_OPTION_INVALID=0,
        COAP_OPTION_IF_MATCH=1,
        COAP_OPTION_URI_HOST=3,
        COAP_OPTION_ETAG,
        COAP_OPTION_IF_NONE_MATCH,
        COAP_OPTION_OBSERVE,
        COAP_OPTION_URI_PORT,
        COAP_OPTION_LOCATION_PATH,
        COAP_OPTION_URI_PATH=11,
        COAP_OPTION_CONTENT_FORMAT,
        COAP_OPTION_MAX_AGE=14,
        COAP_OPTION_URI_QUERY,
        COAP_OPTION_ACCEPT=17,
        COAP_OPTION_LOCATION_QUERY=20,
        COAP_OPTION_BLOCK2=23,
        COAP_OPTION_BLOCK1=27,
        COAP_OPTION_SIZE2,
        COAP_OPTION_PROXY_URI=35,
        COAP_OPTION_PROXY_SCHEME=39,
        COAP_OPTION_SIZE1=60
    };

    /// CoAP content-formats.
    enum class ContentFormat : uint16_t {
        /* https://www.iana.org/assignments/core-parameters/core-parameters.xhtml#content-formats */
        /* 0-255  Expert Review */
        COAP_CONTENT_FORMAT_TEXT_PLAIN           = 0    ,  //  text/plain; charset=utf-8                    /* Ref: [RFC2046][RFC3676][RFC5147] */
        COAP_CONTENT_FORMAT_APP_COSE_ENCRYPT0    = 16   ,  //  application/cose; cose-type="cose-encrypt0"  /* Ref: [RFC8152] */
        COAP_CONTENT_FORMAT_APP_COSE_MAC0        = 17   ,  //  application/cose; cose-type="cose-mac0"      /* Ref: [RFC8152] */
        COAP_CONTENT_FORMAT_APP_COSE_SIGN1       = 18   ,  //  application/cose; cose-type="cose-sign1"     /* Ref: [RFC8152] */
        COAP_CONTENT_FORMAT_APP_LINKFORMAT       = 40   ,  //  application/link-format                      /* Ref: [RFC6690] */
        COAP_CONTENT_FORMAT_APP_XML              = 41   ,  //  application/xml                              /* Ref: [RFC3023] */
        COAP_CONTENT_FORMAT_APP_OCTECT_STREAM    = 42   ,  //  application/octet-stream                     /* Ref: [RFC2045][RFC2046] */
        COAP_CONTENT_FORMAT_APP_EXI              = 47   ,  //  application/exi                              /* Ref: ["Efficient XML Interchange (EXI) Format 1.0 (Second Edition)" ,February 2014] */
        COAP_CONTENT_FORMAT_APP_JSON             = 50   ,  //  application/json                             /* Ref: [RFC4627] */
        COAP_CONTENT_FORMAT_APP_JSON_PATCH_JSON  = 51   ,  //  application/json-patch+json                  /* Ref: [RFC6902] */
        COAP_CONTENT_FORMAT_APP_MERGE_PATCH_JSON = 52   ,  //  application/merge-patch+json                 /* Ref: [RFC7396] */
        COAP_CONTENT_FORMAT_APP_CBOR             = 60   ,  //  application/cbor                             /* Ref: [RFC7049] */
        COAP_CONTENT_FORMAT_APP_CWT              = 61   ,  //  application/cwt                              /* Ref: [RFC8392] */
        COAP_CONTENT_FORMAT_APP_COSE_ENCRYPT     = 96   ,  //  application/cose; cose-type="cose-encrypt"   /* Ref: [RFC8152] */
        COAP_CONTENT_FORMAT_APP_COSE_MAC         = 97   ,  //  application/cose; cose-type="cose-mac"       /* Ref: [RFC8152] */
        COAP_CONTENT_FORMAT_APP_COSE_SIGN        = 98   ,  //  application/cose; cose-type="cose-sign"      /* Ref: [RFC8152] */
        COAP_CONTENT_FORMAT_APP_COSE_KEY         = 101  ,  //  application/cose-key                         /* Ref: [RFC8152] */
        COAP_CONTENT_FORMAT_APP_COSE_KEY_SET     = 102  ,  //  application/cose-key-set                     /* Ref: [RFC8152] */
        COAP_CONTENT_FORMAT_APP_COAP_GROUP_JSON  = 256  ,  //  application/coap-group+json                  /* Ref: [RFC7390] */
        /* 256-9999  IETF Review or IESG Approval */
        COAP_CONTENT_FORMAT_APP_OMA_TLV_OLD      = 1542 ,  //  Keep old value for backward-compatibility    /* Ref: [OMA-TS-LightweightM2M-V1_0] */
        COAP_CONTENT_FORMAT_APP_OMA_JSON_OLD     = 1543 ,  //  Keep old value for backward-compatibility    /* Ref: [OMA-TS-LightweightM2M-V1_0] */
        /* 10000-64999  First Come First Served */
        COAP_CONTENT_FORMAT_APP_VND_OCF_CBOR     = 10000,  //  application/vnd.ocf+cbor                     /* Ref: [Michael_Koster] */
        COAP_CONTENT_FORMAT_APP_OMA_TLV          = 11542,  //  application/vnd.oma.lwm2m+tlv                /* Ref: [OMA-TS-LightweightM2M-V1_0] */
        COAP_CONTENT_FORMAT_APP_OMA_JSON         = 11543   //  application/vnd.oma.lwm2m+json               /* Ref: [OMA-TS-LightweightM2M-V1_0] */
        /* 65000-65535  Experimental use (no operational use) */
    };

    explicit CoapOption(Option name, QObject *parent=nullptr);
    virtual ~CoapOption();
    void setName(Option name);
    void setValue(QByteArray &value);
    Option name() const;
    QByteArray& value();

protected:
    friend class CoapMessage;
    Option name_;
    QByteArray value_;

private:
    Q_DISABLE_COPY(CoapOption);
};

class CoapMessage : public QObject {

public:
    /// CoAP message types. Note, values only work as enum.
    enum class Type : uint8_t {
        COAP_TYPE_CONFIRMABLE=0,    /* Request */
        COAP_TYPE_NON_CONFIRMABLE,  /* Request */
        COAP_TYPE_ACKNOWLEDGEMENT,  /* Response */
        COAP_TYPE_RESET             /* Response */
    };

    /// CoAP response codes.
    enum Code {
        COAP_CODE_EMPTY=0x00,
        COAP_CODE_GET,
        COAP_CODE_POST,
        COAP_CODE_PUT,
        COAP_CODE_DELETE,
        COAP_CODE_LASTMETHOD=0x1F,
        COAP_CODE_CREATED=0x41,
        COAP_CODE_DELETED,
        COAP_CODE_VALID,
        COAP_CODE_CHANGED,
        COAP_CODE_CONTENT,
        COAP_CODE_BAD_REQUEST=0x80,
        COAP_CODE_UNAUTHORIZED,
        COAP_CODE_BAD_OPTION,
        COAP_CODE_FORBIDDEN,
        COAP_CODE_NOT_FOUND,
        COAP_CODE_METHOD_NOT_ALLOWED,
        COAP_CODE_NOT_ACCEPTABLE,
        COAP_CODE_PRECONDITION_FAILED=0x8C,
        COAP_CODE_REQUEST_ENTITY_TOO_LARGE=0x8D,
        COAP_CODE_UNSUPPORTED_CONTENT_FORMAT=0x8F,
        COAP_CODE_INTERNAL_SERVER_ERROR=0xA0,
        COAP_CODE_NOT_IMPLEMENTED,
        COAP_CODE_BAD_GATEWAY,
        COAP_CODE_SERVICE_UNAVAILABLE,
        COAP_CODE_GATEWAY_TIMEOUT,
        COAP_CODE_PROXYING_NOT_SUPPORTED,
        COAP_CODE_UNDEFINED_CODE=0xFF
    };

    explicit CoapMessage(Type type, Code code, QObject *parent=nullptr);
    virtual ~CoapMessage();
    void setVersion(uint8_t version);
    void setType(Type type);
    void setCode(Code code);
    void setMessageId(uint16_t messageId);
    bool setToken(QByteArray &token);
    void addOption(CoapOption *option);
    void setPayload(QByteArray &payload);
    uint8_t version();
    Type type();
    Code code();
    uint16_t messageId();
    QByteArray& token();
    QList<CoapOption*>& options();
    QByteArray& payload();
    bool toByteArray(QByteArray &target);
    static CoapMessage* createFromByteArray(const QByteArray &from, QObject *parent=nullptr);

protected:
    uint8_t version_;
    Type type_;
    Code code_;
    uint16_t messageId_;
    QByteArray token_;
    QList<CoapOption*> options_;
    QByteArray payload_;

private:
    Q_DISABLE_COPY(CoapMessage);

};

#endif // __COAP_MESSAGE_H__