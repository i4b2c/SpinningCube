#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>

/*
Useful Links

https://en.wikipedia.org/wiki/Rotation_matrix
*/

float cube_vertices[8][3] = {
    {-1, -1, -1},
    { 1, -1, -1},
    { 1,  1, -1},
    {-1,  1, -1},
    {-1, -1,  1},
    { 1, -1,  1},
    { 1,  1,  1},
    {-1,  1,  1}
};

int cube_edges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

int spaces = 10;

void project_3D_to_2D(float x3d, float y3d, float z3d, int *x2d, int *y2d, int width, int height)
{
    float scale_factor = 2.0 / (z3d + 5);
    *x2d = (int)(x3d * scale_factor * width / 2 + width / 2);
    *y2d = (int)(y3d * scale_factor * height / 2 + height / 2);
}

void rotate(float vertex[3], float angleX, float angleY, float angleZ)
{
    float x = vertex[0], y = vertex[1], z = vertex[2];
    float newY = y * cos(angleX) - z * sin(angleX);
    float newZ = y * sin(angleX) + z * cos(angleX);

    y = newY;
    z = newZ;

    float newX = x * cos(angleY) + z * sin(angleY);
    newZ = -x * sin(angleY) + z * cos(angleY);
    x = newX;
    z = newZ;

    newX = x * cos(angleZ) - y * sin(angleZ);
    newY = x * sin(angleZ) + y * cos(angleZ);

    vertex[0] = newX;
    vertex[1] = newY;
    vertex[2] = newZ;
}

void clear_screen()
{
    printf("\033[H\033[J");
}

void draw_cube(float angleX, float angleY, float angleZ, float scale)
{
    const int width = 80;
    const int height = 22;
    char screen[height][width];

    for (int i = 0; i < height; i++)
	{
        for (int j = 0; j < width; j++)
            screen[i][j] = ' ';
    }

    for (int i = 0; i < 12; i++)
	{
        int v1 = cube_edges[i][0];
        int v2 = cube_edges[i][1];

        float vertex1[3] = { cube_vertices[v1][0] * scale, cube_vertices[v1][1] * scale, cube_vertices[v1][2] * scale };
        float vertex2[3] = { cube_vertices[v2][0] * scale, cube_vertices[v2][1] * scale, cube_vertices[v2][2] * scale };

        rotate(vertex1, angleX, angleY, angleZ);
        rotate(vertex2, angleX, angleY, angleZ);

        int x1, y1, x2, y2;
        project_3D_to_2D(vertex1[0], vertex1[1], vertex1[2], &x1, &y1, width, height);
        project_3D_to_2D(vertex2[0], vertex2[1], vertex2[2], &x2, &y2, width, height);

        int dx = x2 - x1, dy = y2 - y1;
        int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
        float x_increment = dx / (float)steps;
        float y_increment = dy / (float)steps;
        float x = x1, y = y1;

        for (int j = 0; j <= steps; j++)
		{
            if (x >= 0 && x < width && y >= 0 && y < height)
                screen[(int)y][(int)x] = '#';
            x += x_increment;
            y += y_increment;
        }
    }

    clear_screen();

	printf("Keybinds\nJ - Increase the cube\nK - Decrease the cube\nQ - Exit the program\n");

	for(int idx = 0; idx < spaces; idx++)
		putchar('\n');
    for (int i = 0; i < height; i++)
	{
		for(int idx = 0; idx < spaces * 2; idx++)
			putchar(' ');
        for (int j = 0; j < width; j++)
            putchar(screen[i][j]);
        putchar('\n');
    }
}

void setup_terminal(struct termios *old_tio)
{
    struct termios newt;
    tcgetattr(STDIN_FILENO, old_tio);
    newt = *old_tio;
    newt.c_lflag &= ~(ICANON);
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
}

void restore_terminal(struct termios *old_tio)
{
    tcsetattr(STDIN_FILENO, TCSANOW, old_tio);
}

int main()
{
    float angleX = 0, angleY = 0, angleZ = 0;
    float scale = 1.0;
    char ch;
    struct termios old_tio;

    setup_terminal(&old_tio);

    while (1)
	{
        draw_cube(angleX, angleY, angleZ, scale);

        angleX += 0.05;
        angleY += 0.05;
        angleZ += 0.05;

        if (read(STDIN_FILENO, &ch, 1) == 1)
		{
            if (ch == 'j')
			{
                scale += 0.1;
				spaces += 1;
			}
            else if (ch == 'k')
			{
                scale -= 0.1;
				spaces -= 1;
			}
            else if (ch == 'q')
                break;
        }

        usleep(50000);
    }

    restore_terminal(&old_tio);
    return 0;
}
