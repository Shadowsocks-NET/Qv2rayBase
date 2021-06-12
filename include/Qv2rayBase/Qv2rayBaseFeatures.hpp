#pragma once

#define QVFEATURE_subscriptions 1
#define QVFEATURE_statistics 1
#define QVFEATURE_latency 1

#define QV2RAYBASE_FEATURE(feat) ((1 / QVFEATURE_##feat) == 1)
