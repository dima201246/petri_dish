/*
	:DV company 2015
	
	Пожалуйста, если читаете это, хотя бы просто подумайте, что Дима молодец, мне это важно...
*/

# ifdef WIN32
# include <windows.h>
# include "header\configurator.h";
# define CONF_FILE ".\\pd_win.conf"
# else
# include "header/configurator.h";
# include <unistd.h>
# include <stdlib.h>
# define CONF_FILE ".\/pd_lin.conf" // Реально не знаю как по другому пути к файлам в Линузе прокладывать...
# endif

# define DEBUG 0

# include <string>
# include <curses.h> // Для управления терминалом
# include <sstream> // Для str()

using namespace std;

/*--- Стандартные переменные ---*/
unsigned int maxX, maxY; // Размер терминала
bool auto_resolution = true;
unsigned int MXC, MYC; // Константное разрешение, берётся при запуске
const char *ver = "2.0";

struct bacterium { // Запись
	int half_life /*Период жизни*/, id_clan /*id клана*/, position_dead /*Для памяти о положение смерти брата*/, time_birth /*Время деления*/;
	bool predator /*Хищник или травоядное*/, activate/*Статус активации, для ускорения обработки массива */, eat;
	bool left_right_action; // Из-за двумерной структуры массива, чтобы одна бактерия не обрабатывалась несеолько раз
};

struct settings {
	unsigned int clans_bac; //Кол-во кланов бактерий (0 - рандом)
	unsigned int num_zigzas, num_himanurs; // Кол-во кланов бактерий
	unsigned int full_lives; // Период жизни бактерий
	unsigned int power_of_zigzas, power_of_himanurs; // Кол - во от максимально возможного (%)
	unsigned int lives_zigzas, lives_himanurs; // Процент от максимального кол-ва жизней
	unsigned int division_zigzas, division_himanurs; // Процент задержки дерения (зависит от общего макс. кол-ва жизней), чем меньше , тем выше вероятность
	unsigned int maxXmanual, maxYmanual; // Персональное разрешение
	unsigned int extrapolation_level; // Уровень экстра поляции (%)
	unsigned int eat_probability/*Вероятность добавлении пищи, 1 к set.eat_probability*/, eat_add /*Задержка добавления еды*/;
	unsigned int bac_delay; // Задержка
	unsigned int maxXY; // Поле для бактерий
	unsigned int add_food_zigzas, add_food_himanurs; // Сколько жизней добавит съеденная точка
	unsigned int zigzas_division_barrier, himanurs_division_barrier; // "Барьер" деления, если ниже, то не не будут делиться; Процент от макс. кол-ва жизней
	unsigned int how_many_eat_zigzas, how_many_eat_himanurs; // Процент, сколько получит жизней от отъеденного у врага
	bool angry_zigzas, angry_himanurs; // Атаковать травоядного/плотоядного или нет
	bool full_logic_zigzas, full_logic_himanurs; // Следование за едой, сообщение собрату по клану о своей смерти и тд...
	bool extrapolation; // Экстраполяция разрешения, позваляет выбрать разрешение больше, чем размер терминала
	bool go_to_eat_himanurs, go_to_eat_zigzas; // Если рядом есть еда, то пойти к ней
};

settings standart_settings(settings set) {
	set.clans_bac = 4;
	set.num_zigzas = 2;
	set.num_himanurs = 2;
	set.full_lives = 100;
	set.power_of_zigzas = 30; // %
	set.power_of_himanurs = 15;
	set.lives_zigzas = 85; // %
	set.lives_himanurs = 100; // %
	set.division_zigzas = 80; // %
	set.division_himanurs = 50; // %
	set.zigzas_division_barrier = 25; // %
	set.himanurs_division_barrier = 25; // %
	set.how_many_eat_zigzas = 50; // % 
	set.how_many_eat_himanurs = 50; // % 
	set.add_food_zigzas = 25;
	set.add_food_himanurs = 40;
	set.angry_zigzas = true;
	set.angry_himanurs = true;
	set.maxXmanual = 10;
	set.maxYmanual = 10;
	set.extrapolation_level = 100; // %
	set.eat_probability = 4;
	set.eat_add = 70;
	set.bac_delay = 100;
	set.full_logic_zigzas = true;
	set.full_logic_himanurs = true;
	set.extrapolation = false;
	set.go_to_eat_himanurs = true;
	set.go_to_eat_zigzas = true;
	return set;
}

string str(int input) { // Из int в string
	stringstream ss;
	ss << "";
	ss << input;
	string str;
	ss >> str;
	return str;
}

settings load_conf(settings set, bool save_sate) { // Модуль загрузки или записи в конф. файл
	if (save_sate) { // Если надо сохранить настройки
		if (auto_resolution) configurator(CONF_FILE, "auto_resolution", "1", true); else configurator(CONF_FILE, "auto_resolution", "0", true);
		configurator(CONF_FILE, "clans_bac", str(set.clans_bac), true);
		configurator(CONF_FILE, "full_lives", str(set.full_lives), true);
		configurator(CONF_FILE, "num_zigzas", str(set.num_zigzas), true);
		configurator(CONF_FILE, "num_himanurs", str(set.num_himanurs), true);
		configurator(CONF_FILE, "power_of_zigzas", str(set.power_of_zigzas), true);
		configurator(CONF_FILE, "power_of_himanurs", str(set.power_of_himanurs), true);
		configurator(CONF_FILE, "lives_zigzas", str(set.lives_zigzas), true);
		configurator(CONF_FILE, "lives_himanurs", str(set.lives_himanurs), true);
		configurator(CONF_FILE, "division_zigzas", str(set.division_zigzas), true);
		configurator(CONF_FILE, "division_himanurs", str(set.division_himanurs), true);
		configurator(CONF_FILE, "zigzas_division_barrier", str(set.zigzas_division_barrier), true);
		configurator(CONF_FILE, "himanurs_division_barrier", str(set.himanurs_division_barrier), true);
		configurator(CONF_FILE, "how_many_eat_zigzas", str(set.how_many_eat_zigzas), true);
		configurator(CONF_FILE, "how_many_eat_himanurs", str(set.how_many_eat_himanurs), true);
		configurator(CONF_FILE, "add_food_zigzas", str(set.add_food_zigzas), true);
		configurator(CONF_FILE, "add_food_himanurs", str(set.add_food_himanurs), true);
		if (set.angry_zigzas) configurator(CONF_FILE, "angry_zigzas", "1", true); else configurator(CONF_FILE, "angry_zigzas", "0", true);
		if (set.angry_himanurs) configurator(CONF_FILE, "angry_himanurs", "1", true); else configurator(CONF_FILE, "angry_himanurs", "0", true);
		configurator(CONF_FILE, "maxXmanual", str(set.maxXmanual), true);
		configurator(CONF_FILE, "maxYmanual", str(set.maxYmanual), true);
		configurator(CONF_FILE, "eat_probability", str(set.eat_probability), true);
		configurator(CONF_FILE, "eat_add", str(set.eat_add), true);
		configurator(CONF_FILE, "bac_delay", str(set.bac_delay), true);
		if (set.extrapolation) configurator(CONF_FILE, "extrapolation_resolution", "1", true); else configurator(CONF_FILE, "extrapolation_resolution", "0", true);
		configurator(CONF_FILE, "extrapolation_level", str(set.extrapolation_level), true);
		if (set.full_logic_zigzas) configurator(CONF_FILE, "enemy_warning_zigzas", "1", true); else configurator(CONF_FILE, "enemy_warning_zigzas", "0", true);
		if (set.full_logic_himanurs) configurator(CONF_FILE, "enemy_warning_himanurs", "1", true); else configurator(CONF_FILE, "enemy_warning_himanurs", "0", true);
		if (set.go_to_eat_himanurs) configurator(CONF_FILE, "go_to_eat_himanurs", "1", true); else configurator(CONF_FILE, "go_to_eat_himanurs", "0", true);
		if (set.go_to_eat_zigzas) configurator(CONF_FILE, "go_to_eat_zigzas", "1", true); else configurator(CONF_FILE, "go_to_eat_zigzas", "0", true);
		return set;
	}
	if (configurator(CONF_FILE, "test", "0", false) == "0x0") { // Проверка, существует ли файл, если нет, то создать
		add_to_file(CONF_FILE, "clans_bac = " + str(set.clans_bac));
		add_to_file(CONF_FILE, "num_zigzas = " + str(set.num_zigzas));
		add_to_file(CONF_FILE, "num_himanurs = " + str(set.num_himanurs));
		add_to_file(CONF_FILE, "full_lives = " + str(set.full_lives));
		add_to_file(CONF_FILE, "power_of_zigzas = " + str(set.power_of_zigzas));
		add_to_file(CONF_FILE, "power_of_himanurs = " + str(set.power_of_himanurs));
		add_to_file(CONF_FILE, "lives_zigzas = " + str(set.lives_zigzas));
		add_to_file(CONF_FILE, "lives_himanurs = " + str(set.lives_himanurs));
		add_to_file(CONF_FILE, "division_zigzas = " + str(set.division_zigzas));
		add_to_file(CONF_FILE, "division_himanurs = " + str(set.division_himanurs));
		add_to_file(CONF_FILE, "zigzas_division_barrier = " + str(set.zigzas_division_barrier));
		add_to_file(CONF_FILE, "himanurs_division_barrier = " + str(set.himanurs_division_barrier));
		add_to_file(CONF_FILE, "how_many_eat_zigzas = " + str(set.how_many_eat_zigzas));
		add_to_file(CONF_FILE, "how_many_eat_himanurs = " + str(set.how_many_eat_himanurs));
		add_to_file(CONF_FILE, "add_food_zigzas = " + str(set.add_food_zigzas));
		add_to_file(CONF_FILE, "add_food_himanurs = " + str(set.add_food_himanurs));
		if (set.angry_zigzas) add_to_file(CONF_FILE, "angry_zigzas = 1"); else add_to_file(CONF_FILE, "angry_zigzas = 0");
		if (set.angry_himanurs) add_to_file(CONF_FILE, "angry_himanurs = 1"); else add_to_file(CONF_FILE, "angry_himanurs = 0");
		if (auto_resolution) add_to_file(CONF_FILE, "auto_resolution = 1"); else add_to_file(CONF_FILE, "auto_resolution = 0");
		add_to_file(CONF_FILE, "maxXmanual = " + str(set.maxXmanual));
		add_to_file(CONF_FILE, "maxYmanual = " + str(set.maxYmanual));
		if (set.extrapolation) add_to_file(CONF_FILE, "extrapolation_resolution = 1"); else add_to_file(CONF_FILE, "extrapolation_resolution = 0");
		add_to_file(CONF_FILE, "extrapolation_level = " + str(set.extrapolation_level));
		add_to_file(CONF_FILE, "eat_probability = " + str(set.eat_probability));
		add_to_file(CONF_FILE, "eat_add = " + str(set.eat_add));
		add_to_file(CONF_FILE, "bac_delay = " + str(set.bac_delay));
		if (set.full_logic_zigzas) add_to_file(CONF_FILE, "enemy_warning_zigzas = 1"); else add_to_file(CONF_FILE, "enemy_warning_zigzas = 0");
		if (set.full_logic_himanurs) add_to_file(CONF_FILE, "enemy_warning_himanurs = 1"); else add_to_file(CONF_FILE, "enemy_warning_himanurs = 0");
		if (set.go_to_eat_zigzas) add_to_file(CONF_FILE, "go_to_eat_zigzas = 1"); else add_to_file(CONF_FILE, "go_to_eat_zigzas = 0");
		if (set.go_to_eat_himanurs) add_to_file(CONF_FILE, "go_to_eat_himanurs = 1"); else add_to_file(CONF_FILE, "go_to_eat_himanurs = 0");
	} else { // Если файл существует, то загрузить из него настройки 
		string temp_conf;
		temp_conf = configurator(CONF_FILE, "go_to_eat_zigzas", "0", false);
		if (temp_conf == "0") set.go_to_eat_zigzas = false; else set.go_to_eat_zigzas = true;
		temp_conf = configurator(CONF_FILE, "go_to_eat_himanurs", "0", false);
		if (temp_conf == "0") set.go_to_eat_himanurs = false; else set.go_to_eat_himanurs = true;
		temp_conf = configurator(CONF_FILE, "auto_resolution", "0", false);
		if (temp_conf == "0") auto_resolution = false; else auto_resolution = true;
		temp_conf = configurator(CONF_FILE, "extrapolation_resolution", "0", false);
		if (temp_conf == "0") set.extrapolation = false; else set.extrapolation = true;
		temp_conf = configurator(CONF_FILE, "angry_zigzas", "0", false);
		if (temp_conf == "0") set.angry_zigzas = false; else set.angry_zigzas = true;
		temp_conf = configurator(CONF_FILE, "angry_himanurs", "0", false);
		if (temp_conf == "0") set.angry_himanurs = false; else set.angry_himanurs = true;
		temp_conf = configurator(CONF_FILE, "enemy_warning_zigzas", "0", false);
		if (temp_conf == "0") set.full_logic_zigzas = false; else set.full_logic_zigzas = true;
		temp_conf = configurator(CONF_FILE, "enemy_warning_himanurs", "0", false);
		if (temp_conf == "0") set.full_logic_himanurs = false; else set.full_logic_himanurs = true;
		temp_conf = configurator(CONF_FILE, "extrapolation_level", "0", false);
		set.extrapolation_level = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "clans_bac", "0", false);
		set.clans_bac = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "num_zigzas", "0", false);
		set.num_zigzas = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "num_himanurs", "0", false);
		set.num_himanurs = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "full_lives", "0", false);
		set.full_lives = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "power_of_zigzas", "0", false);
		set.power_of_zigzas = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "power_of_himanurs", "0", false);
		set.power_of_himanurs = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "lives_zigzas", "0", false);
		set.lives_zigzas = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "lives_himanurs", "0", false);
		set.lives_himanurs = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "division_zigzas", "0", false);
		set.division_zigzas = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "division_himanurs", "0", false);
		set.division_himanurs = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "zigzas_division_barrier", "0", false);
		set.zigzas_division_barrier = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "himanurs_division_barrier", "0", false);
		set.himanurs_division_barrier = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "how_many_eat_zigzas", "0", false);
		set.how_many_eat_zigzas = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "how_many_eat_himanurs", "0", false);
		set.how_many_eat_himanurs = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "add_food_zigzas", "0", false);
		set.add_food_zigzas = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "add_food_himanurs", "0", false);
		set.add_food_himanurs = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "maxXmanual", "0", false);
		set.maxXmanual = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "maxYmanual", "0", false);
		set.maxYmanual = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "eat_probability", "0", false);
		set.eat_probability = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "eat_add", "0", false);
		set.eat_add = atoi(temp_conf.c_str());
		temp_conf = configurator(CONF_FILE, "bac_delay", "0", false);
		set.bac_delay = atoi(temp_conf.c_str());
	}
	return set;
}

# if (DEBUG == 1)
void debug(int num) {
	timeout(-1);
	mvprintw(0, 0, "Test %i", num);
	getch();
	timeout(0);
}

void debug_key() {
	timeout(-1);
	int deb_key_press;
	while (deb_key_press != 27) { // Пока не нажат "ESC"
		erase();
		printw("Press key, \"ESC\" - for exit");
		deb_key_press = getch();
		erase();
		printw("Key: \"%i\" Char: \"%c\" \n", deb_key_press, (char)deb_key_press);
		printw("Press any key...");
		getch();
	}
}
# endif

int rand_local(int a, int b) { // Пришлось ввести эту функцию т. к. просто рандом глючит
	if (a >= b) return b; // Чтобы не зависло, например, если 1 - 0
	int rand_num = rand() % (b + 1) + (a - 1);
	while ((rand_num < a) || (rand_num > b))
		rand_num = rand() % (b + 1) + (a - 1);
	return rand_num;
}

settings reslution_set(settings set) {
	unsigned int selected = 0, maxXl, maxYl;
	char char_auto_res, char_extr;
	getmaxyx(stdscr, maxYl, maxXl); // Получение размера терминала
	if (set.extrapolation) {
		maxXl = MXC + (MXC * set.extrapolation_level / 100);
		maxYl = MYC + (MYC * set.extrapolation_level / 100);
	}
	timeout(-1);
	while (true) {
		erase();
		if (auto_resolution) char_auto_res = 'X'; else char_auto_res = '-';
		if (set.extrapolation) char_extr = 'X'; else char_extr = '-';
		if (selected == 0) printw(">"); else printw("-");
		printw("Auto resolution          < %c >\n", char_auto_res);
		if (!auto_resolution) {
			printw("\nResolution of window: %ix%i\n", maxXl, maxYl);
			printw("Resolution now: %ix%i\n\n", set.maxXmanual, set.maxYmanual);
			if (selected == 1) printw(">"); else printw("-");
			printw("X:                       < %i >\n", set.maxXmanual);
			if (selected == 2) printw(">"); else printw("-");
			printw("Y:                       < %i >\n", set.maxYmanual);
			if (selected == 3) printw(">"); else printw("-");
			printw("Restore to standart\n\n");
			if (selected == 4) printw(">"); else printw("-");
			printw("Extrapolation rezolution < %c >\n", char_extr);
			if (selected == 5) printw(">"); else printw("-");
			if (set.extrapolation) {
				printw("Extrapolation level < %i%% >\n", set.extrapolation_level + 100);
				if (selected == 6) printw(">"); else printw("-");
				printw("Back\n");
			} else printw("Back\n");
		} else {
			if (selected == 1) printw(">"); else printw("-");
			printw("Back\n");
		}
		switch (getch()) {
			case KEY_UP: if (selected == 0) {
							if (auto_resolution) selected = 1;
							else {
								if (set.extrapolation) selected = 6;
								else selected = 5;
							}
						} else selected--; break;
			case KEY_DOWN: if (auto_resolution) {
								if ((selected + 1) > 1) selected = 0;
								else selected++;
							} else {
								if (((selected + 1) > 5) && (!set.extrapolation)) selected = 0;
								else if (((selected + 1) > 6) && (set.extrapolation)) selected = 0;
								else  selected++;
							} break;
			case KEY_LEFT: switch (selected) {
								case 0: if (!auto_resolution) auto_resolution = true;
										else auto_resolution = false;
										break;
								case 1: if (!auto_resolution) {
											if (set.maxXmanual <= 10) set.maxXmanual = maxXl;
											else set.maxXmanual--;
										} break;
								case 2: if (set.maxYmanual <= 10) set.maxYmanual = maxYl;
										else set.maxYmanual--;
										break;
								case 4 : if (set.extrapolation) {
											set.extrapolation = false;
											maxXl = MXC;
											maxYl = MYC;
											if (set.maxXmanual > MXC) set.maxXmanual = MXC; 
											if (set.maxYmanual > MYC) set.maxYmanual = MYC; 
										} else {
											maxXl = MXC + (MXC * set.extrapolation_level / 100);
											maxYl = MYC + (MYC * set.extrapolation_level / 100);											
											set.extrapolation = true;
										} break;
								case 5: if (set.extrapolation) {
											if (set.extrapolation_level > 0) set.extrapolation_level--;
											else set.extrapolation_level = 100;
											maxXl = MXC + (MXC * set.extrapolation_level / 100);
											maxYl = MYC + (MYC * set.extrapolation_level / 100);
											if (set.maxXmanual > maxXl) set.maxXmanual = maxXl;
											if (set.maxYmanual > maxYl) set.maxYmanual = maxYl;
										} break;
							} break;
			case KEY_RIGHT: switch (selected) {
								case 0: if (auto_resolution) auto_resolution = false;
										else auto_resolution = true;
										break;
								case 1: if (!auto_resolution) {
											if (set.maxXmanual >= maxXl) set.maxXmanual = 10;
											else set.maxXmanual++;
										} break;
								case 2: if (set.maxYmanual >= maxYl) set.maxYmanual = 10;
										else set.maxYmanual++;
										break;
								case 4 : if (set.extrapolation) {
											set.extrapolation = false;
											maxXl = MXC;
											maxYl = MYC;
											if (set.maxXmanual > MXC) set.maxXmanual = MXC; 
											if (set.maxYmanual > MYC) set.maxYmanual = MYC; 
										} else {
											maxXl = MXC + (MXC * set.extrapolation_level / 100);
											maxYl = MYC + (MYC * set.extrapolation_level / 100);											
											set.extrapolation = true;
										} break;
								case 5: if (set.extrapolation) {
											if (set.extrapolation_level < 100) set.extrapolation_level++;
											else set.extrapolation_level = 0;
											maxXl = MXC + (MXC * set.extrapolation_level / 100);
											maxYl = MYC + (MYC * set.extrapolation_level / 100);
											if (set.maxXmanual > maxXl) set.maxXmanual = maxXl;
											if (set.maxYmanual > maxYl) set.maxYmanual = maxYl;
										} break;
							} break;
			case '\n': if (auto_resolution) {if (selected == 1) return set;}
						else {
							if ((selected == 5) && (!set.extrapolation)) return set;
							if (selected == 6) return set;
						}
						if (selected == 3) {
							set.maxXmanual = maxXl;
							set.maxYmanual = maxYl;
						}
					break;
			case 27: return set; break;
		}
	}
	return set;
}

void help (int point) { // Хелпа работает ваще не правильно (или просто не работает) и в ней полно ошибок...
	erase();
	switch (point) {
		case -1: printw("Tab - for information about selected object\nEsc - exit or close\nM - menu (work on simulation)\nP - pause simulation\n"); break;
		case 0: printw("How bacteria cause damage to the enemy (%%)\n"); break; // Сколько урона нанесёт бактерия врагу
		case 1: printw("Percentage of lives a full life in the early\n"); break;
		case 2: printw("The percentage of delays division of the maximum number of lives, the smaller, the higher the likelihood of imminent division\n"); break;
		case 3: printw("If the health of the cells will be less, the cell will not divide\n"); break;
		case 4: printw("How much food adds lives\n"); break;
		case 5: printw("How many of the enemy adds lives\n"); break;
		case 6: printw("Will the cell to attack the enemy\n"); break;
		case 7: printw("Warning of the enemy\n"); break;
		case 8: printw("Number of bacteria in the beginning\n"); break;
		case 9: printw("The maximum number of lives at the beginning of\n"); break;
		case 10: printw("The rate of bacteria\n"); break;
		case 11: printw("Field size for bacteria\n"); break;
		case 12: printw("How many strokes to add food\n"); break;
		case 13: printw("Chance of add food in the cage is calculated as \"1 / value\"\n"); break;
		case 14: printw("Setting behavior of a particular type of bacteria\n"); break;
		case 15: printw("Setting behavior of a particular type of bacteria\n"); break;
		# ifdef WIN32
		case 16: printw("Download all settings from file pd_win.conf\n"); break;
		case 17: printw("Save all settings to a file pd_win.conf\n"); break;
		# else
		case 16: printw("Download all settings from file pd_lin.conf\n"); break;
		case 17: printw("Save all settings to a file pd_lin.conf\n"); break;
		# endif
		default: printw("Not found information for this element!\n"); break;
	}
	printw("\nPress any key...\n");
	getch();
	return;
}

int int_number (int value) { // Сколько знаков в числе
	int count = 0; // Эта писька вообще где ипользкется!?
	if (value == 0) count = 1; // Это вообще надо!???
	for (; value > 0; count++) // Писал эту хрень точно не я...
		value /= 10; // Да, в коде её нигде нет... Ладно надо предупредить, наверное...
	return count; // ВНИМАНИЕ! ОСОБО ВАЖНЫЙ БЛОК! :)
}

#define MAX_POS_BAC_SET 8 // Сколько пунктов в настройках бактерий, для симуляции

settings zigzas_settings(settings set, bool on_simulation, unsigned int x, unsigned int y, int selected_out, int key_press) { // Не-не-не-не, даже под чем-то очень тяжелым я не буду комментить блоки настроек
	int selected = 0, key_press_local, prefix = 0;
	char char_angry, char_logic;
	if (on_simulation) {selected = selected_out; prefix = 1;} // При симуляции не выводится один пункт, поэтому +1
	while (true) {
		if (!on_simulation) erase();
		else attron(COLOR_PAIR(4) | A_BOLD);
		if (selected == 0) mvprintw(y, x, ">"); else mvprintw(y, x, "-");
		if (on_simulation) mvprintw(y, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y, x + 1, "Power of zigza's         < %i%% > (%i)", set.power_of_zigzas, (set.full_lives * set.power_of_zigzas / 100));
		if (selected == 1) mvprintw(y + 1, x, ">"); else mvprintw(y + 1, x, "-");
		if (!on_simulation) mvprintw(y + 1, x + 1, "Start lives of zigza's   < %i%% > (%i)", set.lives_zigzas, (set.full_lives * set.lives_zigzas / 100));
		if (selected == (2 - prefix)) mvprintw(y + 2 - prefix, x, ">"); else mvprintw(y + 2 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 2 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y + 2 - prefix, x + 1, "Division of zigza's      < %i%% > (%i)", set.division_zigzas, (set.full_lives * set.division_zigzas / 100));
		if (selected == (3 - prefix)) mvprintw(y + 3 - prefix, x, ">"); else mvprintw(y + 3 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 3 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y + 3 - prefix, x + 1, "Division barrier         < %i%% > (%i)", set.zigzas_division_barrier, (set.full_lives * set.zigzas_division_barrier / 100));
		if (selected == (4 - prefix)) mvprintw(y + 4 - prefix, x, ">"); else mvprintw(y + 4 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 4 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y + 4 - prefix, x + 1, "Calorie food             < %i%% > (%i)", set.add_food_zigzas, (set.full_lives * set.add_food_zigzas / 100));
		if (selected == (5 - prefix)) mvprintw(y + 5 - prefix, x, ">"); else mvprintw(y + 5 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 5 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y + 5 - prefix, x + 1, "Calorie enemy            < %i%% > (%i)", set.how_many_eat_zigzas, (set.power_of_zigzas * set.how_many_eat_zigzas / 100));
		if (selected == (6 - prefix)) mvprintw(y + 6 - prefix, x, ">"); else mvprintw(y + 6 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 6 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		if (set.angry_zigzas) char_angry = 'X';
		else char_angry = '-';
		mvprintw(y + 6 - prefix, x + 1, "Angy                     < %c >", char_angry);
		if (selected == (7 - prefix)) mvprintw(y + 7 - prefix, x, ">"); else mvprintw(y + 7 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 7 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		if (set.go_to_eat_zigzas) char_logic = 'X';
		else char_logic = '-';
		mvprintw(y + 7 - prefix, x + 1, "Go to eat                < %c >", char_logic);
		if (selected == (9 - prefix)) mvprintw(y + 9 - prefix, x, ">"); else mvprintw(y + 9 - prefix, x, "-");
		if (set.full_logic_zigzas) char_logic = 'X';
		else char_logic = '-';
		if (on_simulation) mvprintw(y + 8 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		if (selected == 8 - prefix) mvprintw(y + 8 - prefix, x, ">"); else mvprintw(y + 8 - prefix, x, "-");
		mvprintw(y + 8 - prefix, x + 1, "Tell friend about enemy  < %c >", char_logic);
		if (selected == 9 - prefix) mvprintw(y + 9 - prefix, x, ">"); else mvprintw(y + 9 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 9 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y + 9 - prefix, x + 1, "Back");
		if (on_simulation) attroff(COLOR_PAIR(4) | A_BOLD);
		if (!on_simulation) key_press_local = getch();
		else key_press_local = key_press;
		switch (key_press_local) {
			case 27: return set; break;
			case KEY_LEFT: switch (selected) { // Не понял как всё это работает? Не лезь!
								case 0: if (set.power_of_zigzas != 0) set.power_of_zigzas--; break;
								case 1: if (on_simulation) {if (set.division_zigzas != 0) set.division_zigzas--; break;}
										else {if (set.lives_zigzas != 0) set.lives_zigzas--;} break;
								case 2: if (on_simulation) {if (set.zigzas_division_barrier != 0) set.zigzas_division_barrier--;}
										else {if (set.division_zigzas != 0) set.division_zigzas--;} break;
								case 3: if (on_simulation) {if (set.add_food_zigzas != 0) set.add_food_zigzas--;}
										else {if (set.zigzas_division_barrier != 0) set.zigzas_division_barrier--;} break;
								case 4: if (on_simulation) {if (set.how_many_eat_zigzas != 0) set.how_many_eat_zigzas--;}
										else {if (set.add_food_zigzas != 0) set.add_food_zigzas--;} break;
								case 5: if (on_simulation) {if (set.angry_zigzas) set.angry_zigzas = false; else set.angry_zigzas = true;}
										else {if (set.how_many_eat_zigzas != 0) set.how_many_eat_zigzas--;} break;
								case 6: if (!on_simulation) {if (set.angry_zigzas) set.angry_zigzas = false; else set.angry_zigzas = true;}
										else {if (set.go_to_eat_zigzas) set.go_to_eat_zigzas = false; else set.go_to_eat_zigzas = true;} break;
								case 7: if (!on_simulation) {if (set.go_to_eat_zigzas) set.go_to_eat_zigzas = false; else set.go_to_eat_zigzas = true;}
										else {if (set.full_logic_zigzas) set.full_logic_zigzas = false; else set.full_logic_zigzas = true;} break;
								case 8: if (!on_simulation) {if (set.full_logic_zigzas) set.full_logic_zigzas = false; else set.full_logic_zigzas = true;} break;
							} break;
			case KEY_RIGHT: switch (selected) {
								case 0: if (set.power_of_zigzas < 100) set.power_of_zigzas++; break;
								case 1: if (on_simulation) {if (set.division_zigzas < 100) set.division_zigzas++; break;}
										else {if (set.lives_zigzas < 100) set.lives_zigzas++;} break;
								case 2: if (on_simulation) {if (set.zigzas_division_barrier < 100) set.zigzas_division_barrier++;}
										else {if (set.division_zigzas < 100) set.division_zigzas++;} break;
								case 3: if (on_simulation) {if (set.add_food_zigzas < 100) set.add_food_zigzas++;}
										else {if (set.zigzas_division_barrier < 100) set.zigzas_division_barrier++;} break;
								case 4: if (on_simulation) {if (set.how_many_eat_zigzas < 100) set.how_many_eat_zigzas++;}
										else {if (set.add_food_zigzas < 100) set.add_food_zigzas++;} break;
								case 5: if (on_simulation) {if (set.angry_zigzas) set.angry_zigzas = false; else set.angry_zigzas = true;}
										else {if (set.how_many_eat_zigzas < 100) set.how_many_eat_zigzas++;} break;
								case 6: if (!on_simulation) {if (set.angry_zigzas) set.angry_zigzas = false; else set.angry_zigzas = true;}
										else {if (set.go_to_eat_zigzas) set.go_to_eat_zigzas = false; else set.go_to_eat_zigzas = true;} break;
								case 7: if (!on_simulation) {if (set.go_to_eat_zigzas) set.go_to_eat_zigzas = false; else set.go_to_eat_zigzas = true;}
										else {if (set.full_logic_zigzas) set.full_logic_zigzas = false; else set.full_logic_zigzas = true;} break;
								case 8: if (!on_simulation) {if (set.full_logic_zigzas) set.full_logic_zigzas = false; else set.full_logic_zigzas = true;} break;
							} break;
			case KEY_UP: if (selected == 0) selected = 9;
							else selected--;
							break;
			case KEY_DOWN: if (selected == 9) selected = 0;
							else selected++;
							break;
			case 9: if (selected != 9) help(selected); break;
			case '\n': if (selected == 9) return set; break;
		}
		if (on_simulation) break;
	}
	return set;
}

settings himanurs_settings(settings set, bool on_simulation, unsigned int x, unsigned int y, int selected_out, int key_press) {
	int selected = 0, key_press_local, prefix = 0;
	char char_angry, char_logic;
	if (on_simulation) {selected = selected_out; prefix = 1;} // При симуляции не выводится один пункт, поэтому +1
	while (true) {
		if (!on_simulation) erase();
		else attron(COLOR_PAIR(4) | A_BOLD);
		if (selected == 0) mvprintw(y, x, ">"); else mvprintw(y, x, "-");
		if (on_simulation) mvprintw(y, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y, x + 1, "Power of himanur's       < %i%% > (%i)", set.power_of_himanurs, (set.full_lives * set.power_of_himanurs / 100));
		if (!on_simulation) mvprintw(y + 1, x + 1, "Start lives of himanur's < %i%% > (%i)", set.lives_himanurs, (set.full_lives * set.lives_himanurs / 100));
		if (selected == 1) mvprintw(y + 1, x, ">"); else mvprintw(y + 1, x, "-");
		if (selected == (2 - prefix)) mvprintw(y + 2 - prefix, x, ">"); else mvprintw(y + 2 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 2 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y + 2 - prefix, x + 1, "Division of himanur's    < %i%% > (%i)", set.division_himanurs, (set.full_lives * set.division_himanurs / 100));
		if (selected == (3 - prefix)) mvprintw(y + 3 - prefix, x, ">"); else mvprintw(y + 3 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 3 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y + 3 - prefix, x + 1, "Division barrier         < %i%% > (%i)", set.himanurs_division_barrier, (set.full_lives * set.himanurs_division_barrier / 100));
		if (selected == (4 - prefix)) mvprintw(y + 4 - prefix, x, ">"); else mvprintw(y + 4 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 4 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y + 4 - prefix, x + 1, "Calorie food             < %i%% > (%i)", set.add_food_himanurs, (set.full_lives * set.add_food_himanurs / 100));
		if (selected == (5 - prefix)) mvprintw(y + 5 - prefix, x, ">"); else mvprintw(y + 5 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 5 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y + 5 - prefix, x + 1, "Calorie enemy            < %i%% > (%i)", set.how_many_eat_himanurs, (set.power_of_himanurs * set.how_many_eat_himanurs / 100));
		if (selected == (6 - prefix)) mvprintw(y + 6 - prefix, x, ">"); else mvprintw(y + 6 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 6 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		if (set.angry_himanurs) char_angry = 'X';
		else char_angry = '-';
		mvprintw(y + 6 - prefix, x + 1, "Angy                     < %c >", char_angry);
		if (selected == (7 - prefix)) mvprintw(y + 7 - prefix, x, ">"); else mvprintw(y + 7 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 7 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		if (set.go_to_eat_himanurs) char_logic = 'X';
		else char_logic = '-';
		mvprintw(y + 7 - prefix, x + 1, "Go to eat                < %c >", char_logic);
		if (selected == (9 - prefix)) mvprintw(y + 9 - prefix, x, ">"); else mvprintw(y + 9 - prefix, x, "-");
		if (set.full_logic_himanurs) char_logic = 'X';
		else char_logic = '-';
		if (on_simulation) mvprintw(y + 8 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		if (selected == 8 - prefix) mvprintw(y + 8 - prefix, x, ">"); else mvprintw(y + 8 - prefix, x, "-");
		mvprintw(y + 8 - prefix, x + 1, "Tell friend about enemy  < %c >", char_logic);
		if (selected == 9 - prefix) mvprintw(y + 9 - prefix, x, ">"); else mvprintw(y + 9 - prefix, x, "-");
		if (on_simulation) mvprintw(y + 9 - prefix, x + 1, "                                       "); // Для ровного окошка при симуляции
		mvprintw(y + 9 - prefix, x + 1, "Back");
		if (on_simulation) attroff(COLOR_PAIR(4) | A_BOLD);
		if (!on_simulation) key_press_local = getch();
		else key_press_local = key_press;
		switch (key_press_local) { /*Место для вашей шутки*/
			case 27: return set; break;
						case KEY_LEFT: switch (selected) {
								case 0: if (set.power_of_himanurs != 0) set.power_of_himanurs--; break;
								case 1: if (on_simulation) {if (set.division_himanurs != 0) set.division_himanurs--; break;}
										else {if (set.lives_himanurs != 0) set.lives_himanurs--;} break;
								case 2: if (on_simulation) {if (set.himanurs_division_barrier != 0) set.himanurs_division_barrier--;}
										else {if (set.division_himanurs != 0) set.division_himanurs--;} break;
								case 3: if (on_simulation) {if (set.add_food_himanurs != 0) set.add_food_himanurs--;}
										else {if (set.himanurs_division_barrier != 0) set.himanurs_division_barrier--;} break;
								case 4: if (on_simulation) {if (set.how_many_eat_himanurs != 0) set.how_many_eat_himanurs--;}
										else {if (set.add_food_himanurs != 0) set.add_food_himanurs--;} break;
								case 5: if (on_simulation) {if (set.angry_himanurs) set.angry_himanurs = false; else set.angry_himanurs = true;}
										else {if (set.how_many_eat_himanurs != 0) set.how_many_eat_himanurs--;} break;
								case 6: if (!on_simulation) {if (set.angry_himanurs) set.angry_himanurs = false; else set.angry_himanurs = true;}
										else {if (set.go_to_eat_himanurs) set.go_to_eat_himanurs = false; else set.go_to_eat_himanurs = true;} break;
								case 7: if (!on_simulation) {if (set.go_to_eat_himanurs) set.go_to_eat_himanurs = false; else set.go_to_eat_himanurs = true;}
										else {if (set.full_logic_himanurs) set.full_logic_himanurs = false; else set.full_logic_himanurs = true;} break;
								case 8: if (!on_simulation) {if (set.full_logic_himanurs) set.full_logic_himanurs = false; else set.full_logic_himanurs = true;} break;
							} break;
			case KEY_RIGHT: switch (selected) {
								case 0: if (set.power_of_himanurs < 100) set.power_of_himanurs++; break;
								case 1: if (on_simulation) {if (set.division_himanurs < 100) set.division_himanurs++; break;}
										else {if (set.lives_himanurs < 100) set.lives_himanurs++;} break;
								case 2: if (on_simulation) {if (set.himanurs_division_barrier < 100) set.himanurs_division_barrier++;}
										else {if (set.division_himanurs < 100) set.division_himanurs++;} break;
								case 3: if (on_simulation) {if (set.add_food_himanurs < 100) set.add_food_himanurs++;}
										else {if (set.himanurs_division_barrier < 100) set.himanurs_division_barrier++;} break;
								case 4: if (on_simulation) {if (set.how_many_eat_himanurs < 100) set.how_many_eat_himanurs++;}
										else {if (set.add_food_himanurs < 100) set.add_food_himanurs++;} break;
								case 5: if (on_simulation) {if (set.angry_himanurs) set.angry_himanurs = false; else set.angry_himanurs = true;}
										else {if (set.how_many_eat_himanurs < 100) set.how_many_eat_himanurs++;} break;
								case 6: if (!on_simulation) {if (set.angry_himanurs) set.angry_himanurs = false; else set.angry_himanurs = true;}
										else {if (set.go_to_eat_himanurs) set.go_to_eat_himanurs = false; else set.go_to_eat_himanurs = true;} break;
								case 7: if (!on_simulation) {if (set.go_to_eat_himanurs) set.go_to_eat_himanurs = false; else set.go_to_eat_himanurs = true;}
										else {if (set.full_logic_himanurs) set.full_logic_himanurs = false; else set.full_logic_himanurs = true;} break;
								case 8: if (!on_simulation) {if (set.full_logic_himanurs) set.full_logic_himanurs = false; else set.full_logic_himanurs = true;} break;
							} break;
			case KEY_UP: if (selected == 0) selected = 9;
							else selected--;
							break;
			case KEY_DOWN: if (selected == 9) selected = 0;
							else selected++;
							break;
			case 9: if (selected != 9) help(selected); erase(); break;
			case '\n': if (selected == 9) return set; break;
		}
		if (on_simulation) break;
	}
	return set;
}

settings float_settings(settings set, unsigned int x, unsigned int y, int selected, int key_press) {
	attron(COLOR_PAIR(4) | A_BOLD);
	if (selected == 0) mvprintw(y, x, ">"); else mvprintw(y, x, "-");
	mvprintw(y, x + 1, "                                       "); // Для ровного окошка при симуляции
	mvprintw(y, x + 1, "Lives                    < %i >", set.full_lives);
	if (selected == 1) mvprintw(y + 1, x, ">"); else mvprintw(y + 1, x, "-");
	mvprintw(y + 1, x + 1, "                                       "); // Для ровного окошка при симуляции
	mvprintw(y + 1, x + 1, "Delay                    < %ims >", set.bac_delay);
	if (selected == 2) mvprintw(y + 2, x, ">"); else mvprintw(y + 2, x, "-");
	mvprintw(y + 2, x + 1, "                                       "); // Для ровного окошка при симуляции
	mvprintw(y + 2, x + 1, "During addition of food  < %i >", set.eat_add);
	if (selected == 3) mvprintw(y + 3, x, ">"); else mvprintw(y + 3, x, "-");
	mvprintw(y + 3, x + 1, "                                       "); // Для ровного окошка при симуляции
	mvprintw(y + 3, x + 1, "Eat probability          < %i >", set.eat_probability);
	if (selected == 4) mvprintw(y + 4, x, ">"); else mvprintw(y + 4, x, "-");
	mvprintw(y + 4, x + 1, "                                       "); // Для ровного окошка при симуляции
	mvprintw(y + 4, x + 1, "Settings zigza's");
	if (selected == 5) mvprintw(y + 5, x, ">"); else mvprintw(y + 5, x, "-");
	mvprintw(y + 5, x + 1, "                                       "); // Для ровного окошка при симуляции
	mvprintw(y + 5, x + 1, "Settings himanur's");
	if (selected == 6) mvprintw(y + 6, x, ">"); else mvprintw(y + 6, x, "-");
	mvprintw(y + 6, x + 1, "                                       "); // Для ровного окошка при симуляции
	mvprintw(y + 6, x + 1, "Load configuration file");
	if (selected == 7) mvprintw(y + 7, x, ">"); else mvprintw(y + 7, x, "-");
	mvprintw(y + 7, x + 1, "                                       "); // Для ровного окошка при симуляции
	mvprintw(y + 7, x + 1, "Windows position (X)     < %i >", x);
	if (selected == 8) mvprintw(y + 8, x, ">"); else mvprintw(y + 8, x, "-");
	mvprintw(y + 8, x + 1, "                                       "); // Для ровного окошка при симуляции
	mvprintw(y + 8, x + 1, "Windows position (Y)     < %i >", y);
	if (selected == 9) mvprintw(y + 9, x, ">"); else mvprintw(y + 9, x, "-");
	mvprintw(y + 9, x + 1, "                                       "); // Для ровного окошка при симуляции
	mvprintw(y + 9, x + 1, "Close");
	attroff(COLOR_PAIR(4) | A_BOLD);
	switch (key_press) {
		case KEY_LEFT: switch (selected) {
							case 0: if (set.full_lives <= 50) set.full_lives = 999;
									else set.full_lives--;
									break;
							case 1: if (set.bac_delay == 0) set.bac_delay = 500;
									else set.bac_delay -= 10;
									break;
							case 2: if (set.eat_add <= 0) set.eat_add = 999;
									else set.eat_add--;
									break;
							case 3: if (set.eat_probability <= 0) set.eat_probability = 100;
									else set.eat_probability--;
									break;
						} break;
		case KEY_RIGHT: switch (selected) {
							case 0: if (set.full_lives >= 999) set.full_lives = 50;
									else set.full_lives++;
									break;
							case 1: if (set.bac_delay >= 500) set.bac_delay = 0;
									else set.bac_delay += 10;
									break;
							case 2: if (set.eat_add >= 999) set.eat_add = 0;
									else set.eat_add++;
									break;
							case 3: if (set.eat_probability >= 100) set.eat_probability = 0;
									else set.eat_probability++;
									break;
						} break;
	}
	return set;
}

settings main_settings(settings set) {
	int selected = 0, fix_num = 0;
	if (set.clans_bac != 0) fix_num = 2;
	else fix_num = 0;
	while (true) {
		timeout(-1);
		erase();
		if (selected == 0) printw(">"); else printw("-");
		printw("Number of clans          < %i >\n", set.clans_bac);
		if (set.clans_bac != 0) {
			if (selected == 1) printw(">"); else printw("-");
			printw("Number of zigza's        < %i >\n", set.num_zigzas);
			if (selected == 2) printw(">"); else printw("-");
			printw("Number of himanur's      < %i >\n", set.num_himanurs);
		}
		if (selected == (1 + fix_num)) printw(">"); else printw("-");
		printw("Lives                    < %i >\n", set.full_lives);
		if (selected == (2 + fix_num)) printw(">"); else printw("-");
		printw("Delay                    < %ims >\n", set.bac_delay);
		if (selected == (3 + fix_num)) printw(">"); else printw("-");
		printw("Resolution\n");
		if (selected == (4 + fix_num)) printw(">"); else printw("-");
		printw("During addition of food  < %i >\n", set.eat_add);
		if (selected == (5 + fix_num)) printw(">"); else printw("-");
		printw("Eat probability          < %i >\n", set.eat_probability);
		if (selected == (6 + fix_num)) printw(">"); else printw("-");
		printw("Settings zigza's\n");
		if (selected == (7 + fix_num)) printw(">"); else printw("-");
		printw("Settings himanur's\n");
		if (selected == (8 + fix_num)) printw(">"); else printw("-");
		printw("Load configuration file\n");
		if (selected == (9 + fix_num)) printw(">"); else printw("-");
		printw("Save configuration file\n");
		if (selected == (10 + fix_num)) printw(">"); else printw("-");
		printw("Back\n");
		# if (DEBUG == 1)
		if (selected == (11 + fix_num)) printw(">"); else printw("-");
		printw("Debug key\n");
		# endif
		switch(getch()) {
			case 27: return set; break;
			/*case 9: if ((selected != 11) && (selected != 12)) { //                                               ?????BUG!!!!!
						if ((selected != 1) && (set.clans_bac == 0)) help(selected + 7 + fix_num);
						else help(selected + 7);
					} break;*/ // Вот поэтому хэлпа и не работает...
			case KEY_UP:
						# if (DEBUG == 1)
						if (selected == 0) {
							if (set.clans_bac == 0) selected = 11;
							else selected = 13;
						}
						# else
						if (selected == 0) {
							if (set.clans_bac == 0) selected = 10;
							else selected = 12;
						}
						# endif
						else selected--;
						break;
			case KEY_DOWN:
						# if (DEBUG == 1)
						if ((selected == 11) && (set.clans_bac == 0)) selected = 0;
						else if (set.clans_bac == 0) selected++;
						if ((selected == 13) && (set.clans_bac != 0)) selected = 0;
						else if (set.clans_bac != 0) selected++;
						# else
						if ((selected == 10) && (set.clans_bac == 0)) selected = 0;
						else if (set.clans_bac == 0) selected++;
						if ((selected == 12) && (set.clans_bac != 0)) selected = 0;
						else if (set.clans_bac != 0) selected++;
						# endif
						break;
			case KEY_LEFT: switch (selected) {
								case 0: if (set.clans_bac == 0) break;
										else {
											set.clans_bac--;
											if ((set.num_zigzas > set.num_himanurs) && (set.num_zigzas != 0)) set.num_zigzas--;
											else if ((set.num_zigzas == set.num_himanurs) && (set.num_zigzas != 0)) set.num_zigzas--;
											else if ((set.num_zigzas == set.num_himanurs) && (set.num_himanurs != 0)) set.num_zigzas--;
											else if ((set.num_zigzas < set.num_himanurs) && (set.num_himanurs != 0)) set.num_himanurs--;
										}
										if (set.clans_bac == 0) fix_num = 0;
										else fix_num = 2;
										break;
								case 1: if (set.clans_bac == 0) {
											if (set.full_lives <= 50) set.full_lives = 999;
											else set.full_lives--;
										} else {
											if ((set.num_zigzas != 0) && (set.num_himanurs != set.clans_bac)) {
												set.num_zigzas--;
												set.num_himanurs++;
											}
										} break;
								case 2: if (set.clans_bac == 0) {
											if (set.bac_delay == 0) set.bac_delay = 500;
											else set.bac_delay -= 10;
										} else {
											if ((set.num_himanurs != 0) && (set.num_zigzas != set.clans_bac)) {
												set.num_himanurs--;
												set.num_zigzas++;
											}
										} break;
								case 3: if (set.clans_bac != 0) {
											if (set.full_lives <= 50) set.full_lives = 999;
											else set.full_lives--;
										} break;
								case 4: if (set.clans_bac != 0) {
											if (set.bac_delay == 0) set.bac_delay = 500;
											else set.bac_delay -= 10;
										} else {
											if (set.eat_add <= 0) set.eat_add = 999;
											else set.eat_add--;
										} break;
								case 5: if (set.clans_bac == 0) {
											if (set.eat_probability <= 0) set.eat_probability = 100;
											else set.eat_probability--;
										} break;
								case 6: if (set.clans_bac != 0) {
											if (set.eat_add <= 0) set.eat_add = 999;
											else set.eat_add--;
										} break;
								case 7: if (set.clans_bac != 0) {
											if (set.eat_probability <= 0) set.eat_probability = 100;
											else set.eat_probability--;
										} break;
							} break;
			case KEY_RIGHT: switch (selected) {
								case 0: if (set.clans_bac == 15) break;
										else {
											set.clans_bac++;
											if ((set.num_zigzas > set.num_himanurs) && (set.num_zigzas < set.clans_bac)) set.num_zigzas++;
											if ((set.num_zigzas == set.num_himanurs) && (set.num_zigzas < set.clans_bac)) set.num_zigzas++;
											if ((set.num_zigzas < set.num_himanurs) && (set.num_himanurs < set.clans_bac)) set.num_himanurs++;
										}
										if (set.clans_bac == 0) fix_num = 0;
										else fix_num = 2;
										break;
								case 1: if (set.clans_bac == 0) {
											if (set.full_lives >= 999) set.full_lives = 50;
											else set.full_lives++;
										} else {
											if ((set.num_himanurs != 0) && (set.num_zigzas != set.clans_bac)) {
												set.num_himanurs--;
												set.num_zigzas++;
											}
										} break;
								case 2: if (set.clans_bac == 0) {
											if (set.bac_delay >= 500) set.bac_delay = 0;
											else set.bac_delay += 10;
										} else {
											if ((set.num_zigzas != 0) && (set.num_himanurs != set.clans_bac)) {
												set.num_zigzas--;
												set.num_himanurs++;
											}
										}break;
								case 3: if (set.clans_bac != 0) {
											if (set.full_lives >= 999) set.full_lives = 50;
											else set.full_lives++;
										} break;
								case 4: if (set.clans_bac == 0) {
											if (set.eat_add >= 999) set.eat_add = 0;
											else set.eat_add++;
										} else {
											if (set.bac_delay >= 500) set.bac_delay = 0;
											else set.bac_delay += 10;
										}
										break;
								case 5: if (set.clans_bac == 0) {
											if (set.eat_probability >= 100) set.eat_probability = 0;
											else set.eat_probability++;
										} break;
								case 6: if (set.clans_bac != 0) {
											if (set.eat_add >= 999) set.eat_add = 0;
											else set.eat_add++;
										} break;
								case 7: if (set.clans_bac != 0) {
											if (set.eat_probability >= 100) set.eat_probability = 0;
											else set.eat_probability++;
										} break;
							} break;
			case '\n': switch (selected) {
								case 3: if (set.clans_bac == 0) set = reslution_set(set); break;
								case 5: if (set.clans_bac != 0) set = reslution_set(set); break;
								case 6: if (set.clans_bac == 0) set = zigzas_settings(set, false, 0, 0, 0, 0); break;
								case 7: if (set.clans_bac == 0) set = himanurs_settings(set, false, 0, 0, 0, 0); break;
								case 8: if (set.clans_bac == 0) {
											erase(); printw("Are you sure? (Enter = yes/Other key = no)\n");
											if (getch() == '\n') set = load_conf(set, false);
										} else set = zigzas_settings(set, false, 0, 0, 0, 0);
										break;
								case 9: if (set.clans_bac == 0) {
											erase(); printw("Are you sure? (Enter = yes/Other key = no)\n");
											if (getch() == '\n') load_conf(set, true);
										} else set = himanurs_settings(set, false, 0, 0, 0, 0);
										break;
								case 10: if (set.clans_bac != 0) {
											erase(); printw("Are you sure? (Enter = yes/Other key = no)\n");
											if (getch() == '\n') set = load_conf(set, false);
										} else return set;
										break;
								case 11: if (set.clans_bac != 0) {
											erase(); printw("Are you sure? (Enter = yes/Other key = no)\n");
											if (getch() == '\n') load_conf(set, true);
										}
										# if (DEBUG == 1)
										else debug_key(); 
										# endif
										break;
								case 12: if (set.clans_bac != 0) return set; break;
								# if (DEBUG == 1)
								case 13: if (set.clans_bac != 0) debug_key(); break;
								# endif
							} break;
		}
	}
	return set;
}

bool freedom_of_bacterium(bacterium ***bacteriums, int direction, unsigned int x, unsigned int y, bool see_under_bac) { // Проверка свободы движения для бактерий
	int bac_level, x_pos = 0, y_pos = 0;
	if (see_under_bac) bac_level = 1; // Дочерний уровень
	else bac_level = 0;
	switch (direction) {
		case 1: y_pos = 1; if (y == 0) return false; break;
		case 2: y_pos = -1; if ((y + 1) == maxY) return false; break;
		case 3: x_pos = 1; if (x == 0) return false; break;
		case 4: x_pos = -1; if ((x + 1) == maxX) return false; break;
	} // Нуууууу... Тут всё понятно...
	if (!bacteriums[x][y][bac_level].predator) {
		if ((bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].activate) && (!bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].predator)) return false;
		if ((bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].activate) && (bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].predator)) return true;
		if ((see_under_bac) && (!bacteriums[(int)x - x_pos][(int)y - y_pos][0].activate)) return true;
		if (!bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].activate) return true;
	} else {
		if ((bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].activate) && (bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].predator)) return false;
		if ((bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].activate) && (!bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].predator)) return true;
		if ((see_under_bac) && (!bacteriums[(int)x - x_pos][(int)y - y_pos][0].activate)) return true;
		if (!bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].activate) return true;
	}
	return false;
}

int get_eat(bacterium ***bacteriums, const unsigned int x, const unsigned int y, bool see_under_bac) { // Шаг к еде
	int rand_step_count = 4, rand_step;
	while (rand_step_count > 0) {
		rand_step = rand_local(1, rand_step_count);
		if (freedom_of_bacterium(bacteriums, rand_step, x, y, see_under_bac)) {
			switch (rand_step) {
				case 1: if ((!bacteriums[x][y - 1][0].activate) && (bacteriums[x][y - 1][0].eat)) return 1; break;
				case 2: if ((!bacteriums[x][y + 1][0].activate) && (bacteriums[x][y + 1][0].eat)) return 2; break;
				case 3: if ((!bacteriums[x - 1][y][0].activate) && (bacteriums[x - 1][y][0].eat)) return 3; break;
				case 4: if ((!bacteriums[x + 1][y][0].activate) && (bacteriums[x + 1][y][0].eat)) return 4; break;
			}
			if (rand_step_count == rand_step) rand_step_count--;
		} else if (rand_step_count == rand_step) rand_step_count--;
	}
	return 0; // Если поблизости еды не найденойдено, то вернуть 0
}

bool birth(bacterium ***bacteriums, settings set, const unsigned int x, const unsigned int y, bool see_under_bac) { // Деление
	int bac_level = 0, other_bac_level = 1, x_pos = 0, y_pos = 0;
	if (see_under_bac) bac_level = 1;
	if (bacteriums[x][y][bac_level].predator) {
		if (bacteriums[x][y][bac_level].half_life < (int)(set.full_lives * set.zigzas_division_barrier / 100)) return false; // Если кол-во жизней меньше, чем заданное процентнов от макс. жизней, то не делиться
	} else {
		if (bacteriums[x][y][bac_level].half_life < (int)(set.full_lives * set.himanurs_division_barrier / 100)) return false; // Если кол-во жизней меньше, чем заданное процентнов от макс. жизней, то не делиться
	}
	int rand_step = rand_local(1, 4); // Выбор рандомной позиции
	int rand_step_count = 4; // Защита от бесконечного поиска пустой клетки
	while (!freedom_of_bacterium(bacteriums, rand_step, x, y, see_under_bac)) {
		if (rand_step_count == rand_step) rand_step_count--;
		rand_step = rand_local(1, rand_step_count);
		if (rand_step_count <= 0) { // Если свободного места не найдено - то стоять на месте
			bacteriums[x][y][bac_level].time_birth++;
			rand_step = 0;
			break; 
		}
	}
	switch (rand_step) {
			case 1: y_pos = 1; break;
			case 2: y_pos = -1; break;
			case 3: x_pos = 1; break;
			case 4: x_pos = -1; break;
			default: return false; break;
		}
	if (!bacteriums[(int)x - x_pos][(int)y - y_pos][0].activate) other_bac_level = 0;
	bacteriums[(int)x - x_pos][(int)y - y_pos][other_bac_level].activate = true;
	bacteriums[(int)x - x_pos][(int)y - y_pos][other_bac_level].id_clan = bacteriums[x][y][bac_level].id_clan;
	bacteriums[(int)x - x_pos][(int)y - y_pos][other_bac_level].half_life = bacteriums[x][y][bac_level].half_life / 2;
	bacteriums[(int)x - x_pos][(int)y - y_pos][other_bac_level].half_life = bacteriums[x][y][bac_level].half_life / 2;
	if (bacteriums[x][y][bac_level].predator) {
		bacteriums[(int)x - x_pos][(int)y - y_pos][other_bac_level].predator = true;
		bacteriums[(int)x - x_pos][(int)y - y_pos][other_bac_level].time_birth = rand_local(1, set.full_lives * set.division_zigzas / 100);
		bacteriums[x][y][bac_level].time_birth = rand_local(1, set.full_lives * set.division_zigzas / 100);
	} else {
		bacteriums[(int)x - x_pos][(int)y - y_pos][other_bac_level].predator = false;
		bacteriums[(int)x - x_pos][(int)y - y_pos][other_bac_level].time_birth = rand_local(1, set.full_lives * set.division_himanurs / 100);
		bacteriums[x][y][bac_level].time_birth = rand_local(1, set.full_lives * set.division_himanurs / 100);
	}
	return true;
}

void see_down(bacterium ***bacteriums, settings set, const unsigned int x, const unsigned int y, bool see_under_bac) { // Что находиться под бактерией
	if (!see_under_bac) {
		if ((bacteriums[x][y][0].predator) && (bacteriums[x][y][0].eat) && (bacteriums[x][y][0].half_life <= 99999)) bacteriums[x][y][0].half_life += set.full_lives * set.add_food_zigzas / 100;
		if ((!bacteriums[x][y][0].predator) && (bacteriums[x][y][0].eat)  && (bacteriums[x][y][0].half_life <= 99999)) bacteriums[x][y][0].half_life += set.full_lives * set.add_food_himanurs / 100;
	}
	int bac_level, bac_level_enemy;
	if (see_under_bac) {bac_level = 1; bac_level_enemy = 0;}
	else {bac_level = 0; bac_level_enemy = 1;}
	if ((bacteriums[x][y][bac_level].predator) && (!bacteriums[x][y][bac_level_enemy].predator) && (bacteriums[x][y][bac_level_enemy].activate) && (set.angry_zigzas)) { // Если хищник
		bacteriums[x][y][bac_level_enemy].half_life -= set.full_lives * set.power_of_zigzas / 100;
		bacteriums[x][y][bac_level].half_life += set.power_of_zigzas * set.how_many_eat_zigzas / 100;
		if (bacteriums[x][y][bac_level_enemy].half_life <= 0) {
			bacteriums[x][y][bac_level_enemy].activate = false;
			if (set.full_logic_himanurs) { // Сообщение другу о враге
				if ((y != 0) && (!bacteriums[x][y - 1][0].predator) && (bacteriums[x][y - 1][0].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x][y - 1][0].id_clan)) bacteriums[x][y - 1][0].position_dead = 2;
				if ((y != 0) && (!bacteriums[x][y - 1][1].predator) && (bacteriums[x][y - 1][1].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x][y - 1][1].id_clan)) bacteriums[x][y - 1][1].position_dead = 2;
				if (((y + 1) < maxY) && (!bacteriums[x][y + 1][0].predator) && (bacteriums[x][y + 1][0].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x][y + 1][0].id_clan)) bacteriums[x][y + 1][0].position_dead = 1;
				if (((y + 1) < maxY) && (!bacteriums[x][y + 1][1].predator) && (bacteriums[x][y + 1][1].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x][y + 1][1].id_clan)) bacteriums[x][y + 1][1].position_dead = 1;
				if ((x != 0) && (!bacteriums[x - 1][y][0].predator) && (bacteriums[x - 1][y][0].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x - 1][y][0].id_clan)) bacteriums[x - 1][y][0].position_dead = 4;
				if ((x != 0) && (!bacteriums[x - 1][y][1].predator) && (bacteriums[x - 1][y][1].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x - 1][y][1].id_clan)) bacteriums[x - 1][y][1].position_dead = 4;
				if (((x + 1) < maxX) && (!bacteriums[x + 1][y][0].predator) && (bacteriums[x + 1][y][0].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x + 1][y][0].id_clan)) bacteriums[x + 1][y][0].position_dead = 3;
				if (((x + 1) < maxX) && (!bacteriums[x + 1][y][1].predator) && (bacteriums[x + 1][y][1].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x + 1][y][1].id_clan)) bacteriums[x + 1][y][1].position_dead = 3;
			}
		}
		return;
	} else if ((bacteriums[x][y][bac_level_enemy].predator) && (bacteriums[x][y][bac_level_enemy].activate) && (set.angry_himanurs)) { // Если под травоядным плотоядное
		bacteriums[x][y][bac_level_enemy].half_life -= set.full_lives * set.power_of_himanurs / 100;
		bacteriums[x][y][bac_level].half_life += set.power_of_himanurs * set.how_many_eat_himanurs / 100;
		if (bacteriums[x][y][bac_level_enemy].half_life <= 0) {
			bacteriums[x][y][bac_level_enemy].activate = false;
			if (set.full_logic_zigzas) {
				if ((y != 0) && (bacteriums[x][y - 1][0].predator) && (bacteriums[x][y - 1][0].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x][y - 1][0].id_clan)) bacteriums[x][y - 1][0].position_dead = 2;
				if ((y != 0) && (bacteriums[x][y - 1][1].predator) && (bacteriums[x][y - 1][1].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x][y - 1][1].id_clan)) bacteriums[x][y - 1][1].position_dead = 2;
				if (((y + 1) < maxY) && (bacteriums[x][y + 1][0].predator) && (bacteriums[x][y + 1][0].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x][y + 1][0].id_clan)) bacteriums[x][y + 1][0].position_dead = 1;
				if (((y + 1) < maxY) && (bacteriums[x][y + 1][1].predator) && (bacteriums[x][y + 1][1].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x][y + 1][1].id_clan)) bacteriums[x][y + 1][1].position_dead = 1;
				if ((x != 0) && (bacteriums[x - 1][y][0].predator) && (bacteriums[x - 1][y][0].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x - 1][y][0].id_clan)) bacteriums[x - 1][y][0].position_dead = 4;
				if ((x != 0) && (bacteriums[x - 1][y][1].predator) && (bacteriums[x - 1][y][1].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x - 1][y][1].id_clan)) bacteriums[x - 1][y][1].position_dead = 4;
				if (((x + 1) < maxX) && (bacteriums[x + 1][y][0].predator) && (bacteriums[x + 1][y][0].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x + 1][y][0].id_clan)) bacteriums[x + 1][y][0].position_dead = 3;
				if (((x + 1) < maxX) && (bacteriums[x + 1][y][1].predator) && (bacteriums[x + 1][y][1].activate) && (bacteriums[x][y][bac_level_enemy].id_clan == bacteriums[x + 1][y][1].id_clan)) bacteriums[x + 1][y][1].position_dead = 3;
			}
		}
	}
	return;
}

void step_bact(bacterium ***bacteriums, settings set, const unsigned int x, const unsigned int y, int rand_step, bool see_under_bac) {
	int x_pos = 0, y_pos = 0, bac_level;
	switch (rand_step) {
		case 1: y_pos = 1; break;
		case 2: y_pos = -1; break;
		case 3: x_pos = 1; break;
		case 4: x_pos = -1; break;
		default: return; break;
	}
	if (see_under_bac) bac_level = 1;
	else bac_level = 0;
	if ((!see_under_bac) && (bacteriums[(int)x - x_pos][(int)y - y_pos][0].activate) && (!bacteriums[(int)x - x_pos][(int)y - y_pos][1].activate)) { // Перевод бактерии на уровень ниже
		bacteriums[x][y][0].activate = false;
		bacteriums[(int)x - x_pos][(int)y - y_pos][1].activate = true;
		bacteriums[(int)x - x_pos][(int)y - y_pos][1].half_life = bacteriums[x][y][0].half_life;
		if (set.eat_add == 0) bacteriums[x][y][0].eat = true;
		else bacteriums[x][y][0].eat = false; 
		bacteriums[(int)x - x_pos][(int)y - y_pos][1].id_clan = bacteriums[x][y][0].id_clan;
		bacteriums[(int)x - x_pos][(int)y - y_pos][1].position_dead = bacteriums[x][y][0].position_dead;
		bacteriums[(int)x - x_pos][(int)y - y_pos][1].time_birth = bacteriums[x][y][0].time_birth;
		bacteriums[(int)x - x_pos][(int)y - y_pos][1].predator = bacteriums[x][y][0].predator;
	} else if ((see_under_bac) && (!bacteriums[(int)x - x_pos][(int)y - y_pos][0].activate)) { // Переход на уровень выше
		bacteriums[x][y][bac_level].activate = false;
		bacteriums[(int)x - x_pos][(int)y - y_pos][0].activate = true;
		bacteriums[(int)x - x_pos][(int)y - y_pos][0].eat = false;
		bacteriums[(int)x - x_pos][(int)y - y_pos][0].half_life = bacteriums[x][y][bac_level].half_life;
		bacteriums[(int)x - x_pos][(int)y - y_pos][0].id_clan = bacteriums[x][y][bac_level].id_clan;
		bacteriums[(int)x - x_pos][(int)y - y_pos][0].position_dead = bacteriums[x][y][bac_level].position_dead;
		bacteriums[(int)x - x_pos][(int)y - y_pos][0].time_birth = bacteriums[x][y][bac_level].time_birth;
		bacteriums[(int)x - x_pos][(int)y - y_pos][0].predator = bacteriums[x][y][bac_level].predator;
	} else if (!bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].activate) { // Переход на тотже уровень, где находиться бактерия
		bacteriums[x][y][bac_level].activate = false;
		bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].activate = true;
		bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].half_life = bacteriums[x][y][bac_level].half_life;
		if (set.eat_add == 0) bacteriums[x][y][bac_level].eat = true;
		else bacteriums[x][y][bac_level].eat = false;
		bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].id_clan = bacteriums[x][y][bac_level].id_clan;
		bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].position_dead = bacteriums[x][y][bac_level].position_dead;
		bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].time_birth = bacteriums[x][y][bac_level].time_birth;
		bacteriums[(int)x - x_pos][(int)y - y_pos][bac_level].predator = bacteriums[x][y][bac_level].predator;
		if (rand_step == 2) bacteriums[x - x_pos][y - y_pos][bac_level].left_right_action = true;
		if ((rand_step == 4) && ((int)x != ((int)maxX - 1))) bacteriums[x - x_pos][y - y_pos][bac_level].left_right_action = true;
	}
	if ((rand_step == 4) || (rand_step == 2)) bacteriums[(int)x - x_pos][(int)y - y_pos][1].left_right_action = true; // Дабы не обрабатывать одну бактерию несколько раз
	see_down(bacteriums, set, (int)x - x_pos, (int)y - y_pos, see_under_bac);
	return;
}

int action_bacteriums (bacterium ***bacteriums, settings set, const unsigned int x, const unsigned int y, int bac_level) {
	int rand_step;
	if (!bacteriums[x][y][bac_level].activate) return 0;
	if (bacteriums[x][y][bac_level].left_right_action) {bacteriums[x][y][bac_level].left_right_action = false; return 0;}
	if (bacteriums[x][y][bac_level].half_life <= 0) {
		bacteriums[x][y][bac_level].activate = false;
		if (set.eat_add == 0) bacteriums[x][y][bac_level].eat = true;
		else bacteriums[x][y][bac_level].eat = false;
		return 0;
	}
	/*
		1 - UP
		2 - DOWN
		3 - LEFT
		4 - RIGHT
	*/
	if ((bacteriums[x][y][bac_level].time_birth <= 0) && (birth(bacteriums, set, x, y, bac_level))) return 0;
	int rand_step_count = 4, first_rand_step = 1, cont_search = 0; // Защита от бесконечного поиска пустой клетки
	int step_to_eat = 0;
	if ((bacteriums[x][y][bac_level].predator) && (set.go_to_eat_zigzas)) step_to_eat = get_eat(bacteriums, x, y, bac_level);
	if ((!bacteriums[x][y][bac_level].predator) && (set.go_to_eat_himanurs)) step_to_eat = get_eat(bacteriums, x, y, bac_level);
	if (step_to_eat != 0) rand_step = step_to_eat;
	else {
		bacteriums[x][y][bac_level].position_dead = 0;
		switch (bacteriums[x][y][bac_level].position_dead) { // Выбор шага в рандомную сторону в зависимости от позиции смерти брата
			case 1: first_rand_step = 2; break;
			case 2: cont_search = 2; break;
			case 3: cont_search = 3; break;
			case 4: rand_step_count = 3; break;
		}
		rand_step = rand_local(first_rand_step, rand_step_count);
		while ((!freedom_of_bacterium(bacteriums, rand_step, x, y, bac_level)) || (cont_search == rand_step)) {
			if (rand_step_count == rand_step) rand_step_count--;
			rand_step = rand_local(first_rand_step, rand_step_count);
			rand_step_count--;
			if (rand_step_count <= 0) { // Если свободного места не найдено - то стоять на месте
				rand_step = 0;
				break;  
			}
		}
	}
	bacteriums[x][y][bac_level].half_life--;
	bacteriums[x][y][bac_level].time_birth--;
	step_bact(bacteriums, set, x, y, rand_step, bac_level);
	return rand_step;
}

void add_eat_to_map(bacterium ***bacteriums, settings set) {
	for (unsigned int y = 0; y < maxY; y++)
		for (unsigned int x = 0; x < maxX; x++)
			if ((!bacteriums[x][y][0].activate) && (!bacteriums[x][y][0].eat) && (rand_local(1, set.eat_probability) == 1)) bacteriums[x][y][0].eat = true; 
	return;
}

void view_map(bacterium ***bacteriums, settings set, bool menu, int menu_point, bool pause_now, int bac_level, unsigned int x_extr, unsigned int y_extr, bool action) {
	int bac_level_enemy = 0, redraw = 0;
	char bottom_view = '-';
	if (bac_level == 1) bottom_view = 'X';
	if (bac_level == 0) bac_level_enemy = 1;
	// erase(); // Я так и не смог понять, влияет ли очистка экрана на производительность или нет...
	for (unsigned int y = 0; y < maxY; y++)
		for (unsigned int x = 0; x < maxX; x++) {
			if (action) {
				if (bac_level == 0) redraw = action_bacteriums(bacteriums, set, x, y, 0); else action_bacteriums(bacteriums, set, x, y, 0); // Обработка верхнего уровня
				if (bac_level == 1) redraw = action_bacteriums(bacteriums, set, x, y, 1); else action_bacteriums(bacteriums, set, x, y, 1); // Обработка нижнего уровня
			}
			if ((x + x_extr) >= maxX) continue; // Дабы не было вылета при экстаполяции
			if ((y + y_extr) >= maxY) continue; // Наверное, стоит добавить оптимизацию, чтобы не прорисовывались невидимые элементы... но... сейчас 3:37 ночи..
			if ((redraw == 3) || (redraw == 1)) { // Всё прекрасно работает и без этого блока, но на всякий-пожарный он тут...
				int redraw_pos_x = 0, redraw_pos_y = 0; // Смысл в том, что если бактерия ушла влево или вверх, то нарисовать её
				if (redraw == 3) redraw_pos_x = 1; else redraw_pos_y = 1; // Но, повторюсь, всё, почему-то, и так нормально работает!
				if ((bacteriums[x + x_extr - redraw_pos_x][y + y_extr - redraw_pos_y][bac_level].predator) && (bacteriums[x + x_extr - redraw_pos_x][y + y_extr - redraw_pos_y][bac_level].activate) && (!bacteriums[x + x_extr - redraw_pos_x][y + y_extr - redraw_pos_y][bac_level_enemy].activate)) {
					attron(COLOR_PAIR(1) | A_BOLD);
					mvprintw(y - redraw_pos_y, x - redraw_pos_x, "*"); // Хищник
					attroff(COLOR_PAIR(1) | A_BOLD);
				} else if ((!bacteriums[x + x_extr - redraw_pos_x][y + y_extr - redraw_pos_y][bac_level].predator) && (bacteriums[x + x_extr - redraw_pos_x][y + y_extr - redraw_pos_y][bac_level].activate) && (!bacteriums[x + x_extr - redraw_pos_x][y + y_extr - redraw_pos_y][bac_level_enemy].activate)) {
					attron(COLOR_PAIR(2) | A_BOLD);
					mvprintw(y - redraw_pos_y, x - redraw_pos_x, "*"); // Травоядное
					attroff(COLOR_PAIR(2) | A_BOLD);
				} else if ((bacteriums[x + x_extr - redraw_pos_x][y + y_extr - redraw_pos_y][bac_level].activate) && (bacteriums[x + x_extr - redraw_pos_x][y + y_extr - redraw_pos_y][bac_level_enemy].activate)) {
					attron(COLOR_PAIR(5) | A_BOLD);
					mvprintw(y - redraw_pos_y, x - redraw_pos_x, "*"); // Место соприкосновения и того и того
					attroff(COLOR_PAIR(5) | A_BOLD);
				}
			} // Не знаю, стоит ли так подробно комментировать код... Но дополнять у меня уже нет сил...
			if ((bacteriums[x + x_extr][y + y_extr][bac_level].predator) && (bacteriums[x + x_extr][y + y_extr][bac_level].activate) && (!bacteriums[x + x_extr][y + y_extr][bac_level_enemy].activate)) {
				attron(COLOR_PAIR(1) | A_BOLD);
				mvprintw(y, x, "*"); // Хищник
				attroff(COLOR_PAIR(1) | A_BOLD);
			} else if ((!bacteriums[x + x_extr][y + y_extr][bac_level].predator) && (bacteriums[x + x_extr][y + y_extr][bac_level].activate) && (!bacteriums[x + x_extr][y + y_extr][bac_level_enemy].activate)) {
				attron(COLOR_PAIR(2) | A_BOLD);
				mvprintw(y, x, "*"); // Травоядное
				attroff(COLOR_PAIR(2) | A_BOLD);
			} else if ((bacteriums[x + x_extr][y + y_extr][bac_level].activate) && (bacteriums[x + x_extr][y + y_extr][bac_level_enemy].activate)) {
				attron(COLOR_PAIR(5) | A_BOLD);
				mvprintw(y, x, "*"); // Место соприкосновения и того и того
				attroff(COLOR_PAIR(5) | A_BOLD);
			} else if (bacteriums[x + x_extr][y + y_extr][bac_level].eat) {
				attron(COLOR_PAIR(3) | A_BOLD);
				mvprintw(y, x, "."); // Еда
				attroff(COLOR_PAIR(3) | A_BOLD);
			} else mvprintw(y, x, " "); // Пустота...
		} // Да, подробные комментарии точно нужны! А то завтра я ничего не пойму...
	/*if (set.extrapolation) { // Вывож ползунка, при экстраполяции
		attron(COLOR_PAIR(4) | A_BOLD);
		mvprintw(y_extr - 1, MXC - 1, " ");
		attroff(COLOR_PAIR(4) | A_BOLD);
	}*/
	// Ниже идёт бональный вывод стрелок...
	if ((set.extrapolation) && ((y_extr + MYC) < maxY)) {
		attron(COLOR_PAIR(4) | A_BOLD);
		mvprintw(MYC - 1, MXC / 2 - 2, " v ");
		attroff(COLOR_PAIR(4) | A_BOLD);
	}
	if ((set.extrapolation) && ((x_extr + MXC) < maxX)) {
		attron(COLOR_PAIR(4) | A_BOLD);
		mvprintw(MYC / 2, MXC - 1, " ");
		mvprintw(MYC / 2 - 1, MXC - 1, ">");
		mvprintw(MYC / 2 - 2, MXC - 1, " ");
		attroff(COLOR_PAIR(4) | A_BOLD);
	}
	if ((set.extrapolation) && (x_extr > 0)) {
		attron(COLOR_PAIR(4) | A_BOLD);
		mvprintw(MYC / 2, 0, " ");
		mvprintw(MYC / 2 - 1, 0, "<");
		mvprintw(MYC / 2 - 2, 0, " ");
		attroff(COLOR_PAIR(4) | A_BOLD);
	}
	if ((set.extrapolation) && (y_extr > 0)) {
		attron(COLOR_PAIR(4) | A_BOLD);
		mvprintw(0, MXC / 2 - 2, " ^ ");
		attroff(COLOR_PAIR(4) | A_BOLD);
	}
	if (pause_now) { // Не поверишь! Этот блок выводит сообщение о паузе! Здорово, правда!?
		attron(COLOR_PAIR(4) | A_BOLD);
		mvprintw(MYC / 2, MXC / 2 - 8, "Simulation paused");
		attroff(COLOR_PAIR(4) | A_BOLD);
	}
	if (menu) { // Меню и обвязка...
		attron(COLOR_PAIR(4) | A_BOLD);
		mvprintw(0, 0, "\'v\' - menu | \'Esc\' - exit");
		attroff(COLOR_PAIR(4) | A_BOLD);
	}
	if ((menu_point > 0) && (menu)) {
		attron(COLOR_PAIR(4) | A_BOLD);
		if (menu_point == 1) mvprintw(1, 0, ">"); else mvprintw(1, 0, "-");
		mvprintw(1, 1, "Pause simulation        ");
		if (menu_point == 2) mvprintw(2, 0, ">"); else mvprintw(2, 0, "-");
		mvprintw(2, 1, "Settings                ");
		if (menu_point == 3) mvprintw(3, 0, ">"); else mvprintw(3, 0, "-");
		mvprintw(3, 1, "View bottom        < %c >", bottom_view);
		if (menu_point == 4) mvprintw(4, 0, ">"); else mvprintw(4, 0, "-");
		mvprintw(4, 1, "Restart simulation      ");
		if (menu_point == 5) mvprintw(5, 0, ">"); else mvprintw(5, 0, "-");
		mvprintw(5, 1, "Out of simulation       ");
		if (menu_point == 6) mvprintw(6, 0, ">"); else mvprintw(6, 0, "-");
		mvprintw(6, 1, "Close menu              ");
		attroff(COLOR_PAIR(4) | A_BOLD);
	}
	return;
}


settings start_simulation(settings set, bool *restart_sim) {
	*restart_sim = false;
	if (auto_resolution) getmaxyx(stdscr, maxY, maxX); // Получение размера терминала
	else {
		maxX = set.maxXmanual;
		maxY = set.maxYmanual;
	}
	set.maxXY = maxX * maxY;
	bacterium ***bacteriums = new bacterium **[maxX]; // Создание массива для карты
	for (unsigned int x = 0; x < maxX; x++) {
		bacteriums[x] = new bacterium *[maxY];
		for (unsigned int y = 0; y < maxY; y++)
			bacteriums[x][y] = new bacterium [2];
	}
	for (unsigned int y = 0; y < maxY; y++) // Чтобы что-то сделать надо, для начала, всё разрушить...
		for (unsigned int x = 0; x < maxX; x++) {
			bacteriums[x][y][0].activate = false;
			bacteriums[x][y][1].activate = false;
			bacteriums[x][y][0].eat = true;
			bacteriums[x][y][1].eat = false;
			bacteriums[x][y][0].left_right_action = false;
			bacteriums[x][y][1].left_right_action = false;
		}
	int new_x, new_y;
	int clans_bac_local = set.clans_bac; // Если разрешение слишком мало для вмещения всех
	if (clans_bac_local == 0) clans_bac_local = rand_local(1, 15); //Рандомное задание кол-ва кланов
	if (set.clans_bac > set.maxXY) clans_bac_local = set.maxXY - 1; // Если коланы не влезают в экран
	if ((set.clans_bac == 0) || ((set.num_himanurs == 0) && (set.num_zigzas == 0))) { // Если не кого симулировать, то выдать ошибку
		erase();
		printw("Ooopsss... I can't start simulation? not found clans...");
		getch();
		return set;
	}
	int temp_z = set.num_zigzas; // Для создания определёённого кол-ва бактерий определённого типа
	for (int i = 0; i < clans_bac_local; i++) { // Создание кланов бактерий
		new_x = rand_local(0, maxX - 1); new_y = rand_local(0, maxY - 1); // Выбор рандомного местоположения
		while (bacteriums[new_x][new_y][0].activate) { // Чтобы никто друг на друга не залез 
			new_x = rand_local(0, maxX - 1); new_y = rand_local(0, maxY - 1); // Поиск рандомной позиции
		}
		if (set.clans_bac == 0) {
			if (rand_local(1, 2) == 1) bacteriums[new_x][new_y][0].predator = true; // Рандомное задание хищника или плотоядного, если выбрано рандомное кол-во кланов
			else bacteriums[new_x][new_y][0].predator = false;
		} else {
			if (temp_z > 0) {
				bacteriums[new_x][new_y][0].predator = true; // Создавать хищников
				temp_z--;
			} else bacteriums[new_x][new_y][0].predator = false;
		} // В общем дальше идёт инициализация стандартных параметров
		bacteriums[new_x][new_y][0].position_dead = 0;
		if (bacteriums[new_x][new_y][0].predator) bacteriums[new_x][new_y][0].time_birth = rand_local(1, set.full_lives * set.division_zigzas / 100); // Задание времени до деления
		else bacteriums[new_x][new_y][0].time_birth = rand_local(1, (set.full_lives * set.division_himanurs / 100));
		bacteriums[new_x][new_y][0].activate = true; // "Оживление"
		bacteriums[new_x][new_y][0].id_clan = rand_local(1000, 9999); // Рандомный ID клана
		if (bacteriums[new_x][new_y][0].predator) bacteriums[new_x][new_y][0].half_life = set.full_lives * set.lives_zigzas / 100; // Задание периода жизни
		else bacteriums[new_x][new_y][0].half_life = set.full_lives * set.lives_himanurs / 100;
	}
	bool break_state = false, action = true;
	bool menu_local = false;
	int eat_delay_local = set.eat_add, local_delay = set.bac_delay, bac_level = 0;
	int selected_point = 0;
	int state = 0; // Состояния плавающих окон
	int key_press = -1;
	unsigned int float_w_x = 0, float_w_y = 0, x_extr = 0, y_extr = 0;
	bool pause_now = false;
	timeout(0);
	erase();
	while (!break_state) { // Главный цикл
		if ((!pause_now) && (local_delay <= 0)) {
			local_delay = set.bac_delay;
			action = true; // Будет передаваться в выводчик карты, для движения бактерий
			if ((eat_delay_local <= 0) && (set.eat_probability > 0)) { // Счётчик добавления еды
				add_eat_to_map(bacteriums, set);
				eat_delay_local = set.eat_add; // Восстановление стандартных значений
			} else if (set.eat_probability > 0) eat_delay_local--;
		} else if (!pause_now) {
			action = false;
			#ifdef WIN32 // Почему пол секунды? Не знаю. Это просто работает...
			Sleep(0.5); // Задержка для windows
			#else
			usleep(500); // Задержка для unix подобных
			#endif
			local_delay--;
		}
		view_map(bacteriums, set, menu_local, selected_point, pause_now, bac_level, x_extr, y_extr, action);
		if (state == 2) set = float_settings(set, float_w_x, float_w_y, selected_point, key_press); // Я думаю, тут и так всё понятно...
		if (state == 3) set = zigzas_settings(set, true, float_w_x, float_w_y, selected_point, key_press);
		if (state == 4) set = himanurs_settings(set, true, float_w_x, float_w_y, selected_point, key_press);
		key_press = -1; // Интересно, почему -1?..
		switch (getch()) { // Очень навороченный обработчик нажатий
			case 27: switch (state) {
							case 0: timeout(-1); erase(); printw("Are you sure? (Enter = yes/Other key = no)");
									if (getch() == '\n') break_state = true; else timeout(0);
									erase();
									break;
							case 1: if (menu_local) {menu_local = false; selected_point = 0; state = 0;} erase(); break;
							case 2: menu_local = true; state = 1; selected_point = 1; erase(); break;
							case 3: state = 2; menu_local = false; selected_point = 4; erase(); break;
							case 4: state = 2; menu_local = false; selected_point = 5; erase(); break;
						} break;
			case 80: if (pause_now) pause_now = false; else pause_now = true; break;
			case 112: if (pause_now) pause_now = false; else pause_now = true; break;
			case 77: if (menu_local) {menu_local = false; if (state == 1) state = 0;} // M
							else if (state == 0) {menu_local = true; state = 1;} // Вызов меню
							break;
			case 109: if (menu_local) {menu_local = false; if (state == 1) state = 0;} // m
							else if (state == 0) {menu_local = true; state = 1;}
							break;
			case KEY_UP: switch (state) {
							case 0: if (y_extr > 0) y_extr--; break;
							case 1: if (selected_point <= 1) selected_point = 6; else selected_point--; break;
							case 2: if (selected_point == 0) selected_point = 9; else selected_point--; break;
							case 3: if (selected_point == 0) selected_point = MAX_POS_BAC_SET; else selected_point--; break;
							case 4: if (selected_point == 0) selected_point = MAX_POS_BAC_SET; else selected_point--; break;
						} break;
			case KEY_DOWN: switch (state) {
							case 0: if ((y_extr + MYC) < maxY) y_extr++; break;
							case 1: if (selected_point == 6) selected_point = 1; else selected_point++; break;
							case 2: if (selected_point == 9) selected_point = 0; else selected_point++; break;
							case 3: if (selected_point == MAX_POS_BAC_SET) selected_point = 0; else selected_point++; break;
							case 4: if (selected_point == MAX_POS_BAC_SET) selected_point = 0; else selected_point++; break;
						} break;
			case KEY_LEFT: switch (state) {
								case 0: if (x_extr > 0) x_extr--; break;
								case 1: if (selected_point == 3) {
										if (bac_level == 0) bac_level = 1; else bac_level = 0;
									} break;
								case 2: switch (selected_point) {
										case 7: if (float_w_x != 0) float_w_x--; break;
										case 8: if (float_w_y != 0) float_w_y--; break;
										default: key_press = KEY_LEFT; break;
									} break;
								case 3: key_press = KEY_LEFT; break;
								case 4: key_press = KEY_LEFT; break;
						} break;
			case KEY_RIGHT: switch (state) {
								case 0: if ((x_extr + MXC) < maxX) x_extr++; break;
								case 1: if (selected_point == 3) {
										if (bac_level == 0) bac_level = 1; else bac_level = 0;
									} break;
								case 2: switch (selected_point) {
											case 7: if ((float_w_x + 40) < MXC) float_w_x++; break;
											case 8: if ((float_w_y + 10) < MYC) float_w_y++; break;
											default: key_press = KEY_RIGHT; break;
										} break;
								case 3: key_press = KEY_RIGHT; break;
								case 4: key_press = KEY_RIGHT; break;
						} break;
			case '\n': switch (state) {
							case 1: switch (selected_point) {
										case 1: if (pause_now) pause_now = false; else pause_now = true; erase(); break;
										case 2: state = 2; menu_local = false; selected_point = 0; erase(); break;
										case 4: timeout(-1); erase(); printw("Are you sure? (Enter = yes/Other key = no)");
												if (getch() == '\n') {break_state = true; *restart_sim = true;} else timeout(0);
												erase(); break;
										case 5: timeout(-1); erase(); printw("Are you sure? (Enter = yes/Other key = no)");
												if (getch() == '\n') break_state = true; else timeout(0);
												erase(); break;
										case 6: selected_point = 0; menu_local = false; state = 0; break;} erase(); break;
							case 2: switch (selected_point) {
										case 4: state = 3; selected_point = 0; break;
										case 5: state = 4; selected_point = 0; break;
										case 6: timeout(-1); erase(); printw("Are you sure? (Enter = yes/Other key = no)");
												if (getch() == '\n') {set = load_conf(set, false); timeout(0);} else timeout(0);
												break;
										case 9: menu_local = true; state = 1; selected_point = 1;} erase(); break;
							case 3: switch (selected_point) {
										case MAX_POS_BAC_SET: state = 2; menu_local = false; selected_point = 4; break;
										} erase(); break;
							case 4: switch (selected_point) {
										case MAX_POS_BAC_SET: state = 2; menu_local = false; selected_point = 5; break;
										} erase(); break;
						} break;
			default: break;
		} // Не буду я его комментировать! Влом!
	}
	for (unsigned int x = 0; x < maxX; x++) { // Очистка памяти
		for (unsigned int y = 0; y < maxY; y++)
			delete [] bacteriums[x][y];
		delete [] bacteriums[x];
	}
	delete [] bacteriums;
	return set;
}

void info() {
	erase();
	printw("Petri Dish - live simulator\n\n");
	printw("Version: %s\n", ver);
	printw("\n:DV company 2015\n");
	printw("\nPress any key...\n");
	getch();
	return;
}

void main_menu() {
	settings set;
	set = standart_settings(set);
	set = load_conf(set, false);
	int user_select = 0; // Указатель в меню
	bool restart_sim = false;
	while (true) {
		timeout(-1); // Задержка ожидания getch ("-1" - бесконечно)
		erase(); // Очистка экрана
		printw("Petri Dish\n\n");
		if (user_select == 0) printw(">"); else printw("-");
		printw("Start simulation\n");
		if (user_select == 1) printw(">"); else printw("-");
		printw("Settings\n");
		if (user_select == 2) printw(">"); else printw("-");
		printw("Help\n");
		if (user_select == 3) printw(">"); else printw("-");
		printw("Information\n");
		if (user_select == 4) printw(">"); else printw("-");
		printw("Exit\n");
		switch (getch()) { // Обработка нажатия
			case KEY_UP:
						if (user_select == 0) user_select = 4;
						else user_select--;
						break;
			case KEY_DOWN:
						if (user_select == 4) user_select = 0;
						else user_select++;
						break;
			case '\n': // Enter
						switch (user_select) {
							case 0: do {set = start_simulation(set, &restart_sim);} while (restart_sim); break;
							case 1: set = main_settings(set); break;
							case 2: help(-1); break;
							case 3: info(); break;
							case 4: endwin(); return; break; // Отключение графического режима и завершение процедуры
						}
						break;
		}
	}
	return;
}

/*
	This code consists of:
	80%	- Bad code on C++
	25%	- Magic
	5%	- Bear on a small bike
*/

int main() {
	initscr(); // Инициализация экрана
	start_color();
	keypad (stdscr, TRUE);
	noecho(); // Не выводить символы с клавиатуры
	curs_set(0);
	erase(); // Очистка экрана
	getmaxyx(stdscr, MYC, MXC); // Получение размера терминала
	init_pair (1, COLOR_RED, COLOR_BLACK); // Инициализация красного цвета
	init_pair (2, COLOR_GREEN, COLOR_BLACK); // Инициализация зелёного цвета
	init_pair (3, COLOR_WHITE, COLOR_BLACK); // Инициализация белого цвета
	init_pair (4, COLOR_BLACK, COLOR_WHITE);
	init_pair (5, COLOR_CYAN, COLOR_BLACK);
	main_menu(); // Вызов главного меню
	endwin();
	return 0;
}

/*
	Yes... 110%... A lot of magic...
*/