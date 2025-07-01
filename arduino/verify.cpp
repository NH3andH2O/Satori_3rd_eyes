#include "verify.h"

uint8_t is_valid_wifi_password(const char *password)
{
	size_t len = 0;
	while (password[len] != '\0')
	{
		char c = password[len];
		if (c < 32 || c > 126) // 檢查是否為可打印字符
		{
			return false; // 如果有不可打印字符，返回0
		}
		++len;
	}
	if ((len < 8 || len > 63) && len != 0) // 檢查長度
	{
		return false;
	}
	return true;
}