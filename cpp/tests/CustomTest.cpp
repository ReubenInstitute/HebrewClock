/* CustomTest.cpp
 * Manual verification harness for Custom.h/Custom.cpp, cross-checked
 * against LIBS/Custom.py output for the same inputs.
 */
#include "Custom.h"

#include <stdio.h>

int main()
{
	for (int preset = 0; preset < 10; preset++) {
		Custom c(static_cast<Custom::Preset>(preset));
		printf("%d %d %d %d %d %d\n", preset,
		       static_cast<int>(c.dayDefinition), static_cast<int>(c.dawn),
		       static_cast<int>(c.misheyakir), static_cast<int>(c.nightfall), c.watches);
	}

	Custom cDefault;
	printf("default %d %d %d %d %d\n",
	       static_cast<int>(cDefault.dayDefinition), static_cast<int>(cDefault.dawn),
	       static_cast<int>(cDefault.misheyakir), static_cast<int>(cDefault.nightfall), cDefault.watches);

	Custom cWatches(Custom::CHABAD, 4);
	printf("watches %d %d %d %d %d\n",
	       static_cast<int>(cWatches.dayDefinition), static_cast<int>(cWatches.dawn),
	       static_cast<int>(cWatches.misheyakir), static_cast<int>(cWatches.nightfall), cWatches.watches);

	return 0;
}
