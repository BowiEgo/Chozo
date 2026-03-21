#include "Matrix4.h"
#include "Quaternion.h"
#include "Vector3.h"

#include "Test.h"

TEST_SUITE(Vector3Tests) {
    TEST("Default constructor") {
        FVector3 v;
        TEST_ASSERT_EQ(0.0f, v.x);
        TEST_ASSERT_EQ(0.0f, v.y);
        TEST_ASSERT_EQ(0.0f, v.z);
    }
    TEST_END;

    TEST("Parameterized constructor") {
        FVector3 v(1.0f, 2.0f, 3.0f);
        TEST_ASSERT_EQ(1.0f, v.x);
        TEST_ASSERT_EQ(2.0f, v.y);
        TEST_ASSERT_EQ(3.0f, v.z);
    }
    TEST_END;

    TEST("Addition") {
        FVector3 a(1, 2, 3);
        FVector3 b(4, 5, 6);
        FVector3 c = a + b;
        TEST_ASSERT_EQ(5.0f, c.x);
        TEST_ASSERT_EQ(7.0f, c.y);
        TEST_ASSERT_EQ(9.0f, c.z);
    }
    TEST_END;

    TEST("Dot product") {
        FVector3 a(1, 2, 3);
        FVector3 b(4, 5, 6);
        float dot = a.Dot(b);
        TEST_ASSERT_EQ(32.0f, dot);
    }
    TEST_END;

    TEST("Cross product") {
        FVector3 a(1, 0, 0);
        FVector3 b(0, 1, 0);
        FVector3 c = a.Cross(b);
        TEST_ASSERT_EQ(0.0f, c.x);
        TEST_ASSERT_EQ(0.0f, c.y);
        TEST_ASSERT_EQ(1.0f, c.z);
    }
    TEST_END;

    TEST("Length") {
        FVector3 v(3, 4, 0);
        TEST_ASSERT_NEAR(5.0f, v.Length(), 0.001f);
    }
    TEST_END;

    TEST("Normalized") {
        FVector3 v(3, 4, 0);
        FVector3 n = v.Normalized();
        TEST_ASSERT_NEAR(0.6f, n.x, 0.001f);
        TEST_ASSERT_NEAR(0.8f, n.y, 0.001f);
        TEST_ASSERT_EQ(0.0f, n.z);
    }
    TEST_END;
}
