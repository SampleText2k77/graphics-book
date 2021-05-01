#include <stdio.h>

#define EPS 0.01

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>

// Разбил на несколько условий для удобочитаемости, исправил ошибки
bool ptInPoly( const glm::vec2 p [], int n, const glm::vec2 pt )
{
    int i, j, c = 0;

    for ( i = 0, j = n - 1; i < n; j = i++ )
    {
        // Проверяем, что координаты точки по Y в пределах данного ребра
        bool ptYWithinBounds = (
            ((p[i].y <= pt.y) && (pt.y < p[j].y)) ||
            ((p[j].y <= pt.y) && (pt.y < p[i].y))
        );

        // Здесь каким-то хитрым образом выясняется, находится ли данная точка слева
        // от ребра. Если она слева, значит луч, пущенный вправо, его пересечёт. На
        // основании всех таких пересечений можно будет сделать вывод о том, лежит ли
        // точка внутри многоугольника. Но опять же, мы уже делали подобные операции
        // в предыдущих примерах, а здесь что-то абсолютно незнакомое
        bool ptXWithinBounds = (
            pt.x < (p[j].x - p[i].x) * (pt.y - p[i].y) / (p[j].y - p[i].y) + p[i].x
        );

        // Если точка слева от ребра и в его пределах по Y, значит она пересечёт данное ребро
        if (ptYWithinBounds && ptXWithinBounds)
        {
            c = !c;
        }
    }

    return c != 0;
}

int main(int argc, char * argv [])
{
    glm::vec2 points[] = {
        glm::vec2(-1, -1),
        glm::vec2(3, 1),
        glm::vec2(3, 3),
        glm::vec2(2, 2),
        glm::vec2(1, 3)
    };

    glm::vec2 pt(2.4, 2.5);

    bool result = ptInPoly(points, 5, pt);
    printf("Point is within polygon: %d\n", result);
}
