#pragma once
#include <sfizz/Parser.h>

template <class S>
class OpcodeCollectingParser : public sfz::Parser {
public:
    explicit OpcodeCollectingParser(S &set) : _set{set} {}

protected:
    void callback(absl::string_view header, const std::vector<sfz::Opcode> &members) override
    {
        for (const sfz::Opcode &member : members)
            _set.emplace(member.rawOpcode);
    }

private:
    S &_set;
};
