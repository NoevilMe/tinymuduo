#include "proto_codec.h"

#include <zlib.h>

namespace muduo {
namespace event_loop {

#ifdef __WIN32
inline u_short networkToHost16(u_short netshort) { return ntohs(netshort); }

inline u_long networkToHost32(u_long netlong) { return ntohl(netlong); }

inline u_short hostToNetwork16(u_short netshort) { return htons(netshort); }

inline u_long hostToNetwork32(u_long netlong) { return htonl(netlong); }

inline unsigned __int64 networkToHost64(unsigned __int64 value) {
    return ntohll(value);
}

inline unsigned __int64 hostToNetwork64(unsigned __int64 value) {
    return htonll(value);
}
#else
// refer to https://linux.die.net/man/3/be16toh
inline uint16_t networkToHost16(uint16_t net16) { return be16toh(net16); }

inline uint32_t networkToHost32(uint32_t net32) { return be32toh(net32); }

inline uint16_t hostToNetwork16(uint16_t host16) { return htobe16(host16); }

inline uint32_t hostToNetwork32(uint32_t host32) { return htobe32(host32); }

inline uint64_t hostToNetwork64(uint64_t host64) { return htobe64(host64); }

inline uint64_t networkToHost64(uint64_t net64) { return be64toh(net64); }
#endif

ProtobufCodec::ProtobufCodec(const ProtobufMessageCallback &cb)
    : message_cb_(cb), error_cb_(DefaultErrorCallback) {}

ProtobufCodec::ProtobufCodec(const ProtobufMessageCallback &cb,
                             const ErrorCallback &ecb)
    : message_cb_(cb), error_cb_(ecb) {}

ProtobufCodec::~ProtobufCodec() {}

void ProtobufCodec::OnCachedMessage(const char *data, int len,
                                    Timestamp timestamp) {

    left_data_.append(data, len);
    ProcessMessage(timestamp);
}

void ProtobufCodec::OnCachedMessage(const std::string &data,
                                    Timestamp timestamp) {
    left_data_.append(data);
    ProcessMessage(timestamp);
}

void ProtobufCodec::ProcessMessage(Timestamp timestamp) {
    while (left_data_.length() >= kMinMessageBodyLen + kHeaderLen) {
        ProtoMsgHeaderType len = NetworkAsUint32(left_data_.data());

        if (len > kMaxMessageBodyLen || len < kMinMessageBodyLen) {
            error_cb_(left_data_, timestamp, kInvalidLength);
            break;
        } else if (left_data_.length() >=
                   static_cast<size_t>(len + kHeaderLen)) {
            ErrorCode errorCode = kNoError;
            MessagePtr message =
                ParseBody(left_data_.data() + kHeaderLen, len, &errorCode);

            if (errorCode == kNoError && message) {
                message_cb_(message, timestamp);
                left_data_ = left_data_.substr(len + kHeaderLen);
            } else {
                error_cb_(left_data_, timestamp, errorCode);
                break;
            }
        } else {
            break;
        }
    }
}

void ProtobufCodec::OnMessage(const char *data, int len, Timestamp timestamp) {

    const char *pos = data;
    const char *end = data + len;
    while (pos < data + len) {
        uint32_t body_len = NetworkAsUint32(pos);
        pos += sizeof(body_len);

        if (body_len > kMaxMessageBodyLen || len < kMinMessageBodyLen) {
            error_cb_(std::string(pos, end - pos), timestamp, kInvalidLength);
            break;
        } else if ((end - pos) >= body_len) {
            ErrorCode errorCode = kNoError;
            MessagePtr message = ParseBody(pos, body_len, &errorCode);

            if (errorCode == kNoError && message) {
                message_cb_(message, timestamp);
                pos += body_len;
            } else {
                error_cb_(std::string(pos, end - pos), timestamp, errorCode);
                break;
            }
        } else {
            error_cb_(std::string(pos, end - pos), timestamp, kInvalidLength);
            break;
        }
    }
}

std::string ProtobufCodec::ErrorCodeToString(ErrorCode code) {
    switch (code) {
    case ProtobufCodec::kNoError:
        return "NoError";
    case ProtobufCodec::kInvalidLength:
        return "InvalidLength";
    case ProtobufCodec::kInvalidNameLen:
        return "InvalidNameLen";
    case ProtobufCodec::kUnknownMessageType:
        return "UnknownMessageType";
    case ProtobufCodec::kParseError:
        return "ParseError";
    case ProtobufCodec::kCheckSumError:
        return "CheckSumError";
    default:
        return "UnknownError";
    }
}

void ProtobufCodec::PackMessage(const google::protobuf::Message &msg,
                                std::string &bytes) {
    auto type_name = msg.GetTypeName();
    ProtoMsgNameLengthType type_name_len =
        (ProtoMsgNameLengthType)type_name.length();
    auto proto_bytes = msg.SerializeAsString();

    //// total len
    // ProtoMsgLengthType len = sizeof(ProtoMsgNameLengthType) + typeNameLen +
    // (ProtoMsgLengthType)proto_bytes.length();
    ////// msg type len
    ////bytes.append((const char *)&type_name_len, sizeof(type_name_len));
    ////// msg type
    ////bytes.append(type_name);
    ////// protobuf bytes
    ////bytes.append(proto_bytes);

    std::string body;
    // msg type length
    body.append((const char *)&type_name_len, sizeof(type_name_len));
    // msg type
    body.append(type_name);
    // protobuf bytes
    body.append(proto_bytes);

    uint32_t check_sum = ::adler32(
        1, reinterpret_cast<const Bytef *>(body.data()), body.length());
    AppendUint32(check_sum, body);

    ProtoMsgHeaderType body_len = body.length();
    AppendUint32(body_len, bytes);
    bytes.append(body);
}

MessagePtr ProtobufCodec::ParseBody(const char *buf, int len,
                                    ErrorCode *error) {
    MessagePtr message;

    const char *pos = buf;
    uint32_t expectedCheckSum = NetworkAsUint32(pos + len - kCheckSumLen);

    uint32_t checkSum =
        ::adler32(1, reinterpret_cast<const Bytef *>(pos), len - kCheckSumLen);
    if (checkSum == expectedCheckSum) {

        ProtoMsgNameLengthType nameLen = NetworkAsUint8(buf);
        pos += sizeof(ProtoMsgNameLengthType);

        if (nameLen >= kMinMsgNameLen &&
            nameLen <= len - sizeof(ProtoMsgNameLengthType) - kCheckSumLen) {
            std::string typeName(pos, nameLen);
            pos += nameLen;

            message.reset(CreateMessage(typeName));
            if (message) {
                auto size = len - sizeof(ProtoMsgNameLengthType) - nameLen -
                            kCheckSumLen;
                if (message->ParseFromArray(pos, (int)size)) {
                    *error = kNoError;
                } else {
                    *error = kParseError;
                }
            } else {
                *error = kUnknownMessageType;
            }
        } else {
            *error = kInvalidNameLen;
        }
    } else {
        *error = kCheckSumError;
    }

    return message;
}

uint32_t ProtobufCodec::NetworkAsUint32(const char *buf) {
    uint32_t be32 = 0;
#ifdef _WIN32
    ::memcpy_s(&be32, sizeof(be32), buf, sizeof(be32));
#else
    ::memcpy(&be32, buf, sizeof(be32));
#endif
    return networkToHost32(be32);
}

uint8_t ProtobufCodec::NetworkAsUint8(const char *buf) {
    uint8_t value = 0;
    memcpy(&value, buf, sizeof(value));
    return value;
}

void ProtobufCodec::AppendUint32(uint32_t value, std::string &data) {
    uint32_t be32 = hostToNetwork32(value);
    data.append((const char *)(&be32), sizeof(be32));
}

void ProtobufCodec::AppendUshort(unsigned short value, std::string &data) {
    unsigned short be16 = hostToNetwork16(value);
    data.append((const char *)(&be16), sizeof(be16));
}

google::protobuf::Message *
ProtobufCodec::CreateMessage(const std::string &typeName) {
    google::protobuf::Message *message = nullptr;

    const google::protobuf::Descriptor *descriptor =
        google::protobuf::DescriptorPool::generated_pool()
            ->FindMessageTypeByName(typeName);
    if (descriptor) {
        auto prototype =
            google::protobuf::MessageFactory::generated_factory()->GetPrototype(
                descriptor);
        if (prototype) {
            message = prototype->New();
        }
    }

    return message;
}

void ProtobufCodec::DefaultErrorCallback(const std::string &, Timestamp,
                                         ErrorCode) {
    throw std::string("Not implemented");
}

ProtobufDispatcher::ProtobufDispatcher(const ProtobufMessageCallback &defaultCb)
    : default_callback_(defaultCb) {}

ProtobufDispatcher::~ProtobufDispatcher() {}

} // namespace event_loop
} // namespace muduo
