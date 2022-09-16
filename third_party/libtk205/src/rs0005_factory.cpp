#include "rs0005_factory.h"
#include "rs0005.h"
#include <memory>
#include <error_handling_tk205.h>

/// @note  This class has been generated from a template. Local changes will not be saved!

using namespace tk205;

std::shared_ptr<RSInstanceBase> RS0005Factory::create_instance(const char* RS_instance_file) const
{
    auto p_rs = std::make_shared<rs0005_ns::RS0005>();
    auto j = tk205::load_json(RS_instance_file);
    std::string schema_version = j["metadata"]["schema_version"];
    if (SchemVer(schema_version.c_str()) > SchemVer("0.2.1"))
    {
        p_rs = nullptr;
        std::ostringstream oss;
        oss << "Schema version " << schema_version << " is not supported.";
        tk205::show_message(tk205::MsgSeverity::ERR_205, oss.str());
    }
    else if (j["metadata"]["schema"] == "RS0005")
    {
        p_rs->initialize(j);
    }
    else
    {
        p_rs = nullptr;
        std::ostringstream oss;
        oss << RS_instance_file << " is not a valid instance of RS0005; returning nullptr.";
        tk205::show_message(tk205::MsgSeverity::ERR_205, oss.str());
    }
    return p_rs;
}
