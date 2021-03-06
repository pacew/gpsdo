#include <string.h>

int 
strncmp (const char *s1, const char *s2, size_t n)
{
	if (n == 0)
		return 0;

	while (n > 0) {
		n--;
		if (*s1 != *s2)
			break;
		if (n == 0)
			break;
		if (*s1 == 0)
			break;
		s1++;
		s2++;
	}

	return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}
