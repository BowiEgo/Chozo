#include <string>

#include <Core/FileSystem/VFS.hpp>

#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>

using namespace CZ;

namespace {

/// Creates a throw-away directory with one regular file inside.
struct TempTree {
    std::filesystem::path Root;

    explicit TempTree(const std::string& name) {
        Root = std::filesystem::temp_directory_path() / name;
        std::filesystem::remove_all(Root);
        std::filesystem::create_directories(Root);

        std::ofstream(Root / "hello.txt") << "hello";
    }

    ~TempTree() {
        std::error_code ec;
        std::filesystem::remove_all(Root, ec);
    }
};

} // namespace

TEST_SUITE("VFS") {

    TEST_CASE("Mount accepts a protocol with or without the separator") {
        TempTree tree("chozo_vfs_mount_forms");

        // Both spellings must address the same root.
        VFS::Mount("czform", tree.Root);
        CHECK_EQ(VFS::Resolve("czform://hello.txt"), tree.Root / "hello.txt");

        VFS::Mount("czform2://", tree.Root);
        CHECK_EQ(VFS::Resolve("czform2://hello.txt"), tree.Root / "hello.txt");
    }

    TEST_CASE("Resolve maps a virtual path onto the mounted root") {
        TempTree tree("chozo_vfs_resolve");
        VFS::Mount("czresolve", tree.Root);

        CHECK_EQ(VFS::Resolve("czresolve://hello.txt"), tree.Root / "hello.txt");
    }

    TEST_CASE("Redundant leading slashes are ignored") {
        TempTree tree("chozo_vfs_slashes");
        VFS::Mount("czslashes", tree.Root);

        CHECK_EQ(VFS::Resolve("czslashes:///hello.txt"), tree.Root / "hello.txt");
    }

    TEST_CASE("Unknown protocols fall back to the raw path") {
        const std::filesystem::path unresolved("czunknown://hello.txt");

        CHECK_EQ(VFS::Resolve(unresolved.string()), unresolved);
    }

    TEST_CASE("Missing files still resolve to their mapped path") {
        TempTree tree("chozo_vfs_missing");
        VFS::Mount("czmissing", tree.Root);

        // Resolving reports the problem through the log; the path is still returned.
        CHECK_EQ(VFS::Resolve("czmissing://not-there.txt"), tree.Root / "not-there.txt");
    }

    TEST_CASE("Path escaping is not prevented yet") {
        TempTree tree("chozo_vfs_escape");
        VFS::Mount("czescape", tree.Root);

        // Documents today's behaviour (a plain concatenation) so that hardening the resolver
        // has to update this expectation. Tracked in docs/TODO.md (P1-6).
        CHECK_EQ(VFS::Resolve("czescape://../outside.txt"), tree.Root / "../outside.txt");
    }
}
