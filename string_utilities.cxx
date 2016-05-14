#include "sokoh.hxx"

void trim(std::string& str, const char* str_to_trim){
	// Trim at the begin
	size_t len_to_trim(0);
	size_t cursor(0);
	size_t trim_str_len = strlen(str_to_trim);
	while (cursor < str.length() && str[cursor] == str_to_trim[cursor % trim_str_len]) cursor++;
	if (cursor > 0){
		len_to_trim = (cursor / trim_str_len) * trim_str_len;
		str.erase(0, len_to_trim);
	}

	// Trim at the end
	cursor = 0;
	while (str.length() - cursor - 1 >= 0 && str[str.length() - 1 - cursor] == str_to_trim[trim_str_len - 1 - cursor % trim_str_len]) cursor++;
	if (cursor > 0){
		len_to_trim = (cursor / trim_str_len) * trim_str_len;
		str.erase(str.length() - len_to_trim, len_to_trim);
	}
}

std::wstring to_wstring(const std::string& source){
        wchar_t wc;
        std::wstring result;
	mbtowc(NULL, NULL, 0);
        size_t max = source.length();
        const char* str = source.c_str();
        int length = 0;
        while(max > 0){
                length = mbtowc(&wc, str, max);
                if(length < 1) break;
                result.push_back(wc);
                str += length;
                max -= length;
        } 
	return std::move(result);
}

std::string to_string(const std::wstring& source){
        const wchar_t* cursor = source.c_str();
        char buffer[MB_CUR_MAX];
        int length;
        std::string result;
        while(*cursor){
                length = wctomb(buffer, *cursor);
                if(length < 1) break;
                buffer[length] = '\0';
                result.append(buffer);
                ++ cursor;
        }

        return std::move(result);
}

unsigned char get_base64_char_value(char ch){
	if (ch >= 'A' && ch <= 'Z'){
		return (unsigned char)(ch - 'A');
	} 

	if (ch >= 'a' && ch <= 'z'){
		return (unsigned char)(ch - 'a' + 26);
	}

	if (ch >= '0' && ch <= '9'){
		return (unsigned char)(ch - '0' + 52);
	}

	switch (ch){
	case '+':
		return (unsigned char)62;
	case '/':
		return (unsigned char)63;
	case '=':
		return (unsigned char)64;
	}

	return (unsigned char)65;
}

char get_base64_char(unsigned char val){
	if (val < 26){
		return 'A' + val;
	}

	if (val < 52){
		return 'a' + val - 26;
	}

	if (val < 62){
		return '0' + val - 52;
	}

	if (val == 62){
		return '+';
	}

	if (val == 63){
		return '/';
	}

	// error case
	return '!';
}

std::string base64_encode(const std::string& source){
	size_t size = source.length();
	size_t groups = size / 3;
	size_t last_group_size = size % 3;
	
	std::string encoded_string;
	for (size_t i = 0; i < groups; ++i){
		unsigned char byte1 = (source[i * 3] >> 2);
		unsigned char byte2 = (source[i * 3 + 1] >> 4) | ((source[i * 3] & 3) << 4);
		unsigned char byte3 = (source[i * 3 + 2] >> 6) | ((source[i * 3 + 1] & 15) << 2);
		unsigned char byte4 = source[i * 3 + 2] & 63;
		encoded_string.push_back(get_base64_char(byte1));
		encoded_string.push_back(get_base64_char(byte2));
		encoded_string.push_back(get_base64_char(byte3));
		encoded_string.push_back(get_base64_char(byte4));
	}

	if (last_group_size == 1){
		unsigned char byte1 = source[groups * 3] >> 2;
		unsigned char byte2 = (source[groups * 3] & 3) << 4;
		encoded_string.push_back(get_base64_char(byte1));
		encoded_string.push_back(get_base64_char(byte2));
		encoded_string.push_back('=');
		encoded_string.push_back('=');
	}
	else if (last_group_size == 2){
		unsigned char byte1 = source[groups * 3] >> 2;
		unsigned char byte2 = (source[groups * 3 + 1] >> 4) | ((source[groups * 3] & 3) << 4);
		unsigned char byte3 = (source[groups * 3 + 1] & 15) << 2;
		encoded_string.push_back(get_base64_char(byte1));
		encoded_string.push_back(get_base64_char(byte2));
		encoded_string.push_back(get_base64_char(byte3));
		encoded_string.push_back('=');
	}

	return encoded_string;
}

std::string base64_decode(const std::string& source){
	size_t groups = source.length() / 4;
	if (source.length() % 4 != 0){
		return "";
	}

	std::string decoded_string;
	for (size_t i = 0; i < groups; ++i){
		unsigned char byte1 = get_base64_char_value(source[i * 4]);
		unsigned char byte2 = get_base64_char_value(source[i * 4 + 1]);
		unsigned char byte3 = get_base64_char_value(source[i * 4 + 2]);
		unsigned char byte4 = get_base64_char_value(source[i * 4 + 3]);
		if (byte1 == 65 || byte2 == 65 || byte3 == 65 || byte4 == 65){
			return "";
		}

		decoded_string.push_back((char)(byte1 << 2 | (byte2 >> 4)));
		if (byte3 != 64){
			// still have data, one or zero padding
			decoded_string.push_back((char)(((byte2 & 15) << 4) | (byte3 >> 2)));
			if (byte4 != 64){
				// no padding
				decoded_string.push_back((char)(((byte3 & 3) << 6) | byte4));
			}
		}
	}

	return decoded_string;
}

const char HEX2DEC[256] =
{
	/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
	/* 0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 1 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 2 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 3 */  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,

	/* 4 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 5 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 6 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 7 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

	/* 8 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 9 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* A */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* B */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

	/* C */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* D */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* E */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* F */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

std::string uri_decode(const std::string & sSrc)
{
	// Note from RFC1630:  "Sequences which start with a percent sign
	// but are not followed by two hexadecimal characters (0-9, A-F) are reserved
	// for future extension"
	// decode + to <space>

	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const int SRC_LEN = sSrc.length();
	const unsigned char * const SRC_END = pSrc + SRC_LEN;
	const unsigned char * const SRC_LAST_DEC = SRC_END - 2;   // last decodable '%' 

	char * const pStart = new char[SRC_LEN];
	char * pEnd = pStart;

	while (pSrc < SRC_LAST_DEC)
	{
		if (*pSrc == '%')
		{
			char dec1, dec2;
			if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
				&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
			{
				*pEnd++ = (dec1 << 4) + dec2;
				pSrc += 3;
				continue;
			}
		}
		else if(*pSrc == '+'){
			*pEnd++ = ' ';
			pSrc++;
			continue;
		}

		*pEnd++ = *pSrc++;
	}

	// the last 2- chars
	while (pSrc < SRC_END){
		if(*pSrc == '+'){
			*pEnd++ = ' ';
			pSrc++;
			continue;
		}

		*pEnd++ = *pSrc++;
	}

	std::string sResult(pStart, pEnd);
	delete[] pStart;
	return sResult;
}

// Only alphanum is safe.
const char SAFE[256] =
{
	/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
	/* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,

	/* 4 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	/* 6 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,

	/* 8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* A */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* B */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	/* C */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* D */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* E */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* F */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

std::string uri_encode(const std::string & sSrc)
{
	const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const int SRC_LEN = sSrc.length();
	unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
	unsigned char * pEnd = pStart;
	const unsigned char * const SRC_END = pSrc + SRC_LEN;

	for (; pSrc < SRC_END; ++pSrc)
	{
		if (SAFE[*pSrc])
			*pEnd++ = *pSrc;
		else
		{
			// escape this char
			*pEnd++ = '%';
			*pEnd++ = DEC2HEX[*pSrc >> 4];
			*pEnd++ = DEC2HEX[*pSrc & 0x0F];
		}
	}

	std::string sResult((char *)pStart, (char *)pEnd);
	delete[] pStart;
	return sResult;
}
