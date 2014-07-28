/*! \file mathutil.h
 *  \brief Math utility functions
 */

#ifndef _MATHUTIL_H
#define _MATHUTIL_H

#include <ogc/gu.h>

/*! \brief Convert Euler angles to a quaternion
 *  \param[out] q Result quaternion
 *  \param[in] rX X axis Euler angle
 *  \param[in] rY Y axis Euler angle
 *  \param[in] rZ Z axis Euler angle
 */
void EulerToQuaternion(guQuaternion* q, const f32 rX, const f32 rY, const f32 rZ);

/*! \brief Create quaternion from a single axis angle
 *  \param[out] q     Result quaternion
 *  \param[in] rAxis  Axis of the Euler angle
 *  \param[in] rAngle Width of the euler angle
 */
void AxisAngleToQuaternion(guQuaternion* q, guVector rAxis, const f32 rAngle);

/*! \brief Multiplies a pair of float32 with another pair
           opA[0] = opA[0] * opB[0];
		   opA[1] = opA[1] * opB[1];
 *  \param opA First pair of float32 to multiply
 *  \param opB Second pair of float32 to multiply
 */
void ps_float2Mul(f32* opA, f32* opB, f32* result);

#endif


