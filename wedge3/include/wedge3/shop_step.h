#ifndef WEDGE3_SHOP_STEP_H
#define WEDGE3_SHOP_STEP_H

#include "wedge3/main.h"
#include "wedge3/inventory.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Shop_Step : public Step
{
public:
	Shop_Step(std::vector<Object> items, Task *task);
	virtual ~Shop_Step();

	void start();
	bool run();

private:
	std::vector<Object> items;
};

}

#endif // WEDGE3_SHOP_STEP_H
