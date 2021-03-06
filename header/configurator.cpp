 /*
	Данный код не распространяется свободно

	:DV company 2015
*/
# ifdef WIN32
# include <windows.h>
# else
# include <unistd.h>
# endif

# include <string>
# include <fstream>
# include <vector>

using namespace std;

bool always_read = false, edit_found_value = false;

bool save_commit (unsigned first_pos,string line) {
	bool find_char = false;
	for (unsigned int j = first_pos; j < line.length(); j++) {
		if ((line[j] == '#') && (line[j - 1] != '\\')) return true;
		if (line[j] != ' ') find_char = true;
	}
	if (!find_char) return true; // если не было найдено ни одного знака после пробелов 
	return false; // Если комментария не было найдено, то вернуть ложь
}

bool search_value(string line, string parametr, string new_value, bool edit, string *returned_value) {
	bool only_read = true /*Пока ищется параметр*/, continue_stat = false, find_first_char = false, not_found = true;
	string temp_line, temp_return;
	temp_line.clear(); temp_return.clear();
	for (unsigned int i = 0; i < line.length(); i++) {
		if ((only_read) && (!always_read)) { // Чтение, пока не найдено равно или не активно постоянное чтение
			if (line[i] == ' ') continue;
			if (line[i] == '=') {
				if (temp_line == parametr) { // Если найденный параметр совпал с искомым
					not_found = false;
					only_read = false; continue; // Пропуск, чтобы не добавлять равно в ответ
				} else if (edit) {
					*returned_value = line;
					return true;	
				} else return false;
			}
			temp_line = temp_line + line[i]; // Накопление параметра
		} else {
			if (edit) {
				edit_found_value = true;
				for (unsigned int j = i; j < line.length(); j++) {
					if ((line[j] == '\"') && (line[j - 1] != '\\')) {
						if (always_read) {
							always_read = false;
							return false;
						} else {
							always_read = true;
							line.erase(i, line.length());
							*returned_value = line + new_value;
							return false;
						}
					}
				}
				if (always_read) return false;
				line.erase(i, line.length());
				*returned_value = line + new_value;
				return true;
			} else {
				if ((!find_first_char) && (line[i] == ' ')) continue; // Если обнаружены пробелы в самом начале
				else find_first_char = true;
				if (continue_stat) {continue_stat = false; continue;} // Если надо что-то пропустить
				if (line[i] == ' ') if (save_commit(i, line)) break; // Если обнаружен пробел и за ним следует комментарий, то закончить
				if (line[i] == '\\') { // Если обнаружен знак экранирования
					if (line[i + 1] == '#') {temp_return = temp_return + line[i + 1]; continue_stat = true; continue;}
					if (line[i + 1] == '\"') {temp_return = temp_return + line[i + 1]; continue_stat = true; continue;}
					if (line[i + 1] == '%') {temp_return = temp_return + line[i + 1]; continue_stat = true; continue;}
				}
				if (line[i] == '#') break;
				if (line[i] == '%') {temp_return = temp_return + " "; continue;}
				if (line[i] == '\"') { // Если обнаружены кавычки
					if (always_read) { // Если это вторые кавычки
						always_read = false;
						*returned_value = temp_return;
						return true;
					} else {
						always_read = true;
						continue;
					}
				}
				temp_return = temp_return + line[i]; // Накопление результата
			}
		}
	}
	if (edit) {
		*returned_value = line;
		return true;
	}
	*returned_value = temp_return;
	if (not_found) return false; // Если искомый параметр не был найден
	return true;
}

string configurator(string link_to_file, string parametr, string new_value, bool edit) {
	always_read = false;
	edit_found_value = false;
	FILE *fp;
	if ((fp = fopen(link_to_file.c_str(), "r")) == NULL)
		return("0x0"); // файл не найден
	string readText, returned_value, always_read_temp;
	returned_value.clear(); always_read_temp.clear();
	bool found_result = false, always_read_bool = false;
	vector<string> base;
	ifstream i(link_to_file.c_str()); // Открытие файла
	while(true) {
		getline(i, readText);
		if (i) {
			if (edit) {
				if ((search_value(readText, parametr, new_value, true, &returned_value)) && (!always_read))
					base.insert(base.end(), returned_value);
				else
					if ((always_read) && (!returned_value.empty()))
						base.insert(base.end(), returned_value);
			} else {
				if (always_read) always_read_bool = true;
				else always_read_bool = false;
				if ((search_value(readText, parametr, new_value, false, &returned_value)) && (!always_read)) {
					found_result = true;
					if (always_read_bool) return always_read_temp + returned_value;
					else return returned_value;
				} else
					if (always_read)
						always_read_temp = always_read_temp + returned_value;
			}
		} else break;
	}
	i.close(); // Закрытие файла
	if ((edit) && (edit_found_value)) {
		ofstream o(link_to_file.c_str()); // Открытие файла для записи
		for (unsigned int i = 0; i < base.size(); i++)
			o << base[i] << endl; // Запись
		o.close(); // Закрыть файл для записи
		return "0x2"; // Изменение завершено
	} else return "0x1"; // Параметр не найден
	if (!found_result) return "0x1"; // Параметр не найден
	else return returned_value;
}

string add_to_file(string link_to_file, string parametr) {
	FILE *fp;
	if ((fp = fopen(link_to_file.c_str(), "r")) == NULL) {
		ofstream o(link_to_file.c_str()); // Открытие файла для записи
		o << parametr << endl; // Запись
		o.close(); // Закрыть файл для записи
		return "0x2";
	} else {
		vector<string> base;
		string readText;
		ifstream i(link_to_file.c_str()); // Открытие файла
		while(true) {
			getline(i, readText);
			if (i) {
				base.insert(base.end(), readText);
			} else break;
		}
		i.close(); // Закрытие файла
		ofstream o(link_to_file.c_str()); // Открытие файла для записи
		for (unsigned int i = 0; i < base.size(); i++)
			o << base[i] << endl; // Запись
		o << parametr << endl; // Запись
		o.close(); // Закрыть файл для записи
		return "0x2";
	}
	return "0x0";
}