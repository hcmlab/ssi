// ****************************************************************************************
//
// Fubi Math Functions
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
//
// ****************************************************************************************
#pragma once

/** \file FubiMath.h
 * \brief Math structures and functions such as vectors, matrices, ...
*/

#include "FubiUtils.h"

#include <algorithm>

namespace Fubi
{
	/** \addtogroup FUBIMATH FUBI Math library
	* Math structures and functions such as vectors, matrices, ...
	*
	* @{
	*/

	// The following is a modified version of the utMath.h within the Horde3D Engine by Nicolas Schulz (http://www.horde3d.org).
	// Horde3D namespace and some unnescessary functions are removed.
	// Some special vector functions are added

	// *************************************************************************************************
	//
	// Horde3D
	//   Next-Generation Graphics Engine
	// --------------------------------------
	// Copyright (C) 2006-2011 Nicolas Schulz
	//
	// This software is distributed under the terms of the Eclipse Public License v1.0.
	// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
	//
	// *************************************************************************************************

	// -------------------------------------------------------------------------------------------------
	//
	// Math library
	//
	// Conventions:
	//
	// - Coordinate system is right-handed with positive y as up axis
	// - All rotation angles are counter-clockwise when looking from the positive end of the rotation
	//	 axis towards the origin
	// - An unrotated view vector points along the negative z-axis
	//
	// -------------------------------------------------------------------------------------------------


	/**
	* \namespace Math
	*
	* \brief Contains math constants, e.g. min and max values for different data types, Pi, Epsilon, NaN, and the NO_INIT hint
	*
	*/
	namespace Math
	{
		const unsigned char MaxUChar8 = 0xFF;
		const unsigned int MaxUInt32 = 0xFFFFFFFF;
		const unsigned short MaxUShort16 = 0xFFFF;
		const int MinInt32 = 0x80000000;
		const int MaxInt32 = 0x7FFFFFFF;
		const float MaxFloat = 3.402823466e+38F;
		const double MaxDouble = 1.7976931348623157e+308;
		const float MinPosFloat = 1.175494351e-38F;

		const float Pi = 3.141592654f;
		const float TwoPi = 6.283185307f;
		const float PiHalf = 1.570796327f;

		const float Epsilon = 0.000001f;
		const float ZeroEpsilon = 32.0f * MinPosFloat;  // Very small epsilon for checking against 0.0f
#ifdef __GNUC__
		const float NaN = __builtin_nanf("");
#else
		const float NaN = *(float *)&MaxUInt32;
#endif

		enum NoInitHint
		{
			NO_INIT
		};
	};


	// -------------------------------------------------------------------------------------------------
	// General
	// -------------------------------------------------------------------------------------------------

	/**
	* \brief Converts degrees to radians
	*/
	static inline float degToRad( float f )
	{
		return f * 0.017453293f;
	}

	/**
	* \brief Converts radians to degrees
	*/
	static inline float radToDeg( float f )
	{
		return f * 57.29577951f;
	}

	/**
	* \brief Restricts a value to a certain minimum and maximum
	*/
	static inline float clampf( float f, float min, float max )
	{
		if( f < min ) f = min;
		else if( f > max ) f = max;

		return f;
	}

	/**
	* \brief Returns the minimum
	*/
	static inline float minf( float a, float b )
	{
		return a < b ? a : b;
	}

	/**
	* \brief Returns the maximum
	*/
	static inline float maxf( float a, float b )
	{
		return a > b ? a : b;
	}

	/**
	* \brief Returns a if test >= 0, else b
	*/
	static inline float fsel( float test, float a, float b )
	{
		// Branchless selection
		return test >= 0 ? a : b;
	}

	/**
	* \brief Returns the square
	*/
	static inline float squaref(float f)
	{
		return f*f;
	}


	// -------------------------------------------------------------------------------------------------
	// Conversion
	// -------------------------------------------------------------------------------------------------

	/**
	* \brief Float to int conversion using truncation
	*/
	static inline int ftoi_t( double val )
	{

		return (int)val;
	}

	/**
	* \brief Fast round (banker's round) using Sree Kotay's method
	* This function is much faster than a naive cast from float to int
	*/
	static inline int ftoi_r(double val)
	{
		union
		{
			double dval;
			int ival[2];
		} u;

		u.dval = val + 6755399441055744.0;  // Magic number: 2^52 * 1.5;
		return u.ival[0];         // Needs to be [1] for big-endian
	}

	/**
	* \brief Vector class with three components
	*/
	class Vec3f
	{
	public:
		float x, y, z;


		// ------------
		// Constructors
		// ------------
		Vec3f() : x( 0.0f ), y( 0.0f ), z( 0.0f )
		{
		}

		explicit Vec3f( Math::NoInitHint )
		{
			// Constructor without default initialization
		}

		Vec3f( const float x, const float y, const float z ) : x( x ), y( y ), z( z )
		{
		}

		Vec3f( const Vec3f &v ) : x( v.x ), y( v.y ), z( v.z )
		{
		}

		// ------
		// Access
		// ------
		float &operator[]( unsigned int index )
		{
			return *(&x + index);
		}

		// -----------
		// Comparisons
		// -----------
		bool operator==( const Vec3f &v ) const
		{
			return (x > v.x - Math::Epsilon && x < v.x + Math::Epsilon &&
				y > v.y - Math::Epsilon && y < v.y + Math::Epsilon &&
				z > v.z - Math::Epsilon && z < v.z + Math::Epsilon);
		}

		bool operator!=( const Vec3f &v ) const
		{
			return (x < v.x - Math::Epsilon || x > v.x + Math::Epsilon ||
				y < v.y - Math::Epsilon || y > v.y + Math::Epsilon ||
				z < v.z - Math::Epsilon || z > v.z + Math::Epsilon);
		}

		// ---------------------
		// Arithmetic operations
		// ---------------------
		Vec3f operator-() const
		{
			return Vec3f( -x, -y, -z );
		}

		Vec3f operator+( const Vec3f &v ) const
		{
			return Vec3f( x + v.x, y + v.y, z + v.z );
		}

		Vec3f &operator+=( const Vec3f &v )
		{
			return *this = *this + v;
		}

		Vec3f operator-( const Vec3f &v ) const
		{
			return Vec3f( x - v.x, y - v.y, z - v.z );
		}

		Vec3f &operator-=( const Vec3f &v )
		{
			return *this = *this - v;
		}

		Vec3f operator*( const float f ) const
		{
			return Vec3f( x * f, y * f, z * f );
		}

		Vec3f &operator*=( const float f )
		{
			return *this = *this * f;
		}

		Vec3f operator/( const float f ) const
		{
			return Vec3f( x / f, y / f, z / f );
		}

		Vec3f &operator/=( const float f )
		{
			return *this = *this / f;
		}

		Vec3f operator*(const Vec3f &v) const
		{
			return Vec3f( x * v.x, y * v.y, z * v.z);
		}

		Vec3f &operator*=(const Vec3f &v)
		{
			return *this = *this * v;
		}

		Vec3f operator/(const Vec3f &v) const
		{
			return Vec3f( x / v.x, y / v.y, z / v.z);
		}

		Vec3f &operator/=(const Vec3f &v)
		{
			return *this = *this / v;
		}

		// ----------------
		// Special products
		// ----------------
		float dot( const Vec3f &v ) const
		{
			return x * v.x + y * v.y + z * v.z;
		}

		Vec3f cross( const Vec3f &v ) const
		{
			return Vec3f( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x );
		}

		// ----------------
		// Other operations
		// ----------------
		float length2() const
		{
			return x*x + y*y + z*z;
		}

		float length() const
		{
			return sqrtf( length2() );
		}

		float lengthManhattan() const
		{
			return fabsf(x) + fabsf(y) + fabsf(z);
		}

		float boundingBoxVolume(bool ignoreZeros = false)
		{
			float volume = 1.0f;
			if (!ignoreZeros || x > Math::Epsilon || x < -Math::Epsilon)
				volume *= x;
			if (!ignoreZeros || y > Math::Epsilon || y < -Math::Epsilon)
				volume *= y;
			if (!ignoreZeros || z > Math::Epsilon || z < -Math::Epsilon)
				volume *= z;
			return fabsf(volume);
		}

		Vec3f normalized() const
		{
			float invLen = 1.0f / length();
			return Vec3f( x * invLen, y * invLen, z * invLen );
		}

		void normalize()
		{
			float invLen = 1.0f / length();
			x *= invLen;
			y *= invLen;
			z *= invLen;
		}

		/*void fromRotation( float angleX, float angleY )
		{
		x = cosf( angleX ) * sinf( angleY );
		y = -sinf( angleX );
		z = cosf( angleX ) * cosf( angleY );
		}*/

		Vec3f toRotation() const
		{
			// Assumes that the unrotated view vector is (0, 0, -1)
			Vec3f v;

			if( y != 0 ) v.x = atan2f( y, sqrtf( x*x + z*z ) );
			if( x != 0 || z != 0 ) v.y = atan2f( -x, -z );

			return v;
		}

		Vec3f lerp( const Vec3f &v, float f ) const
		{
			return Vec3f( x + (v.x - x) * f, y + (v.y - y) * f, z + (v.z - z) * f );
		}
	};

   /**
	* \brief Converts radians to degrees for a Vec3f
	*/
	static inline void radToDeg( Vec3f& v )
	{
		v *= 57.29577951f;
	}
	/**
	* \brief Converts degrees to radians for a Vec3f
	*/
	static inline void degToRad( Vec3f& v )
	{
		v *= 0.017453293f;
	}
	/**
	* \brief Checks wether each component of a Vec3f is between certain bounds
	*
	* @param  value the value to be checked
	* @param  min the minimum allowed for the value
	* @param  max the maximum allowed for the value
	* @return returns the distance to the value, i.e. (value-min) if too small, (value-max) if too large, 0 if in range
	*/
	static inline Fubi::Vec3f inRange(const Vec3f& value, const Vec3f& min, const Vec3f& max)
	{
		Vec3f ret(Math::NO_INIT);
		ret.x = inRange(value.x, min.x, max.x);
		ret.y = inRange(value.y, min.y, max.y);
		ret.z = inRange(value.z, min.z, max.z);
		return ret;
	}

	/**
	* \brief Quaternion class (for representing orientations)
	*/
	class Quaternion
	{
	public:

		float x, y, z, w;

		// ------------
		// Constructors
		// ------------
		Quaternion() : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f )
		{
		}

		explicit Quaternion( const float x, const float y, const float z, const float w ) :
		x( x ), y( y ), z( z ), w( w )
		{
		}

		Quaternion( const float eulerX, const float eulerY, const float eulerZ )
		{
			Quaternion roll( sinf( eulerX * 0.5f ), 0, 0, cosf( eulerX * 0.5f ) );
			Quaternion pitch( 0, sinf( eulerY * 0.5f ), 0, cosf( eulerY * 0.5f ) );
			Quaternion yaw( 0, 0, sinf( eulerZ * 0.5f ), cosf( eulerZ * 0.5f ) );

			// Order: y * x * z
			*this = pitch * roll * yaw;
		}

		// ---------------------
		// Arithmetic operations
		// ---------------------
		Quaternion operator*( const Quaternion &q ) const
		{
			return Quaternion(
				y * q.z - z * q.y + q.x * w + x * q.w,
				z * q.x - x * q.z + q.y * w + y * q.w,
				x * q.y - y * q.x + q.z * w + z * q.w,
				w * q.w - (x * q.x + y * q.y + z * q.z) );
		}

		Quaternion &operator*=( const Quaternion &q )
		{
			return *this = *this * q;
		}

		// ----------------
		// Other operations
		// ----------------

		Quaternion slerp( const Quaternion &q, const float t ) const
		{
			// Spherical linear interpolation between two quaternions
			// Note: SLERP is not commutative

			Quaternion q1( q );

			// Calculate cosine
			float cosTheta = x * q.x + y * q.y + z * q.z + w * q.w;

			// Use the shortest path
			if( cosTheta < 0 )
			{
				cosTheta = -cosTheta;
				q1.x = -q.x; q1.y = -q.y;
				q1.z = -q.z; q1.w = -q.w;
			}

			// Initialize with linear interpolation
			float scale0 = 1 - t, scale1 = t;

			// Use spherical interpolation only if the quaternions are not very close
			if( (1 - cosTheta) > 0.001f )
			{
				// SLERP
				float theta = acosf( cosTheta );
				float sinTheta = sinf( theta );
				scale0 = sinf( (1 - t) * theta ) / sinTheta;
				scale1 = sinf( t * theta ) / sinTheta;
			}

			// Calculate final quaternion
			return Quaternion( x * scale0 + q1.x * scale1, y * scale0 + q1.y * scale1,
				z * scale0 + q1.z * scale1, w * scale0 + q1.w * scale1 );
		}

		Quaternion nlerp( const Quaternion &q, const float t ) const
		{
			// Normalized linear quaternion interpolation
			// Note: NLERP is faster than SLERP and commutative but does not yield constant velocity

			Quaternion qt;
			float cosTheta = x * q.x + y * q.y + z * q.z + w * q.w;

			// Use the shortest path and interpolate linearly
			if( cosTheta < 0 )
				qt = Quaternion( x + (-q.x - x) * t, y + (-q.y - y) * t,
				z + (-q.z - z) * t, w + (-q.w - w) * t );
			else
				qt = Quaternion( x + (q.x - x) * t, y + (q.y - y) * t,
				z + (q.z - z) * t, w + (q.w - w) * t );

			// Return normalized quaternion
			float invLen = 1.0f / sqrtf( qt.x * qt.x + qt.y * qt.y + qt.z * qt.z + qt.w * qt.w );
			return Quaternion( qt.x * invLen, qt.y * invLen, qt.z * invLen, qt.w * invLen );
		}

		Quaternion inverted() const
		{
			float len = x * x + y * y + z * z + w * w;
			if( len > 0 )
			{
				float invLen = 1.0f / len;
				return Quaternion( -x * invLen, -y * invLen, -z * invLen, w * invLen );
			}
			else return Quaternion();
		}
	};


	/**
	* \brief Matrix class with 3x3 layout (rotation matrices)
	*/
	class Matrix3f
	{
	public:
		union
		{
			float c[3][3];
			float x[9];
		};

		static Matrix3f RotMat( float x, float y, float z )
		{
			// Rotation order: YXZ [* Vector]
			return Matrix3f( Quaternion( x, y, z ) );
		}
		static Matrix3f RotMat( const Vec3f rot )
		{
			// Rotation order: YXZ [* Vector]
			return Matrix3f( Quaternion( rot.x, rot.y, rot.z ) );
		}
		static Matrix3f RotMat(Vec3f axis, float angle)
		{
			axis = axis * sinf(angle * 0.5f);
			return Matrix3f(Quaternion(axis.x, axis.y, axis.z, cosf(angle * 0.5f)));
		}
		// ------------
		// Constructors
		// ------------
		Matrix3f()
		{
			c[0][0] = 1; c[1][0] = 0; c[2][0] = 0;
			c[0][1] = 0; c[1][1] = 1; c[2][1] = 0;
			c[0][2] = 0; c[1][2] = 0; c[2][2] = 1;
		}

		explicit Matrix3f( Math::NoInitHint )
		{
			// Constructor without default initialization
		}

		Matrix3f( const float *floatArray9 )
		{
			for( unsigned int i = 0; i < 3; ++i )
			{
				for( unsigned int j = 0; j < 3; ++j )
				{
					c[i][j] = floatArray9[i * 3 + j];
				}
			}
		}

		Matrix3f( const Quaternion &q )
		{
			// Calculate coefficients
			float x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
			float xx = q.x * x2,  xy = q.x * y2,  xz = q.x * z2;
			float yy = q.y * y2,  yz = q.y * z2,  zz = q.z * z2;
			float wx = q.w * x2,  wy = q.w * y2,  wz = q.w * z2;

			c[0][0] = 1 - (yy + zz);  c[1][0] = xy - wz;		c[2][0] = xz + wy;
			c[0][1] = xy + wz;        c[1][1] = 1 - (xx + zz);	c[2][1] = yz - wx;
			c[0][2] = xz - wy;        c[1][2] = yz + wx;		c[2][2] = 1 - (xx + yy);
		}

		// ----------
		// Matrix sum
		// ----------
		Matrix3f operator+( const Matrix3f &m ) const
		{
			Matrix3f mf( Math::NO_INIT );

			mf.x[0] = x[0] + m.x[0];
			mf.x[1] = x[1] + m.x[1];
			mf.x[2] = x[2] + m.x[2];
			mf.x[3] = x[3] + m.x[3];
			mf.x[4] = x[4] + m.x[4];
			mf.x[5] = x[5] + m.x[5];
			mf.x[6] = x[6] + m.x[6];
			mf.x[7] = x[7] + m.x[7];
			mf.x[8] = x[8] + m.x[8];

			return mf;
		}

		Matrix3f &operator+=( const Matrix3f &m )
		{
			return *this = *this + m;
		}

		// ---------------------
		// Matrix multiplication
		// ---------------------
		Matrix3f operator*( const Matrix3f &m ) const
		{
			Matrix3f mf( Math::NO_INIT );

			mf.x[0] = x[0] * m.x[0] + x[3] * m.x[1] + x[6] * m.x[2];
			mf.x[1] = x[1] * m.x[0] + x[4] * m.x[1] + x[7] * m.x[2];
			mf.x[2] = x[2] * m.x[0] + x[5] * m.x[1] + x[8] * m.x[2];

			mf.x[3] = x[0] * m.x[3] + x[3] * m.x[4] + x[6] * m.x[5];
			mf.x[4] = x[1] * m.x[3] + x[4] * m.x[4] + x[7] * m.x[5];
			mf.x[5] = x[2] * m.x[3] + x[5] * m.x[4] + x[8] * m.x[5];

			mf.x[6] = x[0] * m.x[6] + x[3] * m.x[7] + x[6] * m.x[8];
			mf.x[7] = x[1] * m.x[6] + x[4] * m.x[7] + x[7] * m.x[8];
			mf.x[8] = x[2] * m.x[6] + x[5] * m.x[7] + x[8] * m.x[8];

			return mf;
		}

		Matrix3f operator*( const float f ) const
		{
			Matrix3f m( *this );

			m.x[0]  *= f; m.x[1]  *= f; m.x[2]  *= f; m.x[3]  *= f;
			m.x[4]  *= f; m.x[5]  *= f; m.x[6]  *= f; m.x[7]  *= f;
			m.x[8]  *= f;

			return m;
		}

		// ----------------------------
		// Vector-Matrix multiplication
		// ----------------------------
		Vec3f operator*( const Vec3f &v ) const
		{
			return Vec3f( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0],
				v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1],
				v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] );
		}


		// ---------------
		// Transformations
		// ---------------
		void rotate( const float x, const float y, const float z )
		{
			*this = RotMat( x, y, z ) * *this;
		}

		// ---------------
		// Other
		// ---------------

		Matrix3f transposed() const
		{
			Matrix3f m( *this );

			for( unsigned int y = 0; y < 3; ++y )
			{
				for( unsigned int x = y + 1; x < 3; ++x )
				{
					float tmp = m.c[x][y];
					m.c[x][y] = m.c[y][x];
					m.c[y][x] = tmp;
				}
			}

			return m;
		}

		Matrix3f adjugated() const
		{
			Matrix3f m(Math::NO_INIT);

			m.c[0][0] = c[1][1] * c[2][2] - c[1][2] * c[2][1]; m.c[0][1] = c[0][2] * c[2][1] - c[0][1] * c[2][2]; m.c[0][2] = c[0][1] * c[1][2] - c[0][2] * c[1][1];
			m.c[1][0] = c[1][2] * c[2][0] - c[1][0] * c[2][2]; m.c[1][1] = c[0][0] * c[2][2] - c[0][2] * c[2][0]; m.c[1][2] = c[0][2] * c[1][0] - c[0][0] * c[1][2];
			m.c[2][0] = c[1][0] * c[2][1] - c[1][1] * c[2][0]; m.c[2][1] = c[0][1] * c[2][0] - c[0][0] * c[2][1]; m.c[2][2] = c[0][0] * c[1][1] - c[0][1] * c[1][0];

			return m;
		}

		float determinant() const
		{
			return
				c[0][0]*c[1][1]*c[2][2] + c[0][1]*c[1][2]*c[2][0] + c[0][2]*c[1][0]*c[2][1]
			- c[0][2]*c[1][1]*c[2][0] - c[0][1]*c[1][0]*c[2][2] - c[0][0]*c[1][2]*c[2][1];
		}

		// Note: if you have a rotation matrix, you should use transposed() instead, as that is cheaper, but equal for this type of matrix
		Matrix3f inverted() const
		{
			float d = determinant();
			if( d == 0 )
				return Matrix3f();
			d = 1.0f / d;

			return this->adjugated() * d;
		}


		Vec3f getRot(bool inDegree = true) const
		{
			Vec3f rot;

			rot.x = asinf(-x[7]);

			// Special case: Cos[x] == 0 (when Sin[x] is +/-1)
			float f = fabsf(x[7]);
			if (f > 0.999f && f < 1.001f)
			{
				// Pin arbitrarily one of y or z to zero
				// Mathematical equivalent of gimbal lock
				rot.y = 0;

				// Now: Cos[x] = 0, Sin[x] = +/-1, Cos[y] = 1, Sin[y] = 0
				// => m[0][0] = Cos[z] and m[1][0] = Sin[z]
				rot.z = atan2f(-x[3], x[0]);
			}
			// Standard case
			else
			{
				rot.y = atan2f(x[6], x[8]);
				rot.z = atan2f(x[1], x[4]);
			}

			if (inDegree)
				radToDeg(rot);

			return rot;
		}
	};

	/**
	* \brief Matrix class with 4x4 layout (transformation matrices)
	*/
	class Matrix4f
	{
	public:

		union
		{
			float c[4][4];	// Column major order for OpenGL: c[column][row]
			float x[16];
		};

		// --------------
		// Static methods
		// --------------
		static Matrix4f TransMat( float x, float y, float z )
		{
			Matrix4f m;

			m.c[3][0] = x;
			m.c[3][1] = y;
			m.c[3][2] = z;

			return m;
		}

		static Matrix4f ScaleMat( float x, float y, float z )
		{
			Matrix4f m;

			m.c[0][0] = x;
			m.c[1][1] = y;
			m.c[2][2] = z;

			return m;
		}

		static Matrix4f RotMat( float x, float y, float z )
		{
			// Rotation order: YXZ [* Vector]
			return Matrix4f( Quaternion( x, y, z ) );
		}

		static Matrix4f RotMat( Vec3f axis, float angle )
		{
			axis = axis * sinf( angle * 0.5f );
			return Matrix4f( Quaternion( axis.x, axis.y, axis.z, cosf( angle * 0.5f ) ) );
		}

		static Matrix4f PerspectiveMat( float l, float r, float b, float t, float n, float f )
		{
			Matrix4f m;

			m.x[0] = 2 * n / (r - l);
			m.x[5] = 2 * n / (t - b);
			m.x[8] = (r + l) / (r - l);
			m.x[9] = (t + b) / (t - b);
			m.x[10] = -(f + n) / (f - n);
			m.x[11] = -1;
			m.x[14] = -2 * f * n / (f - n);
			m.x[15] = 0;

			return m;
		}

		static Matrix4f OrthoMat( float l, float r, float b, float t, float n, float f )
		{
			Matrix4f m;

			m.x[0] = 2 / (r - l);
			m.x[5] = 2 / (t - b);
			m.x[10] = -2 / (f - n);
			m.x[12] = -(r + l) / (r - l);
			m.x[13] = -(t + b) / (t - b);
			m.x[14] = -(f + n) / (f - n);

			return m;
		}

		static void fastMult43( Matrix4f &dst, const Matrix4f &m1, const Matrix4f &m2 )
		{
			// Note: dst may not be the same as m1 or m2

			float *dstx = dst.x;
			const float *m1x = m1.x;
			const float *m2x = m2.x;

			dstx[0] = m1x[0] * m2x[0] + m1x[4] * m2x[1] + m1x[8] * m2x[2];
			dstx[1] = m1x[1] * m2x[0] + m1x[5] * m2x[1] + m1x[9] * m2x[2];
			dstx[2] = m1x[2] * m2x[0] + m1x[6] * m2x[1] + m1x[10] * m2x[2];
			dstx[3] = 0.0f;

			dstx[4] = m1x[0] * m2x[4] + m1x[4] * m2x[5] + m1x[8] * m2x[6];
			dstx[5] = m1x[1] * m2x[4] + m1x[5] * m2x[5] + m1x[9] * m2x[6];
			dstx[6] = m1x[2] * m2x[4] + m1x[6] * m2x[5] + m1x[10] * m2x[6];
			dstx[7] = 0.0f;

			dstx[8] = m1x[0] * m2x[8] + m1x[4] * m2x[9] + m1x[8] * m2x[10];
			dstx[9] = m1x[1] * m2x[8] + m1x[5] * m2x[9] + m1x[9] * m2x[10];
			dstx[10] = m1x[2] * m2x[8] + m1x[6] * m2x[9] + m1x[10] * m2x[10];
			dstx[11] = 0.0f;

			dstx[12] = m1x[0] * m2x[12] + m1x[4] * m2x[13] + m1x[8] * m2x[14] + m1x[12] * m2x[15];
			dstx[13] = m1x[1] * m2x[12] + m1x[5] * m2x[13] + m1x[9] * m2x[14] + m1x[13] * m2x[15];
			dstx[14] = m1x[2] * m2x[12] + m1x[6] * m2x[13] + m1x[10] * m2x[14] + m1x[14] * m2x[15];
			dstx[15] = 1.0f;
		}

		// ------------
		// Constructors
		// ------------
		Matrix4f()
		{
			c[0][0] = 1; c[1][0] = 0; c[2][0] = 0; c[3][0] = 0;
			c[0][1] = 0; c[1][1] = 1; c[2][1] = 0; c[3][1] = 0;
			c[0][2] = 0; c[1][2] = 0; c[2][2] = 1; c[3][2] = 0;
			c[0][3] = 0; c[1][3] = 0; c[2][3] = 0; c[3][3] = 1;
		}

		explicit Matrix4f( Math::NoInitHint )
		{
			// Constructor without default initialization
		}

		Matrix4f( const float *floatArray16 )
		{
			for( unsigned int i = 0; i < 4; ++i )
			{
				for( unsigned int j = 0; j < 4; ++j )
				{
					c[i][j] = floatArray16[i * 4 + j];
				}
			}
		}

		Matrix4f( const Quaternion &q )
		{
			// Calculate coefficients
			float x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
			float xx = q.x * x2,  xy = q.x * y2,  xz = q.x * z2;
			float yy = q.y * y2,  yz = q.y * z2,  zz = q.z * z2;
			float wx = q.w * x2,  wy = q.w * y2,  wz = q.w * z2;

			c[0][0] = 1 - (yy + zz);  c[1][0] = xy - wz;
			c[2][0] = xz + wy;        c[3][0] = 0;
			c[0][1] = xy + wz;        c[1][1] = 1 - (xx + zz);
			c[2][1] = yz - wx;        c[3][1] = 0;
			c[0][2] = xz - wy;        c[1][2] = yz + wx;
			c[2][2] = 1 - (xx + yy);  c[3][2] = 0;
			c[0][3] = 0;              c[1][3] = 0;
			c[2][3] = 0;              c[3][3] = 1;
		}

		Matrix4f( const Matrix3f& m3)
		{
			c[0][0] = m3.c[0][0]; c[1][0] = m3.c[1][0]; c[2][0] = m3.c[2][0]; c[3][0] = 0;
			c[0][1] = m3.c[0][1]; c[1][1] = m3.c[1][1]; c[2][1] = m3.c[2][1]; c[3][1] = 0;
			c[0][2] = m3.c[0][2]; c[1][2] = m3.c[1][2]; c[2][2] = m3.c[2][2]; c[3][2] = 0;
			c[0][3] = 0;		  c[1][3] = 0;			c[2][3] = 0;		  c[3][3] = 1;
		}

		// ----------
		// Matrix sum
		// ----------
		Matrix4f operator+( const Matrix4f &m ) const
		{
			Matrix4f mf( Math::NO_INIT );

			mf.x[0] = x[0] + m.x[0];
			mf.x[1] = x[1] + m.x[1];
			mf.x[2] = x[2] + m.x[2];
			mf.x[3] = x[3] + m.x[3];
			mf.x[4] = x[4] + m.x[4];
			mf.x[5] = x[5] + m.x[5];
			mf.x[6] = x[6] + m.x[6];
			mf.x[7] = x[7] + m.x[7];
			mf.x[8] = x[8] + m.x[8];
			mf.x[9] = x[9] + m.x[9];
			mf.x[10] = x[10] + m.x[10];
			mf.x[11] = x[11] + m.x[11];
			mf.x[12] = x[12] + m.x[12];
			mf.x[13] = x[13] + m.x[13];
			mf.x[14] = x[14] + m.x[14];
			mf.x[15] = x[15] + m.x[15];

			return mf;
		}

		Matrix4f &operator+=( const Matrix4f &m )
		{
			return *this = *this + m;
		}

		// ---------------------
		// Matrix multiplication
		// ---------------------
		Matrix4f operator*( const Matrix4f &m ) const
		{
			Matrix4f mf( Math::NO_INIT );

			mf.x[0] = x[0] * m.x[0] + x[4] * m.x[1] + x[8] * m.x[2] + x[12] * m.x[3];
			mf.x[1] = x[1] * m.x[0] + x[5] * m.x[1] + x[9] * m.x[2] + x[13] * m.x[3];
			mf.x[2] = x[2] * m.x[0] + x[6] * m.x[1] + x[10] * m.x[2] + x[14] * m.x[3];
			mf.x[3] = x[3] * m.x[0] + x[7] * m.x[1] + x[11] * m.x[2] + x[15] * m.x[3];

			mf.x[4] = x[0] * m.x[4] + x[4] * m.x[5] + x[8] * m.x[6] + x[12] * m.x[7];
			mf.x[5] = x[1] * m.x[4] + x[5] * m.x[5] + x[9] * m.x[6] + x[13] * m.x[7];
			mf.x[6] = x[2] * m.x[4] + x[6] * m.x[5] + x[10] * m.x[6] + x[14] * m.x[7];
			mf.x[7] = x[3] * m.x[4] + x[7] * m.x[5] + x[11] * m.x[6] + x[15] * m.x[7];

			mf.x[8] = x[0] * m.x[8] + x[4] * m.x[9] + x[8] * m.x[10] + x[12] * m.x[11];
			mf.x[9] = x[1] * m.x[8] + x[5] * m.x[9] + x[9] * m.x[10] + x[13] * m.x[11];
			mf.x[10] = x[2] * m.x[8] + x[6] * m.x[9] + x[10] * m.x[10] + x[14] * m.x[11];
			mf.x[11] = x[3] * m.x[8] + x[7] * m.x[9] + x[11] * m.x[10] + x[15] * m.x[11];

			mf.x[12] = x[0] * m.x[12] + x[4] * m.x[13] + x[8] * m.x[14] + x[12] * m.x[15];
			mf.x[13] = x[1] * m.x[12] + x[5] * m.x[13] + x[9] * m.x[14] + x[13] * m.x[15];
			mf.x[14] = x[2] * m.x[12] + x[6] * m.x[13] + x[10] * m.x[14] + x[14] * m.x[15];
			mf.x[15] = x[3] * m.x[12] + x[7] * m.x[13] + x[11] * m.x[14] + x[15] * m.x[15];

			return mf;
		}

		Matrix4f operator*( const float f ) const
		{
			Matrix4f m( *this );

			m.x[0]  *= f; m.x[1]  *= f; m.x[2]  *= f; m.x[3]  *= f;
			m.x[4]  *= f; m.x[5]  *= f; m.x[6]  *= f; m.x[7]  *= f;
			m.x[8]  *= f; m.x[9]  *= f; m.x[10] *= f; m.x[11] *= f;
			m.x[12] *= f; m.x[13] *= f; m.x[14] *= f; m.x[15] *= f;

			return m;
		}

		// ----------------------------
		// Vector-Matrix multiplication
		// ----------------------------
		Vec3f operator*( const Vec3f &v ) const
		{
			return Vec3f( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + c[3][0],
				v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + c[3][1],
				v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + c[3][2] );
		}

		Vec3f mult33Vec( const Vec3f &v ) const
		{
			return Vec3f( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0],
				v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1],
				v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] );
		}

		// ---------------
		// Transformations
		// ---------------
		void translate( const float x, const float y, const float z )
		{
			*this = TransMat( x, y, z ) * *this;
		}

		void scale( const float x, const float y, const float z )
		{
			*this = ScaleMat( x, y, z ) * *this;
		}

		void rotate( const float x, const float y, const float z )
		{
			*this = RotMat( x, y, z ) * *this;
		}

		// ---------------
		// Other
		// ---------------

		Matrix4f transposed() const
		{
			Matrix4f m( *this );

			for( unsigned int y = 0; y < 4; ++y )
			{
				for( unsigned int x = y + 1; x < 4; ++x )
				{
					float tmp = m.c[x][y];
					m.c[x][y] = m.c[y][x];
					m.c[y][x] = tmp;
				}
			}

			return m;
		}

		float determinant() const
		{
			return
				c[0][3]*c[1][2]*c[2][1]*c[3][0] - c[0][2]*c[1][3]*c[2][1]*c[3][0] - c[0][3]*c[1][1]*c[2][2]*c[3][0] + c[0][1]*c[1][3]*c[2][2]*c[3][0] +
				c[0][2]*c[1][1]*c[2][3]*c[3][0] - c[0][1]*c[1][2]*c[2][3]*c[3][0] - c[0][3]*c[1][2]*c[2][0]*c[3][1] + c[0][2]*c[1][3]*c[2][0]*c[3][1] +
				c[0][3]*c[1][0]*c[2][2]*c[3][1] - c[0][0]*c[1][3]*c[2][2]*c[3][1] - c[0][2]*c[1][0]*c[2][3]*c[3][1] + c[0][0]*c[1][2]*c[2][3]*c[3][1] +
				c[0][3]*c[1][1]*c[2][0]*c[3][2] - c[0][1]*c[1][3]*c[2][0]*c[3][2] - c[0][3]*c[1][0]*c[2][1]*c[3][2] + c[0][0]*c[1][3]*c[2][1]*c[3][2] +
				c[0][1]*c[1][0]*c[2][3]*c[3][2] - c[0][0]*c[1][1]*c[2][3]*c[3][2] - c[0][2]*c[1][1]*c[2][0]*c[3][3] + c[0][1]*c[1][2]*c[2][0]*c[3][3] +
				c[0][2]*c[1][0]*c[2][1]*c[3][3] - c[0][0]*c[1][2]*c[2][1]*c[3][3] - c[0][1]*c[1][0]*c[2][2]*c[3][3] + c[0][0]*c[1][1]*c[2][2]*c[3][3];
		}

		Matrix4f inverted() const
		{
			Matrix4f m( Math::NO_INIT );

			float d = determinant();
			if( d == 0 ) return m;
			d = 1.0f / d;

			m.c[0][0] = d * (c[1][2]*c[2][3]*c[3][1] - c[1][3]*c[2][2]*c[3][1] + c[1][3]*c[2][1]*c[3][2] - c[1][1]*c[2][3]*c[3][2] - c[1][2]*c[2][1]*c[3][3] + c[1][1]*c[2][2]*c[3][3]);
			m.c[0][1] = d * (c[0][3]*c[2][2]*c[3][1] - c[0][2]*c[2][3]*c[3][1] - c[0][3]*c[2][1]*c[3][2] + c[0][1]*c[2][3]*c[3][2] + c[0][2]*c[2][1]*c[3][3] - c[0][1]*c[2][2]*c[3][3]);
			m.c[0][2] = d * (c[0][2]*c[1][3]*c[3][1] - c[0][3]*c[1][2]*c[3][1] + c[0][3]*c[1][1]*c[3][2] - c[0][1]*c[1][3]*c[3][2] - c[0][2]*c[1][1]*c[3][3] + c[0][1]*c[1][2]*c[3][3]);
			m.c[0][3] = d * (c[0][3]*c[1][2]*c[2][1] - c[0][2]*c[1][3]*c[2][1] - c[0][3]*c[1][1]*c[2][2] + c[0][1]*c[1][3]*c[2][2] + c[0][2]*c[1][1]*c[2][3] - c[0][1]*c[1][2]*c[2][3]);
			m.c[1][0] = d * (c[1][3]*c[2][2]*c[3][0] - c[1][2]*c[2][3]*c[3][0] - c[1][3]*c[2][0]*c[3][2] + c[1][0]*c[2][3]*c[3][2] + c[1][2]*c[2][0]*c[3][3] - c[1][0]*c[2][2]*c[3][3]);
			m.c[1][1] = d * (c[0][2]*c[2][3]*c[3][0] - c[0][3]*c[2][2]*c[3][0] + c[0][3]*c[2][0]*c[3][2] - c[0][0]*c[2][3]*c[3][2] - c[0][2]*c[2][0]*c[3][3] + c[0][0]*c[2][2]*c[3][3]);
			m.c[1][2] = d * (c[0][3]*c[1][2]*c[3][0] - c[0][2]*c[1][3]*c[3][0] - c[0][3]*c[1][0]*c[3][2] + c[0][0]*c[1][3]*c[3][2] + c[0][2]*c[1][0]*c[3][3] - c[0][0]*c[1][2]*c[3][3]);
			m.c[1][3] = d * (c[0][2]*c[1][3]*c[2][0] - c[0][3]*c[1][2]*c[2][0] + c[0][3]*c[1][0]*c[2][2] - c[0][0]*c[1][3]*c[2][2] - c[0][2]*c[1][0]*c[2][3] + c[0][0]*c[1][2]*c[2][3]);
			m.c[2][0] = d * (c[1][1]*c[2][3]*c[3][0] - c[1][3]*c[2][1]*c[3][0] + c[1][3]*c[2][0]*c[3][1] - c[1][0]*c[2][3]*c[3][1] - c[1][1]*c[2][0]*c[3][3] + c[1][0]*c[2][1]*c[3][3]);
			m.c[2][1] = d * (c[0][3]*c[2][1]*c[3][0] - c[0][1]*c[2][3]*c[3][0] - c[0][3]*c[2][0]*c[3][1] + c[0][0]*c[2][3]*c[3][1] + c[0][1]*c[2][0]*c[3][3] - c[0][0]*c[2][1]*c[3][3]);
			m.c[2][2] = d * (c[0][1]*c[1][3]*c[3][0] - c[0][3]*c[1][1]*c[3][0] + c[0][3]*c[1][0]*c[3][1] - c[0][0]*c[1][3]*c[3][1] - c[0][1]*c[1][0]*c[3][3] + c[0][0]*c[1][1]*c[3][3]);
			m.c[2][3] = d * (c[0][3]*c[1][1]*c[2][0] - c[0][1]*c[1][3]*c[2][0] - c[0][3]*c[1][0]*c[2][1] + c[0][0]*c[1][3]*c[2][1] + c[0][1]*c[1][0]*c[2][3] - c[0][0]*c[1][1]*c[2][3]);
			m.c[3][0] = d * (c[1][2]*c[2][1]*c[3][0] - c[1][1]*c[2][2]*c[3][0] - c[1][2]*c[2][0]*c[3][1] + c[1][0]*c[2][2]*c[3][1] + c[1][1]*c[2][0]*c[3][2] - c[1][0]*c[2][1]*c[3][2]);
			m.c[3][1] = d * (c[0][1]*c[2][2]*c[3][0] - c[0][2]*c[2][1]*c[3][0] + c[0][2]*c[2][0]*c[3][1] - c[0][0]*c[2][2]*c[3][1] - c[0][1]*c[2][0]*c[3][2] + c[0][0]*c[2][1]*c[3][2]);
			m.c[3][2] = d * (c[0][2]*c[1][1]*c[3][0] - c[0][1]*c[1][2]*c[3][0] - c[0][2]*c[1][0]*c[3][1] + c[0][0]*c[1][2]*c[3][1] + c[0][1]*c[1][0]*c[3][2] - c[0][0]*c[1][1]*c[3][2]);
			m.c[3][3] = d * (c[0][1]*c[1][2]*c[2][0] - c[0][2]*c[1][1]*c[2][0] + c[0][2]*c[1][0]*c[2][1] - c[0][0]*c[1][2]*c[2][1] - c[0][1]*c[1][0]*c[2][2] + c[0][0]*c[1][1]*c[2][2]);

			return m;
		}

		void decompose( Vec3f &trans, Vec3f &rot, Vec3f &scale ) const
		{
			// Getting translation is trivial
			trans = Vec3f( c[3][0], c[3][1], c[3][2] );

			// Scale is length of columns
			scale.x = sqrtf( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
			scale.y = sqrtf( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
			scale.z = sqrtf( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );

			if( scale.x == 0 || scale.y == 0 || scale.z == 0 ) return;

			// Detect negative scale with determinant and flip one arbitrary axis
			if( determinant() < 0 ) scale.x = -scale.x;

			// Combined rotation matrix YXZ
			//
			// Cos[y]*Cos[z]+Sin[x]*Sin[y]*Sin[z]   Cos[z]*Sin[x]*Sin[y]-Cos[y]*Sin[z]  Cos[x]*Sin[y]
			// Cos[x]*Sin[z]                        Cos[x]*Cos[z]                       -Sin[x]
			// -Cos[z]*Sin[y]+Cos[y]*Sin[x]*Sin[z]  Cos[y]*Cos[z]*Sin[x]+Sin[y]*Sin[z]  Cos[x]*Cos[y]

			rot.x = asinf( -c[2][1] / scale.z );

			// Special case: Cos[x] == 0 (when Sin[x] is +/-1)
			float f = fabsf( c[2][1] / scale.z );
			if( f > 0.999f && f < 1.001f )
			{
				// Pin arbitrarily one of y or z to zero
				// Mathematical equivalent of gimbal lock
				rot.y = 0;

				// Now: Cos[x] = 0, Sin[x] = +/-1, Cos[y] = 1, Sin[y] = 0
				// => m[0][0] = Cos[z] and m[1][0] = Sin[z]
				rot.z = atan2f( -c[1][0] / scale.y, c[0][0] / scale.x );
			}
			// Standard case
			else
			{
				rot.y = atan2f( c[2][0] / scale.z, c[2][2] / scale.z );
				rot.z = atan2f( c[0][1] / scale.x, c[1][1] / scale.y );
			}
		}

		Vec3f getTrans() const
		{
			return Vec3f( c[3][0], c[3][1], c[3][2] );
		}

		Vec3f getScale() const
		{
			Vec3f scale;
			// Scale is length of columns
			scale.x = sqrtf( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
			scale.y = sqrtf( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
			scale.z = sqrtf( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );
			return scale;
		}
	};


	/**
	* \brief Plane class (e.g. for representing the floor plane)
	*/
	class Plane
	{
	public:
		Vec3f normal;
		float dist;

		// ------------
		// Constructors
		// ------------
		Plane()
		{
			normal.x = 0; normal.y = 0; normal.z = 0; dist = 0;
		};

		explicit Plane( const float a, const float b, const float c, const float d )
		{
			normal = Vec3f( a, b, c );
			float invLen = 1.0f / normal.length();
			normal *= invLen;	// Normalize
			dist = d * invLen;
		}

		Plane( const Vec3f &v0, const Vec3f &v1, const Vec3f &v2 )
		{
			normal = v1 - v0;
			normal = normal.cross( v2 - v0 );
			normal.normalize();
			dist = -normal.dot( v0 );
		}

		// ----------------
		// Other operations
		// ----------------
		float distToPoint( const Vec3f &v ) const
		{
			return normal.dot( v ) + dist;
		}
	};


	/**
	* \brief Test whether a ray intersects with a triangle
	*/
	static inline bool rayTriangleIntersection( const Vec3f &rayOrig, const Vec3f &rayDir,
		const Vec3f &vert0, const Vec3f &vert1, const Vec3f &vert2,
		Vec3f &intsPoint )
	{
		// Idea: Tomas Moeller and Ben Trumbore
		// in Fast, Minimum Storage Ray/Triangle Intersection

		// Find vectors for two edges sharing vert0
		Vec3f edge1 = vert1 - vert0;
		Vec3f edge2 = vert2 - vert0;

		// Begin calculating determinant - also used to calculate U parameter
		Vec3f pvec = rayDir.cross( edge2 );

		// If determinant is near zero, ray lies in plane of triangle
		float det = edge1.dot( pvec );


		// *** Culling branch ***
		/*if( det < Math::Epsilon )return false;

		// Calculate distance from vert0 to ray origin
		Vec3f tvec = rayOrig - vert0;

		// Calculate U parameter and test bounds
		float u = tvec.dot( pvec );
		if (u < 0 || u > det ) return false;

		// Prepare to test V parameter
		Vec3f qvec = tvec.cross( edge1 );

		// Calculate V parameter and test bounds
		float v = rayDir.dot( qvec );
		if (v < 0 || u + v > det ) return false;

		// Calculate t, scale parameters, ray intersects triangle
		float t = edge2.dot( qvec ) / det;*/


		// *** Non-culling branch ***
		if( det > -Math::Epsilon && det < Math::Epsilon ) return 0;
		float inv_det = 1.0f / det;

		// Calculate distance from vert0 to ray origin
		Vec3f tvec = rayOrig - vert0;

		// Calculate U parameter and test bounds
		float u = tvec.dot( pvec ) * inv_det;
		if( u < 0.0f || u > 1.0f ) return 0;

		// Prepare to test V parameter
		Vec3f qvec = tvec.cross( edge1 );

		// Calculate V parameter and test bounds
		float v = rayDir.dot( qvec ) * inv_det;
		if( v < 0.0f || u + v > 1.0f ) return 0;

		// Calculate t, ray intersects triangle
		float t = edge2.dot( qvec ) * inv_det;


		// Calculate intersection point and test ray length and direction
		intsPoint = rayOrig + rayDir * t;
		Vec3f vec = intsPoint - rayOrig;
		if( vec.dot( rayDir ) < 0 || vec.length() > rayDir.length() ) return false;

		return true;
	}

	/**
	* \brief Test whether a ray intersects with an AABB bounding box
	*/
	static inline bool rayAABBIntersection( const Vec3f &rayOrig, const Vec3f &rayDir,
		const Vec3f &mins, const Vec3f &maxs )
	{
		// SLAB based optimized ray/AABB intersection routine
		// Idea taken from http://ompf.org/ray/

		float l1 = (mins.x - rayOrig.x) / rayDir.x;
		float l2 = (maxs.x - rayOrig.x) / rayDir.x;
		float lmin = minf( l1, l2 );
		float lmax = maxf( l1, l2 );

		l1 = (mins.y - rayOrig.y) / rayDir.y;
		l2 = (maxs.y - rayOrig.y) / rayDir.y;
		lmin = maxf( minf( l1, l2 ), lmin );
		lmax = minf( maxf( l1, l2 ), lmax );

		l1 = (mins.z - rayOrig.z) / rayDir.z;
		l2 = (maxs.z - rayOrig.z) / rayDir.z;
		lmin = maxf( minf( l1, l2 ), lmin );
		lmax = minf( maxf( l1, l2 ), lmax );

		if( (lmax >= 0.0f) & (lmax >= lmin) )
		{
			// Consider length
			const Vec3f rayDest = rayOrig + rayDir;
			Vec3f rayMins( minf( rayDest.x, rayOrig.x), minf( rayDest.y, rayOrig.y ), minf( rayDest.z, rayOrig.z ) );
			Vec3f rayMaxs( maxf( rayDest.x, rayOrig.x), maxf( rayDest.y, rayOrig.y ), maxf( rayDest.z, rayOrig.z ) );
			return
				(rayMins.x < maxs.x) && (rayMaxs.x > mins.x) &&
				(rayMins.y < maxs.y) && (rayMaxs.y > mins.y) &&
				(rayMins.z < maxs.z) && (rayMaxs.z > mins.z);
		}
		else
			return false;
	}


	/**
	* \brief Calculate the nearest distance of a point to a AABB bounding box
	*/
	static inline float nearestDistToAABB( const Vec3f &pos, const Vec3f &mins, const Vec3f &maxs )
	{
		const Vec3f center = (mins + maxs) * 0.5f;
		const Vec3f extent = (maxs - mins) * 0.5f;

		Vec3f nearestVec;
		nearestVec.x = maxf( 0, fabsf( pos.x - center.x ) - extent.x );
		nearestVec.y = maxf( 0, fabsf( pos.y - center.y ) - extent.y );
		nearestVec.z = maxf( 0, fabsf( pos.z - center.z ) - extent.z );

		return nearestVec.length();
	}

	/**
	* \brief Calculate the closest point on a ray or line segment to a given point
	*/
	static inline Vec3f closestPointFromPointToRay(const Vec3f& point, const Vec3f& rayOrigin, const Vec3f& rayDir, bool withinLineSegmentOnly = false)
	{
		float rayLengthSquared = rayDir.x * rayDir.x + rayDir.y * rayDir.y + rayDir.z * rayDir.z;

		if (rayLengthSquared > 0)
		{
			float u = (point - rayOrigin).dot(rayDir) / rayLengthSquared;

			if( withinLineSegmentOnly)
			{
				if (u < 0.0f)
					return rayOrigin; // intersect is before origin, so take this one instead
				else if (u > 1.0f)
					return rayOrigin+rayDir; // intersect is after line end so take this one instead
			}

			// Default case: project the intersect on the ray
			Vec3f intersect;
			intersect.x = rayOrigin.x + u * rayDir.x;
			intersect.y = rayOrigin.y + u * rayDir.y;
			intersect.z = rayOrigin.z + u * rayDir.z;
			return intersect;
		}
		// else ray has length 0, so the intersect is the original point
		return point;
	}

	/**
	* \brief Calculate the distance of a point to a ray or line
	*/
	static inline float distancePointToRay(const Vec3f& point, const Vec3f& rayOrigin, const Vec3f& rayDir, bool withinLineSegmentOnly = false)
	{
		Vec3f intersect = closestPointFromPointToRay(point, rayOrigin, rayDir, withinLineSegmentOnly);

		return (point - intersect).length();
	}

	/**
	* \brief SkeletonJointPosition contains the position (Vec3f) and tracking confidence for a skeleton joint
	*/
	struct SkeletonJointPosition
	{
		SkeletonJointPosition() : m_confidence(0) {}
		SkeletonJointPosition(Fubi::Math::NoInitHint noInitHint) : m_position(noInitHint) {}
		SkeletonJointPosition(const Vec3f& position, float confidence)
			: m_position(position), m_confidence(confidence) {}
		SkeletonJointPosition(float x, float y, float z, float confidence)
			: m_position(x, y, z), m_confidence(confidence)	{}
		float m_confidence;
		Vec3f m_position;
	};

	/**
	* \brief SkeletonJointOrientation contains the orientation (Matrix3f) and tracking confidence for a skeleton joint
	*/
	struct SkeletonJointOrientation
	{
		SkeletonJointOrientation() : m_confidence(0), m_orientation() {}
		SkeletonJointOrientation(Fubi::Math::NoInitHint noInitHint) : m_orientation(noInitHint) {}
		SkeletonJointOrientation(const Matrix3f& rotMat, float confidence)
			: m_orientation(rotMat), m_confidence(confidence)	{}
		SkeletonJointOrientation(float* array9, float confidence)
			: m_orientation(array9), m_confidence(confidence)	{}
		SkeletonJointOrientation(const Quaternion& quaternion, float confidence)
			: m_orientation(quaternion), m_confidence(confidence)	{}
		float m_confidence;
		Matrix3f m_orientation;
	};

	/**
	* \brief BodyMeasurementDistance contains the distance and tracking confidence for a body measurement
	*/
	struct BodyMeasurementDistance
	{
		BodyMeasurementDistance() : m_confidence(0), m_dist(0) {}
		BodyMeasurementDistance(Fubi::Math::NoInitHint noInit) {}
		BodyMeasurementDistance(float distance, float confidence) : m_confidence(confidence), m_dist(distance) {}
		float m_dist;
		float m_confidence;
	};

	/**
	* \brief Abstract tracking data with global and local positions/orientations and a timestamp
	*		 Only the subclassed UserTrackingData and FingerTrackingData initialize the arrays with the correct size
	*/
	struct TrackingData
	{
		/**
		* \brief Dummy constructor with zero joints, only for being able to put TrackingData into a vector
		*/
		TrackingData() : timeStamp(0), numJoints(0), jointPositions(0x0),
			localJointPositions(0x0), jointOrientations(0x0), localJointOrientations(0x0)
		{}
		/**
		* \brief Copy constructor needs to copy the data
		*/
		TrackingData(const TrackingData& other) : timeStamp(other.timeStamp), numJoints(other.numJoints)
		{
			// Create arrays without initializations
			jointPositions = (SkeletonJointPosition*) operator new[](numJoints * sizeof(SkeletonJointPosition));
			localJointPositions = (SkeletonJointPosition*) operator new[](numJoints * sizeof(SkeletonJointPosition));
			jointOrientations = (SkeletonJointOrientation*) operator new[](numJoints * sizeof(SkeletonJointOrientation));
			localJointOrientations = (SkeletonJointOrientation*) operator new[](numJoints * sizeof(SkeletonJointOrientation));
			// Now for the actual initialization
			for (int i = 0; i < numJoints; ++i)
			{
				jointPositions[i] = other.jointPositions[i];
				localJointPositions[i] = other.localJointPositions[i];
				jointOrientations[i] = other.jointOrientations[i];
				localJointOrientations[i] = other.localJointOrientations[i];
			}
		}

		SkeletonJointPosition* jointPositions;
		SkeletonJointPosition* localJointPositions;
		SkeletonJointOrientation* jointOrientations;
		SkeletonJointOrientation* localJointOrientations;
		double timeStamp;
		int numJoints;
		virtual ~TrackingData()
		{
			delete[] jointPositions;
			delete[] localJointPositions;
			delete[] jointOrientations;
			delete[] localJointOrientations;
		}
		// getJointName only works in subclasses
		virtual const char* getJointName(unsigned int jointIndex) const { return ""; };
	protected:
		/**
		* \brief Constructor initializes all data with zeros, orientations with identity matrices respectively (calling the standard contructors)
		*/
		TrackingData(unsigned int numJoints, double timeStamp = 0) : timeStamp(timeStamp), numJoints(numJoints)
		{
			jointPositions = new SkeletonJointPosition[numJoints];
			localJointPositions = new SkeletonJointPosition[numJoints];
			jointOrientations = new SkeletonJointOrientation[numJoints];
			localJointOrientations = new SkeletonJointOrientation[numJoints];
		}
	};

	/**
	* \brief User Tracking data with global and local positions/orientations and a timestamp
	*/
	struct UserTrackingData : public TrackingData
	{
		/**
		* \brief Constructor initializes all data with zeros, orientations with identity matrices respectively (calling the standard contructors)
		*/
		UserTrackingData() : TrackingData(SkeletonJoint::NUM_JOINTS) {}
		/**
		* \brief Copy constructor needs to copy the data
		*/
		UserTrackingData(const UserTrackingData& other) : TrackingData(other) {}
		virtual const char* getJointName(unsigned int jointIndex) const { return Fubi::getJointName((SkeletonJoint::Joint) jointIndex);  }
	};

	/**
	* \brief Empty user tracking data to be returned on failure or missing data
	*/
	const UserTrackingData EmptyUserTrackingData;

	/**
	* \brief Finger Tracking data with global and local positions/orientations, the current finger count and a timestamp
	*/
	struct FingerTrackingData : TrackingData
	{
		/**
		* \brief Constructor initializes all data with zeros, orientations with identity matrices respectively (calling the standard contructors)
		*/
		FingerTrackingData() : fingerCount(0), TrackingData(SkeletonHandJoint::NUM_JOINTS) {}
		/**
		* \brief Copy constructor needs to copy the data
		*/
		FingerTrackingData(const FingerTrackingData& other) : fingerCount(other.fingerCount), TrackingData(other) {}
		virtual const char* getJointName(unsigned int jointIndex) const { return Fubi::getHandJointName((SkeletonHandJoint::Joint) jointIndex); }
		int fingerCount;
	};

	/**
	* \brief Empty finger tracking data to be returned on failure or missing data
	*/
	const FingerTrackingData EmptyFingerTrackingData;

	/**
	* \brief Data used and provided by TemplateRecognizers
	*/
	struct TemplateData
	{
		TemplateData() : m_joint(Fubi::SkeletonJoint::NUM_JOINTS), m_relJoint(Fubi::SkeletonJoint::NUM_JOINTS) {}
		Fubi::SkeletonJoint::Joint m_joint;
		Fubi::SkeletonJoint::Joint m_relJoint;
		std::vector<Fubi::Vec3f> m_data;
		std::vector<Fubi::Matrix3f> m_inverseCovs;
		Fubi::Vec3f m_indicativeOrient;
		std::vector<int> m_polyLineIndices;
	};

	/**
	* \brief Empty TemplateData to be returned on failure or missing data
	*/
	const TemplateData EmptyTemplateData;
	const std::vector<TemplateData> EmptyTemplateDataVec;

	/**
	* \brief Normalizes a rotation for -180 to 180 range
	*/
	static float& normalizeRotation(float& rotValue)
	{
		// Ensure upper bound of 180
		while (rotValue > 180.000001f)
			rotValue -= 360.0f;
		//Ensure lower bound of -180
		while (rotValue < -180.000001f)
			rotValue += 360.0f;
		return rotValue;
	}
	static Vec3f& normalizeRotationVec(Vec3f& rotVec)
	{
		normalizeRotation(rotVec.x);
		normalizeRotation(rotVec.y);
		normalizeRotation(rotVec.z);
		return rotVec;
	}

	/**
	* \brief Calculate alpha value for the 1€ filter
	*/
	static float oneEuroAlpha(float timeStep, float cutOffFrequency)
	{
		if (cutOffFrequency > Math::Epsilon && timeStep > Math::Epsilon)
			return 1.0f / (1.0f + (1.0f / (Math::TwoPi * cutOffFrequency * timeStep)));
		return 0;
	}

	/**
	* \brief Vec3f containing only min float values
	*/
	static const Vec3f DefaultMinVec = Vec3f(-Math::MaxFloat, -Math::MaxFloat, -Math::MaxFloat);
	/**
	* \brief Vec3f containing only max float values
	*/
	static const Vec3f DefaultMaxVec = Vec3f(Math::MaxFloat, Math::MaxFloat, Math::MaxFloat);
	/**
	* \brief Vec3f containing only 0s
	*/
	static const Vec3f NullVec = Vec3f(0,0,0);

	/**
	* \brief Calculate global position out of local position and global parent position
	*/
	static inline void calculateGlobalPosition(const Vec3f& localPos, const Vec3f& absParentPos, const Matrix3f& absParentRot, Vec3f& dstPos)
	{
		// Combine transformation as rotatation
		Matrix4f parentTrans(absParentRot);
		// + translation
		parentTrans.x[12] = absParentPos.x;
		parentTrans.x[13] = absParentPos.y;
		parentTrans.x[14] = absParentPos.z;
		// Add local translation
		dstPos = parentTrans * localPos;
	}
	/**
	* \brief Calculate local positions out of global position and parent transformation
	*/
	static inline void calculateLocalPosition(const Vec3f& absPos, const Vec3f& absParentPos, const Matrix3f& absParentRot, Vec3f& dstPos)
	{
		// Combine transformation and rotatation
		Matrix4f parentTrans(absParentRot);
		// + translation
		parentTrans.x[12] = absParentPos.x;
		parentTrans.x[13] = absParentPos.y;
		parentTrans.x[14] = absParentPos.z;
		// Remove translation and rotation of the parent by applying the inverted transformation
		dstPos = parentTrans.inverted() * absPos;
	}

	/**
	* \brief Calculate local positions out of global position and parent transformation
	*/
	static inline void calculateLocalPosition(const SkeletonJointPosition& absPos, const SkeletonJointPosition& absParentPos, const SkeletonJointOrientation& absParentRot, SkeletonJointPosition& dstPos)
	{
		// First calculate the position
		calculateLocalPosition(absPos.m_position, absParentPos.m_position, absParentRot.m_orientation, dstPos.m_position);
		// Confidence is the minima of all used transformations
		dstPos.m_confidence = minf(absPos.m_confidence, minf(absParentPos.m_confidence, absParentRot.m_confidence));
	}

	/**
	* \brief Calculate body measurement (distance between two joints with filtering applied)
	*/
	static inline void calculateBodyMeasurement(const SkeletonJointPosition& joint1, const SkeletonJointPosition& joint2, BodyMeasurementDistance& dstMeasurement, float filterFactor = 1.0f)
	{
		float currentConfidence = minf(joint1.m_confidence, joint2.m_confidence);
		if (currentConfidence > dstMeasurement.m_confidence + 0.1f) // current confidence much more accurate than the last measure
			filterFactor = 1.0f;	// ..so forget about the last one

		float reverseFac = 1.0f - filterFactor;
		dstMeasurement.m_confidence = (reverseFac * dstMeasurement.m_confidence) + (filterFactor * currentConfidence);
		dstMeasurement.m_dist = (reverseFac * dstMeasurement.m_dist) + (filterFactor * (joint1.m_position-joint2.m_position).length());
	}

	/**
	* \brief Calculate local orientation out of global orientaion and parent orientation
	*/
	static inline void calculateLocalRotMat(const float* absRotMat, const float* absParentMat, float* dstMat)
	{
		// Inverse and transposed rotation matrices are equal, but this is faster to calculate
		Fubi::Matrix3f matLocal = Fubi::Matrix3f(absRotMat) * Fubi::Matrix3f(absParentMat).transposed();
		for (int i = 0; i< 9; ++i)
		{
			dstMat[i] = matLocal.x[i];
		}
	}

	/**
	* \brief Calculate local orientation out of global orientaion and parent orientation
	*/
	static inline void calculateLocalRotation(const SkeletonJointOrientation& absRot, const SkeletonJointOrientation& absParentRot, SkeletonJointOrientation& dstRot)
	{
		calculateLocalRotMat(absRot.m_orientation.x, absParentRot.m_orientation.x, dstRot.m_orientation.x);
		dstRot.m_confidence = minf(absRot.m_confidence, absParentRot.m_confidence);
	}

	/**
	* \brief Copy the three orientation columns to the right position in the orientation matrix
	*/
	static void orientationVectorsToRotMat(SkeletonJointOrientation &jointOrientation, const Vec3f& xCol,const Vec3f& yCol, const Vec3f& zCol)
	{
		jointOrientation.m_orientation.x[0] = xCol.x;
		jointOrientation.m_orientation.x[3] = xCol.y;
		jointOrientation.m_orientation.x[6] = xCol.z;

		jointOrientation.m_orientation.x[1] = yCol.x;
		jointOrientation.m_orientation.x[4] = yCol.y;
		jointOrientation.m_orientation.x[7] = yCol.z;

		jointOrientation.m_orientation.x[2] = zCol.x;
		jointOrientation.m_orientation.x[5] = zCol.y;
		jointOrientation.m_orientation.x[8] = zCol.z;
	}

	/**
	* \brief Calculate local transformations out of the global ones
	*/
	static void calculateLocalTransformations(const SkeletonJointPosition* globalPos, const SkeletonJointOrientation* globalOrients,
		SkeletonJointPosition* localPos, SkeletonJointOrientation* localOrients)
	{
		// Calculate new relative orientations
		// Torso is the root, so the local orientation is the same as the global one
		localOrients[SkeletonJoint::TORSO] = globalOrients[SkeletonJoint::TORSO];
		// Neck
		calculateLocalRotation(globalOrients[SkeletonJoint::NECK],
			globalOrients[SkeletonJoint::TORSO], localOrients[SkeletonJoint::NECK]);
		// Head
		calculateLocalRotation(globalOrients[SkeletonJoint::HEAD],
			globalOrients[SkeletonJoint::NECK], localOrients[SkeletonJoint::HEAD]);
		// Nose
		calculateLocalRotation(globalOrients[SkeletonJoint::FACE_NOSE],
			globalOrients[SkeletonJoint::HEAD], localOrients[SkeletonJoint::FACE_NOSE]);
		// Chin
		calculateLocalRotation(globalOrients[SkeletonJoint::FACE_CHIN],
			globalOrients[SkeletonJoint::HEAD], localOrients[SkeletonJoint::FACE_CHIN]);
		// Forehead
		calculateLocalRotation(globalOrients[SkeletonJoint::FACE_FOREHEAD],
			globalOrients[SkeletonJoint::HEAD], localOrients[SkeletonJoint::FACE_FOREHEAD]);
		// Left ear
		calculateLocalRotation(globalOrients[SkeletonJoint::FACE_LEFT_EAR],
			globalOrients[SkeletonJoint::HEAD], localOrients[SkeletonJoint::FACE_LEFT_EAR]);
		// Right ear
		calculateLocalRotation(globalOrients[SkeletonJoint::FACE_RIGHT_EAR],
			globalOrients[SkeletonJoint::HEAD], localOrients[SkeletonJoint::FACE_RIGHT_EAR]);
		// Left shoulder
		calculateLocalRotation(globalOrients[SkeletonJoint::LEFT_SHOULDER],
			globalOrients[SkeletonJoint::NECK], localOrients[SkeletonJoint::LEFT_SHOULDER]);
		// Left elbow
		calculateLocalRotation(globalOrients[SkeletonJoint::LEFT_ELBOW],
			globalOrients[SkeletonJoint::LEFT_SHOULDER], localOrients[SkeletonJoint::LEFT_ELBOW]);
		// Left wrist
		calculateLocalRotation(globalOrients[SkeletonJoint::LEFT_WRIST],
			globalOrients[SkeletonJoint::LEFT_ELBOW], localOrients[SkeletonJoint::LEFT_WRIST]);
		// Left hand
		calculateLocalRotation(globalOrients[SkeletonJoint::LEFT_HAND],
			globalOrients[SkeletonJoint::LEFT_WRIST], localOrients[SkeletonJoint::LEFT_HAND]);
		// Right shoulder
		calculateLocalRotation(globalOrients[SkeletonJoint::RIGHT_SHOULDER],
			globalOrients[SkeletonJoint::NECK], localOrients[SkeletonJoint::RIGHT_SHOULDER]);
		// Right elbow
		calculateLocalRotation(globalOrients[SkeletonJoint::RIGHT_ELBOW],
			globalOrients[SkeletonJoint::RIGHT_SHOULDER], localOrients[SkeletonJoint::RIGHT_ELBOW]);
		// Right wrist
		calculateLocalRotation(globalOrients[SkeletonJoint::RIGHT_WRIST],
			globalOrients[SkeletonJoint::RIGHT_ELBOW], localOrients[SkeletonJoint::RIGHT_WRIST]);
		// Right hand
		calculateLocalRotation(globalOrients[SkeletonJoint::RIGHT_HAND],
			globalOrients[SkeletonJoint::RIGHT_WRIST], localOrients[SkeletonJoint::RIGHT_HAND]);
		// Waist
		calculateLocalRotation(globalOrients[SkeletonJoint::WAIST],
			globalOrients[SkeletonJoint::TORSO], localOrients[SkeletonJoint::WAIST]);
		// Left hip
		calculateLocalRotation(globalOrients[SkeletonJoint::LEFT_HIP],
			globalOrients[SkeletonJoint::WAIST], localOrients[SkeletonJoint::LEFT_HIP]);
		// Left knee
		calculateLocalRotation(globalOrients[SkeletonJoint::LEFT_KNEE],
			globalOrients[SkeletonJoint::LEFT_HIP], localOrients[SkeletonJoint::LEFT_KNEE]);
		// Left ankle
		calculateLocalRotation(globalOrients[SkeletonJoint::LEFT_ANKLE],
			globalOrients[SkeletonJoint::LEFT_KNEE], localOrients[SkeletonJoint::LEFT_ANKLE]);
		// Left foot
		calculateLocalRotation(globalOrients[SkeletonJoint::LEFT_FOOT],
			globalOrients[SkeletonJoint::LEFT_ANKLE], localOrients[SkeletonJoint::LEFT_FOOT]);
		// Right hip
		calculateLocalRotation(globalOrients[SkeletonJoint::RIGHT_HIP],
			globalOrients[SkeletonJoint::WAIST], localOrients[SkeletonJoint::RIGHT_HIP]);
		// Right knee
		calculateLocalRotation(globalOrients[SkeletonJoint::RIGHT_KNEE],
			globalOrients[SkeletonJoint::RIGHT_HIP], localOrients[SkeletonJoint::RIGHT_KNEE]);
		// Right ankle
		calculateLocalRotation(globalOrients[SkeletonJoint::RIGHT_ANKLE],
			globalOrients[SkeletonJoint::RIGHT_KNEE], localOrients[SkeletonJoint::RIGHT_ANKLE]);
		// Right foot
		calculateLocalRotation(globalOrients[SkeletonJoint::RIGHT_FOOT],
			globalOrients[SkeletonJoint::RIGHT_ANKLE], localOrients[SkeletonJoint::RIGHT_FOOT]);

		// Calculate local positions (removing the torso transformation=loc+rot from the position data)
		// Torso is the root, so the local pos is the same as the global one
		localPos[SkeletonJoint::TORSO] = globalPos[SkeletonJoint::TORSO];
		const SkeletonJointPosition& torsoPos = globalPos[SkeletonJoint::TORSO];
		const SkeletonJointOrientation& torsoRot = globalOrients[SkeletonJoint::TORSO];
		for (unsigned int i = 0; i < SkeletonJoint::NUM_JOINTS; ++i)
		{
			if (i != SkeletonJoint::TORSO)
				calculateLocalPosition(globalPos[i], torsoPos, torsoRot, localPos[i]);
		}
	}

	/**
	* \brief Calculate local hand transformations out of the global ones
	*/
	static void calculateLocalHandTransformations(const SkeletonJointPosition* globalPos, const SkeletonJointOrientation* globalOrients,
		SkeletonJointPosition* localPos, SkeletonJointOrientation* localOrients)
	{
		// Calculate new relative orientations
		// Palm is the root, so the local orientation is the same as the global one
		localOrients[SkeletonHandJoint::PALM] = globalOrients[SkeletonHandJoint::PALM];
		localPos[SkeletonHandJoint::PALM] = globalPos[SkeletonHandJoint::PALM];
		// Fingers
		for (int i = SkeletonHandJoint::THUMB; i < SkeletonHandJoint::NUM_JOINTS; ++i)
		{
			calculateLocalRotation(globalOrients[i],
				globalOrients[SkeletonHandJoint::PALM], localOrients[i]);
			calculateLocalPosition(globalPos[i], globalPos[SkeletonHandJoint::PALM], globalOrients[SkeletonHandJoint::PALM], localPos[i]);
		}
	}

	/**
	* \brief Calculate orientation out of a position vector
	*/
	static void orientVectorsFromVecX(const Vec3f& x, SkeletonJointOrientation &jointOrientation)
	{
		// Columnn vectors (initialized with 0,0,0)
		Vec3f xCol;
		Vec3f yCol;
		Vec3f zCol;

		// x vector is used directly
		xCol = x.normalized();

		// y vector is set to be orthogonal to x, and pointing in parallel to the y-axis if the x vector is pointing in parallel to the x axis
		if (xCol.y != 0 || xCol.x != 0)
		{
			yCol.x = -xCol.y;
			yCol.y = xCol.x;
			yCol.z = 0;
			yCol.normalize();
		}
		else
		{
			yCol.y = 1.0f;
		}

		// z vector can now be calculated as the cross product of the others
		zCol = xCol.cross(yCol);

		// Now copy the values into matrix
		orientationVectorsToRotMat(jointOrientation, xCol, yCol, zCol);
	}

	/**
	* \brief Calculate orientation out of a position vector
	*/
	static void orientVectorsFromVecY(const Vec3f& v1, SkeletonJointOrientation &jointOrientation)
	{
		// Columnn vectors (initialized with 0,0,0)
		Vec3f xCol;
		Vec3f yCol;
		Vec3f zCol;


		// y vector is used directly
		yCol = v1.normalized();

		// x vector is set to be orthogonal to y, and pointing in parallel to the x-axis if the y vector is pointing in parallel to the y axis
		if (yCol.x != 0 || yCol.y != 0)
		{
			xCol.x = yCol.y;
			xCol.y = -yCol.x;
			xCol.z = 0.0f;
			xCol.normalize();
		}
		else
		{
			xCol.x = 1.0f;
		}

		// z vector can now be calculated as the cross product of the others
		zCol = xCol.cross(yCol);

		// Now copy the values into matrix
		orientationVectorsToRotMat(jointOrientation, xCol, yCol, zCol);
	}

	/**
	* \brief Calculate orientation out of position vectors
	*/
	static void orientVectorsFromVecYX(const Vec3f& yUnnormalized, const Vec3f& xUnnormalized, SkeletonJointOrientation &jointOrientation)
	{
		// Columnn vectors (initialized with 0,0,0)
		Vec3f xCol;
		Vec3f yCol;
		Vec3f zCol;

		// y vector is used directly
		yCol = yUnnormalized.normalized();
		// z vector is calculated as the cross product of x and y
		zCol = xUnnormalized.normalized().cross(yCol).normalized();
		// x vector is again calculated as the cross product of y and z (may be different from given x vector)
		xCol = yCol.cross(zCol);

		//copy values into matrix
		orientationVectorsToRotMat(jointOrientation, xCol, yCol, zCol);
	}

	/**
	* \brief Calculate orientation out of position vectors
	*/
	static void jointOrientationFromPositionsYX(const SkeletonJointPosition& yStart, const SkeletonJointPosition& yEnd,
		const SkeletonJointPosition& xStart, const SkeletonJointPosition& xEnd, SkeletonJointOrientation &jointOrientation)
	{
		float xConf = (xStart.m_confidence + xEnd.m_confidence) / 2.0f;
		float yConf = (yStart.m_confidence + yEnd.m_confidence) / 2.0f;
		jointOrientation.m_confidence = (xConf + yConf) / 2.0f;

		if (xConf > 0 && yConf > 0)
		{
			Vec3f vx = xEnd.m_position-xStart.m_position;
			Vec3f vy = yEnd.m_position-yStart.m_position;
			orientVectorsFromVecYX(vy, vx, jointOrientation);
		}
		else if (xConf > 0)
		{
			Vec3f vx = xEnd.m_position-xStart.m_position;
			orientVectorsFromVecX(vx, jointOrientation);
		}
		else if (yConf > 0)
		{
			Vec3f vy = yEnd.m_position-yStart.m_position;
			orientVectorsFromVecY(vy, jointOrientation);
		}
	}

	/**
	* \brief Calculate orientation out of position vectors
	*/
	static void jointOrientationFromPositionY(const SkeletonJointPosition& yStart, const SkeletonJointPosition& yEnd, SkeletonJointOrientation &jointOrientation)
	{
		jointOrientation.m_confidence = (yStart.m_confidence + yEnd.m_confidence) / 2.0f;

		if (jointOrientation.m_confidence > 0)
		{
			Vec3f vy = yEnd.m_position - yStart.m_position;
			orientVectorsFromVecY(vy, jointOrientation);
		}
	}

	/**
	* \brief Calculate orientation out of a position vector
	*/
	static void jointOrientationFromPositionX(const SkeletonJointPosition& xStart, const SkeletonJointPosition& xEnd,
		SkeletonJointOrientation &jointOrientation)
	{
		jointOrientation.m_confidence = (xStart.m_confidence + xEnd.m_confidence) / 2.0f;
		if (jointOrientation.m_confidence > 0)
		{
			Vec3f vx = xEnd.m_position-xStart.m_position;
			orientVectorsFromVecX(vx, jointOrientation);
		}
	}

	/**
	* \brief Approximate global orientations out of the global positions
	*/
	static void approximateGlobalOrientations(const SkeletonJointPosition* globalPos, SkeletonJointOrientation* globalOrients)
	{
		Vec3f vx;
		Vec3f vy;
		Vec3f temp;

		// Torso uses left-to-right-shoulder for x and torso-to-neck for y
		jointOrientationFromPositionsYX(globalPos[SkeletonJoint::TORSO], globalPos[SkeletonJoint::NECK],
			globalPos[SkeletonJoint::LEFT_SHOULDER], globalPos[SkeletonJoint::RIGHT_SHOULDER],
			globalOrients[SkeletonJoint::TORSO]);

		// Waist uses the same as torso
		globalOrients[SkeletonJoint::WAIST] = globalOrients[SkeletonJoint::TORSO];

		// Neck uses left-to-right-shoulder for x and neck-to-head for y
		jointOrientationFromPositionsYX(globalPos[SkeletonJoint::NECK], globalPos[SkeletonJoint::HEAD],
			globalPos[SkeletonJoint::LEFT_SHOULDER], globalPos[SkeletonJoint::RIGHT_SHOULDER],
			globalOrients[SkeletonJoint::NECK]);

		// Head uses same as neck
		globalOrients[SkeletonJoint::HEAD] = globalOrients[SkeletonJoint::NECK];

		// Shoulder uses elbow-to-shoulder for x
		jointOrientationFromPositionX(globalPos[SkeletonJoint::LEFT_ELBOW], globalPos[SkeletonJoint::LEFT_SHOULDER],
			globalOrients[SkeletonJoint::LEFT_SHOULDER]);

		// Elbow uses hand-to-elbow for x
		jointOrientationFromPositionX(globalPos[SkeletonJoint::LEFT_HAND], globalPos[SkeletonJoint::LEFT_ELBOW],
			globalOrients[SkeletonJoint::LEFT_ELBOW]);

		// Hand/wrist uses the same as elbow
		globalOrients[SkeletonJoint::LEFT_WRIST] = globalOrients[SkeletonJoint::LEFT_HAND] = globalOrients[SkeletonJoint::LEFT_ELBOW];

		// Hip uses knee-to-hip for y and left-to-right-hip for x
		jointOrientationFromPositionsYX(globalPos[SkeletonJoint::LEFT_KNEE], globalPos[SkeletonJoint::LEFT_HIP],
			globalPos[SkeletonJoint::LEFT_HIP], globalPos[SkeletonJoint::RIGHT_HIP],
			globalOrients[SkeletonJoint::LEFT_HIP]);

		// Knee users foot-to-knee for y
		jointOrientationFromPositionY(globalPos[SkeletonJoint::LEFT_FOOT], globalPos[SkeletonJoint::LEFT_KNEE],
			globalOrients[SkeletonJoint::LEFT_KNEE]);

		// Foot/Ankle uses the same as knee
		globalOrients[SkeletonJoint::LEFT_ANKLE] = globalOrients[SkeletonJoint::LEFT_FOOT] = globalOrients[SkeletonJoint::LEFT_KNEE];


		// Shoulder uses shoulder-to-elbow for x
		jointOrientationFromPositionX(globalPos[SkeletonJoint::RIGHT_SHOULDER], globalPos[SkeletonJoint::RIGHT_ELBOW],
			globalOrients[SkeletonJoint::RIGHT_SHOULDER]);

		// Elbow uses elbow-to-hand for x
		jointOrientationFromPositionX(globalPos[SkeletonJoint::RIGHT_ELBOW], globalPos[SkeletonJoint::RIGHT_HAND],
			globalOrients[SkeletonJoint::RIGHT_ELBOW]);

		// Hand/wrist uses the same as elbow
		globalOrients[SkeletonJoint::RIGHT_WRIST] = globalOrients[SkeletonJoint::RIGHT_HAND] = globalOrients[SkeletonJoint::RIGHT_ELBOW];

		// Hip uses knee-to-hip for y and left-to-right-hip for x
		jointOrientationFromPositionsYX(globalPos[SkeletonJoint::RIGHT_KNEE], globalPos[SkeletonJoint::RIGHT_HIP],
			globalPos[SkeletonJoint::LEFT_HIP], globalPos[SkeletonJoint::RIGHT_HIP],
			globalOrients[SkeletonJoint::RIGHT_HIP]);

		// Knee users foot-to-knee for y
		jointOrientationFromPositionY(globalPos[SkeletonJoint::RIGHT_FOOT], globalPos[SkeletonJoint::RIGHT_KNEE],
			globalOrients[SkeletonJoint::RIGHT_KNEE]);

		// Foot/ankle uses the same as knee
		globalOrients[SkeletonJoint::RIGHT_ANKLE] = globalOrients[SkeletonJoint::RIGHT_FOOT] = globalOrients[SkeletonJoint::RIGHT_KNEE];
	}

	/**
	* \brief Returns the minimum
	*/
	static inline int mini(int a, int b)
	{
		return a < b ? a : b;
	}

	/**
	* \brief Returns the maximum
	*/
	static inline int maxi(int a, int b)
	{
		return a > b ? a : b;
	}

	/**
	* \brief Returns the centroid of a vector of Vec3fs
	*/
	static Vec3f centroid(const std::vector<Vec3f>::const_iterator& begin, const std::vector<Vec3f>::const_iterator& end)
	{
		Vec3f centroid;
		auto len = end-begin;
		if (len > 0)
		{
			for (auto iter = begin; iter != end; ++iter)
			{
				centroid += *iter;
			}
			centroid /= (float)len;
		}
		return centroid;
	}
	static Vec3f centroid(const std::vector<Vec3f>& points)
	{
		return centroid(points.begin(), points.end());
	}

	/**
	* \brief Returns the path length of a vector of Vec3fs
	*/
	static float pathLength(const std::vector<Vec3f>::const_iterator& begin, const std::vector<Vec3f>::const_iterator& end)
	{
		float length = 0;
		auto size = end - begin;
		if (size > 1)
		{
			for (auto iter = begin; iter != end-1; ++iter)
			{
				length += (*(iter+1)-*iter).length();
			}
		}
		return length;
	}
	static float pathLength(const std::vector<Vec3f>& points)
	{
		return pathLength(points.begin(), points.end());
	}

	/**
	* \brief Calculates the axis-aligned bounding box (AABB) of a vector of Vec3f and stores it in the given two Vec3fs (should be unintitialized)
	*/
	static void calculateAABB(const std::vector<Vec3f>& points, Vec3f& minAABB, Vec3f& maxAABB)
	{
		if (points.size() == 0)
		{
			minAABB = Vec3f(0,0,0);
			maxAABB = Vec3f(0,0,0);
		}
		else
		{
			minAABB = DefaultMaxVec;
			maxAABB = DefaultMinVec;
			auto end = points.end();
			for (auto iter = points.begin(); iter != end; ++iter)
			{
				if (iter->x < minAABB.x)
					minAABB.x = iter->x;
				if (iter->y < minAABB.y)
					minAABB.y = iter->y;
				if (iter->z < minAABB.z)
					minAABB.z = iter->z;
				if (iter->x > maxAABB.x)
					maxAABB.x = iter->x;
				if (iter->y > maxAABB.y)
					maxAABB.y = iter->y;
				if (iter->z > maxAABB.z)
					maxAABB.z = iter->z;
			}
		}
	}

	/**
	* \brief Rotates a vector of Vec3f around the origin for the given rotation (in radians)
	*/
	static void rotate(std::vector<Vec3f>& points, const Vec3f& rotation)
	{
		Matrix3f rotMat = Matrix3f::RotMat(rotation);
		auto end = points.end();
		for (auto iter = points.begin(); iter != end; ++iter)
		{
			*iter = rotMat * *iter;
		}
	}

	/**
	* \brief Rotates a vector of Vec3f around the given axis with the given angle (in radians)
	*/
	static void rotate(std::vector<Vec3f>& points, const Vec3f& axis, float angle)
	{
		if (points.size() > 0)
		{
			Matrix3f rotMat = Matrix3f::RotMat(axis, angle);
			auto end = points.end();
			for (auto iter = points.begin(); iter != end; ++iter)
			{
				*iter = rotMat * *iter;
			}
		}
	}

	/**
	* \brief Translates a vector of Vec3f for the given translation
	*/
	static void translate(std::vector<Vec3f>& points, const Vec3f& translation)
	{
		auto end = points.end();
		for (auto iter = points.begin(); iter != end; ++iter)
		{
			*iter += translation;
		}
	}

	/**
	* \brief Scales a vector of Vec3f to a given AABB size if its larger then the provided squared min size, else returns false
	*/
	static bool scale(std::vector<Vec3f>& points, const Vec3f& desiredAaabbSize, float minAabbSize = 0.0f,
		bool keepAspect = false, bool onlyFitLargestSide = false, bool translateMinToZero = false, Vec3f* appliedScale = 0x0)
	{
		// Calculate current AABB
		Vec3f minAABB(Math::NO_INIT);
		Vec3f maxAABB(Math::NO_INIT);
		calculateAABB(points, minAABB, maxAABB);
		Vec3f currentSize = maxAABB - minAABB;
		if (currentSize.boundingBoxVolume(true) >= minAabbSize)
		{
			if (keepAspect)
			{
				float singleScale;
				if (onlyFitLargestSide)
				{
					// Take the scaling of the largest axis, but don't touch if we have a zero aabb
					if (currentSize.x > currentSize.y && currentSize.x > currentSize.z)
						singleScale = (currentSize.x > Math::Epsilon) ? (desiredAaabbSize.x / currentSize.x) : 1.0f;
					else if (currentSize.y > currentSize.z)
						singleScale = (currentSize.y > Math::Epsilon) ? (desiredAaabbSize.y / currentSize.y) : 1.0f;
					else
						singleScale = (currentSize.z > Math::Epsilon) ? (desiredAaabbSize.z / currentSize.z) : 1.0f;
				}
				else
				{
					if (currentSize.x < Math::Epsilon)
						currentSize.x = Math::Epsilon;
					if (currentSize.y < Math::Epsilon)
						currentSize.y = Math::Epsilon;
					if (currentSize.z < Math::Epsilon)
						currentSize.z = Math::Epsilon;
					Vec3f scaling = desiredAaabbSize / currentSize;
					// Take the axis with the minimum scaling to keep the aspect, but stay below the desired aabbSize on all axis
					singleScale = minf(scaling.x, minf(scaling.y, scaling.z));
				}
				if (appliedScale)
					*appliedScale = Vec3f(singleScale, singleScale, singleScale);
				auto end = points.end();
				for (auto iter = points.begin(); iter != end; ++iter)
				{
					*iter *= singleScale;
				}
				if (translateMinToZero)
					translate(points, -(minAABB*singleScale));
			}
			else
			{
				// Scaling = desired size / current size
				Vec3f scaling(Math::NO_INIT);
				// Don't touch sides with zero length
				scaling.x = (currentSize.x > Math::Epsilon) ? (desiredAaabbSize.x / currentSize.x) : 1.0f;
				scaling.y = (currentSize.y > Math::Epsilon) ? (desiredAaabbSize.y / currentSize.y) : 1.0f;
				scaling.z = (currentSize.z > Math::Epsilon) ? (desiredAaabbSize.z / currentSize.z) : 1.0f;
				if (appliedScale)
					*appliedScale = scaling;
				auto end = points.end();
				for (auto iter = points.begin(); iter != end; ++iter)
				{
					*iter *= scaling;
				}
				if (translateMinToZero)
					translate(points, -(minAABB*scaling));
			}
			return true;
		}
		return false;
	}

	/**
	* \brief Rescale a vector of points to fit a desired length using an hermite spline interpolation
	*
	* @param inputVec the raw input vector
	* @param desiredSize the size to which the input vector should be scaled to
	* @param outputVec the resampled output vector
	*/
	static void hermiteSplineRescale(const std::vector<Vec3f>& inputVec, size_t desiredSize, std::vector<Vec3f>& outputVec)
	{
		auto inSize = inputVec.size();
		if (desiredSize == 0 && inSize == 0)
			return;
		if (inSize == 1)
		{
			outputVec.push_back(inputVec[0]);
			return;
		}

		outputVec.reserve(outputVec.size() + desiredSize);
		for (unsigned int i = 0; i < desiredSize-1; ++i)
		{
			// Find the closest data pair and their tangents
			const float cTime = float(inSize - 1) * float(i) / float(desiredSize - 1);
			const unsigned int prev = (unsigned int)(ftoi_t(cTime));
			const unsigned int next = prev + 1;
			const Vec3f p0 = (prev > 0 ? inputVec[prev - 1] : inputVec[prev]);
			const Vec3f p1 = inputVec[prev];
			const Vec3f p2 = inputVec[next];
			const Vec3f p3 = (next < inSize - 1 ? inputVec[next + 1] : inputVec[next]);
			const Vec3f t1 = (p2 - p0)*0.5f;
			const Vec3f t2 = (p3 - p1)*0.5f;

			// Calculate the hermite spline attributes
			const float prevTime = float(prev);
			const float nextTime = float(next);
			const float nphase = (cTime - prevTime) / (nextTime - prevTime);
			const float s1 = nphase;
			const float s2 = s1*nphase;
			const float s3 = s2*nphase;
			const float h1 = 2.0f*s3 - 3.0f*s2 + 1.0f;
			const float h2 = -2.0f*s3 + 3.0f*s2;
			const float h3 = s3 - 2.0f*s2 + s1;
			const float h4 = s3 - s2;

			// And apply them on the data pair and tangents to get the interpolated point
			outputVec.push_back(p1*h1 + p2*h2 + t1*h3 + t2*h4);
		}
		// Add unchanged last point at the end
		outputVec.push_back(inputVec.back());
	}

	/**
	* \brief Resample a vector of points to fit a desired length with equal distances between succeeding points
	*
	* @param inputVec the raw input vector
	* @param desiredSize the size to which the input vector should be scaled to
	* @param outputVec the resampled output vector
	*/
	static void equiDistResample(const std::vector<Vec3f>& inputVec, size_t desiredSize, std::vector<Vec3f>& outputVec)
	{
		outputVec.resize(0);
		outputVec.reserve(desiredSize);
		if (inputVec.size() > 0)
			outputVec.push_back(inputVec.front());
		if (inputVec.size() < 2)
			return;

		const float intervalLength = pathLength(inputVec) / (desiredSize - 1);
		float distSum = 0.0;
		auto last = inputVec.begin();
		auto iter = last + 1;
		while (iter != inputVec.end())
		{
			float dist = (*iter - *last).length();
			if ((distSum + dist) >= intervalLength)
			{
				outputVec.push_back(*last + (*iter - *last) * ((intervalLength - distSum) / dist));
				last = outputVec.end()-1;
				distSum = 0.0;
			}
			else
			{
				distSum += dist;
				last = iter;
				++iter;
			}
		}
		if (outputVec.size() == desiredSize - 1) // somtimes we fall a rounding-error short of adding the last point, so add it if so
			outputVec.push_back(inputVec.back());
	}

	/**
	* \brief Calculate the angle between two vectors
	*
	* @param vec1 first vector
	* @param vec2 second vector
	* @return angle difference in degree
	*/
	static float angleDiff(const Vec3f& vec1, const Vec3f& vec2)
	{
		float len = vec1.length() * vec2.length();
		if (len > Math::Epsilon)
			return fabsf(radToDeg(acosf(vec1.dot(vec2) / len)));
		return 0;
	}

	/**
	* \brief Calculate the angle at a point of a vector of points using its predecessor and successor
	*
	* @param vec a vector of points
	* @param index the index at which the direction change should be calculated
	* @return angle difference of the two lines vec[i-1]->vec[i] and vec[i]->vec[i+1] in degree
	*/
	static float calcPathAngleAtIndex(const std::vector<Vec3f>& vec, size_t index)
	{
		if (index == 0 || index >= vec.size() - 1)
			return 0;
		return angleDiff(vec[index] - vec[index - 1], vec[index + 1] - vec[index]);
	}

	/**
	* \brief Calculate the path length within a vector of points from start to end
	*
	* @param vec a vector of points
	* @param start the index at which the path starts
	* @param end the index at which the path ends
	* @return the path length
	*/
	static float calcPathLength(const std::vector<Vec3f>& vec, size_t start, size_t end)
	{
		float length = 0;
		for (size_t i = start; i < end && i < vec.size() - 1; ++i)
			length += (vec[i + 1] - vec[i]).length();
		return length;
	}

	/**
	* \brief Find a point on a given path closest to the provided progress
	*
	* @param vec a vector of points
	* @param start the index at which the path starts
	* @param end the index at which the path ends
	* @param progress path length progress between start and end in [0,1]
	* @return index of the point closest to the given distance, or -1 on failure
	*/
	static size_t findPointOnPath(const std::vector<Vec3f>& vec, size_t start, size_t end, float progress)
	{
		float progressPathLength = calcPathLength(vec, start, end) * progress;
		float lengthPassed = 0;
		for (size_t i = start+1; i <= end && i < vec.size(); ++i)
		{
			lengthPassed += calcPathLength(vec, i - 1, i);
			if (lengthPassed >= progressPathLength)
				return i;
		}
		return vec.size()-1;
	}


	/**
	* \brief Reduce a vector of points by removing points that build and angle with their pre- and suc lower than minAngle
	* points are actually not removed, but only the indices in the separate vector
	*
	* @param inputVec the vector to be reduced
	* @param indicesInInputVec indices of points currently included from input vector lineFusionReduction will remove indices if applicable
	* first and last element of indicesInInputVec mark the boundaries and will not be touched
	*/
	static void lineFusionReduction(const std::vector<Vec3f>& vec, std::vector<int>& indicesInInputVec, float minAngle = 26.0f)
	{
		while (indicesInInputVec.size() > 2)
		{
			size_t indexForSmallestAngle = 0;
			float smallestAngle = 360.0f;

			// Find point with smallest angle
			for (size_t i = 1; i < indicesInInputVec.size() - 1; ++i)
			{
				Vec3f l1 = vec[indicesInInputVec[i]] - vec[indicesInInputVec[i - 1]];
				Vec3f l2 = vec[indicesInInputVec[i + 1]] - vec[indicesInInputVec[i]];
				float l1Len = l1.length();
				float l2Len = l2.length();
				if (l1Len < Math::Epsilon && l2Len < Math::Epsilon)
				{
					indexForSmallestAngle = i;
					smallestAngle = 0;
					break;
				}
				else
				{
					float angle = fabsf(radToDeg(acosf(l1.dot(l2) / (l1Len * l2Len))));
					if (angle < smallestAngle)
					{
						indexForSmallestAngle = i;
						smallestAngle = angle;
					}
				}
			}

			// Remove it if its angle is below the minAngle
			if (smallestAngle > minAngle)
				break;
			else
				indicesInInputVec.erase(indicesInInputVec.begin() + indexForSmallestAngle);
		}
	}

	/**
	* \brief Douglas-Peucker reduction algorithm
	* Recursively remove points if they are not far enough from the line from first to last
	* by marking them in a seperate vector (-> points are actually not removed)
	*
	* @param inputVec the vector to be reduced
	* @param indicesInInputVec all indices of points that should be kept are put in here
	* @param minDist the minimum distance, points should have from the line from first to last
	* @param firstIdx start of the current line segment
	* @param lastIdx end of the current line segment
	*/
	static void douglasPeuckerReduction(const std::vector<Vec3f>& inputVec, std::vector<int>& indicesInInputVec,
		float minDist, size_t firstIdx, size_t lastIdx)
	{
		if (lastIdx <= firstIdx + 1)
			// overlapping indexes, just return
			return;

		// loop over the points between the first and last points
		// and find the point that is the farthest away
		float maxDistance = 0;
		size_t indexFarthest = 0;
		Vec3f firstPoint = inputVec[firstIdx];
		Vec3f lastPoint = inputVec[lastIdx];
		for (size_t idx = firstIdx + 1; idx < lastIdx; idx++)
		{
			float distance = distancePointToRay(inputVec[idx], firstPoint, lastPoint, true);
			// keep the point with the greatest distance
			if (distance > maxDistance)
			{
				maxDistance = distance;
				indexFarthest = idx;
			}
		}
		if (maxDistance > minDist)
		{
			// The farthest point is outside the tolerance: it is marked and the algorithm continues. 
			indicesInInputVec.push_back((int)indexFarthest);

			// reduce the shape between the starting point to newly found point
			douglasPeuckerReduction(inputVec, indicesInInputVec, minDist, firstIdx, indexFarthest);

			// reduce the shape between the newly found point and the finishing point
			douglasPeuckerReduction(inputVec, indicesInInputVec, minDist, indexFarthest, lastIdx);
		}
		//else: the farthest point is within the tolerance, the whole segment is discarded.
	}

	/**
	* \brief Calculate polyline out of a vector of points which has a reduced as possible number of connected lines
	* based on the Polyline recognizer by Vittorio Fuccella - http://weblab.di.unisa.it/ published in:
	*	Vittorio Fuccella, Gennaro Costagliola. "Unistroke Gesture Recognition
	*	Through Polyline Approximation and Alignment". In Proceedings of the 33rd
	*	annual ACM conference on Human factors in computing systems (CHI '15).
  	*	April 18-23, 2015, Seoul, Republic of Korea.
	*
	* @param inputVec the raw input vector
	* @param indicesInInputVec indices in the inputVec that determine the polyline
	*/
	static void polyLineReduction(const std::vector<Vec3f>& inputVec, std::vector<int>& indicesInInputVec)
	{
		// Catch some special cases
		if (inputVec.size() == 0)
			return;
		if (inputVec.size() == 1)
		{
			indicesInInputVec.push_back(0);
			return;
		}

		// automatically add the first and last point
		indicesInInputVec.push_back(0);
		indicesInInputVec.push_back((int)inputVec.size() - 1);

		// min distance defined as the bounding box diagonal/22 as in the original paper
		Vec3f minAABB(Math::NO_INIT);
		Vec3f maxAABB(Math::NO_INIT);
		calculateAABB(inputVec, minAABB, maxAABB);
		float minDist = (maxAABB - minAABB).length() / 22.0f;
		if (inputVec.size() < 3 || minDist <= 0) // already finished
			return;

		// the first and last points in the original shape are
		// used as the entry point to the algorithm.
		douglasPeuckerReduction(inputVec, indicesInInputVec, minDist, 0, inputVec.size() - 1);

		// indices are probably unsorted now, so we should sort them
		std::sort(indicesInInputVec.begin(), indicesInInputVec.end());

		// additional line fusion
		lineFusionReduction(inputVec, indicesInInputVec);
	}

	/**
	* \brief Match the points of two vectors using the Needleman-Wunsch algorithm
	* based on the Polyline recognizer by Vittorio Fuccella - http://weblab.di.unisa.it/ published in:
	*	Vittorio Fuccella, Gennaro Costagliola. "Unistroke Gesture Recognition
	*	Through Polyline Approximation and Alignment". In Proceedings of the 33rd
	*	annual ACM conference on Human factors in computing systems (CHI '15).
	*	April 18-23, 2015, Seoul, Republic of Korea.
	*
	* @param vec1 first vector of points
	* @param vec2 second vector of points
	* @param indices1 indices in vec1 to be considered
	* @param indices2 indices in vec2 to be considereed
	* @param[out] matching a deque containing pairs of indices in indices1, indices2 that have been matched or not (if one of them is -1)
	* @param gapCost parameter for adjusting the cost of skipping points
	* @param balance parameter for weighting path length and path angle differently
	*/
	static void needlemanWunschMatching(const std::vector<Vec3f>& vec1, const std::vector<Vec3f>& vec2,
		const std::vector<int>& indices1, const std::vector<int>& indices2,
		std::deque<std::pair<int, int>>& matching,	float gapCost = 0.4f, float balance = 0.7f)
	{
		// Vector lenghts
		size_t len1 = indices1.size();
		size_t len2 = indices2.size();
		
		// Calculate path lenghts for the polylines
		std::vector<float> pathLen1, pathLen2;
		pathLen1.reserve(len1);
		pathLen1.push_back(0);
		for (size_t i = 1; i < len1; ++i)
			pathLen1.push_back(pathLen1.back() + (vec1[indices1[i]] - vec1[indices1[i - 1]]).length());
		float maxPathLen1 = pathLen1.back();
		pathLen2.reserve(len2);
		pathLen2.push_back(0);
		for (size_t i = 1; i < len2; ++i)
			pathLen2.push_back(pathLen2.back() + (vec2[indices2[i]] - vec2[indices2[i - 1]]).length());
		float maxPathLen2 = pathLen2.back();


		// Calculate score and similarity matrices
		float** scores = new float*[len1];
		float** similarities = new float*[len1];
		for (size_t i = 0; i < len1; i++)
		{
			scores[i] = new float[len2];
			similarities[i] = new float[len2];
			for (size_t j = 0; j < len2; j++)
			{
				if (i == 0)
					scores[i][j] = j*gapCost;//-j;
				else if (j == 0)
					scores[i][j] = i*gapCost;//-i;
				else
				{
					similarities[i][j] = 1.0f - (balance * fabsf(pathLen1[i - 1] / maxPathLen1 - pathLen2[j - 1] / maxPathLen2)
						+ (1 - balance) * fabsf(calcPathAngleAtIndex(vec1, indices1[i - 1]) - calcPathAngleAtIndex(vec2, indices2[j - 1])) / 180.0f);
					float scoreDiag = scores[i - 1][j - 1] + similarities[i][j];
					float scoreLeft = scores[i][j - 1] + gapCost;
					float scoreUp = scores[i - 1][j] + gapCost;
					scores[i][j] = maxf(maxf(scoreDiag, scoreLeft), scoreUp);
				}
			}
		}


		// Now get the matchings
		int index1 = (int)len1 - 1;
		int index2 = (int)len2 - 1;
		// Always match the last points
		matching.push_front(std::pair<int, int>(index1, index2));
		while (index1 > 0 && index2 > 0)
		{
			if (scores[index1][index2] == scores[index1 - 1][index2 - 1] + similarities[index1][index2])
			{
				matching.push_front(std::pair<int, int>(--index1, --index2));
			}
			else if (scores[index1][index2] == scores[index1][index2 - 1] + gapCost)
			{
				matching.push_front(std::pair<int, int>(-1, --index2));
			}
			else
			{
				matching.push_front(std::pair<int, int>(--index1, -1));
			}
		}

		// Cleanup
		for (size_t k = 0; k < len1; ++k)
		{
			delete[] scores[k];
			delete[] similarities[k];
		}
		delete[] scores;
		delete[] similarities;
	}

	/**
	* \brief Align two vectors for having the same number of points by intelligently adding points at places which could not be matched
	* based on the Polyline recognizer by Vittorio Fuccella - http://weblab.di.unisa.it/ published in:
	*	Vittorio Fuccella, Gennaro Costagliola. "Unistroke Gesture Recognition
	*	Through Polyline Approximation and Alignment". In Proceedings of the 33rd
	*	annual ACM conference on Human factors in computing systems (CHI '15).
	*	April 18-23, 2015, Seoul, Republic of Korea.
	*
	* @param vec1 first vector of raw points
	* @param vec2 second vector of raw points
	* @param[inout] polyLineIndices1 polyline indices in vec1 (will be enhanced)
	* @param[inout] polyLineIndices2 polyline indices in vec1 (will be enhanced)
	* @param[out] addedPoints number of points added to alignedVec1 or alignedVec2 in comparison to their corrsponding polylines
	* @param[out] matchedPoints number of points that could be matched between the vectors
	*/
	static void polyLineAlign(const std::vector<Vec3f>& vec1, const std::vector<Vec3f>& vec2, 
		std::vector<int>& polyLineIndices1, std::vector<int>& polyLineIndices2, unsigned int& addedPoints, unsigned int& matchedPoints)
	{
		std::deque<std::pair<int, int>> matching;
		needlemanWunschMatching(vec1, vec2, polyLineIndices1, polyLineIndices2, matching);
		
		std::vector<int> pointsToAddTo1, pointsToAddTo2;
		int prev1 = 0, prev2 = 0;
		for (auto iter = matching.begin(); iter != matching.end(); ++iter)
		{			
			if (iter->first >= 0 && iter->second >= 0)
			{
				++matchedPoints;

				int insertTo1 = iter->second - prev2 - 1;
				float fullLen2 = calcPathLength(vec2, polyLineIndices2[prev2], polyLineIndices2[iter->second]);
				float midLen2 = 0;
				for (int i = 0; i < insertTo1; ++i)
				{
					midLen2 += calcPathLength(vec2, polyLineIndices2[prev2 + i], polyLineIndices2[prev2 + i + 1]);
					float dist2 = midLen2 / fullLen2;
					pointsToAddTo1.push_back((int)findPointOnPath(vec1, polyLineIndices1[prev1], polyLineIndices1[iter->first], dist2));
				}

				int insertTo2 = iter->first - prev1 - 1;
				float fullLen1 = calcPathLength(vec1, polyLineIndices1[prev1], polyLineIndices1[iter->first]);
				float midLen1 = 0;
				for (int i = 0; i < insertTo2; ++i)
				{
					midLen1 += calcPathLength(vec1, polyLineIndices1[prev1 + i], polyLineIndices1[prev1 + i + 1]);
					float dist1 = midLen1 / fullLen1;
					pointsToAddTo2.push_back((int)findPointOnPath(vec2, polyLineIndices2[prev2], polyLineIndices2[iter->second], dist1));
				}

				prev1 = iter->first;
				prev2 = iter->second;
			}
		}

		// Insert points if now and sort the vector again
		if (pointsToAddTo1.size()  > 0)
		{
			polyLineIndices1.insert(polyLineIndices1.end(), pointsToAddTo1.begin(), pointsToAddTo1.end());
			std::sort(polyLineIndices1.begin(), polyLineIndices1.end());
		}
		if (pointsToAddTo2.size()  > 0)
		{
			polyLineIndices2.insert(polyLineIndices2.end(), pointsToAddTo2.begin(), pointsToAddTo2.end());
			std::sort(polyLineIndices2.begin(), polyLineIndices2.end());
		}

		addedPoints = (unsigned int) (pointsToAddTo1.size() + pointsToAddTo2.size());
	}

	/**
	* \brief Calculate the Malhanobis distance between a point and a distribution of points
	*
	* @param vec the vector to be compared
	* @param mean the mean vector of the distribution to compare against
	* @param inverseCov the inverse covariance matrix of the distribution to compare against
	* @return the Malhanobis distance of vec to the distribution defined by mean and inverseCov
	*/
	static float calculateMalhanobisDistance(const Vec3f& vec, const Vec3f& mean, const Matrix3f& inverseCov)
	{
		Vec3f d = vec - mean;
		return sqrtf(d.dot(inverseCov * d));
	}

	/**
	* \brief Calculate mean of a vector of points
	*
	* @param vec of points for which the mean should be calculated
	* @return the mean value
	*/
	static Vec3f calculateMean(const std::vector<Vec3f>& vec)
	{
		Vec3f res;
		if (vec.size() > 0)
		{
			for (auto iter = vec.begin(); iter != vec.end(); ++iter)
				res += *iter;
			res /= (float) vec.size();
		}
		return res;
	}


	/**
	* \brief Calculate the mean distance between two input vectors using the given distance measure
	*
	* @param baseVec, testVec the two vectors containing values that need to be compared
	* @param distanceMeasure the metric used to measure distances between two points
	* @param inverseCovs the inverse covariance matrices for the values in the base vec (only used for distanceMeasure == Malhanobis)
	* @return the mean distance between baseVec and testVec
	*/
	static float calculateMeanDistance(const std::vector<Vec3f>& baseVec, const std::vector<Vec3f>& testVec,
		DistanceMeasure::Measure distanceMeasure = DistanceMeasure::Euclidean, const std::vector<Matrix3f>* inverseCovs = 0x0)
	{
		int len = mini((int)baseVec.size(), (int)testVec.size());
		if (len == 0)
			return -1.0f;
		if (distanceMeasure == DistanceMeasure::Malhanobis && (inverseCovs == 0x0 || inverseCovs->size() < (unsigned)len))
		{
			// Malhanobis distance cannot be calculated so fall back to euclidean
			distanceMeasure = DistanceMeasure::Euclidean;
		}

		// Calculate sum of pair-wise distance
		float dist = 0;
		if (distanceMeasure == DistanceMeasure::Malhanobis)
		{
			for (int i = 0; i < len; ++i)
			{
				dist += calculateMalhanobisDistance(testVec[i], baseVec[i], inverseCovs->at(i));
			}
		}
		else if (distanceMeasure == DistanceMeasure::Manhattan)
		{
			for (int i = 0; i < len; ++i)
			{
				dist += (baseVec[i] - testVec[i]).lengthManhattan();
			}
		}
		else if (distanceMeasure == DistanceMeasure::TurningAngleDiff)
		{
			for (int i = 1; i < len; ++i)
			{
				Vec3f baseDir = baseVec[i] - baseVec[i - 1];
				Vec3f testDir = testVec[i] - testVec[i - 1];
				dist += angleDiff(baseDir, testDir);
			}
		}
		else
		{
			for (int i = 0; i < len; ++i)
			{
				dist += (baseVec[i] - testVec[i]).length();
			}
		}

		// Divide by len to get the mean
		return dist / len;
	}

	/**
	* \brief Calculate the dynamic time warping (DTW) distance between two input vectors using the given distance measure
	*
	* @param baseVec, testVec the two vectors containing values that need to be compared
	* @param distanceMeasure the metric used to measure distances between two points
	* @param maxWarpingDistance how much warping is allowed to be done
	* @param applyInReverse the warping will be applied using the last data point in both vecs as starting point and iterating reversely through them
	* @param[out] distanceBuffer (=0x0) buffer used for storing the distances, needs to be a matrix with size baseVec.size()*testVec.size()
	* @param inverseCovs the inverse covariance matrices for the values in the base vec (only used for distanceMeasure == Malhanobis)
	* @return the dtw distance between baseVec and testVec
	*/
	static float calculateDTW(const std::vector<Vec3f>& baseVec, const std::vector<Vec3f>& testVec,
		DistanceMeasure::Measure distanceMeasure = DistanceMeasure::Euclidean, unsigned int maxWarpingDistance = 5,
		bool applyInReverse = false,
		std::vector<std::vector<float> >* distanceBuffer = 0x0,
		const std::vector<Matrix3f>* inverseCovs = 0x0)
	{
		int len1 = (int)baseVec.size();
		int len2 = mini((int)testVec.size(), len1 + maxWarpingDistance);
		if (len1 == 0 || len2 == 0)
			return -1.0f;

		// Take the provided buffer or create a new one
		std::vector<std::vector<float> >* distances = distanceBuffer;
        if (!distanceBuffer)
			distances = new std::vector<std::vector<float> >(len1, std::vector<float>(len2, Math::MaxFloat));

		if (distanceMeasure == DistanceMeasure::Malhanobis && (inverseCovs == 0x0 || inverseCovs->size() < (unsigned)len1))
		{
			// Malhanobis distance cannot be calculated so fall back to euclidean
			distanceMeasure = DistanceMeasure::Euclidean;
		}

		for (int i = 0; i < len1; ++i)
		{
			int end = mini(len2, i + maxWarpingDistance + 1);
			for (int j = maxi(0, (i - maxWarpingDistance)); j < end; ++j)
			{
				// Calc current cost
				unsigned int baseIndex = applyInReverse ? len1 - 1 - i : i;
				unsigned int testIndex = applyInReverse ? len2 - 1 - j : j;
				if (distanceMeasure == DistanceMeasure::Malhanobis)
				{
					(*distances)[i][j] = calculateMalhanobisDistance(testVec[testIndex], baseVec[baseIndex], inverseCovs->at(baseIndex));
				}
				else if (distanceMeasure == DistanceMeasure::TurningAngleDiff)
				{
					// We don't have an angle for the last point, so we reuse the one of the 
					if (baseIndex == len1 - 1)
						baseIndex = len1 - 2;
					if (testIndex == len2 - 1)
						testIndex = len2 - 2;
					if (baseIndex < 0 || testIndex < 0)
						continue;
					
					Vec3f baseDir = baseVec[baseIndex + 1] - baseVec[baseIndex];
					Vec3f testDir = testVec[testIndex + 1] - testVec[testIndex];
					(*distances)[i][j] = angleDiff(baseDir, testDir);
				}
				else
				{
					Vec3f diff = baseVec[baseIndex] - testVec[testIndex];
					(*distances)[i][j] = (distanceMeasure == DistanceMeasure::Manhattan) ? diff.lengthManhattan() : diff.length();
				}

				// Add the cost of previous nodes
				if (i > 0 || j > 0)
				{
					float minPrevCost = Math::MaxFloat;
					if (i > 0)
						minPrevCost = (*distances)[i - 1][j];
					if (j > 0)
						minPrevCost = minf(minPrevCost, (*distances)[i][j - 1]);
					if (i > 0 && j > 0)
						minPrevCost = minf(minPrevCost, (*distances)[i - 1][j - 1]);
					(*distances)[i][j] += minPrevCost;
				}
			}
		}

		// The actual path in the test vec can be shorter so we look for the minimum within the warping range
		float minEndDistance = Math::MaxFloat;
		int end = mini(len2, len1 + maxWarpingDistance);
		for (int j = maxi(0, (len1 - 1 - maxWarpingDistance)); j < end; ++j)
		{
			if ((*distances)[len1 - 1][j] < minEndDistance)
				minEndDistance = (*distances)[len1 - 1][j];
		}
		//minEndDistance = (*distances)[len1 - 1][end - 1];

		if (!distanceBuffer)
			delete distances;
		return minEndDistance;
	}

	/**
	* \brief return the median value of the last windowSize elements of a deque
	*/
	static int heapMedian(const std::deque<int>& vec, size_t windowSize)
	{
		// correct window size and catch special cases (windowSize 0-3 can't be handled by the later algorithm)
		if (windowSize > vec.size())
			windowSize = vec.size();
		if (windowSize % 2 == 0)
		{
			if (windowSize < vec.size())
				windowSize++;
			else if (windowSize > 0)
				windowSize--;
		}
		if (windowSize == 0)
			return -1;
		if (windowSize == 1)
			return vec.back();
		if (windowSize == 3)
		{
			std::deque<int>::const_iterator p = vec.end()-3;
			int val1 = *p++;
			int val2 = *p++;
			int val3 = *p++;
			if (val1 < val2)
			{
				if (val2 < val3)
					return val2;
				else if (val1 < val3)
					return val3;
				return val1;
			}
			else
			{
				if (val1 < val3)
					return val1;
				else if (val2 < val3)
					return val3;
				return val2;
			}
		}

		// The rest is adapted from chmike (http://stackoverflow.com/users/75517/chmike)
		unsigned char halfSize = (unsigned char)(windowSize / 2) + 1;
		int* left = new int[halfSize];
		int* right = new int[halfSize];
		unsigned char nLeft = 1, nRight = 1;

		// pick first value of window as median candidate
		std::deque<int>::const_iterator p = vec.end()-windowSize;
		int median = *p++;

		for (;;)
		{
			// get next value
			int val = *p++;

			// if value is smaller than median, append to left heap
			if (val < median)
			{
				// move biggest value to the heap top
				unsigned char child = nLeft++, parent = (child - 1) / 2;
				while (parent && val > left[parent])
				{
					left[child] = left[parent];
					child = parent;
					parent = (parent - 1) / 2;
				}
				left[child] = val;

				// if left heap is full
				if (nLeft == halfSize)
				{
					// handle remaining values
					for (; p != vec.end(); p++)
					{
						// get next value
						val = *p;

						// if value is to be inserted in the left heap
						if (val < median)
						{
							child = left[2] > left[1] ? 2 : 1;
							if (val >= left[child])
								median = val;
							else
							{
								median = left[child];
								parent = child;
								child = parent*2 + 1;
								while (child < halfSize)
								{
									if (child < halfSize-1 && left[child+1] > left[child])
										++child;
									if (val >= left[child])
										break;
									left[parent] = left[child];
									parent = child;
									child = parent*2 + 1;
								}
								left[parent] = val;
							}
						}
					}
					break;
				}
			}
			else // else append to right heap
			{
				// move smallest value to the heap top
				unsigned char child = nRight++, parent = (child - 1) / 2;
				while (parent && val < right[parent])
				{
					right[child] = right[parent];
					child = parent;
					parent = (parent - 1) / 2;
				}
				right[child] = val;

				// if right heap is full
				if (nRight == halfSize)
				{
					// for each remaining value
					for(; p != vec.end(); p++)
					{
						// get next value
						val = *p;

						// if value is to be inserted in the right heap
						if (val > median)
						{
							child = right[2] < right[1] ? 2 : 1;
							if (val <= right[child])
								median = val;
							else
							{
								median = right[child];
								parent = child;
								child = parent*2 + 1;
								while (child < halfSize)
								{
									if (child < halfSize-1 && right[child+1] < right[child])
										++child;
									if (val <= right[child])
										break;
									right[parent] = right[child];
									parent = child;
									child = parent*2 + 1;
								}
								right[parent] = val;
							}
						}
					}
					break;
				}
			}
		}

		delete[] left;
		delete[] right;
		return median;
	}

	/*! @}*/
};
