#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Main.hpp>
#include <SFML/Graphics.hpp>

//TODO: Next steps:
//	- Optimizations for rendering

unsigned int screen_width = 800;
unsigned int screen_height = 600;
int cell_size = 5; // square cells, so only one dimension is needed

// Divide the window into a grid consisting of 5x5 pixel squares
int grid_row_size = screen_width / cell_size;  // 160 (159 w/ zero index)
int grid_col_size = screen_height / cell_size; // 120 (119 w/ zero index)

std::vector<std::vector<int>> grid(grid_col_size, std::vector<int>(grid_row_size, 0));

// Draw every active space of the grid to the screen
void drawGrid(sf::RenderWindow& window)
{
	sf::RectangleShape rectangle({ (float)cell_size, (float)cell_size });

	for (int y_index = 0; y_index < grid_col_size; ++y_index) {
		for (int x_index = 0; x_index < grid_row_size; ++x_index) {
			if (grid[y_index][x_index] == 0) continue;

			rectangle.setPosition({ static_cast<float>(x_index * cell_size),
									static_cast<float>(y_index * cell_size) });

			window.draw(rectangle);
		}
	}
}

// Currently draws a single cell to the screen
void drawScreen(sf::RenderWindow& window)
{

	window.clear(sf::Color::Black);
	drawGrid(window);
	window.display();
}

bool validGridRange(int x_pos, int y_pos)
{
	if ((x_pos < 0 || x_pos >= (grid_row_size)) ||
		(y_pos < 0 || y_pos >= (grid_col_size)))
	{

#ifdef DEBUG
		std::cout << "NOT in range - X: " << x_pos << ", Y: " << y_pos << std::endl;
#endif

		return false;
	}

#ifdef DEBUG
	std::cout << "in range - X: " << x_pos << ", Y: " << y_pos << std::endl;
#endif

	return true;
}

bool emptyGridSpace(int x_index, int y_index)
{
	if (grid[y_index][x_index] == 0) return true;
	return false;
}

bool validPositionRange(sf::Vector2i& mouse_pos)
{
	if ((mouse_pos.x < 0 || mouse_pos.x >= screen_width) ||
		(mouse_pos.y < 0 || mouse_pos.y >= screen_height))
	{

#ifdef DEBUG
		std::cout << "Mouse position out of bounds: " <<
			"X: " << mouse_pos.x << ", " <<
			"Y: " << mouse_pos.y << std::endl;
#endif

		return false;
	}

	return true;
}

void updatePositions(sf::RenderWindow& window)
{
	// iterate bottom to top to prevent multiple position updates per frame
	for (int y_index = grid_col_size - 1; y_index >= 0; --y_index) 
	{
		for (int x_index = 0; x_index < grid_row_size; ++x_index) 
		{
			if (grid[y_index][x_index] == 0) continue;

			// Falling down - check the space directly below the current pixel
			if (!validGridRange(x_index, y_index + 1)) continue;

			// Unoccupied space below - move down
			if (emptyGridSpace(x_index, y_index + 1))
			{
				grid[y_index][x_index] = 0;
				grid[y_index + 1][x_index] = 1;
			}
			// Occupied space below - check down-left and down-right
			else
			{
				bool down_left_empty = false;
				bool down_right_empty = false;

				// Check down-left
				if (validGridRange(x_index - 1, y_index + 1) && emptyGridSpace(x_index - 1, y_index + 1))
				{
					down_left_empty = true;
				}
				
				// Check down-right
				if (validGridRange(x_index + 1, y_index + 1) && emptyGridSpace(x_index + 1, y_index + 1))
				{
					down_right_empty = true;
				}

				// If both unoccupied, randomly select one to move into
				if (down_left_empty && down_right_empty)
				{
					if (rand() % 2 == 0)
					{
						// fill down_left
						grid[y_index][x_index] = 0;
						grid[y_index + 1][x_index - 1] = 1;
					}
					else
					{
						// fill down_right
						grid[y_index][x_index] = 0;
						grid[y_index + 1][x_index + 1] = 1;
					}
				}
				// If one is occupied and the other is unoccupied, move into the unoccupied space
				else if (!down_left_empty && down_right_empty)
				{
					// fill down_right
					grid[y_index][x_index] = 0;
					grid[y_index + 1][x_index + 1] = 1;
				}
				else if (down_left_empty && !down_right_empty)
				{
					// fill down_left
					grid[y_index][x_index] = 0;
					grid[y_index + 1][x_index - 1] = 1;
				}
			}
		}
	}
}

// While click held...
void addPixels(sf::RenderWindow& window)
{

	// first get the current position of the mouse relative to the window...
	sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
	
	if (!validPositionRange(mouse_pos)) return;

	// then use integer division to truncate (floor), then multiply by cell size to snap to pixel coordinates
	sf::Vector2i grid_pos = (mouse_pos / cell_size);

#ifdef DEBUG
	// Print out coordinates of the mouse relative to the window and grid
	std::cout << "Window: " <<
		"X: " << sf::Mouse::getPosition(window).x << ", " <<
		"Y: " << sf::Mouse::getPosition(window).y << " ";
	std::cout << "Grid: " <<
		"X: " << grid_pos.x << ", " <<
		"Y: " << grid_pos.y << std::endl;
#endif

	// Set the corresponding grid cell to 1 (indicating it's active)
	grid[grid_pos.y][grid_pos.x] = 1;

}

void handleEvents(sf::RenderWindow& window)
{
	while (const std::optional event = window.pollEvent())
	{
		// 'getIf' returns a pointer to the event subtype.
		// 'is' returns true if event subtype matches the template parameter, false otherwise.

		// Close the window
		if (event->is<sf::Event::Closed>())
			window.close();
	}
}

int main()
{

	// Set the dimensions, title, and create the window
	sf::RenderWindow window(sf::VideoMode({ (unsigned)screen_width, (unsigned)screen_height }), "Sand Simulator");
	window.setFramerateLimit(60);

	// master loop
	while (window.isOpen())
	{
		// master event handler - calls event functions as needed
		handleEvents(window);
		updatePositions(window);
		drawScreen(window);

		// handle inputs
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
		{
			addPixels(window);
		}
	}

	return 0;
}