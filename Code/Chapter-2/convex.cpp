#include <stdio.h>

#define EPS 0.01

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>

bool isConvex ( const glm::vec2 v [], int n )
{
    glm::vec2 prev = v[n-1];
    for ( int i = 0; i < n; i++ )
    {
        // Текущая точка
        glm::vec2 cur = v [i];

        // Вектор текущего ребра
        glm::vec2 dir = cur - prev;

        // Вектор нормали к текущему ребру
        glm::vec2 c = glm::vec2 ( dir.y, -dir.x );

        // Расстояние от текущего ребра до начала координат (формула (n,u) + d = 0; d = -(n,u))
        float d = -glm::dot ( c, cur );

        // Где находятся остальные точки - пока неизвестно
        int sign = 0;

        for ( int j = 0; j < n; j++ )
        {
            // Подставляем текущую точку в уравнение прямой. Тем самым находим её расстояние от этой прямой
            float dp = d + glm::dot ( c, v [j] );

            // Проверка на то, что точка лежит на данной прямой. Вообще говоря, даёт возможность не
            // делать дополнительной проверки для точек, являющихся компонентами текущего ребра
            if ( fabs ( dp ) < EPS )
               continue;

            // Находим, с какой стороны от прямой находится данная точка
            int s = dp > 0 ? 1 : -1;

            // Если ещё не вычисляли, где находятся другие точки, будем считать местоположение этой как базовое
            if ( sign == 0 )
            {
               sign = s;
            }

            // Иначе если точка находится с другой стороны - многоугольник не выпуклый
            else if ( sign != s )
            {
                return false;
            }
        }

        // А эту инструкцию указать в примере забыли. Без неё ничего не заработает
        prev = cur;
    }

    return true;
}

int main (int argc, char * argv [])
{
    glm::vec2 points[] = {
        glm::vec2(1, 1),
        glm::vec2(1, 3),
        glm::vec2(2, 4),
        glm::vec2(3, 3),
        glm::vec2(3, 1)
    };

    bool result = isConvex(points, 5);
    printf("%d\n", result);
}
