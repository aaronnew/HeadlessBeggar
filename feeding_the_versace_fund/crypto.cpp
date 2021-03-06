//
// by Buya (developer of xanadu)
// with small modifications by brhsiao
//
#include <cstring>
#include "aes/aes.h"

constexpr unsigned char kAesKeys[32] =
{
	0x13, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xB4, 0x00, 0x00, 0x00,
	0x1B, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00
};

constexpr unsigned char kIvTable[256] =
{
	0xEC, 0x3F, 0x77, 0xA4, 0x45, 0xD0, 0x71, 0xBF, 0xB7, 0x98, 0x20, 0xFC, 0x4B, 0xE9, 0xB3, 0xE1,
	0x5C, 0x22, 0xF7, 0x0C, 0x44, 0x1B, 0x81, 0xBD, 0x63, 0x8D, 0xD4, 0xC3, 0xF2, 0x10, 0x19, 0xE0,
	0xFB, 0xA1, 0x6E, 0x66, 0xEA, 0xAE, 0xD6, 0xCE, 0x06, 0x18, 0x4E, 0xEB, 0x78, 0x95, 0xDB, 0xBA,
	0xB6, 0x42, 0x7A, 0x2A, 0x83, 0x0B, 0x54, 0x67, 0x6D, 0xE8, 0x65, 0xE7, 0x2F, 0x07, 0xF3, 0xAA,
	0x27, 0x7B, 0x85, 0xB0, 0x26, 0xFD, 0x8B, 0xA9, 0xFA, 0xBE, 0xA8, 0xD7, 0xCB, 0xCC, 0x92, 0xDA,
	0xF9, 0x93, 0x60, 0x2D, 0xDD, 0xD2, 0xA2, 0x9B, 0x39, 0x5F, 0x82, 0x21, 0x4C, 0x69, 0xF8, 0x31,
	0x87, 0xEE, 0x8E, 0xAD, 0x8C, 0x6A, 0xBC, 0xB5, 0x6B, 0x59, 0x13, 0xF1, 0x04, 0x00, 0xF6, 0x5A,
	0x35, 0x79, 0x48, 0x8F, 0x15, 0xCD, 0x97, 0x57, 0x12, 0x3E, 0x37, 0xFF, 0x9D, 0x4F, 0x51, 0xF5,
	0xA3, 0x70, 0xBB, 0x14, 0x75, 0xC2, 0xB8, 0x72, 0xC0, 0xED, 0x7D, 0x68, 0xC9, 0x2E, 0x0D, 0x62,
	0x46, 0x17, 0x11, 0x4D, 0x6C, 0xC4, 0x7E, 0x53, 0xC1, 0x25, 0xC7, 0x9A, 0x1C, 0x88, 0x58, 0x2C,
	0x89, 0xDC, 0x02, 0x64, 0x40, 0x01, 0x5D, 0x38, 0xA5, 0xE2, 0xAF, 0x55, 0xD5, 0xEF, 0x1A, 0x7C,
	0xA7, 0x5B, 0xA6, 0x6F, 0x86, 0x9F, 0x73, 0xE6, 0x0A, 0xDE, 0x2B, 0x99, 0x4A, 0x47, 0x9C, 0xDF,
	0x09, 0x76, 0x9E, 0x30, 0x0E, 0xE4, 0xB2, 0x94, 0xA0, 0x3B, 0x34, 0x1D, 0x28, 0x0F, 0x36, 0xE3,
	0x23, 0xB4, 0x03, 0xD8, 0x90, 0xC8, 0x3C, 0xFE, 0x5E, 0x32, 0x24, 0x50, 0x1F, 0x3A, 0x43, 0x8A,
	0x96, 0x41, 0x74, 0xAC, 0x52, 0x33, 0xF0, 0xD9, 0x29, 0x80, 0xB1, 0x16, 0xD3, 0xAB, 0x91, 0xB9,
	0x84, 0x7F, 0x61, 0x1E, 0xCF, 0xC5, 0xD1, 0x56, 0x3D, 0xCA, 0xF4, 0x05, 0xC6, 0xE5, 0x08, 0x49
};

namespace crypto
{
	unsigned char rotate_right(unsigned char val, unsigned short shifts)
	{
		shifts &= 7;
		return static_cast<unsigned char>((val >> shifts) | (val << (8 - shifts)));
	}

	unsigned char rotate_left(unsigned char val, unsigned short shifts)
	{
		shifts &= 7;
		return static_cast<unsigned char>((val << shifts) | (val >> (8 - shifts)));
	}

	void shuffle_iv(unsigned char *iv)
	{
		unsigned char new_iv[4] = { 0xF2, 0x53, 0x50, 0xC6 };
		unsigned char input;
		unsigned char value_input;
		unsigned int full_iv;
		unsigned int shift;
		int loop_counter = 0;

		for (; loop_counter < 4; loop_counter++)
		{
			input = iv[loop_counter];
			value_input = kIvTable[input];

			new_iv[0] += (kIvTable[new_iv[1]] - input);
			new_iv[1] -= (new_iv[2] ^ value_input);
			new_iv[2] ^= (kIvTable[new_iv[3]] + input);
			new_iv[3] -= (new_iv[0] - value_input);

			full_iv = (new_iv[3] << 24) | (new_iv[2] << 16) | (new_iv[1] << 8) | new_iv[0];
			shift = (full_iv >> 0x1D) | (full_iv << 0x03);

			new_iv[0] = static_cast<unsigned char>(shift & 0xFFu);
			new_iv[1] = static_cast<unsigned char>((shift >> 8) & 0xFFu);
			new_iv[2] = static_cast<unsigned char>((shift >> 16) & 0xFFu);
			new_iv[3] = static_cast<unsigned char>((shift >> 24) & 0xFFu);
		}

		// set iv
		memcpy(iv, new_iv, 4);
	}

	void aes_crypt(unsigned char *buffer, unsigned char *iv, unsigned short size)
	{
		unsigned char temp_iv[16];
		unsigned short pos = 0;
		unsigned short t_pos = 1456;
		unsigned short bytes_amount;

		aes_encrypt_ctx cx[1];
		aes_init();

		while (size > pos)
		{
      for (int i = 0; i < 4; i++)
        memcpy(temp_iv + i*4, iv, 4);

			aes_encrypt_key256(kAesKeys, cx);

			if (size > (pos + t_pos))
			{
				bytes_amount = t_pos;
			}
			else
			{
				bytes_amount = size - pos;
			}

			aes_ofb_crypt(buffer + pos, buffer + pos, bytes_amount, temp_iv, cx);

			pos += t_pos;
			t_pos = 1460;
		}
	}

	void decrypt(unsigned char *buffer, unsigned char *iv, unsigned short size)
	{
		aes_crypt(buffer, iv, size);
		shuffle_iv(iv);

		unsigned char a;
		unsigned char b;
		unsigned char c;
		unsigned short temp_size;
		int loop_counter = 0;

		for (; loop_counter < 3; ++loop_counter)
		{
			a = 0;
			b = 0;
			for (temp_size = size; temp_size > 0; --temp_size)
			{
				c = buffer[temp_size - 1];
				c = rotate_left(c, 3);
				c = c ^ 0x13;
				a = c;
				c = c ^ b;
				c = static_cast<unsigned char>(c - temp_size);
				c = rotate_right(c, 4);
				b = a;
				buffer[temp_size - 1] = c;
			}
			a = 0;
			b = 0;
			for (temp_size = size; temp_size > 0; --temp_size)
			{
				c = buffer[size - temp_size];
				c = c - 0x48;
				c = c ^ 0xFF;
				c = rotate_left(c, temp_size);
				a = c;
				c = c ^ b;
				c = static_cast<unsigned char>(c - temp_size);
				c = rotate_right(c, 3);
				b = a;
				buffer[size - temp_size] = c;
			}
		}
	}

	void encrypt(unsigned char *buffer, unsigned char *iv, unsigned short size)
	{
		unsigned char a;
		unsigned char c;
		unsigned short temp_size;
		int loop_counter = 0;

		for (; loop_counter < 3; ++loop_counter)
		{
			a = 0;
			for (temp_size = size; temp_size > 0; --temp_size)
			{
				c = buffer[size - temp_size];
				c = rotate_left(c, 3);
				c = static_cast<unsigned char>(c + temp_size);
				c = c ^ a;
				a = c;
				c = rotate_right(a, temp_size);
				c = c ^ 0xFF;
				c = c + 0x48;
				buffer[size - temp_size] = c;
			}
			a = 0;
			for (temp_size = size; temp_size > 0; --temp_size)
			{
				c = buffer[temp_size - 1];
				c = rotate_left(c, 4);
				c = static_cast<unsigned char>(c + temp_size);
				c = c ^ a;
				a = c;
				c = c ^ 0x13;
				c = rotate_right(c, 3);
				buffer[temp_size - 1] = c;
			}
		}

		aes_crypt(buffer, iv, size);
		shuffle_iv(iv);
	}

	void create_packet_header(unsigned char *buffer, unsigned char *iv, unsigned short size, unsigned short game_version)
	{
		unsigned short version = ((*(unsigned short*)&iv[2]) ^ game_version);
		size = version ^ size;

		buffer[0] = version & 0xFF;
		buffer[1] = (version >> 8) & 0xFF;

		buffer[2] = size & 0xFF;
		buffer[3] = (size >> 8) & 0xFF;
	}

	unsigned short get_packet_length(unsigned char *buffer)
	{
		return ((*(unsigned short *)(buffer)) ^ (*(unsigned short *)(buffer + 2)));
	}
}
