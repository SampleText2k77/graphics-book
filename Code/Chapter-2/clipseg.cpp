#include <stdio.h>
#include <math.h>

#define EPS 0.01

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <glm/mat2x2.hpp>
#include <glm/geometric.hpp>

// Исправил ошибки
glm::vec2 outerNormal ( const glm::vec2 p [], int n , int i )
{
    // Находим индекс следующей точки в массиве для составления отрезка ребра
	const int i1 = (i+1) % n;

    // Выбираем точку для последующего определения, что нормаль внешняя. Выбрать можно любую точку
	const int j  = (i+2) % n;

    // Находим вектор нормали
	glm::vec2 nn ( p[i].y - p[i1].y, p[i1].x - p[i].x );

    // Если нормаль внутренняя, меняем её на внешнюю
	if ( glm::dot ( p[j] - p[i], nn ) > 0 )
		nn = -nn;

	return nn;
}

// TODO: не проходит тест a = (0, 0), b = (0, 4), polygon - квадрат с углами (1, 1), (3, 3)
// TODO: не проходит тест a = (0, 0), b = (2, 2), polygon - тот же самый
bool clipSegByConvexPoly ( const glm::vec2 p [], int n,
                           glm::vec2* a, glm::vec2* b )
{
    // Находим вектор, представляющий отрезок, который будет подвергнут отсечению
	const glm::vec2 d = *b - *a;

    // Инициализируем значения "настоящего" входного и выходного
    // параметра. При первом же сравнении эти значения изменятся
	float tMin = 1;
	float tMax = 0;

    // Если отрезок целиком находится вне многогранника, то эта переменная будет false
    bool hasIntersection = false;

	for ( int j = n - 1, i = 0; i < n; j = i++ )
	{
        // Находим внешнюю нормаль ребра (j, i)
		glm::vec2 nn = outerNormal ( p, n, j );

        // Находим косинус угла между внешней нормалью ребра и отрезком AB. Векторы
        // нормализуем, чтобы значение косинуса было в пределах [0, 1]
        const float ncos = glm::dot( glm::normalize(d), glm::normalize(nn) );

        // Если угол между внешней нормалью ребра и отрезком AB равен 90 градусов (косинус равен нулю),
        // то ребро и отрезок AB параллельны (или совпадают), в таком случае дальнейшие действия не имеют смысла
		if ( fabs (ncos) < EPS)
		{
			continue;
		}

        // Находим значение параметра t, при котором прямые, одна из которых содержит отрезок ребра,
        // а другая отрезок AB, пересекаются. При t = [0, 1] эта точка будет внутри отрезка ребра.
        // Это сделано по формуле, что дана в книге
		float pn = glm::dot ( p[j] - *a, nn );
        float dn = glm::dot ( d, nn );
		float t  = pn / dn;
        float tt = glm::length(p[j] - *a) / glm::length(d);

        // Обновляем значения "настоящих" входных и выходных значений.
        // Если t лежит не в пределах интервала [0, 1], конец отрезка внутри многоугольника
        if (t < 1) tMax = fmax(t, tMax);
        if (t > 0) tMin = fmin(t, tMin);

        // Проверяем, пересекает ли отрезок в данном случае многогранник
        if (t >= 0 && t <= 1) hasIntersection = true;
	}

    // Если мы не изменили значение tMin или tMax, значит все варианты были либо 0, либо 1. Делаем замену
    if (tMin == 1) tMin = 0;
    if (tMax == 0) tMax = 1;

    // Если мы не нашли значения параметра t, при котором отрезок пересекает
    // многогранник - весь отрезок должен быть отсечён
    if (!hasIntersection)
    {
        return false;
    }

    // Подставляем значения параметров в параметрическое уравнение прямой, на которой
    // лежит отрезок AB. Тем самым получаем отрезок, который останется после отсечения
    *b = *a + tMax * d;
	*a = *a + tMin  * d;

	return true;
}

bool clipSegByRect ( const glm::vec2& rMin, const glm::vec2& rMax,
                     glm::vec2& a, glm::vec2& b )
{
	float		p [4], q [4];
	glm::vec2	d = b - a;
	float	t1 = 0, t2 = 0;


	p [0] = -d.x;
	p [1] = d.x;
	p [2] = -d.y;
	p [3] = d.y;
	q [0] = a.x - rMin.x;
	q [1] = rMax.x - a.x;
	q [2] = a.y - rMin.y;
	q [3] = rMax.y - a.y;

	for ( int i = 0; i < 4; i++ )
	{
		if ( fabs ( p [i] ) < EPS )	// line is parallel
		{
			if ( q [i] >= 0 )
			{
				if ( i < 2 )
				{
					if ( a.y < rMin.y )
						a.y = rMin.y;

					if ( b.y > rMax.y )
						b.y = rMax.y;
				}

				if ( i > 1 )
				{
					if ( a.x < rMin.x )
						a.x = rMin.x;

					if ( b.x > rMax.x )
						b.x = rMax.x;
				}
			}
		}
	}

	for ( int i = 0; i < 4; i++ )
	{
		if ( fabs ( p [i] ) < EPS )
			continue;

		float	temp = q [i] / p [i];

		if ( p [i] < 0 )
		{
			if ( t1 <= temp )
				t1 = temp;
		}
		else
		{
			if ( t2 > temp )
				t2 = temp;
		}
	}


	if ( t1 >= t2 )
		return false;

	b = a + t2 * d;
	a = a + t1 * d;

	return true;
}

int main(int argc, char * argv [])
{
    glm::vec2 points[] = {
        glm::vec2(1, 1),
        glm::vec2(1, 3),
        glm::vec2(3, 3),
        glm::vec2(3, 1)
    };

    glm::vec2 a(0, 0);
    glm::vec2 b(2, 2);

    bool result = clipSegByConvexPoly(points, 4, &a, &b);
    if (result)
    {
        printf("After clipping: A(%f, %f) B(%f %f)\n", a.x, a.y, b.x, b.y);
    }
    else
    {
        printf("Whole segment is clipped\n");
    }
}
