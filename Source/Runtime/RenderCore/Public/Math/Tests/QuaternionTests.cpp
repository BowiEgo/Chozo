#include "Quaternion.h"
#include "Vector3.h"

#include "Test.h"

TEST_SUITE(QuaternionTests) {
    TEST("Identity") {
        FQuaternion q = FQuaternion::Identity();
        TEST_ASSERT_EQ(0.0f, q.x);
        TEST_ASSERT_EQ(0.0f, q.y);
        TEST_ASSERT_EQ(0.0f, q.z);
        TEST_ASSERT_EQ(1.0f, q.w);
    }
    TEST_END;

    TEST("FromEuler") {
        FQuaternion q = FQuaternion::FromEuler(90, 0, 0);
        FVector3 v(0, 1, 0);
        FVector3 rotated = q * v;
        TEST_ASSERT_NEAR(0.0f, rotated.x, 0.001f);
        TEST_ASSERT_NEAR(0.0f, rotated.y, 0.001f);
        TEST_ASSERT_NEAR(1.0f, rotated.z, 0.001f);
    }
    TEST_END;
}