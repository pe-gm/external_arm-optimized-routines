/*
 * Single-precision vector acosh(x) function.
 * Copyright (c) 2023, Arm Limited.
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include "v_math.h"
#include "pl_sig.h"
#include "pl_test.h"

#define WANT_V_LOG1P_K0_SHORTCUT 1
#include "v_log1p_inline.h"

#define BigBoundTop 0x5fe /* top12 (asuint64 (0x1p511)).  */

static NOINLINE VPCS_ATTR float64x2_t
special_case (float64x2_t x)
{
  return v_call_f64 (acosh, x, x, v_u64 (-1));
}

/* Vector approximation for double-precision acosh, based on log1p.
   The largest observed error is 3.02 ULP in the region where the
   argument to log1p falls in the k=0 interval, i.e. x close to 1:
   __v_acosh(0x1.00798aaf80739p+0) got 0x1.f2d6d823bc9dfp-5
				  want 0x1.f2d6d823bc9e2p-5.  */
VPCS_ATTR float64x2_t V_NAME_D1 (acosh) (float64x2_t x)
{
  uint64x2_t itop = vreinterpretq_u64_f64 (x) >> 52;
  uint64x2_t special = (itop - OneTop) >= (BigBoundTop - OneTop);

  /* Fall back to scalar routine for all lanes if any of them are special.  */
  if (unlikely (v_any_u64 (special)))
    return special_case (x);

  float64x2_t xm1 = x - 1;
  float64x2_t u = xm1 * (x + 1);
  return log1p_inline (xm1 + vsqrtq_f64 (u));
}

PL_SIG (V, D, 1, acosh, 1.0, 10.0)
PL_TEST_ULP (V_NAME_D1 (acosh), 2.53)
PL_TEST_EXPECT_FENV_ALWAYS (V_NAME_D1 (acosh))
PL_TEST_INTERVAL (V_NAME_D1 (acosh), 1, 0x1p511, 90000)
PL_TEST_INTERVAL (V_NAME_D1 (acosh), 0x1p511, inf, 10000)
PL_TEST_INTERVAL (V_NAME_D1 (acosh), 0, 1, 1000)
PL_TEST_INTERVAL (V_NAME_D1 (acosh), -0, -inf, 10000)
