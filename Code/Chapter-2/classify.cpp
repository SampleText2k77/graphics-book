#include <stdio.h>

#define EPS 0.01

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <glm/mat2x2.hpp>
#include <glm/geometric.hpp>

#define DEGENERATE_COINCIDE 0
#define DEGENERATE_DIFFERENT 1
#define LEFT 2
#define RIGHT 3
#define START 4
#define END 5
#define BEHIND 6
#define BEYOND 7
#define BETWEEN 8
#define IN_POSITIVE 9
#define IN_NEGATIVE 10
#define IN_BOTH 11

int classify ( const glm::vec2& a, const glm::vec2& b, const glm::vec2& p )
{
	const glm::vec2 a2b = b - a; // от a к b
	const glm::vec2 a2p = p - a; // от a к p

    printf("%f %f\n", a2b.x, a2b.y);
    printf("%f %f\n", a2p.x, a2p.y);

    // Проверка на совпадение концов отрезка
    // В таком случае координаты x и y будут равны нулю
    // Чтобы не делать 4 сравнения здесь сделано 2
	if ( fabs ( a2b.x ) + fabs ( a2b.y ) < EPS )
    {
		if ( fabs ( a2p.x ) + fabs ( a2p.y ) < EPS )
			return DEGENERATE_COINCIDE;	// a == b == p
		else
	        return DEGENERATE_DIFFERENT; // a == b != p
    }

    // Здесь векторное произведение двух векторов
    // Но площадь треугольника в таком случае 0.5 * |a x b|
	float area = 0.5 * (a2b.x * a2p.y - a2p.x * a2b.y);
    printf("Triangle area: %f\n", area);

	if ( area > EPS )
		return LEFT;

	if ( area < -EPS )
		return RIGHT;

	if ( glm::length ( p - a ) < EPS )
		return START;

	if ( glm::length ( p - b ) < EPS )
		return END;

    // ???
	if ((a2b.x*a2p.x < 0) || (a2b.y*a2p.y < 0))
		return BEHIND;

	if ( glm::length(a2b) < glm::length(a2p) )
		return BEYOND;

	return BETWEEN;
}

int classifyLineCircle ( const glm::vec2& n, float d,
                         const glm::vec2& c, float r )
{
	const float signedDistance = glm::dot(c, n) + d;

	if ( signedDistance >= r + EPS )
		return IN_POSITIVE;

	if ( signedDistance <= -r - EPS )
		return IN_NEGATIVE;

	return IN_BOTH;
}

int classifyBox ( const glm::vec2& pMin, const glm::vec2& pMax, const glm::vec2& n, float d )
{
    // Находим ближайшую к прямой вершину
	glm::vec2 pn(n.x >= 0 ? pMin.x : pMax.x, n.y >= 0 ? pMin.y : pMax.y);

    // Подставляем это вершину в уравнение прямой, находим расстояние до этой прямой
	const float f = glm::dot ( n, pn ) + d;

	if (f > 0)
		return IN_POSITIVE;

    // Меняем P от n на P от -n
	pn.x = n.x >= 0 ? pMax.x : pMin.x;
	pn.y = n.y >= 0 ? pMax.y : pMin.y;

	if ( glm::dot ( n, pn ) + d < 0 )
		return IN_NEGATIVE;

	return IN_BOTH;
}

int main (int argc, char * argv [])
{
    glm::vec2 a(1, 1);
    glm::vec2 b(5, 5);
    glm::vec2 c(0.5, 0.5);

    int result = classify(a, b, c);
    printf("%d\n", result);
}

