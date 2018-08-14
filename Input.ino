
#include "Nintendo.h"
CGamecubeController controller(2); //sets D2 on arduino to read data from controller
CGamecubeConsole console(3);       //sets D3 on arduino to write data to console
Gamecube_Report_t gcc_send;
Gamecube_Report_t gcc_last;
Gamecube_Report_t gcc;

typedef uint8_t bit;

uint8_t deadzone_end, ess_end;

uint32_t setup_timer = 0;
bool in_setup_mode = false;

unsigned char input_counter = 0;
char send_buffer[65];

#define pinLed LED_BUILTIN

//#ifndef DEBUG
//#define DEBUG
//#endif

void setup()
{
	// Set up debug led
	pinMode(pinLed, OUTPUT);

	deadzone_end = 7;
	ess_end = 50;
	input_counter = 0;
}

int8_t remap(int8_t in) {
	uint8_t tmp = abs(in);

	if (tmp > 0 && tmp <= deadzone_end) {
		tmp = map(tmp, 0, deadzone_end, 0, 22);
	}
	else if (tmp > deadzone_end && tmp <= ess_end) {
		tmp = map(tmp, deadzone_end + 1, ess_end, 23, 36);
	}
	else if (tmp > ess_end) {
		tmp = map(tmp, ess_end + 1, 127, 37, 127);
	}

	return in > 0 ? tmp : tmp * -1;
}

void setup_mode() {
	gcc.cyAxis = 0;

	if (gcc.r) {
		in_setup_mode = false;
	}

	if (gcc.x) {
		if (deadzone_end < 127) {
			deadzone_end += 1;
		}
	}
	else if (gcc.y) {
		if (deadzone_end > 0) {
			deadzone_end -= 1;
		}
	}
	else if (gcc.a) {
		if (ess_end < 127) {
			ess_end += 1;
		}
	}
	else if (gcc.b) {
		if (ess_end > 0) {
			ess_end -= 1;
		}
	}
	gcc.buttons0 = gcc.buttons1 = 0;
	gcc.xAxis = deadzone_end + 127;
	gcc.yAxis = ess_end + 127;

}

void modify_inputs() {
	int8_t ax = gcc.xAxis - 128; 
	int8_t ay = gcc.yAxis - 128;

	ax = remap(ax);
	ay = remap(ay);

	gcc.xAxis = ax + 128;
	gcc.yAxis = ay + 128;
}

void loop()
{
	noInterrupts();
	controller.read();
	gcc = controller.getReport();

	if (gcc.start && gcc_last.start) {
		setup_timer += 1;
		if (setup_timer > 200)
			in_setup_mode = true;
	}
	else {
		setup_timer = 0;
	}

	if (in_setup_mode) {
		setup_mode();
	}
	else {
		modify_inputs();
	}
	gcc_last = gcc;
	console.write(gcc);
	interrupts();
}
