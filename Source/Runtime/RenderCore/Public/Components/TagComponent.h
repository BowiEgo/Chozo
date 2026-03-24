#pragma once

#include <string>
#include <unordered_set>
#include <vector>

struct FTagComponent {
    std::unordered_set<std::string> Tags;

    FTagComponent() = default;
    explicit FTagComponent(const std::string& tag) { Tags.insert(tag); }

    FTagComponent(std::initializer_list<std::string> tags) {
        for (const auto& tag : tags) {
            Tags.insert(tag);
        }
    }

    // Tag operations
    bool HasTag(const std::string& tag) const { return Tags.find(tag) != Tags.end(); }

    void AddTag(const std::string& tag) { Tags.insert(tag); }

    void RemoveTag(const std::string& tag) { Tags.erase(tag); }

    void ClearTags() { Tags.clear(); }

    // Batch operations
    void AddTags(const std::vector<std::string>& tags) {
        for (const auto& tag : tags) {
            Tags.insert(tag);
        }
    }

    std::vector<std::string> GetTags() const {
        return std::vector<std::string>(Tags.begin(), Tags.end());
    }

    size_t GetTagCount() const { return Tags.size(); }

    bool IsEmpty() const { return Tags.empty(); }
};
