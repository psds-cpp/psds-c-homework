#include <algorithm>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

extern "C"
{
#include "cstack.h"
}

class AllAPITest : public ::testing::TestWithParam<hstack_t>
{
};

TEST_P(AllAPITest, InvalidStackHandlers)
{
    const auto handler = GetParam();
    stack_free(handler);
    EXPECT_EQ(stack_valid_handler(handler), 0);
    EXPECT_EQ(stack_size(handler), 0u);
    EXPECT_EQ(stack_top_size(handler), 0u);
    const int data_in = 1;
    stack_push(handler, &data_in, sizeof(data_in));
    int data_out = 0;
    EXPECT_EQ(stack_pop(handler, &data_out, sizeof(data_out)), 0u);
    EXPECT_EQ(data_out, 0);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidHandlers,
    AllAPITest,
    ::testing::Values(-1000, -100, -10, -1, 0, 1, 10, 100, 1000)
);

TEST(AllocationTests, SingleAllocation)
{
    const hstack_t stack = stack_new();
    EXPECT_EQ(stack_valid_handler(stack), 1);
    EXPECT_EQ(stack_size(stack), 0u);
    stack_free(stack);
    EXPECT_EQ(stack_valid_handler(stack), 0);
}

TEST(AllocationTests, SeveralAllocations)
{
    const size_t count = 10;
    hstack_t stacks[count] = {-1};
    for (size_t i = 0; i < count; ++i)
    {
        stacks[i] = stack_new();
        EXPECT_EQ(stack_valid_handler(stacks[i]), 1);
        EXPECT_EQ(stack_size(stacks[i]), 0u);
    }
    for (size_t i = 0; i < count; ++i)
    {
        stack_free(stacks[i]);
        EXPECT_EQ(stack_valid_handler(stacks[i]), 0);
    }
}

struct ModifyTests : ::testing::Test
{
    void SetUp()
    {
        stack = stack_new();
        ASSERT_EQ(stack_valid_handler(stack), 1);
    }
    void TearDown()
    {
        stack_free(stack);
        ASSERT_EQ(stack_valid_handler(stack), 0);
    }
    hstack_t stack = -1;
};

TEST_F(ModifyTests, PushBadArgs)
{
    stack_push(stack, nullptr, 0u);
    EXPECT_EQ(stack_size(stack), 0u);

    const int data_out = 1;
    stack_push(stack, &data_out, 0u);
    EXPECT_EQ(stack_size(stack), 0u);

    stack_push(stack, nullptr, sizeof(data_out));
    EXPECT_EQ(stack_size(stack), 0u);
}

TEST_F(ModifyTests, PopBadArgs)
{
    const size_t size = 5;
    const int data_in[size] = {1};
    stack_push(stack, &data_in, sizeof(data_in));
    ASSERT_EQ(stack_size(stack), 1u);

    EXPECT_EQ(stack_pop(stack, nullptr, 0u), 0u);

    int data_out[size - 1] = {0};
    EXPECT_EQ(stack_pop(stack, data_out, sizeof(data_out)), 0u);
    EXPECT_THAT(data_out, ::testing::Each(0));

    EXPECT_EQ(stack_pop(stack, nullptr, sizeof(data_in)), 0u);
    EXPECT_THAT(data_out, ::testing::Each(0));
}

TEST_F(ModifyTests, PopFromEmptyStack)
{
    ASSERT_EQ(stack_size(stack), 0u);
    int data_out = 0;
    EXPECT_EQ(stack_pop(stack, &data_out, sizeof(data_out)), 0u);
}

TEST_F(ModifyTests, TopSizeEmptyStack)
{
    ASSERT_EQ(stack_size(stack), 0u);
    EXPECT_EQ(stack_top_size(stack), 0u);
}

TEST_F(ModifyTests, TopSizeSinglePushPop)
{
    EXPECT_EQ(stack_top_size(stack), 0u);

    const int data_in[3] = {0, 1, 2};
    stack_push(stack, data_in, sizeof(data_in));
    ASSERT_EQ(stack_size(stack), 1u);
    EXPECT_EQ(stack_top_size(stack), sizeof(data_in));

    int data_out[3] = {0};
    EXPECT_EQ(stack_pop(stack, data_out, sizeof(data_out)), sizeof(data_out));
    EXPECT_EQ(stack_size(stack), 0u);
    EXPECT_EQ(stack_top_size(stack), 0u);
}

TEST_F(ModifyTests, TopSizeSeveralPushPop)
{
    const size_t count = 3;
    const unsigned int sizes[count] = {1u, 4u, 8u};
    unsigned char data_in[8] = {0};
    unsigned char data_out[8] = {0};
    for (size_t i = 0; i < count; ++i)
    {
        stack_push(stack, data_in, sizes[i]);
        EXPECT_EQ(stack_top_size(stack), sizes[i]);
    }
    for (size_t i = count; i > 0; --i)
    {
        ASSERT_EQ(stack_pop(stack, data_out, sizes[i - 1]), sizes[i - 1]);
        EXPECT_EQ(stack_top_size(stack), i > 1 ? sizes[i - 2] : 0u);
    }
    EXPECT_EQ(stack_size(stack), 0u);
}

TEST_F(ModifyTests, SinglePushPop)
{
    const int data_in[3] = {0, 1, 2};
    int data_out[3] = {2, 1, 0};
    stack_push(stack, data_in, sizeof(data_in));
    EXPECT_EQ(stack_size(stack), 1u);
    EXPECT_EQ(stack_pop(stack, data_out, sizeof(data_out)), sizeof(data_out));
    EXPECT_EQ(stack_size(stack), 0u);
    EXPECT_THAT(data_out, ::testing::ElementsAre(0, 1, 2));
}

TEST_F(ModifyTests, SeveralPushPop)
{
    const size_t size = 3;
    const int data_in[size] = {0, 1, 2};
    int data_out[size] = {0, 1, 2};
    for (size_t i = 0; i < size; ++i)
    {
        stack_push(stack, &data_in[i], sizeof(data_in[i]));
        EXPECT_EQ(stack_size(stack), i + 1u);
    }
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(
            stack_pop(stack, &data_out[i], sizeof(data_out[i])),
            sizeof(data_out[i])
        );
        EXPECT_EQ(stack_size(stack), size - 1u - i);
    }
    EXPECT_THAT(data_out, ::testing::ElementsAre(2, 1, 0));
}

TEST(GetAllTests, NoOtherStacks)
{
    const hstack_t all = stack_get_all();
    ASSERT_EQ(stack_valid_handler(all), 1);
    EXPECT_EQ(stack_size(all), 0u);

    int data_out = 0;
    EXPECT_EQ(stack_pop(all, &data_out, sizeof(data_out)), 0u);
    EXPECT_EQ(data_out, 0);

    stack_free(all);
    EXPECT_EQ(stack_valid_handler(all), 0);
}

TEST(GetAllTests, SeveralStacks)
{
    const size_t count = 5;
    hstack_t stacks[count] = {-1};
    for (size_t i = 0; i < count; ++i)
    {
        stacks[i] = stack_new();
        ASSERT_EQ(stack_valid_handler(stacks[i]), 1);
        for (size_t j = 0; j < i; ++j)
        {
            ASSERT_NE(stacks[i], stacks[j]);
        }
    }

    const hstack_t all = stack_get_all();
    ASSERT_EQ(stack_valid_handler(all), 1);
    for (size_t i = 0; i < count; ++i)
    {
        ASSERT_NE(all, stacks[i]);
    }
    ASSERT_EQ(stack_size(all), count);

    std::vector<hstack_t> descriptors;
    hstack_t data_out = 0;
    while (stack_pop(all, &data_out, sizeof(data_out)) == sizeof(data_out))
    {
        EXPECT_NE(data_out, all);
        EXPECT_EQ(stack_valid_handler(data_out), 1);
        descriptors.push_back(data_out);
    }
    EXPECT_EQ(stack_size(all), 0u);

    std::vector<hstack_t> expected(stacks, stacks + count);
    std::sort(descriptors.begin(), descriptors.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(descriptors, expected);

    stack_free(all);
    EXPECT_EQ(stack_valid_handler(all), 0);
    for (size_t i = 0; i < count; ++i)
    {
        stack_free(stacks[i]);
        EXPECT_EQ(stack_valid_handler(stacks[i]), 0);
    }
}

TEST(FreeAllTests, NoStacksExist)
{
    stack_free_all();

    const hstack_t stack = stack_new();
    ASSERT_EQ(stack_valid_handler(stack), 1);
    stack_free(stack);
    EXPECT_EQ(stack_valid_handler(stack), 0);
}

TEST(FreeAllTests, SeveralStacks)
{
    const size_t count = 10;
    hstack_t stacks[count] = {-1};
    for (size_t i = 0; i < count; ++i)
    {
        stacks[i] = stack_new();
        ASSERT_EQ(stack_valid_handler(stacks[i]), 1);
    }
    const int data = 1;
    for (size_t i = 0; i < count; ++i)
    {
        stack_push(stacks[i], &data, sizeof(data));
        ASSERT_EQ(stack_size(stacks[i]), 1u);
    }

    stack_free_all();

    for (size_t i = 0; i < count; ++i)
    {
        EXPECT_EQ(stack_valid_handler(stacks[i]), 0);
        EXPECT_EQ(stack_size(stacks[i]), 0u);
    }

    const hstack_t fresh = stack_new();
    ASSERT_EQ(stack_valid_handler(fresh), 1);
    EXPECT_EQ(stack_size(fresh), 0u);
    stack_free(fresh);
    EXPECT_EQ(stack_valid_handler(fresh), 0);
}

TEST(FreeAllTests, SomeStacksAlreadyFreed)
{
    const size_t count = 5;
    hstack_t stacks[count] = {-1};
    for (size_t i = 0; i < count; ++i)
    {
        stacks[i] = stack_new();
        ASSERT_EQ(stack_valid_handler(stacks[i]), 1);
    }
    stack_free(stacks[0]);

    stack_free_all();

    for (size_t i = 0; i < count; ++i)
    {
        EXPECT_EQ(stack_valid_handler(stacks[i]), 0);
    }
}
