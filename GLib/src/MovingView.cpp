#include "../include/GLib.h"

namespace GLib
{
	void MovingView::init(bool _horizontal, bool _vertical)
	{
		int xSize = place.right - place.left;
		int ySize = place.bottom - place.top;

		horizontal = _horizontal;
		vertical = _vertical;

		horizontalScrollZoom = false;
		verticalScrollZoom = false;

		if (vertical && !horizontal)
		{
			staticView = addView<View>(0, 0, xSize - buttonSize, ySize);
			movingView = staticView->addView<View>(0, 0, xSize - buttonSize, ySize);
		}
		else if (!vertical && horizontal)
		{
			staticView = addView<View>(0, 0, xSize, ySize - buttonSize);
			movingView = staticView->addView<View>(0, 0, xSize, ySize - buttonSize);
		}
		else
		{
			throw std::runtime_error("Not suported");
		}

		setup(xSize, ySize);
	}

	void MovingView::update()
	{
		bool doResize = false;

		float xMax = 0;
		float yMax = 0;
		for (auto v : movingView->subViews)
		{
			if (xMax < v->place.right)
			{
				xMax = v->place.right;
			}
			if (yMax < v->place.bottom)
			{
				yMax = v->place.bottom;
			}
		}

		float xSize = movingView->place.right - movingView->place.left;
		float ySize = movingView->place.bottom - movingView->place.top;
		if (abs(xMax - xSize) > 0.001 || abs(yMax - ySize) > 0.001)
		{
			movingView->place.right = movingView->place.left + xMax;
			movingView->place.bottom = movingView->place.top + yMax;
			doResize = true;
		}

		if (movingView->place.right < place.right - place.left - (vertical ? buttonSize : 0) 
			|| movingView->place.bottom < place.bottom - place.top - (horizontal ? buttonSize : 0))
		{
			float dif1 = place.right - place.left - (vertical ? buttonSize : 0) - movingView->place.right;
			float dif2 = place.bottom - place.top - (horizontal ? buttonSize : 0) - movingView->place.bottom;
			movingView->move(max(dif1, 0), max(dif2, 0));
			doResize = true;
		}

		if (doResize)
		{
			resize(xMax, yMax);
		}
	}

	void MovingView::winEvent(Frame * frame, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_MOUSEWHEEL)
		{
			if (vertical && verticalBar->activated && (!verticalScrollZoom && !horizontalScrollZoom))
			{
				verticalBar->moveVerticalPlace(GET_WHEEL_DELTA_WPARAM(wParam)*-0.2);
			}

			if (horizontalScrollZoom)
			{
				int mouseX = getMousePosition().first;
				float zoom = GET_WHEEL_DELTA_WPARAM(wParam) * (movingView->place.right - movingView->place.left) / 500.0;
				float ratio = (mouseX - movingView->place.left) / (movingView->place.right - movingView->place.left);

				movingView->place.left -= zoom * ratio;
				if (movingView->place.left > 0)
				{
					movingView->place.left = 0;
				}

				movingView->place.right += zoom * (1 - ratio);
				if (movingView->place.right < (place.right - place.left))
				{
					movingView->place.right = place.right - place.left;
				}
				
				float xSize = movingView->place.right - movingView->place.left;
				float ySize = movingView->place.bottom - movingView->place.top;
				resize(xSize, ySize);

				for (auto v : movingView->subViews)
				{
					v->parentResized(movingView->place);
				}
			}

			if (verticalScrollZoom)
			{
				int mouseY = getMousePosition().second;
			}
		}
	}

	View * MovingView::getMovingView()
	{
		return movingView;
	}

	void MovingView::setScrollZoom(bool _horizontal, bool _vertical)
	{
		horizontalScrollZoom = _horizontal;
		verticalScrollZoom = _vertical;
	}

	void MovingView::makeVissible(D2D1_RECT_F box)
	{
		bool doResize = false;

		if (box.left + movingView->place.left  < 0)
		{
			getMovingView()->move(-(box.left + movingView->place.left), 0);
			doResize = true;
		}

		if (place.right - place.left < box.right + movingView->place.left)
		{
			float dif = box.right + movingView->place.left - place.right + place.left;
			getMovingView()->move(-dif, 0);
			doResize = true;
		}

		if (doResize)
		{
			float xSize = movingView->place.right - movingView->place.left;
			float ySize = movingView->place.bottom - movingView->place.top;
			resize(xSize, ySize);
		}
	}

	void MovingView::setup(int xSize, int ySize)
	{
		if (horizontal)
		{
			horizontalBar = addView<Button>(buttonSize + spaceSize, ySize - buttonSize, xSize - 2 * buttonSize - 2 * spaceSize, buttonSize, [&]() {});
			horizontalBar->setHorizontalDragable(buttonSize + spaceSize, xSize - buttonSize - spaceSize, [&](float pos)
			{
				float overshoot = (movingView->place.right - movingView->place.left) - (staticView->place.right - staticView->place.left);
				float viewSize = (movingView->place.right - movingView->place.left);

				movingView->place.left = -pos * overshoot;
				movingView->place.right = -pos * overshoot + viewSize;
			});

			left = addView<Button>(0, ySize - buttonSize, buttonSize, buttonSize, [&]()
			{
				horizontalBar->moveHorizontalPlace(-10);
			});

			right = addView<Button>(xSize - buttonSize, ySize - buttonSize, buttonSize, buttonSize, [&]()
			{
				horizontalBar->moveHorizontalPlace(10);
			});

			horizontalBar->activated = false;
			left->activated = false;
			right->activated = false;
		}

		if (vertical)
		{
			verticalBar = addView<Button>(xSize - buttonSize, buttonSize + spaceSize, buttonSize, ySize - 2 * buttonSize - 2 * spaceSize, [&]() {});
			verticalBar->setVerticalDragable(buttonSize + spaceSize, ySize - 2 * buttonSize - 2 * spaceSize, [&](float pos)
			{
				float overshoot = (movingView->place.bottom - movingView->place.top) - (staticView->place.bottom - staticView->place.top);
				float viewSize = (movingView->place.bottom - movingView->place.top);

				movingView->place.top = -pos * overshoot;
				movingView->place.bottom = -pos * overshoot + viewSize;
			});

			up = addView<Button>(xSize - buttonSize, 0, buttonSize, buttonSize, [&]()
			{
				verticalBar->moveVerticalPlace(-10);
			});

			down = addView<Button>(xSize - buttonSize, ySize - buttonSize, buttonSize, buttonSize, [&]()
			{
				verticalBar->moveVerticalPlace(10);
			});

			verticalBar->activated = false;
			up->activated = false;
			down->activated = false;
		}
	}

	void MovingView::resize(float horizontalSize, float verticalSize)
	{
		float xSize = place.right - place.left;
		float ySize = place.bottom - place.top;

		if (!horizontal && vertical)
		{
			if (verticalSize > ySize)
			{
				float verticalViewRatio = float(ySize) / float(verticalSize);
				float verticalTravelLength = ySize - 2 * buttonSize - 2 * spaceSize;

				float offset = -movingView->place.top / verticalSize * verticalTravelLength;

				verticalBar->place.top = buttonSize + spaceSize + offset;
				verticalBar->place.bottom = buttonSize + spaceSize + verticalViewRatio * verticalTravelLength + offset;
				verticalBar->box.bottom = verticalViewRatio * verticalTravelLength;

				verticalBar->activated = true;
				up->activated = true;
				down->activated = true;
			}
			else
			{
				verticalBar->place.top = buttonSize + spaceSize;
				verticalBar->place.bottom = ySize - buttonSize - spaceSize;
				verticalBar->box.bottom = ySize - 2 * buttonSize - 2 * spaceSize;

				verticalBar->activated = false;
				up->activated = false;
				down->activated = false;
			}
		}
		else if (horizontal && !vertical)
		{
			if (horizontalSize > xSize)
			{
				float horizontalViewRatio = float(xSize) / float(horizontalSize);
				float horizontalTravelLength = xSize - 2 * buttonSize - 2 * spaceSize;

				float offset = -movingView->place.left / horizontalSize * horizontalTravelLength;

				horizontalBar->place.left = buttonSize + spaceSize + offset;
				horizontalBar->place.right = buttonSize + spaceSize + horizontalViewRatio * horizontalTravelLength + offset;
				horizontalBar->box.right = horizontalViewRatio * horizontalTravelLength;

				horizontalBar->activated = true;
				left->activated = true;
				right->activated = true;
			}
			else
			{
				horizontalBar->place.left = buttonSize + spaceSize;
				horizontalBar->place.right = xSize - buttonSize - spaceSize;
				horizontalBar->box.right = xSize - 2 * buttonSize - 2 * spaceSize;

				horizontalBar->activated = false;
				left->activated = false;
				right->activated = false;
			}
		}

		/*int xSize = place.right - place.left;
		int ySize = place.bottom - place.top;

		if (verticalSize > ySize)
		{
			if (horizontalSize > xSize - buttonSize)
			{
				staticView->place.right = xSize - buttonSize;
				staticView->place.bottom = ySize - buttonSize;

				right->place.left = xSize - 2*buttonSize;
				right->place.right = xSize - buttonSize;

				float horizontalViewRatio = float(xSize) / float(horizontalSize);
				int horizonalTravelLength = xSize - 3 * buttonSize - 2 * spaceSize;

				horizontalBar->place.left = buttonSize + spaceSize;
				horizontalBar->place.right = buttonSize + spaceSize + horizontalViewRatio * horizonalTravelLength;
				horizontalBar->box.right = horizontalViewRatio * horizonalTravelLength;
				horizontalBar->place.bottom = buttonSize + spaceSize + horizontalViewRatio * horizonalTravelLength;
				horizontalBar->maxLeft = buttonSize + spaceSize;
				horizontalBar->maxRight = xSize - (2*buttonSize + spaceSize);

				horizontalBar->activateFlag = true;
				left->activateFlag = true;
				right->activateFlag = true;

				float verticalViewRatio = float(ySize) / float(verticalSize);
				int verticalTravelLength = ySize - 2 * buttonSize - 2 * spaceSize;

				verticalBar->place.top = buttonSize + spaceSize;
				verticalBar->place.bottom = buttonSize + spaceSize + verticalViewRatio * verticalTravelLength;
				verticalBar->box.bottom = verticalViewRatio * verticalTravelLength;
				verticalBar->place.bottom = buttonSize + spaceSize + verticalViewRatio * verticalTravelLength;
				verticalBar->maxTop = buttonSize + spaceSize;
				verticalBar->maxBottom = ySize - (buttonSize + spaceSize);

				verticalBar->activateFlag = true;
				up->activateFlag = true;
				down->activateFlag = true;
			}
			else
			{
				staticView->place.right = xSize - buttonSize;
				staticView->place.bottom = ySize;

				horizontalBar->activateFlag = false;
				left->activateFlag = false;
				right->activateFlag = false;

				float verticalViewRatio = float(ySize) / float(verticalSize);
				int verticalTravelLength = ySize - 2 * buttonSize - 2 * spaceSize;

				int offset = -movingView->place.top / verticalSize * verticalTravelLength;

				verticalBar->place.top = buttonSize + spaceSize + offset;
				verticalBar->place.bottom = buttonSize + spaceSize + verticalViewRatio * verticalTravelLength + offset;
				verticalBar->box.bottom = verticalViewRatio * verticalTravelLength;
				verticalBar->maxTop = buttonSize + spaceSize;
				verticalBar->maxBottom = ySize - (buttonSize + spaceSize);

				verticalBar->activateFlag = true;
				up->activateFlag = true;
				down->activateFlag = true;
			}
		}
		else
		{
			if (horizontalSize > xSize)
			{
				staticView->place.right = xSize;
				staticView->place.bottom = ySize - buttonSize;

				right->place.left = xSize - buttonSize;
				right->place.right = xSize;

				float horizontalViewRatio = float(xSize) / float(horizontalSize);
				int horizonalTravelLength = xSize - 2 * buttonSize - 2 * spaceSize;

				horizontalBar->place.left = buttonSize + spaceSize;
				horizontalBar->place.right = buttonSize + spaceSize + horizontalViewRatio * horizonalTravelLength;
				horizontalBar->box.right = horizontalViewRatio * horizonalTravelLength;
				horizontalBar->place.bottom = buttonSize + spaceSize + horizontalViewRatio * horizonalTravelLength;
				horizontalBar->maxLeft = buttonSize + spaceSize;
				horizontalBar->maxRight = xSize - (buttonSize + spaceSize);

				horizontalBar->activateFlag = true;
				left->activateFlag = true;
				right->activateFlag = true;

				verticalBar->activateFlag = false;
				up->activateFlag = false;
				down->activateFlag = false;
			}
			else
			{
				staticView->place.right = xSize;
				staticView->place.bottom = ySize;

				horizontalBar->activateFlag = false;
				left->activateFlag = false;
				right->activateFlag = false;

				verticalBar->activateFlag = false;
				up->activateFlag = false;
				down->activateFlag = false;
			}
		}*/
	}
}