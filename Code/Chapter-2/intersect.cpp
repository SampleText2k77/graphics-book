#include <stdio.h>
#include <math.h>

#define EPS 0.01

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <glm/mat2x2.hpp>
#include <glm/geometric.hpp>

#define COINCIDE 0
#define PARALLEL 1
#define INTERSECT 2

#define NO_INTERSECTIONS 0
#define SINGLE_INTERSECTION 1
#define MULTIPLE_INTERSECTIONS 2

// Пример по учебнику не работает. Похоже там имелась в виду другая логика, см. комментарии в функции
bool doLinesCoincide (glm::vec2 n1, float d1, glm::vec2 n2, float d2 )
{
    // Нормализуем векторы
    n1 = glm::normalize(n1);
    n2 = glm::normalize(n2);

    // Находим угол между нормалями
    float ncos = glm::dot(n1 , n2);

    // Если угол между нормалями равен 0 градусов (ncos = 1)
	if (fabs(ncos - 1) < EPS)
    {
        // И если прямые находятся на одном расстоянии от начала координат
		return fabs ( d1 - d2 ) < EPS;
    }

    // Если угол между нормалями равен 180 градусов (ncos = -1)
	else if (fabs(ncos + 1) < EPS)
    {
        // И если прямые находятся на одном расстоянии от начала координат
		return fabs ( d1 + d2 ) < EPS;
    }

    return false;
}

// Вариант по формулам чуть выше примера в книге
int findLinesRelation (glm::vec2 n1, float d1, glm::vec2 n2, float d2 )
{
    // Нормализуем векторы (хотя в принципе можно и без этого обойтись)
    n1 = glm::normalize(n1);
    n2 = glm::normalize(n2);

    // Находим определитель матрицы СЛАУ
    const float delta = n1.x * n2.y - n2.x * n1.y;

    // Если определитель равен нулю, решений, вообще говоря, нет
	if (fabs(delta) < EPS)
    {
        // Условие совпадения прямых
        if ((n1 == n2 && d1 == d2) || (n1 == -n2 && d1 == -d2))
        {
            return COINCIDE;
        }

        return PARALLEL;
    }

    // Иначе есть одно и только одно решение, но это пусть уже думает findLineIntersection
    return INTERSECT;
}

// Убрал вызов doLinesCoincide
bool findLineIntersection ( const glm::vec2& n1, float d1,
                            const glm::vec2& n2, float d2,
                            glm::vec2& p )
{
    // Находим определитель матрицы СЛАУ
	const float det = n1.x * n2.y - n1.y * n2.x;

    // Если определитель равен нулю, решений, вообще говоря, нет
    if ( fabs ( det ) < EPS )
    {
        return false;
    }

    // Находим решение по формулам Крамера
	p.x = (d2*n1.y - d1*n2.y)/det;
	p.y = (d1*n2.x - d2*n1.x)/det;

	return true;
}

// Для моих тестов функция давала неверные результаты. Переписал
int findSegIntersection ( const glm::vec2& a1, const glm::vec2& b1,
                          const glm::vec2& a2, const glm::vec2& b2,
                          glm::vec2& p )
{
    // Находим направляющие векторы отрезков
    const glm::vec2 l1 = b1 - a1;
    const glm::vec2 l2 = b2 - a2;

    // И их векторы нормалей
    const glm::vec2 n1 ( l1.y , -l1.x );
    const glm::vec2 n2 ( l2.y , -l2.x );

    // Также для уравнения прямых находим их расстояние до начала координат
    const float d1 = -glm::dot ( a1, n1 );
    const float d2 = -glm::dot ( a2, n2 );

    // При помощи определителя проверяем на параллельность. Второе условие убрал,
    // зачем оно - я не разобрался. Мы имеем дело с прямыми, как проверять их на
    // параллельность мы уже знаем
    const float delta = n1.x*n2.y - n1.y*n2.x;

    // Если определитель равен нулю, то прямые могут совпасть
    // и точек пересечения, вообще говоря, может быть бесчисленное множество
    if ( fabs ( delta ) < EPS )
    {
        // Это произойдёт, если один из концов одного из отрезков будет внутри другого.
        // Найдём, есть ли внутри одного из отрезков точка другого
        bool hasX = (a1.x >= a2.x && a1.x <= b2.x) ||
                    (a1.x >= b2.x && a2.x <= a2.x) ||
                    (a2.x >= a1.x && a2.x <= b1.x) ||
                    (a2.x >= b1.x && a2.x <= a1.x);

        bool hasY = (a1.y >= a2.y && a1.y <= b2.y) ||
                    (a1.y >= b2.y && a2.y <= a2.y) ||
                    (a2.y >= a1.y && a2.y <= b1.y) ||
                    (a2.y >= b1.y && a2.y <= a1.y);

        // Если есть точки внутри другого отрезка, то пересечений множество
        // На совпадение точек не проверяю, а вообще, может быть одно пересечение
        if (hasX && hasY)
        {
            return MULTIPLE_INTERSECTIONS;
        }

        return NO_INTERSECTIONS;
    }

    // Находим точку пересечения прямых по формулам Крамера
	p.x = (d2*n1.y - d1*n2.y) / delta;
	p.y = (d1*n2.x - d2*n1.x) / delta;

    // Проверяем, что точка пересечения находится внутри обоих отрезков.
    // Условие из примера не учитывает второй отрезок по непонятной причине,
    // поэтому переписал. Возможно, есть более эффективный способ сделать эти проверки.
    bool xIn1 = (p.x >= a1.x && p.x <= b1.x) || (p.x >= b1.x && p.x <= a1.x);
    bool yIn1 = (p.y >= a1.y && p.x <= b1.y) || (p.x >= b1.y && p.x <= a1.y);
    bool xIn2 = (p.x >= a2.x && p.x <= b2.x) || (p.x >= b2.x && p.x <= a2.x);
    bool yIn2 = (p.x >= a2.x && p.x <= b2.x) || (p.x >= b2.x && p.x <= a2.x);
    if (xIn1 && yIn1 && xIn2 && yIn2)
    {
        return SINGLE_INTERSECTION;
    }

    return NO_INTERSECTIONS;
}

int main(int argc, char * argv [])
{
    glm::vec2 n1(1, 1);
    glm::vec2 n2(-2, -2);

    float d1 = 1;
    float d2 = 0;

    int result = findLinesRelation(n1, d1, n2, d2);
    printf("%d\n", result);

    bool result2 = doLinesCoincide(n1, d1, n2, d2);
    printf("%d\n", result2);

    glm::vec2 p;
    bool result3 = findLineIntersection(n1, d1, n2, d2, p);
    if (result3)
    {
        printf("%f %f\n", p.x, p.y);
    }
    else
    {
        printf("Lines do not intersect\n");
    }

    glm::vec2 a1(1, 1);
    glm::vec2 b1(5, 5);
    glm::vec2 a2(5, 1);
    glm::vec2 b2(1, 5);

    int result4 = findSegIntersection(a1, b1, a2, b2, p);
    if (result4 == SINGLE_INTERSECTION)
    {
        printf("Intersection point is x=%f y=%f\n", p.x, p.y);
    }
    else printf("%d\n", result4);
}
