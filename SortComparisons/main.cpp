#define _USE_MATH_DEFINES
#include <cmath>
#include <time.h>
#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <glut.h>
#include <vector>
#include "hsv.h"

#pragma region Vec2Stuff
class Vec2
{
public:
	double x;
	double y;

	Vec2::Vec2() { x = 0; y = 0; };
	Vec2::Vec2(double _x, double _y) { x = _x; y = _y; }

	double Vec2::magnitude()
	{
		return sqrt(pow(x, 2) + pow(y, 2));
	}

	double Vec2::distance(Vec2 other)
	{
		return Vec2(x - other.x, other.y - y).magnitude();
	}

	Vec2 Vec2::normalise()
	{
		double mag = magnitude();
		return Vec2(x / mag, y / mag);
	}
};
#pragma endregion Vec2Stuff

#pragma region Inits
void idle();
void draw();
void vertex2d(Vec2);
Vec2 convertCoords(Vec2);

const unsigned int segments = 512;
const double col_step = 360 / (double)(segments-1);

Vec2 window = Vec2(1200, 600);
Vec2 hlf_window = Vec2(window.x*0.5, window.y*0.5);
Vec2 outset = Vec2(150, 150);
Vec2 inner = Vec2(window.x - outset.x, window.y - outset.y);
Vec2 segment_size = Vec2(inner.x / segments, inner.y / segments);
Vec2 offset = Vec2(outset.x * 0.5, outset.y * 0.5);

unsigned int values[segments];
// --- Sorting Methods ---
bool sorted = false;
// Bubble sort
int bubble_stepper = 0;
bool val_swapped = false;
void bubble_sort_step();
// Insertion Sort
int max_id = -1, stack = 0, insertion_stepper = 0;
void insertion_sort_step();
// QUICK SORT
#define  MAX_LEVELS  300
int piv, beg[MAX_LEVELS], end[MAX_LEVELS], incr = 0, L, R, swap;
void quick_sort_step(unsigned int *arr);
int startScan = 0;
#pragma endregion Inits

#pragma region Main

int main(int argc, char **argv)
{
	srand(time(NULL));
	beg[0] = 0; end[0] = segments;

	std::vector<int> assign_vals;
	for (int i = 1; i <= segments; i++)
	{
		assign_vals.push_back(i);
	}
	
	for (int i = 0; i < segments; i++)
	{
		int index = (assign_vals.size() <= 1 ? 0 : (rand() % (assign_vals.size() - 1)));
		values[i] = assign_vals[index];
		assign_vals.erase(assign_vals.begin() + index);
	}

	glutInit(&argc, argv);

	glutInitWindowPosition(32, 32);
	glutInitWindowSize(window.x, window.y);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	glutCreateWindow("Sort Comparisons");

	glutDisplayFunc(draw);
	glutIdleFunc(idle);

	glutMainLoop();

	return 0;
}

#pragma endregion Main

#pragma region Drawing

void draw()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
	for (int i = 0; i < segments; i++)
	{
		double test_val = 1;
		//if (insertion_stepper == i && !sorted) { test_val = 0.2; }
		//if (bubble_stepper == i && !sorted) { test_val = 0.2; }
		if (startScan == i && !sorted) { test_val = 0.2; }
		rgb col = hsv2rgb(col_step * (values[i]-1), test_val, test_val);
		glColor3d(col.r, col.g, col.b);
		Vec2 bar_size = Vec2(segment_size.x, values[i] * segment_size.y);
		Vec2 draw_origin = Vec2(offset.x + i * bar_size.x, offset.y + inner.y);
		vertex2d(draw_origin);
		draw_origin.y -= bar_size.y;
		vertex2d(draw_origin);
		draw_origin.x += bar_size.x;
		vertex2d(draw_origin);
		draw_origin.y += bar_size.y;
		vertex2d(draw_origin);
	}
	glEnd();

	glutSwapBuffers();
}

void vertex2d(Vec2 _pos)
{
	Vec2 glCoords = convertCoords(_pos);
	glVertex2d(glCoords.x, -glCoords.y);
}

Vec2 convertCoords(Vec2 _px)
{
	return Vec2(_px.x / hlf_window.x - 1, _px.y / hlf_window.y - 1);
}

#pragma endregion Drawing

#pragma region Sorting

void insertion_sort_step()
{
	if ((unsigned)insertion_stepper < (segments - stack))
	{
		int cur_val = values[insertion_stepper];
		max_id = (insertion_stepper == 0 ? insertion_stepper : ((unsigned)cur_val > values[max_id] ? insertion_stepper : max_id));
		++insertion_stepper;
	}
	else
	{
		int stack_val = values[segments - stack - 1];
		values[segments - stack - 1] = values[max_id];
		values[max_id] = stack_val;

		insertion_stepper = 0;
		++stack;
		max_id = -1;
	}
	if (stack == segments - 1) { sorted = true; }
}

void bubble_sort_step()
{
	if (bubble_stepper < segments - 1)
	{
		if (values[bubble_stepper] > values[bubble_stepper+1])
		{
			int val_f = values[bubble_stepper];
			values[bubble_stepper] = values[bubble_stepper + 1];
			values[bubble_stepper + 1] = val_f;
			val_swapped = true;
		}
		++bubble_stepper;
	}
	else
	{
		if (val_swapped)
		{
			bubble_stepper = 0;
			val_swapped = false;
		}
		else
		{
			sorted = true;
		}
	}
}

void selection_sort_step()
{
	int minIndex, minValue;
	if(startScan < (segments - 1))
	{
		minIndex = startScan;
		minValue = values[startScan];
		for (int index = startScan + 1; index < segments; index++)
		{
			if (values[index] < minValue)
			{
				minValue = values[index];
				minIndex = index;
			}
		}
		values[minIndex] = values[startScan];
		values[startScan] = minValue;

		startScan++;
	}
}

// NOT MY WORK
void quick_sort_step(unsigned int *arr)
{
	if (incr >= 0)
	{
		L = beg[incr]; R = end[incr] - 1;
		if (L<R)
		{
			piv = arr[L];
			while (L<R) 
			{
				while (arr[R] >= (unsigned)piv && L<R) R--; if (L<R) arr[L++] = arr[R];
				while (arr[L] <= (unsigned)piv && L<R) L++; if (L<R) arr[R--] = arr[L];
			}
			arr[L] = piv; beg[incr + 1] = L + 1; end[incr + 1] = end[incr]; end[incr++] = L;
			if (end[incr] - beg[incr]>end[incr - 1] - beg[incr - 1])
			{
				swap = beg[incr]; beg[incr] = beg[incr - 1]; beg[incr - 1] = swap;
				swap = end[incr]; end[incr] = end[incr - 1]; end[incr - 1] = swap;
			}
		}
		else 
		{
			incr--;
		}
	}
	else
	{
		sorted = true;
	}
}

void idle()
{
	if (!sorted)
	{
		//insertion_sort_step();
		//bubble_sort_step();
		//quick_sort_step(&values[0]);
		selection_sort_step();
	}
	glutPostRedisplay();
}


#pragma endregion Sorting