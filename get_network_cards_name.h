
typedef struct string_array{
	unsigned int num;
	char **str;
}string_array;
void destory_string_array(string_array *a);
string_array *get_network_cards_name();
int get_ipaddress(char *dev, char *ip_buf);
