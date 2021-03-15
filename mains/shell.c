#include "interact.h"
#include "dispatcher.h"

int main(int argc, char *argv[])
{
	return interact(default_prompt_generator, shell_command_dispatcher);
}
