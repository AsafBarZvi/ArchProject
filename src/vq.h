#include <string>

class VQ
{
    std::string tag_     = "none";
    float       value_   = -1.0;

public:
    bool is_ready() { return tag_.empty();}
    void set_tag(const std::string & ntag) { tag_ = ntag ; value_ = -1;}
    void set_val(const float val) { tag_ = "" ; value_ = val; }
    float val() const { return value_; }
    const std::string& tag() const {return tag_;}
};
