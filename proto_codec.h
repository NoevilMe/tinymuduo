#ifndef __PROTO_CODEC_H_
#define __PROTO_CODEC_H_

#include "timestamp.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <google/protobuf/message.h>

//|----header----|---------------------------body-----------------|--checksum--|
//|----header----|--name len--|----name----|-------msg bytes------|--checksum--|
//|---int32_t----|--uint8_t-_-|----char----|---------char---------|--checksum--|
//
// struct ProtobufTransportFormat __attribute__ ((__packed__))
// {
//   int32_t  len;
//   int8_t   nameLen;
//   char     typeName[nameLen];
//   char     protobufData[len-nameLen-8];
//   int32_t  checkSum; // adler32 of nameLen, typeName and protobufData
// }

namespace muduo {
namespace event_loop {

using MessagePtr = std::shared_ptr<google::protobuf::Message>;
using ProtoMsgLengthType = int32_t;
using ProtoMsgNameLengthType = uint8_t;
using ProtoMsgHeaderType = ProtoMsgLengthType;

constexpr auto kMinMsgNameLen = 1;

class ProtobufCodec {
public:
    enum ErrorCode {
        kNoError = 0,
        kCheckSumError,
        kInvalidLength,
        kInvalidNameLen,
        kUnknownMessageType,
        kParseError
    };

    typedef std::function<void(const MessagePtr &, Timestamp)>
        ProtobufMessageCallback;
    typedef std::function<void(const std::string &, Timestamp, ErrorCode)>
        ErrorCallback;

    explicit ProtobufCodec(const ProtobufMessageCallback &cb);
    ProtobufCodec(const ProtobufMessageCallback &cb, const ErrorCallback &ecb);
    ~ProtobufCodec();

    void OnCachedMessage(const std::string &data, Timestamp timestamp);
    void OnCachedMessage(const char *data, int len, Timestamp timestamp);
    void OnMessage(const char *data, int len, Timestamp timestamp);

    static std::string ErrorCodeToString(ErrorCode code);

    void PackMessage(const google::protobuf::Message &msg, std::string &bytes);

private:
    void ProcessMessage(Timestamp timestamp);

    // parse msg outer body
    MessagePtr ParseBody(const char *buf, int len, ErrorCode *error);

    uint32_t NetworkAsUint32(const char *buf);
    uint8_t NetworkAsUint8(const char *buf);

    void AppendUint32(uint32_t value, std::string &data);
    void AppendUshort(unsigned short value, std::string &data);

    google::protobuf::Message *CreateMessage(const std::string &typeName);
    static void DefaultErrorCallback(const std::string &data,
                                     Timestamp timestamp, ErrorCode code);

    std::string left_data_;
    ProtobufMessageCallback message_cb_;
    ErrorCallback error_cb_;

    const static int kHeaderLen =
        sizeof(ProtoMsgHeaderType); // 4 bytes, message length
    const static int kMinMessageBodyLen =
        sizeof(ProtoMsgNameLengthType) +
        kMinMsgNameLen; // name len + typename(>=1)
    const static int kMaxMessageBodyLen =
        64 * 1024 * 1024; // limited in protobuf source code
    const static int kCheckSumLen = sizeof(int32_t);
};

/*
 *
 *  Dispatcher
 *
 */

class Callback {
public:
    virtual ~Callback(){};
    virtual void OnMessage(const MessagePtr &message, Timestamp timestamp) = 0;
};

template <typename T> class CallbackT : public Callback {
public:
    using ProtobufMessageTCallback = std::function<void(
        const std::shared_ptr<T> &message, Timestamp timestamp)>;

    CallbackT(const ProtobufMessageTCallback &callback) : callback_(callback) {}

    virtual void OnMessage(const MessagePtr &message,
                           long long timestamp) override {
        std::shared_ptr<T> concrete = std::static_pointer_cast<T>(message);
        callback_(concrete, timestamp);
    }

private:
    ProtobufMessageTCallback callback_;
};

class ProtobufDispatcher {
public:
    using ProtobufMessageCallback =
        std::function<void(const MessagePtr &msg, Timestamp timestamp)>;

    explicit ProtobufDispatcher(const ProtobufMessageCallback &defaultCb);
    ~ProtobufDispatcher();

    void OnProtobufMesssage(const MessagePtr &message, Timestamp timestamp) {
        auto it = callbacks_.find(message->GetDescriptor());

        if (it != callbacks_.end()) {
            it->second->OnMessage(message, timestamp);
        } else {
            default_callback_(message, timestamp);
        }
    }

    template <typename T>
    void registerMessageCallback(
        const typename CallbackT<T>::ProtobufMessageTCallback &callback) {
        std::shared_ptr<CallbackT<T>> pd(new CallbackT<T>(callback));
        callbacks_[T::descriptor()] = pd;
    }

private:
    typedef std::map<const google::protobuf::Descriptor *,
                     std::shared_ptr<Callback>>
        CallbackMap;

    CallbackMap callbacks_;
    ProtobufMessageCallback default_callback_;
};

} // namespace event_loop
} // namespace muduo

#endif
