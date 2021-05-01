#include <stdio.h>

#define EPS 0.01

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <glm/mat2x2.hpp>
#include <glm/geometric.hpp>

using namespace glm;

bool pointInTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 p)
{
	const glm::vec2 a1 = a - c;
	const glm::vec2 b1 = b - c;
	const float aa = glm::dot(a1, a1);
	const float	ab = glm::dot(a1, b1);
	const float bb = glm::dot(b1, b1);
	const float	d  = aa*bb - ab*ab;

	if ( fabs ( d ) < EPS )
		return false;

	const glm::vec2 p1 = p - c;
	const float pa = glm::dot(p1, a1);
	const float	pb = glm::dot(p1, b1);
	const float u  = (pa*bb - pb*ab) / d;

	if ( u < 0 || u > 1 )
		return false;

	const float v = (pb*aa - pa*ab) / d;

	if ( v < 0 || v > 1 )
		return false;

	return u + v <= 1;
}

int main (int argc, char * argv [])
{
    glm::vec2 a(1, 1);
    glm::vec2 b(1, 3);
    glm::vec2 c(3, 1);
    glm::vec2 p(-10, -1);

    bool result = pointInTriangle(a, b, c, p);

    printf("%d\n", result);
}
