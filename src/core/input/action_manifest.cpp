#include "action_manifest.h"
#include <fstream>
#include <sstream>
#include <stack>

namespace gbe::input {

VDFNode* VDFNode::GetChild(const std::string& key) const {
    for (const auto& child : children) {
        if (child->key == key) return child.get();
    }
    return nullptr;
}

std::string VDFNode::GetValue(const std::string& key, const std::string& default_val) const {
    VDFNode* child = GetChild(key);
    return child ? child->value : default_val;
}

ActionManifest::ActionManifest() : m_root(std::make_unique<VDFNode>()) {
    m_root->key = "root";
}

ActionManifest::~ActionManifest() = default;

bool ActionManifest::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    return Parse(buffer.str());
}

bool ActionManifest::Parse(const std::string& buffer) {
    std::stack<VDFNode*> stack;
    stack.push(m_root.get());

    size_t i = 0;
    while (i < buffer.size()) {
        // Skip whitespace
        if (std::isspace(buffer[i])) {
            i++;
            continue;
        }

        // Comment
        if (buffer[i] == '/' && i + 1 < buffer.size() && buffer[i+1] == '/') {
            while (i < buffer.size() && buffer[i] != '\n') i++;
            continue;
        }

        // Open block
        if (buffer[i] == '{') {
            // The previous token should have been the key for this block
            // In a well-formed VDF, we should already have a child created by the key
            if (!stack.top()->children.empty()) {
                stack.push(stack.top()->children.back().get());
            }
            i++;
            continue;
        }

        // Close block
        if (buffer[i] == '}') {
            if (stack.size() > 1) stack.pop();
            i++;
            continue;
        }

        // Parse token (quoted or unquoted)
        std::string token;
        if (buffer[i] == '"') {
            i++;
            while (i < buffer.size() && buffer[i] != '"') {
                if (buffer[i] == '\\' && i + 1 < buffer.size()) i++; // Skip escaped
                token += buffer[i++];
            }
            if (i < buffer.size()) i++; // skip closing quote
        } else {
            while (i < buffer.size() && !std::isspace(buffer[i]) && buffer[i] != '{' && buffer[i] != '}' && buffer[i] != '"') {
                token += buffer[i++];
            }
        }

        if (token.empty()) continue;

        // Peek next non-whitespace char
        size_t next = i;
        while (next < buffer.size() && std::isspace(buffer[next])) next++;

        if (next < buffer.size() && buffer[next] == '{') {
            // Token is a block key
            auto node = std::make_unique<VDFNode>();
            node->key = token;
            stack.top()->children.push_back(std::move(node));
            // We'll push to stack in the '{' case above
        } else {
            // Token is either a key or a value
            // For now, let's assume it's a key and look for a value
            std::string key = token;
            i = next;
            
            std::string value;
            if (i < buffer.size() && buffer[i] != '{' && buffer[i] != '}') {
                if (buffer[i] == '"') {
                    i++;
                    while (i < buffer.size() && buffer[i] != '"') {
                        if (buffer[i] == '\\' && i + 1 < buffer.size()) i++;
                        value += buffer[i++];
                    }
                    if (i < buffer.size()) i++;
                } else {
                    while (i < buffer.size() && !std::isspace(buffer[i]) && buffer[i] != '{' && buffer[i] != '}' && buffer[i] != '"') {
                        value += buffer[i++];
                    }
                }
                
                auto node = std::make_unique<VDFNode>();
                node->key = key;
                node->value = value;
                stack.top()->children.push_back(std::move(node));
            } else {
                // Key without value (e.g. block key followed by a newline and then '{')
                auto node = std::make_unique<VDFNode>();
                node->key = key;
                stack.top()->children.push_back(std::move(node));
            }
        }
    }

    return true;
}

} // namespace gbe::input
