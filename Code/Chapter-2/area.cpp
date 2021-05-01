#include <stdio.h>

#define EPS 0.01

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <glm/mat2x2.hpp>
#include <glm/geometric.hpp>

// По формуле в книге
float signedArea2(const glm::vec2 p[], int n)
{
    float square = 0;

    square += p[n-1].x * p[0].y - p[0].x * p[n-1].y;

    for (int i = 0; i <= n-2; i++)
    {
        square += p[i].x * p[i+1].y - p[i+1].x * p[i].y;
    }

    return 0.5f * square;
}

// Другой вариант этой же формулы из примера as is
// По сути тут просто определители раскрыты по формуле и объединены другим способом
float signedArea ( const glm::vec2 p [], int n )
{
    float sum = p [0].x   * (p [1].y - p [n-1].y) +
                p [n-1].x * (p [0].y - p [n-2].y);

    for ( int i = 1; i < n - 1; i++ )
    {
        sum += p [i].x * (p [i+1].y - p [i-1].y);
    }

    return 0.5f * sum;
}

bool isClockwise ( const glm::vec2 p [], int n )
{
	return signedArea ( p, n ) > 0;
}

int main (int argc, char * argv [])
{
    glm::vec2 points[] = {
        glm::vec2(1, 1),
        glm::vec2(1, 700),
        glm::vec2(150, 150),
        glm::vec2(250, 1)
    };

    float result = signedArea(points, 4);
    printf("%f\n", result);

    result = signedArea2(points, 4);
    printf("%f\n", result);

    bool clockwise = isClockwise(points, 4);
    printf("%d\n", clockwise);
}
