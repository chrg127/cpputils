#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <flags.hpp>

enum RenderPass {
    Geometry, Lighting
};

TEST_CASE("Test for most methods", "[flags]")
{
    auto flags = Flags<RenderPass> { RenderPass::Geometry, RenderPass::Lighting };
    REQUIRE(bool(flags));
    REQUIRE(flags.value() == 3);
    REQUIRE(flags.contains(RenderPass::Geometry));
    REQUIRE(flags.contains(RenderPass::Lighting));
    REQUIRE(flags.count() == 2);
}
