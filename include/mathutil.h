/*! \file mathutil.h
 *  \brief Math utility functions
 */

#ifndef _MATHUTIL_H
#define _MATHUTIL_H

#include <ogc/gu.h>
#include "game.h"

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
 *         opA[0] = opA[0] * opB[0];
 *         opA[1] = opA[1] * opB[1];
 *  \param[in] opA First pair of float32 to multiply
 *  \param[in] opB Second pair of float32 to multiply
 *  \param[out] result Result pair
 */
inline void ps_float2Mul(f32* opA, f32* opB, f32* result);

/*! \brief ASM part of EulerToQuaternion, don't use alone!
 *  \param[in] x Sin/Cos of Euler X
 *  \param[in] y Sin/Cos of Euler Y
 *  \param[in] z Sin/Cos of Euler Z
 *  \param[out] out Resultant quaternion
 */
inline void ps_eulerQuat(f32* x, f32* y, f32* z, guQuaternion* out);

/*! \brief Quaternion lookAt
 *  \param[in]  forward Forward vector (Direction)
 *  \param[in]  up      Up vector (Orientation)
 *  \param[out] out     Out quaternion
 */
void QUAT_lookat(guVector* forward, guVector *up, guQuaternion* out);

/*! \brief Quaternion dot product
 *  \param[in]  p   First quaternion
 *  \param[in]  q   Second quaternion
 *  \param[out] res Dot product of the two quaternions
 */
void QUAT_dotProduct(guQuaternion *p, guQuaternion *q, f32* res);

/*! \brief Quaternion scalar multiplication
 *  \param[in]  q     Input quaternion
 *  \param[out] r     Output quaternion
 *  \param[in]  scale Scale
 */
void QUAT_scale(guQuaternion* q, guQuaternion* r, f32* scale);

/*! \brief Spherical linear interpolation between quaternions
 *  \param[in]  q0  From Quaternion
 *  \param[in]  q1  To Quaternion
 *  \param[in]  t   Interpolation parameter
 *  \param[out] out Interpolated quaternion
 */
void QUAT_slerp(guQuaternion* q0, guQuaternion* q1, const float t, guQuaternion* out);

/*! \brief Vector magnitude
 *  \param vec Vector to calculate magnitude of
 *  \return Magnitude as a f32
 */
f32 guVecMag(guVector* vec);

/*! \brief Squared vector magnitude
 *  \param vec Vector to calculate squared magnitude of
 *  \return Squared magnitude as a f32
 */
f32 guVecSquareMag(guVector* vec);

/*! \brief Get distance between two 3d points
 *  \param point1 First 3d point
 *  \param point2 Second 3d point
 *  \return Distance between point1 and point2
 */
f32 vecDistance(guVector* point1, guVector* point2);

/*! \brief Make players bounce if they touch
*  \param a First player in the collision
*  \param b Second player in the collision
*  \return TRUE if the players collided, FALSE otherwise
*/
BOOL CalculateBounce(player_t* a, player_t* b);

#endif


