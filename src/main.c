
#include<display.h>

int main(void)
{
        AWM_Display *Display = AWM_OpenDisplay("/dev/fb0", "/dev/input/mouse0", "/dev/stdin", AWM_DEFAULT, AWM_DEFAULT, AWM_DEFAULT);
        AWM_Rect     Rect = {.x=8,.y=8,.w=128,.h=128};
        AWM_Colour   Colour = {.rgba_888_w = 0xffffffff};
        AWM_DrawRect(Display, Colour, Rect);
        while (Display->Running)
        {
                AWM_Event Event = {0};
                while (AWM_PollEvent(&Event, Display))
                {
                        switch (Event.Kind)
                        {
                                case AWM_EV_QUIT:
                                        Display->Running = false;
                                        goto end;
                                default:
                                        break;
                        }
                }

                AWM_Present(Display);
        }

end:
        AWM_CloseDisplay(Display);
        return 0;
}

