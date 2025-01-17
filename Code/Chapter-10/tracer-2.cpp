//
// Small ray tracer
//
// Author: Alexey V. Boreskov
//

#include <stdio.h>
#include <chrono>
#include "tracer-2.h"
#include "randUtils.h"

Medium	air   = { 1, 0 };
Medium	glass = { 1.5, 0 };

Scene * Scene :: scene    = nullptr;
int		Scene :: level    = 0;
int		Scene :: rayCount = 0;

////////////////////////////// Scene ////////////////////////////////

Scene :: Scene ()
{
	scene = this;
}

Scene :: ~Scene ()
{
	for ( auto it : objects )
		delete it;
	
	for ( auto it : lights )
		delete it;
}

glm::vec3 Scene :: trace ( const Medium& curMedium, float weight, int depth, const ray& r ) const
{
	Object    * obj = nullptr;
	glm::vec3	color;
	HitData		hit;

	rayCount ++;

	if ( ( obj = intersect ( r, hit ) ) != nullptr )	// ray hit some object
	{
		color = shade ( curMedium, weight, depth + 1, r.pointAt ( hit.t ), r.getDir (), hit );

		if ( curMedium.extinct > EPS )
			color *= expf ( -hit.t * curMedium.extinct );
	}
	else
		color = getBackground ( r );

	return color;
}
					  
glm::vec3 compureCookTorrance ( const glm::vec3& n, const glm::vec3& l, const glm::vec3 &v, float roughness, const glm::vec3& r0, const glm::vec3 clr )
{
	glm::vec3	h  = glm::normalize ( l + v );
	float 		nh = std::max ( 0.0f, glm::dot ( n, h ) );
	float		nv = std::max ( 0.0f, glm::dot ( n, v ) );
	float		nl = glm::dot ( n, l );
	
					// compute Beckman
	float		r2   = roughness * roughness;
	float		nh2  = nh * nh;
	float		ex   = -(1.0f - nh2)/(nh2 * r2);
	float		d    = powf ( 2.7182818284f, ex ) / (r2*nh2*nh2); 
	
					// compute Fresnel
	float		ft   = powf (1.0f - nv, 5.0f );
	glm::vec3	f    = (1.0f -ft) * glm::vec3 ( 1.0f ) + ft* r0;
	
					// default G
	float x    = 2.0f * nh / glm::dot(v, h);
	float g    = std::min ( 1.0f, std::min ( x * nl, x * nv ) );
	
					// resulting color
	glm::vec3	ct   = f * (d * g / nv);

	return ct;
}

glm::vec3 Scene :: shade ( const Medium& curMedium, float weight, int depth, const glm::vec3& p, const glm::vec3& v, HitData& hit ) const
{
	SurfaceData	txt;
	glm::vec3	l;					// light vector
	bool		entering = true;	// flag whether we're entering or leaving object
	float		shadow;

	hit.object -> getSurface ( hit, txt );

	glm::vec3	r0   = glm::vec3 ( 0.92, 1.0, 0.23 );
	glm::vec3	clr  = txt.color;		//glm::vec3 ( 0.7, 0.1, 0.1, 1.0 );
	float		roughness = 0.09f;
	
	float		vn = glm::dot ( v, txt.n );
	
	if ( vn  > 0 )					// force ( -v, n ) > 0
	{
		txt.n    = -txt.n;
		vn       = -vn;
		entering = false;
	}

	glm::vec3	color ( ambient * txt.color * txt.ka );			// get ambient light

	for ( auto it : lights )
		if ( ( shadow = it -> shadow ( hit.pos, l ) ) > EPS )
		{
			float	ln = glm::dot ( l, txt.n );
			
			if ( ln > EPS )					// if light is visible
			{
				if ( txt.kd > EPS )			// compute direct diffuse light
					color += it -> color * txt.color * (txt.kd * shadow * ln);

				if ( txt.ks > EPS )			// compute direct specular light, via Cook-Torrance
					color += txt.ks * it -> color * compureCookTorrance ( txt.n, l, -v, roughness, r0, clr );
			}
		}
		
	float	rWeight = weight * txt.kr;		// weight of reflected ray
	float	tWeight = weight * txt.kt;		// weight of transmitted ray
											// check for reflected ray
	if ( rWeight > EPS && depth < MAX_DEPTH )
	{										// ideal reflection direction
		glm::vec3	r  = v - txt.n * (2 * vn);
		glm::vec3	c ( 0 );				// average reflected color
		int			numSamples = (int)std::max ( 1.0f, std::min ( 10.0f, txt.glossiness * 20.0f ) );
		float		sum        = 0;
											// trace rays to simulate soft reflections
		for ( int i = 0; i < numSamples; i++ )
		{
			glm::vec3	rNew = glm::normalize ( r + 0.3f * txt.glossiness * getRandomVector () );
			float		w    = powf ( glm::dot ( r, rNew ), 1 + txt.glossiness * 15.0f );

			c   += w * trace ( curMedium, rWeight, depth + 1, ray ( hit.pos, rNew ) );
			sum += w;
		}

		color += txt.kr * (c / sum);
	}
											// check for transmitted ray
	if ( tWeight > EPS && depth < MAX_DEPTH )
	{										// relative index of refraction
		float	eta  = curMedium.nRefr / ( entering ? txt.medium.nRefr : 1 );
		float	ci   = -vn;					// cosine of incident angle
		float	ctSq = 1 + eta*eta*( ci*ci - 1 );	// squared cosine of transmittance angle

		if ( ctSq > EPS )					// not a Total Internal Reflection
		{
			glm::vec3	t = v * eta + txt.n * ( eta*ci - sqrtf ( ctSq ) );

			if ( entering )					// ray enters object ( txt.Med )
				color += txt.kt * trace ( txt.medium, tWeight, depth + 1, ray ( hit.pos, t ) );
			else							// ray leaves object ( air )
				color += txt.kt * trace ( air, tWeight, depth + 1, ray ( hit.pos, t ) );
		}
	}

	return color;
}
					  
Object * Scene :: intersect ( const ray& r, HitData& hit ) const
{
	Object * closestObj  = nullptr;
	float	 closestDist = 1e10;			// assume it is far enough
	HitData	 ht;

	for ( auto obj : objects )				// check every object
		if ( obj -> intersect ( r, ht ) )	// if intersects then check distance
			if ( ht.t < closestDist )
			{
				closestDist = ht.t;
				closestObj  = obj;
				hit         = ht;
			}

	return closestObj;	
}

////////////////////////////// Camera ////////////////////////////////

Camera :: Camera ( const glm::vec3& p, const glm::vec3& v, const glm::vec3& u, float fx, int w, int h ) : pos ( p ), view ( v ), up ( u ), width ( w ), height ( h )
{
	right      = glm::cross ( view, up );
	fovx       = fx;
	fovy       = fovx * (float) height / (float) width;
	image      = new TgaImage ( width, height );
}

Camera :: ~Camera ()
{
	delete image;
}

bool Camera :: traceFrame ( const char * fileName ) const
{
	auto 	start = std::chrono::steady_clock::now ();
	float	tfx   = tanf ( fovx * M_PI / 180 / 2 );		// tan (fovx/2)
	float	tfy   = tanf ( fovy * M_PI / 180 / 2 );		// tan (fovy/2)
	
	for ( int i = 0; i < width; i++ )	
		for ( int j = 0; j < height; j++ )
		{
			float		x = (i - width/2)  * tfx * 2 / (float)width;
			float		y = -(j - height/2) * tfy * 2 / (float)height;
			ray			r ( pos, glm::normalize (view + x*right + y*up) );
			glm::vec3	c   = Scene::scene -> trace ( air, 1, 1, r );
			uint32_t	red   = (uint32_t)(255.0f * std::min ( 1.0f, c.x ));
			uint32_t	green = (uint32_t)(255.0f * std::min ( 1.0f, c.y ));
			uint32_t	blue  = (uint32_t)(255.0f * std::min ( 1.0f, c.z ));
			
			image -> putPixel ( i, j, image -> rgbToInt ( red, green, blue ) );
		}
	
	auto finish = std::chrono::steady_clock::now ();
	
	printf ( "Time to render frame %lf secs\n", std::chrono::duration_cast<std::chrono::duration<double>>(finish-start).count () );

	image -> writeToFile ( fileName );
	
	return true;
}

bool Camera :: traceFrameDistributed ( const char * fileName, int n1, int n2 ) const
{
	auto 	start      = std::chrono::steady_clock::now ();
	float	tfx        = tanf ( fovx * M_PI / 180 / 2 );		// tan (fovx/2)
	//float	tfy        = tanf ( fovy * M_PI / 180 / 2 );		// tan (fovy/2)
	float	hx         = tfx * 2 / (float)width;
	float	hy         = tfx * 2 / (float)width;
	float	hxSub      = hx / n1;
	float	hySub      = hy / n2;
	int		numSamples = n1 * n2;
	
	for ( int i = 0; i < width; i++ )	
		for ( int j = 0; j < height; j++ )
		{
			float		x = (i - width/2)  * hx;
			float		y = -(j - height/2) * hy;
			float		x1 = x - 0.5f * hx;
			float		y1 = y - 0.5f * hy;
			glm::vec3	c ( 0 );
			
			for ( int iSub = 0; iSub < n1; iSub++ )
				for ( int jSub = 0; jSub < n2; jSub++ )
				{
					float	xt = x1 + hxSub * (iSub + randUniform() );
					float	yt = y1 + hySub * (jSub + randUniform() );
					
					ray		r ( pos, glm::normalize (view + xt*right + yt*up) );
					
					c += Scene::scene -> trace ( air, 1, 1, r );
				}

			c /= numSamples;
			
			uint32_t	red   = (uint32_t)(255.0f * std::min ( 1.0f, c.x ));
			uint32_t	green = (uint32_t)(255.0f * std::min ( 1.0f, c.y ));
			uint32_t	blue  = (uint32_t)(255.0f * std::min ( 1.0f, c.z ));
			
			image -> putPixel ( i, j, image -> rgbToInt ( red, green, blue ) );
		}

	auto finish = std::chrono::steady_clock::now ();
	
	printf ( "Time to render frame %lf secs\n", std::chrono::duration_cast<std::chrono::duration<double>>(finish-start).count () );

	image -> writeToFile ( fileName );
	
	return true;
}

////////////////////////////// Lights ////////////////////////////////

float PointLight :: shadow ( const glm::vec3& pt, glm::vec3& l ) const
{
	l = pos - pt;		// vector to light source
							// distance to light source
	float	dist = glm::length ( l );
	HitData	hit;
	
	l /= dist;				// normalize vector l

	ray	r ( pt, l );		// shadow ray

	return ((Scene::scene -> intersect ( r, hit ) ) == nullptr || hit.t > dist) ? 1 : 0;
}

float SphericLight :: shadow ( const glm::vec3& pt, glm::vec3& l ) const
{
							// choose random point on sphere
	glm::vec3	p = pos + radius * getRandomVector ();
	
	l = p - pt;				// vector to this point on light sphere
							// distance to light source
	float	dist = glm::length ( l );
	HitData	hit;
	
	l /= dist;				// normalize vector l

	ray	r ( pt, l );		// shadow ray

	return ((Scene::scene -> intersect ( r, hit ) ) == nullptr || hit.t > dist) ? 1 : 0;
}

////////////////////////////// Geometry ////////////////////////////////

Sphere :: Sphere ( const glm::vec3& c, float r ) : Object (), center ( c )
{
	radius   = r;
	radiusSq = r * r;
}

bool	Sphere :: intersect ( const ray& r, HitData& hit ) const
{
	glm::vec3	l    = center - r.getOrigin();			// direction vector
	float		L2OC = glm::dot ( l,  l );				// squared distance
	float		tca  = glm::dot ( l, r.getDir () );		// closest dist to center
	float		t2hc = radiusSq - L2OC + tca*tca;
	float		t1, t2;

	if ( t2hc <= 0.0f )
		return false;

	t2hc = sqrtf ( t2hc );

	if ( tca < t2hc )		// we are inside
	{
		t1 = tca + t2hc;
		t2 = tca - t2hc;
	}
	else					// we are outside
	{
		t1 = tca - t2hc;
		t2 = tca + t2hc;
	}

	if ( fabs ( t1 ) < EPS )
		t1 = t2;

	if ( t1 > EPS )			// there is an intersection
	{
		hit.t         = t1;
		hit.pos       = r.pointAt ( t1 );
		hit.object    = this;
		hit.cache [0] = t1;
		hit.cache [1] = t2;
		
		return true;
	}
	
	return false;
}

void Sphere :: getSurface ( HitData& hit, SurfaceData& surface ) const
{
	surface.pos      = hit.pos;
	surface.n        = glm::normalize ( hit.pos - center );
	surface.color    = defSurface.color;
	surface.emission = defSurface.emission;
	surface.ka       = defSurface.ka;
	surface.kd       = defSurface.kd;
	surface.ks       = defSurface.ks;
	surface.kr       = defSurface.kr;
	surface.kt       = defSurface.kt;
	surface.p        = defSurface.p;
	surface.glossiness = defSurface.glossiness;
	surface.medium     = defSurface.medium;

	if ( texture != nullptr )
		texture -> apply ( hit, surface );
}

bool Plane :: intersect ( const ray& r, HitData& hit ) const
{
	float	vd = glm::dot ( n, r.getDir () );

	if ( vd > -EPS && vd < EPS )
		return false;

	hit.t         = -( glm::dot ( n, r.getOrigin () ) + d ) / vd;
	hit.pos       = r.pointAt ( hit.t );
	hit.object    = this;
	hit.cache [0] = vd;
	
	return hit.t > EPS;
}

void Plane :: getSurface ( HitData& hit, SurfaceData& surface ) const
{
	surface.pos      = hit.pos;
	surface.n        = n;
	surface.color    = defSurface.color;
	surface.emission = defSurface.emission;
	surface.ka       = defSurface.ka;
	surface.kd       = defSurface.kd;
	surface.ks       = defSurface.ks;
	surface.kr       = defSurface.kr;
	surface.kt       = defSurface.kt;
	surface.p        = defSurface.p;
	surface.glossiness = defSurface.glossiness;
	surface.medium     = defSurface.medium;

	if ( texture != nullptr )
		texture -> apply ( hit, surface );
}
