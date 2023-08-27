#include <algorithm>
#include <cstdint>
#include <iostream>
#include <optional>
#include <span>
#include <string>

#include "pb_encode.h"
#include "pb_decode.h"
#include "messages.pb.h"

template <typename TPayload>
class MsgTypes {
public:
    
    template <TPayload init, const pb_msgdesc_t *fields>
    class MsgDesc {
    public:
        using Payload = TPayload;
        static constexpr Payload sInit = init;
        static constexpr const pb_msgdesc_t *sFields = fields;

        static bool encode(const Payload &payload, pb_ostream_t &stream) {

            // Now we are ready to encode the message.
            bool status = pb_encode(&stream, sFields, &payload);

            // Then just check for any errors.
            if (!status) {
                std::cout << "Encoding failed: " << PB_GET_ERROR(&stream) << std::endl;
                return false;
            }
            return true;
        }

        static bool decode(Payload &payload, pb_istream_t &stream) {

            bool status = pb_decode(&stream, sFields, &payload);

            if (!status) {
                printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
                return false;
            }
            return true;
        }
    };
};

using ConfigDesc = MsgTypes<Config>::MsgDesc<Config(Config_init_default), Config_fields>;
using LogMessageDesc = MsgTypes<LogMessage>::MsgDesc<LogMessage(LogMessage_init_default), LogMessage_fields>;
using SimpleMessageDesc = MsgTypes<SimpleMessage>::MsgDesc<SimpleMessage(SimpleMessage_init_default), SimpleMessage_fields>;

bool writeString(pb_ostream_t *stream, const pb_field_iter_t *field, void * const *arg) {

    const std::string &str = *reinterpret_cast<const std::string *>(*arg);
    if (!pb_encode_tag_for_field(stream, field))
        return false;
    return pb_encode_string(stream, reinterpret_cast<const uint8_t *>(str.data()), str.size());
}

bool readString(pb_istream_t *stream, const pb_field_t *field, void **arg) {

    std::string &str = *reinterpret_cast<std::string *>(*arg);
    
    // If we had a fixed capacity container to write to, we can see how many bytes is needed with stream->bytes_left,
    // and if the container doesn't have enough space we'd return false.

    // Let's read chunks of the data in the stream and append it to the string.
    uint8_t buf[16] = {0};
    while (stream->bytes_left > 0) {
        size_t len = std::min(stream->bytes_left, size_t(16));
        if (!pb_read(stream, buf, len)) {
            return false;
        }
        str.append(reinterpret_cast<char *>(buf), len);
    }
    return true;
}

int main() {

    {
        // Create a stream that will write to our buffer.
        uint8_t buffer[1024];
        pb_ostream_t ostream = pb_ostream_from_buffer(buffer, 1024);

        // It is a good idea to always initialize your structures so that you do not have garbage data from RAM in 
        // there.
        typename SimpleMessageDesc::Payload message = SimpleMessageDesc::sInit;
        message.luckyNumber = 13;
        bool ok = SimpleMessageDesc::encode(message, ostream);

        if (ok) {
            // Create a stream that reads from the buffer. Note that the second parameter is the length of the message
            // not the size of the buffer. These can be different.
            typename SimpleMessageDesc::Payload inputMessage = SimpleMessageDesc::sInit;
            pb_istream_t istream = pb_istream_from_buffer(buffer, ostream.bytes_written);
            SimpleMessageDesc::decode(inputMessage, istream);
            std::cout << "Your lucky number was " << inputMessage.luckyNumber << std::endl;
        }
    }

    {
        // Output stream that receives the encode data.
        uint8_t buffer[1024];
        pb_ostream_t ostream = pb_ostream_from_buffer(buffer, 1024);

        // For fields without a fixed size, you must provide a callback function that encodes you data into the 
        // stream. Usually you'd have a callback function for a given type, and the particular instance is passed
        // in using the generic args input.
        typename LogMessageDesc::Payload outputLogMessage = LogMessageDesc::sInit;
        std::string outputActualLogMessage = "This is a log message";
        outputLogMessage.message.funcs.encode = writeString;
        outputLogMessage.message.arg = reinterpret_cast<void *>(&outputActualLogMessage);
        bool ok = LogMessageDesc::encode(outputLogMessage, ostream);

        if (ok) {
            std::cout << "Log message encoding is ok" << std::endl;
            // Need to say create the input stream object which reads from a buffer, network stack, stdin etc.
            pb_istream_t istream = pb_istream_from_buffer(buffer, ostream.bytes_written);

            // Allocate memory for the decoded message. You don't have to but this would be the most common scenario.
            typename LogMessageDesc::Payload inputLogMessage = LogMessageDesc::sInit;
            std::string inputActualLogMessage;
            inputLogMessage.message.funcs.decode = readString;
            inputLogMessage.message.arg = reinterpret_cast<void *>(&inputActualLogMessage);
            bool inOK = LogMessageDesc::decode(inputLogMessage, istream);
            if (inOK) {
                std::cout << "The log message was: " << inputActualLogMessage << std::endl;
            }
        }
    }
}