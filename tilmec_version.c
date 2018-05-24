#include <stdio.h>
#include "tilmec_version.h"

int
print_tilmec_version (FILE* filep)
{
	if (filep == NULL)
		return -1;
	return fprintf (filep, "That's all, Folks! Reformatted csv generated using Tilmec version %s. Disclaimer: No warranty of anykind is expressed or implied.\n", TILMEC_VERSION);	
}
