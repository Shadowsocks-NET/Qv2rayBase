#pragma once

// clang-format off
    #define QVFEATURE_subscriptions 1

#ifndef QVFEATURE_statistics
    #define QVFEATURE_statistics                     1
#endif

#ifndef QVFEATURE_latency
    #define QVFEATURE_latency                        -1
#endif

// clang-format on

#define QV2RAYBASE_FEATURE(feat) ((1 / QVFEATURE_##feat) == 1)
