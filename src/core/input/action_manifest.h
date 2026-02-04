#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace gbe::input {

/**
 * A node in the hierarchical VDF structure.
 */
struct VDFNode {
    std::string key;
    std::string value; // Only populated if it's a leaf key-value pair
    std::vector<std::unique_ptr<VDFNode>> children;

    VDFNode* GetChild(const std::string& key) const;
    std::string GetValue(const std::string& key, const std::string& default_val = "") const;
};

/**
 * Parser for Valve Data Format (.vdf) files used for Steam Input manifests.
 */
class ActionManifest {
public:
    ActionManifest();
    ~ActionManifest();

    bool LoadFromFile(const std::string& path);
    bool Parse(const std::string& buffer);

    const VDFNode* GetRoot() const { return m_root.get(); }

private:
    std::unique_ptr<VDFNode> m_root;
};

} // namespace gbe::input
