
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

#define ZERO  '0'
#define ONE    '1'
#define SPLIT '\n'

void setup()
{
	// Set up debug led
	pinMode(pinLed, OUTPUT);

	deadzone_end = 7;
	ess_end = 50;
	input_counter = 0;

	// Start debug serial
	Serial.begin(115200);
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

	char buffer[256];

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

	sprintf(buffer, "Deadzone: %d\tEss: %d", deadzone_end, ess_end);
	Serial.println(buffer);

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

void send_inputs() {
	switch (input_counter)
	{
	case 0:
		send_buffer[0] = ZERO;
		send_buffer[1] = ZERO;
		send_buffer[2] = ZERO;
		send_buffer[3] = gcc_send.start ? ONE : ZERO;
		send_buffer[4] = gcc_send.y ? ONE : ZERO;
		send_buffer[5] = gcc_send.x ? ONE : ZERO;
		send_buffer[6] = gcc_send.b ? ONE : ZERO;
		send_buffer[7] = gcc_send.a ? ONE : ZERO;
		input_counter++;
		break;
	case 1:
		send_buffer[8] = ONE;
		send_buffer[9] = gcc_send.l ? ONE : ZERO;
		send_buffer[10] = gcc_send.r ? ONE : ZERO;
		send_buffer[11] = gcc_send.z ? ONE : ZERO;
		send_buffer[12] = gcc_send.dup ? ONE : ZERO;
		send_buffer[13] = gcc_send.ddown ? ONE : ZERO;
		send_buffer[14] = gcc_send.dright ? ONE : ZERO;
		send_buffer[15] = gcc_send.dleft ? ONE : ZERO;
		input_counter++;
		break;
	case 2:
		for (unsigned int i = 0; i < 8; i++) {
			send_buffer[(8 * input_counter) + i] = (gcc_send.raw8[2] & (1 << i)) ? ONE : ZERO;
		}
		input_counter++;
		break;
	case 3:
		for (unsigned int i = 0; i < 8; i++) {
			send_buffer[(8 * input_counter) + i] = (gcc_send.raw8[2] & (1 << i)) ? ONE : ZERO;
		}
		input_counter++;
		break;
	case 4:
		for (unsigned int i = 0; i < 8; i++) {
			send_buffer[(8 * input_counter) + i] = (gcc_send.raw8[2] & (1 << i)) ? ONE : ZERO;
		}
		input_counter++;
		break;
	case 5:
		for (unsigned int i = 0; i < 8; i++) {
			send_buffer[(8 * input_counter) + i] = (gcc_send.raw8[2] & (1 << i)) ? ONE : ZERO;
		}
		input_counter++;
		break;
	case 6:
		for (unsigned int i = 0; i < 8; i++) {
			send_buffer[(8 * input_counter) + i] = (gcc_send.raw8[2] & (1 << i)) ? ONE : ZERO;
		}
		input_counter++;
		break;
	case 7:
		for (unsigned int i = 0; i < 8; i++) {
			send_buffer[(8 * input_counter) + i] = (gcc_send.raw8[2] & (1 << i)) ? ONE : ZERO;
		}
		send_buffer[64] = '\n';
		input_counter++;
		break;
	case 8:
		Serial.write(send_buffer);
		gcc_send = gcc;
		input_counter = 0;
		break;
	default:
		break;
	}
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
	//send_inputs();
	console.write(gcc);
	interrupts();
}