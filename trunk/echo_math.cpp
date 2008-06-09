// echo_math.cpp

/*
    This file is part of L-Echo.

    L-Echo is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    L-Echo is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with L-Echo.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <echo_debug.h>
#include <echo_error.h>
#include <echo_math.h>
//#include <iostream>
#include <cmath>

#ifdef ARM9
	#include <nds.h>
	#include <nds/arm9/trig_lut.h>
	
	#define ECHO_COSI(deg) (f32tofloat(COS[(int)( ( ABS((deg)) % 360 ) / 360.0f * LUT_SIZE )]))
	#define ECHO_COSF(deg) (ECHO_COSI((int)(deg)))
	#define ECHO_SINI(deg) (ECHO_COSI(90 - (deg)))
	#define ECHO_SINF(deg) (ECHO_SINI((int)(deg)))
	
	void init_math(){}
	
#else
	static float cos_table[360];
	
	void init_math()
	{
	    int deg = 0;
	    while(deg < 360)
	    {
		cos_table[deg] = cosf(TO_RAD(deg));
		deg++;
	    }
	}
	
	#define ECHO_COSI(deg) (cos_table[ABS((deg)) % 360])
	#define ECHO_COSF(deg) (ECHO_COSI((int)(deg)))
	#define ECHO_SINI(deg) (ECHO_COSI(90 - (deg)))
	#define ECHO_SINF(deg) (ECHO_SINI((int)(deg)))
#endif

vector3f::vector3f()
{
	x = 0;
	y = 0;
	z = 0;
}

vector3f::vector3f(float my_x, float my_y, float my_z)
{
	x = my_x;
	y = my_y;
	z = my_z;
}

int vector3f::angle_similar(vector3f v)
{
	return(ABS(x - v.x) < EPSILON &&
		(ABS(y - v.y) < EPSILON || ABS(ABS(y - v.y) - 360) < EPSILON) &&
		ABS(z - v.z) < EPSILON);
}

int vector3f::operator !=(vector3f v)
{
    return(!(*this == v));
}

int vector3f::operator ==(vector3f v)
{
	return(ABS(x - v.x) < EPSILON &&
		ABS(y - v.y) < EPSILON &&
		ABS(z - v.z) < EPSILON);
}

float vector3f::length()
{
	return(sqrt(x * x + y * y + z * z));
}

void vector3f::dump()
{
#ifdef ARM9
	ECHO_PRINT("vector3f (* 100): [%i,%i,%i]", (int)(x * 100), (int)(y * 100), (int)(z * 100));
#else
	ECHO_PRINT("vector3f: [%f,%f,%f]", x, y, z);
#endif
}

void vector3f::set(vector3f copy_from)
{
	x = copy_from.x;
	y = copy_from.y;
	z = copy_from.z;
}

vector3f vector3f::negate()
{
	vector3f* ret = new vector3f(-x, -y, -z);
	CHKPTR(ret);
	return(*ret);
}

void dump_line3f(line3f ln)
{
	ECHO_PRINT("lines3f: [");
	ln.p1.dump();
	ECHO_PRINT(",");
	ln.p2.dump();
	ECHO_PRINT("]");
}

int operator ==(line3f ln1, line3f ln2)
{
	return( ( (ln1.p1 == ln2.p1) && (ln1.p2 == ln2.p2) )
		|| ( (ln1.p1 == ln2.p2) && (ln1.p2 == ln2.p1) ) );
}

vector3f vector3f::operator +(vector3f vec)
{
    vector3f* ret = new vector3f(x + vec.x, y + vec.y, z + vec.z);
    CHKPTR(ret);
    return(*ret);
}

vector3f vector3f::operator -(vector3f vec)
{
    vector3f* ret = new vector3f(x - vec.x, y - vec.y, z - vec.z);
    CHKPTR(ret);
    return(*ret);
}

vector3f vector3f::operator *(float f)
{
	vector3f* ret = new vector3f(x * f, y * f, z * f);
	CHKPTR(ret);
	return(*ret);
}

vector3f* vector3f::angle_xy()
{
	vector3f* ret = NULL;
	if(z == 0)
		ret = new vector3f(0, TO_DEG(atan2f(x, z)), 0);
	else
	{
		float temp = sqrt(x * x  + z * z);
		ret = new vector3f(-TO_DEG(atan2f(y, z > 0 ? temp : -temp)),
					TO_DEG(atan2f(x, z)), 0);
	}
        CHKPTR(ret);
        return(ret);

}

float vector3f::dist(vector3f other)
{
	return(sqrt(pow(x - other.x, 2) + pow(y - other.y, 2) + pow(z - other.z, 2)));
}

vector3f* vector3f::rotate_xy(vector3f rot)
{
	if(rot.x == 0 && rot.y == 0 && rot.z == 0)
		return(this);
	vector3f* ret = new vector3f(x
			, y * ECHO_COSF(rot.x) - z * ECHO_SINF(rot.x)
			, y * ECHO_SINF(rot.x) + z * ECHO_COSF(rot.x));
	CHKPTR(ret);
	float z_save = ret->z;
	ret->z = ret->z * ECHO_COSF(rot.y) - ret->x * ECHO_SINF(rot.y);
	ret->x = z_save * ECHO_SINF(rot.y) + ret->x * ECHO_COSF(rot.y);
	return(ret);
}

void vector3f::set(float my_x, float my_y, float my_z)
{
	x = my_x;
	y = my_y;
	z = my_z;
}

vector3f* vector3f::neg_rotate_yx(vector3f rot)
{
	//float rad_x = -TO_RAD(rot.x), rad_y = -TO_RAD(rot.y);
	if(rot.x == 0 && rot.y == 0 && rot.z == 0)
		return(this);
	vector3f* ret = new vector3f(z * ECHO_SINF(-rot.y) + x * ECHO_COSF(-rot.y), y
                                , z * ECHO_COSF(-rot.y) - x * ECHO_SINF(-rot.y));
	CHKPTR(ret);
	float y_save = ret->y;
	ret->y = ret->y * ECHO_COSF(-rot.x) - ret->z * ECHO_SINF(-rot.x);
	ret->z = y_save * ECHO_SINF(-rot.x) + ret->z * ECHO_COSF(-rot.x);
	return(ret);
}

angle_range::angle_range(vector3f* my_v1, vector3f* my_v2)
{
	v1 = my_v1;
	v2 = my_v2;
}

//is b in between a and c?
#define IN_BETWEEN(a,b,c) (((a) <= (b) && (b) <= (c)) || ((c) <= (b) && (b) <= (a)))

int angle_range::is_vec_in(vector3f v)
{
	return(IN_BETWEEN(v1->x, v.x, v2->x) 
		&& IN_BETWEEN(v1->y, v.y, v2->y)
		&& IN_BETWEEN(v1->z, v.z, v2->z));
}

vector3f* vector3f::rotate_about_y(float angle)
{
#ifdef STRICT_MEM
	vector3f* ret = new vector3f(z * ECHO_SINF(angle) + x * ECHO_COSF(angle)
			, y, z * ECHO_COSF(angle) - x * ECHO_SINF(angle));
	CHKPTR(ret);
	return(ret);
#else
	return(new vector3f(z * ECHO_SINF(angle) + x * ECHO_COSF(angle)
			, y, z * ECHO_COSF(angle) - x * ECHO_SINF(angle)));
#endif
}

vector3f* vector3f::angle_to_real()
{
#ifdef STRICT_MEM
	vector3f* cam_norm = new vector3f(0, 0, 10);
	return(cam_norm->rotate_xy(*this));
#else
	return((new vector3f(0, 0, 10))->rotate_xy(*this));
#endif
}

