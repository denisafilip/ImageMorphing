#include "stdafx.h"
#include "common.h"
#include "morph.h"

int main()
{
	int op;
	do
	{
		system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf(" 1 - Morph images\n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d",&op);
		switch (op)
		{
			case 1:
				morphImage();
				break;
		}
	}
	while (op!=0);
	return 0;
}