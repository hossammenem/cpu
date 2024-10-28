#include <stdio.h>
#include <string.h>
#include <sys/types.h>

// one is added to all of the subfield's lengths cuz of the null terminator
typedef struct {
	char op[7]; // operation code
	char rs[6]; // first source operand
	char rt[6]; // second source operand
	char rd[6]; // destination of the operation stored here
	char shamt[6]; // shift amount ( idk wtf is that )
	char funct[7]; // function ( same as above )
} field_t;


// you assign a table like the one for instructions, but for the registers

typedef struct {
	char* opcode;
	char* instruction;
} isa_entry_t;

const isa_entry_t isa_lookup[] = {
	{"000000", "jmp"},
	{"000001", "add"},
	{"000010", "sub"},
	{"000011", "load"},
	{"000100", "store"},
	{NULL, NULL}  // sentinel to mark end of array
};

const char* get_instruction(const char* opcode) {
	for(const isa_entry_t* entry = isa_lookup; entry->opcode != NULL; entry++) {
		if(strcmp(entry->opcode, opcode) == 0) {
			return entry->instruction;
		}
	}

	return NULL;
}

// how gonna handle EOF, null terminators, all that shit
const u_int8_t FIELD_SIZE = 32 + 1; // for EOF

int main() {
	char file_name[] = "data";
	FILE *fd = fopen(file_name, "r");

	if (fd == NULL) {
		printf("Could not open file %s\n", file_name);
		return -1;
	}

	// seek to the end to get the character count
	fseek(fd, 0, SEEK_END);
	long count = ftell(fd);

	// re-position to the beginning
	fseek(fd, 0, SEEK_SET);

	if(count % FIELD_SIZE != 0) {
		printf("%s file corrupted\n", file_name);
		fclose(fd);
		return -1;
	}

	unsigned char buffer[FIELD_SIZE];
	field_t field;

	while(fread(buffer, 1, FIELD_SIZE, fd) == FIELD_SIZE) {
		// Copy with proper size limits and null termination
		strncpy(field.op, (char*)buffer, 6);
		field.op[6] = '\0';

		strncpy(field.rs, (char*)(buffer + 6), 5);
		field.rs[5] = '\0';

		strncpy(field.rt, (char*)(buffer + 11), 5);
		field.rt[5] = '\0';

		strncpy(field.rd, (char*)(buffer + 16), 5);
		field.rd[5] = '\0';

		strncpy(field.shamt, (char*)(buffer + 21), 5);
		field.shamt[5] = '\0';

		strncpy(field.funct, (char*)(buffer + 26), 6);
		field.funct[6] = '\0';

		const char* instruction = get_instruction(field.op);

		if(instruction == NULL) {
			printf("insuffecient instruction: %s\n", field.op);
			return -1;
		}

		printf("%s\n", field.rs);
	}

	fclose(fd);
	return 0;
}
