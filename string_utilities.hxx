#ifndef __SPARTA_STRING_UTILITIES__
#define __SPARTA_STRING_UTILITIES__

void trim(std::string& str, const char* str_to_trim);

std::string uri_decode(const std::string& source);

std::string uri_encode(const std::string& source);

std::wstring to_wstring(const std::string& source);

std::string to_string(const std::wstring& source);

std::string base64_encode(const std::string& source);

std::string base64_decode(const std::string& source);
#endif