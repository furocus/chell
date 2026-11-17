#include <stdio.h>
#include <wchar.h>
#include "builtins/echo.c"
#include "builtins/pwd.c"

int main(void)
{

    const wchar_t *args[] = {L"echo", L"hello", L"world"};

    return echo(3, args);
}