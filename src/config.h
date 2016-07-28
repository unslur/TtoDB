#ifndef CONFIG_H_36B9A0B6_1FC0_4A77_B227_E4ED70AAB41D
#define CONFIG_H_36B9A0B6_1FC0_4A77_B227_E4ED70AAB41D

#ifdef __cplusplus
extern "C" {
#endif

#define LINE_LEN 128

#define ERR_NONE     0
#define ERR_NOFILE   2
#define ERR_READFILE 3
#define ERR_FORMAT   4
#define ERR_NOTHING  5

int getconfigstr(const char* section, const char* keyname, char* keyvalue, unsigned int len, const char* filename);
int getconfigint(const char* section, const char* keyname, int* keyvalue, const char* filename);

#ifdef __cplusplus
}
#endif

#endif

