#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "Clock.h"
#include "Custom.h"
#include "Date.h"
#include "Display.h"
#include "Geo.h"
#include "HebrewDate.h"

int main() {
	Location location = Place::JERUSALEM.location();
	Custom custom(Custom::LOCAL);

	time_t now = time(NULL);
	struct tm* local = localtime(&now);
	DateTime dt(Date(local->tm_year + 1900, local->tm_mon + 1, local->tm_mday),
				Time(local->tm_hour, local->tm_min, local->tm_sec));

	StandardClock clock(dt, location, custom, 480);

	while (true) {
		now = time(NULL);
		local = localtime(&now);
		Date gregorian(local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
		DateTime current(gregorian, Time(local->tm_hour, local->tm_min, local->tm_sec));
		HebrewDate hebrew = HebrewDate::fromdate(gregorian);

		printf("Gregorian: %d-%d-%d  Time: %02d:%02d:%02d  Hebrew: %d-%d-%d\n",
			   gregorian.year, gregorian.month, gregorian.day,
			   local->tm_hour, local->tm_min, local->tm_sec,
			   hebrew.year, hebrew.month, hebrew.day);
		fflush(stdout);

		clock.update(current);
		Display disp(480);
		clock.draw(disp);
		disp.save("clock.png");

		sleep(1);
	}

	return 0;
}
