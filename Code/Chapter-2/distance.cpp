#include <stdio.h>

#define EPS 0.01

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>

// Функция из примера не работала. Переписал
float pointToSegDistance ( const glm::vec2 p, const glm::vec2 a, const glm::vec2 b )
{
    // Находим направляющий вектор прямой и вектор нормали
    const glm::vec2 l = b - a;
    const glm::vec2 n(l.y, -l.x);

    // Находим коэффициенты канонического уравнения прямой
    float pA = n.x;
    float pB = n.y;
    float pC = -glm::dot(n, a);

    // По формулам находим координаты ближайшей точки на прямой к данной.
    // Формулы из учебника по высшей математике
    glm::vec2 q;
    q.x = (pB * (pB * p.x - pA * p.y) - pA * pC) / (pA * pA + pB * pB);
    q.y = (pA * (pA * p.y - pB * p.x) - pB * pC) / (pA * pA + pB * pB);

    // Проверяем, находится ли данная точка на отрезке
    bool xInAB = (q.x > a.x && q.x < b.x) || (q.x > b.x && q.x < a.x);
    bool yInAB = (q.y > a.y && q.y < b.y) || (q.y > b.y && q.y < a.y);
    if (xInAB && yInAB)
    {
        // Возвращаем расстояние до данной точки
        return glm::length(q - p) * glm::length(q - p);
    }

    // Иначе возвращаем координаты ближайшего конца отрезка
    float lengthToA = glm::length(a - p) * glm::length(a - p);
    float lengthToB = glm::length(b - p) * glm::length(b - p);

    if (lengthToA < lengthToB)
    {
        return lengthToA;
    }
    else
    {
        return lengthToB;
    }
}

int main(int argc, char * argv [])
{
    glm::vec2 a(1, 1);
    glm::vec2 b(4, 4);
    glm::vec2 p(-1, -1);

    float result = pointToSegDistance(p, a, b);
    printf("Distance to this point = %f\n", sqrt(result));
}
