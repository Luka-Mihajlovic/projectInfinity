//125 image width 1920x1080 for ascii art
//(upper - lower + 1)) + lower for rng

//give_item for item getting

//TODO: FLOOR SCALING, MAIN MENU, DEATH SCREEN

#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strings
#include <math.h>
#include <Windows.h> //sleep
#include <time.h> //random seed, time

struct perks{
	int perk_ID;
	char perk_name[30];
	char perk_desc[200];
	int counter;
};

struct statuses{
	int status_ID;
	int turns_left;
	
	char status_name[30];
};

struct equipment{
	int item_ID;
	char item_name[50];
	
	char item_description[200];
	
	int hp_boost;
	int xp_mult;
	int healing_mult;
	
	int atk_boost;
	int def_boost;
	int agi_boost;
	int dex_boost;
	int spd_boost;
	int luck_boost;
	
	int equipment_type;
};

struct inventory{
	int item_ID;
	char item_name[50];
	
	char item_description[200];
	
	int hp_boost;
	int xp_mult;
	int healing_mult;
	
	int atk_boost;
	int def_boost;
	int agi_boost;
	int dex_boost;
	int spd_boost;
	int luck_boost;
	
	int equipment_type; // slots, ABC = 123 OR 0 = anything, reserved for empty
	int is_equipment; //0 = consumable, 1 = equipment;
	int is_taken; //0 = empty, 1 = taken;
};

struct character{
	int level;
	int xp;
	int reqxp;
	
	int state; //0 = dead, 1 = exploration, 2 = fighting, 3 = event
	int menu_state; //STATE 1: 0 = main, 1 = map, 2 = inventory, 3 = level up, 4 = status, 5 = choosing levelup, 6 = info on equipment |FLIP ->| STATE 2: 0 = thinking, 1 = attacking, 2 = inventory, 3 = defending
	
	int hp;
	int maxhp;
	
	//stats
	int atk; //atk
	int def; //def
	int agi; //hit chance
	int dex; //dodge chance
	int spd; //turn order
	int luck; //crit chance / anti crit chance
	
	int character_class; //1 = knight
	
	struct inventory inv[5]; //inventory, 5 slots
	struct equipment eq[3]; //equipment, 3 slots
	
	struct statuses buffs[10];
	struct perks perks[3];
	
	int floor;
	int location_x;
	int location_y;
	
	int last_location_x;
	int last_location_y;
};

struct basestats{
	
	int maxhp;
	
	int atk;
	int def;
	int agi;
	int dex;
	int spd;
	int luck;
	
};

struct room{
	int room_type;	//0 = nonexistant, 1 = hallway, 2 = event, 3 = encounter, 4 = floor up, 5 = starting room
	int enemy_ID;
	
	int event_ID; //what event
	char background[100]; //whats the bg	
	int is_cleared; //show on map, cleared
	int is_visible; //reveal to player when next to
	char flavor_text[300]; //description :)
};

struct loot{
	int item_ID;
	char item_name[50];
};

struct event_ongoing{
	int event_ID;
	char event_name[50];
	char event_desc1[200];
	char event_desc2[200];
	
	int num_of_choices;
	char event_choice1[100];
	char event_choice2[100];
	char event_choice3[100];
};

struct enemy_to_fight{
	char enemy_name[50];
	
	int hp;
	int maxhp;
	
	int atk;
	int def;
	int agi;
	int dex;
	int spd;

	char fight_bg[100];
	int counter;
};

struct enemy_to_fight enemy; //enemy being fought

struct basestats bstats; //player's base stats, used in calculation
struct character player; //the player

struct room map[7][7]; //map
struct loot loot[5]; //loot being rendered
struct event_ongoing ongoing_event; //event happening rn

int turn;
int turn_priority; //0 = player, 1 = enemy
char combat_action_1[200]; //line 1 combat info
char combat_action_2[200]; //line 2 combat info

//LOOT POOLS vvv
int enemy_pool_f1[33] = 	{1,2,3,4,6,7,10,11,14,15,24,25,27,31,32,37,38,45,46,47,49,52,55,56,58,60,61,62,63,67,68,72,73};
int enemy_pool_f2[36] = 	{4,5,8,10,12,16,17,18,19,20,21,22,26,28,33,35,36,37,39,42,44,48,51,53,54,55,56,57,60,61,64,65,66,69,72,73};
int enemy_pool_f3[24] = 	{9,10,13,20,23,29,30,34,35,40,41,42,43,44,50,51,54,59,64,65,69,70,71,73};

int event_pool_f1f2[23] = 	{5,8,10,16,17,18,19,26,28,35,37,45,60,61,64,65,66,67,68,69,70,71,73};
int event_pool_f3[13] = 	{9,10,13,23,30,34,40,41,50,51,64,66,69};

int text_color; //text color :3
int dice; //THE ALPHA AND OMEGA

void heal_player(heal){
	player.hp = player.hp + heal;
	if(player.hp > player.maxhp){
		player.hp = player.maxhp;
	}
}

int check_for_perk(int ID){
	int i;
	for(i=0;i<3;i++){
		if(player.perks[i].perk_ID == ID){
			return 1;
		}
	}
	
	return 0;	
}

void remove_buff(int slot){
	int i, temp_id, temp_turns;
	
	for(i=slot;i<10;i++){
		if(i == 9){
			player.buffs[i].status_ID = 0;
		}else{
			temp_id = player.buffs[i+1].status_ID;
			temp_turns = player.buffs[i+1].turns_left;
			
			player.buffs[i].status_ID = temp_id;
			player.buffs[i].turns_left = temp_turns;
			
			player.buffs[i+1].turns_left = 0;
			player.buffs[i+1].status_ID = 0;
		}
	}
}

void buff_handler(){
	int i;
	
	for(i=0;i<10;i++){
		if(player.buffs[i].status_ID == 0){
			break;
		}
		
		if(player.buffs[i].turns_left <= 0){
			remove_buff(i);
			i--;
		}
	}
	
	for(i=0;i<10;i++){
		if(player.buffs[i].status_ID == 0){
			break;
		} else
		player.buffs[i].turns_left--;
	}
}

void give_buff(int id, int time){
	int i, slot;
	
	for(i=0;i<10;i++){
		if(player.buffs[i].status_ID == 0){
			slot = i;
			break;
		}
	}
	
	switch(id){
		case(1):
			strcpy(player.buffs[slot].status_name, "Odbrambeni stav");
		break;
		
		case(2):
			strcpy(player.buffs[slot].status_name, "Izbegavanje");
		break;
		
		case(3):
			strcpy(player.buffs[slot].status_name, "Prva pomoc");
		break;
		
		case(4):
			strcpy(player.buffs[slot].status_name, "Naciljan napad");
		break;
		
		case(5):
			strcpy(player.buffs[slot].status_name, "Spremno oruzje");
		break;
		
		case(6):
			strcpy(player.buffs[slot].status_name, "Planiran napad");
		break;
		
		case(7):
			strcpy(player.buffs[slot].status_name, "Umor");
		break;
		
		case(8):
			strcpy(player.buffs[slot].status_name, "Snaga");
		break;
		
		case(9):
			strcpy(player.buffs[slot].status_name, "Dimna bomba");
		break;
		
		case(10):
			strcpy(player.buffs[slot].status_name, "Teski napad");
		break;
		
		case(11):
			strcpy(player.buffs[slot].status_name, "Razoruzan");
		break;
		
		case(13):
			strcpy(player.buffs[slot].status_name, "Probijena odbrana");
		break;
		
		case(14):
			strcpy(player.buffs[slot].status_name, "Usporen");
		break;
		
		case(15):
			strcpy(player.buffs[slot].status_name, "Krvarenje");
		break;
	}
	
	player.buffs[slot].status_ID = id;
	player.buffs[slot].turns_left = time;
}

void inventory_check(){
	int i;
	for(i=0;i<5;i++){
		if(player.inv[i].item_ID == 0){
			player.inv[i].is_taken = 0;
		}
	}
}

void setup_item(int ID, int slot, int mode){
	
	FILE *item;
	int counter=1;
	char IDstring[100];
	
	sprintf(IDstring, "%i", ID);
	
	char temporary[200];
	char filearea[25] = "items/";
	
	strcat(filearea, IDstring);
	strcpy(IDstring, filearea);	
	strcat(IDstring, ".txt");
	
	item = (fopen(IDstring, "r"));
	
	if(mode == 0){
		while(!feof(item)){ //read stats from file
			fgets(temporary,200,item);
			
			switch(counter){
				case 1:
					strcpy(player.inv[slot].item_name, temporary);	
					strtok(player.inv[slot].item_name, "\n"); //delete the trailing newline
					break;
					
				case 2:
					strcpy(player.inv[slot].item_description, temporary);
					strtok(player.inv[slot].item_description, "\n"); //delete the trailing newline
					break;
					
				case 4:
					player.inv[slot].hp_boost = atoi(temporary);
					break;
					
				case 5:
					player.inv[slot].xp_mult = atoi(temporary);
					break;
					
				case 6:
					player.inv[slot].healing_mult = atoi(temporary);
					break;
					
				case 8:
					player.inv[slot].atk_boost = atoi(temporary);
					break;
					
				case 9:
					player.inv[slot].def_boost = atoi(temporary);
					break;
					
				case 10:
					player.inv[slot].agi_boost = atoi(temporary);
					break;
					
				case 11:
					player.inv[slot].dex_boost = atoi(temporary);
					break;
					
				case 12:
					player.inv[slot].spd_boost = atoi(temporary);
					break;
					
				case 13:
					player.inv[slot].luck_boost = atoi(temporary);
					break;
					
				case 15:
					player.inv[slot].equipment_type = atoi(temporary);
					break;
					
				case 16:
					player.inv[slot].is_equipment = atoi(temporary);
					break;
					
				case 18:
					player.inv[slot].item_ID = atoi(temporary);
					break;
			}
			counter++;
	}
	
	if(player.inv[slot].item_ID == 0){
		player.inv[slot].is_taken = 0;
	} else
	player.inv[slot].is_taken = 1;
	
}
else{
	while(!feof(item)){ //read stats from file
			fgets(temporary,100,item);
			
			switch(counter){
				case 1:
					strcpy(player.eq[slot].item_name, temporary);	
					strtok(player.eq[slot].item_name, "\n"); //delete the trailing newline
					break;
					
				case 2:
					strcpy(player.eq[slot].item_description, temporary);
					strtok(player.eq[slot].item_description, "\n"); //delete the trailing newline
					break;
					
				case 4:
					player.eq[slot].hp_boost = atoi(temporary);
					break;
					
				case 5:
					player.eq[slot].xp_mult = atoi(temporary);
					break;
					
				case 6:
					player.eq[slot].healing_mult = atoi(temporary);
					break;
					
				case 8:
					player.eq[slot].atk_boost = atoi(temporary);
					break;
					
				case 9:
					player.eq[slot].def_boost = atoi(temporary);
					break;
					
				case 10:
					player.eq[slot].agi_boost = atoi(temporary);
					break;
					
				case 11:
					player.eq[slot].dex_boost = atoi(temporary);
					break;
					
				case 12:
					player.eq[slot].spd_boost = atoi(temporary);
					break;
					
				case 13:
					player.eq[slot].luck_boost = atoi(temporary);
					break;
					
				case 15:
					player.eq[slot].equipment_type = atoi(temporary);
					break;
					
				case 18:
					player.eq[slot].item_ID = atoi(temporary);
					break;
			}
			counter++;
	}
}
	
	fclose(item);
}

void setup_enemy(int ID){
	FILE *enemy_file;
	int counter=1;
	char IDstring[100];
	
	sprintf(IDstring, "%i", ID);
	
	char temporary[200];
	char filearea[25] = "enemies/";
	
	strcat(filearea, IDstring);
	strcpy(IDstring, filearea);	
	strcat(IDstring, ".txt");
	
	enemy_file = (fopen(IDstring, "r"));
	
		while(!feof(enemy_file)){ //read stats from file
			fgets(temporary,200,enemy_file);
			
			switch(counter){
				case 1:
					strcpy(enemy.enemy_name, temporary);	
					strtok(enemy.enemy_name, "\n"); //delete the trailing newline
					break;

				case 3:
					enemy.maxhp	= atoi(temporary);
					enemy.hp = enemy.maxhp;
					break;
				
				case 5:
					enemy.atk = atoi(temporary);
					break;
					
				case 6:
					enemy.def = atoi(temporary);
					break;
					
				case 7:
					enemy.agi = atoi(temporary);
					break;
					
				case 8:
					enemy.dex = atoi(temporary);
					break;
					
				case 9:
					enemy.spd = atoi(temporary);
					break;
					
				case 11:
					strcpy(enemy.fight_bg, temporary);
					strtok(enemy.enemy_name, "\n"); //delete the trailing newline
					break;
			}
			counter++;
	}	
	fclose(enemy_file);
	
	int scaling;
	scaling = player.floor/2;
	
	enemy.atk = enemy.atk + scaling;
	enemy.maxhp = enemy.maxhp + scaling;
	enemy.hp = enemy.maxhp;	
}

void setup_loot(int ID, int slot){
	
	FILE *item;
	char IDstring[100];
	
	sprintf(IDstring, "%i", ID);
	
	char temporary[200];
	char filearea[25] = "items/";
	
	strcat(filearea, IDstring);
	strcpy(IDstring, filearea);	
	strcat(IDstring, ".txt");
	
	item = (fopen(IDstring, "r"));
	
	fgets(temporary,100,item);
	
	strcpy(loot[slot].item_name, temporary); //put da name	
	strtok(loot[slot].item_name, "\n"); //delete the trailing newline
	
	loot[slot].item_ID = ID; //put da ID
	
	fclose(item);
}

void give_item(int ID){
	int slot = 0; int i; int is_full = 0; char answer; //0=not full, 1=full
	
	for (i=0;i<5;i++){
		if(player.inv[i].is_taken == 1){
			slot++;
			if(slot==5){
				is_full = 1;
			}
		}else
		
		break;
	}
	
	if(is_full == 1){
		printf("Inventar vam je pun! Da li zelite da izbacite nesto iz njega? (y/n) \n");
		scanf(" %c", &answer);
		
		if (answer == 'y'){
			printf("Napisite slot (1-5) koji biste hteli da odbacite. \n");
			scanf("%i", &slot);	
			slot--; //korisnik ukucava slot 1-5, ali je zapravo 0-4
		} else
		return;
	}
	
	setup_item(ID, slot, 0);
}

void render_image(char image[100]){
	
	int finished = 0;
	
	FILE *render_screen;
	char c;
	
	render_screen = (fopen( ("%s", image), "r"));
	
	while((c=fgetc(render_screen))!=EOF){
        printf("%c",c);
    }
    
    fclose(render_screen);
}

void setup_visible(){
	
	if(player.location_x+1 < 7){
		if (map[player.location_y][player.location_x+1].room_type != 0){
			map[player.location_y][player.location_x+1].is_visible = 1;	
		}
	}
	
	if(player.location_x-1 > 0){			
		if (map[player.location_y][player.location_x-1].room_type != 0){
			map[player.location_y][player.location_x-1].is_visible = 1;	
		}
	}
	
	if(player.location_y+1 < 7){
		if (map[player.location_y+1][player.location_x].room_type != 0){
			map[player.location_y+1][player.location_x].is_visible = 1;	
		}	
	}			
	
	if(player.location_y-1 > 0){
		if (map[player.location_y-1][player.location_x].room_type != 0){
				map[player.location_y-1][player.location_x].is_visible = 1;	
		}	
	}				
}

void render_map(){
	int i,j;
	
	for (i=0;i<7;i++){
		setup_visible();	
		for(j=0;j<7;j++){
			if (i == player.location_y && j==player.location_x){
				printf("[\xFE]");
				map[i][j].is_cleared = 1;
			} else
			
			if (map[i][j].is_cleared == 1){
				switch(map[i][j].room_type){
					case(1): //hallway
						printf("[O]");
						break;
					case(2): //event
						printf("[\x13]");
						break;
					case(3): //encounter
						printf("[\x9E]");
						break;
					case(4): //floor up
						printf("[\x18]");
						break;
					case(5): //starting room
						printf("[\x7F]");
						break;
				}
			} else
			
			if (map[i][j].is_cleared == 0 && map[i][j].is_visible == 0){
				printf("[ ]");
			} else
			
			if (map[i][j].is_visible == 1){
				printf("[?]");
			}
		}
		printf("\n");
	}
}

void render_stats(){
	int i;
	
	printf("ATK: "); //print attack
	for(i=0;i<player.atk;i++){
		printf("\xFE ");
	}
	printf("\n");
	
	printf("DEF: "); //print def
	for(i=0;i<player.def;i++){
		printf("\xFE ");
	}
	printf("\n");
	
	printf("AGI: "); //print agi
	for(i=0;i<(player.agi)/10;i++){
		printf("\xFE ");
	}
	printf("\n");
	
	printf("DEX: "); //print dex
	for(i=0;i<(player.dex)/10;i++){
		printf("\xFE ");
	}
	printf("\n");
	
	printf("SPD: "); //print spd
	for(i=0;i<player.spd;i++){
		printf("\xFE ");
	}
	printf("\n");
	
	printf("LCK: "); //print luck
	for(i=0;i<player.luck;i++){
		printf("\xFE ");
	}
	printf("\n\n");
	
}

void render_inventory(){
	printf("TRENUTNA OPREMA:\t\tINVENTAR:\n");
	
	printf("Slot A: "); //eq slot 1
	printf("%s", player.eq[0].item_name);
	if(player.eq[0].item_ID == 0){
		printf("\t");
	}
	
	printf("\t\tSlot 1: "); //inv slot 1
	printf("%s\n", player.inv[0].item_name);
	
	printf("Slot B: "); //eq slot 2
	printf("%s", player.eq[1].item_name);
	if(player.eq[1].item_ID == 0){
		printf("\t");
	}
	
	printf("\t\tSlot 2: "); //inv slot 2
	printf("%s\n", player.inv[1].item_name);
	
	printf("Slot C: "); //eq slot 3
	printf("%s", player.eq[2].item_name);
	if(player.eq[2].item_ID == 0){
		printf("\t");
	}
	
	printf("\t\tSlot 3: "); //inv slot 3
	printf("%s\n", player.inv[2].item_name);
	
	printf("\t\t\t\tSlot 4: "); //inv slot 4
	printf("%s\n", player.inv[3].item_name);
	
	printf("\t\t\t\tSlot 5: "); //inv slot 5
	printf("%s\n", player.inv[4].item_name);
	
	printf("\n");
}

void equipment_info_eq(int n){
	printf("---\n");
	printf("IME: %s\n", player.eq[n].item_name);
	printf("OPIS: %s\n", player.eq[n].item_description);
	printf("ATK/DEF: +%i/+%i \n", player.eq[n].atk_boost, player.eq[n].def_boost);
	printf("AGI/DEX: +%i/+%i \n", player.eq[n].agi_boost, player.eq[n].dex_boost);
	printf("SPD/LUCK: +%i%/+%i% \n", player.eq[n].spd_boost, player.eq[n].luck_boost);
	printf("HP/HEAL: +%i/x%i \n", player.eq[n].hp_boost, player.eq[n].healing_mult);
	printf("XP: x%i\n", player.eq[n].xp_mult);
	printf("---\n");
}

void equipment_info_inv(int n){
	
	if(player.inv[n].is_equipment == 1){
		printf("---\n");
		printf("(EQUIPMENT) - IME: %s\n", player.inv[n].item_name);
		printf("OPIS: %s\n", player.inv[n].item_description);
		printf("ATK/DEF: +%i/+%i \n", player.inv[n].atk_boost, player.inv[n].def_boost);
		printf("AGI/DEX: +%i/+%i \n", player.inv[n].agi_boost, player.inv[n].dex_boost);
		printf("SPD/LUCK: +%i%/+%i% \n", player.inv[n].spd_boost, player.inv[n].luck_boost);
		printf("HP/HEAL: +%i/x%i \n", player.inv[n].hp_boost, player.inv[n].healing_mult);
		printf("XP: x%i\n", player.inv[n].xp_mult);
		printf("---\n");
	} else{
		printf("---\n");
		printf("(CONSUMABLE) - Ime: %s\n", player.inv[n].item_name);
		printf("OPIS: %s\n", player.inv[n].item_description);
		printf("---\n");
	}
	
	
}

void equipment_info(){
	char selected_equipment;
	char choice;
	
	printf("Unesite slot o kojem zelite vise informacija. (A,B,C,1,2...) \n");
	
	equipment_printer:
	scanf(" %c", &selected_equipment);
	
	switch(selected_equipment){
		case('a'):
			equipment_info_eq(0);
			break;
			
		case('b'):
			equipment_info_eq(1);
			break;
		
		case('c'):
			equipment_info_eq(2);
			break;
			
		case('1'):
			equipment_info_inv(0);
			break;
		
		case('2'):
			equipment_info_inv(1);
			break;
			
		case('3'):
			equipment_info_inv(2);
			break;
			
		case('4'):
			equipment_info_inv(3);
			break;
			
		case('5'):
			equipment_info_inv(4);
			break;		
	}
	
	printf("Da li zelite da vidite statistike jos necega? (y/n)\n");
	scanf(" %c", &choice);
	if (choice == 'y'){
		goto equipment_printer;
	} else
	player.menu_state = 2;
}

void equip_item(int slot){
	int t;
	t = player.inv[slot].item_ID; //put the selected item into holding for later
	
	switch(player.inv[slot].equipment_type){
		case(1): //head
			player.inv[slot].item_ID = player.eq[0].item_ID; //copy selected inventory to the head slot			
			setup_item(player.inv[slot].item_ID, slot, 0); //set it up, inventory slot turns into said item
			
			player.eq[0].item_ID = t; //from holding copy that onto the head slot
			setup_item(player.eq[0].item_ID, 0, 1); //set it up, head slot turns into held
			break;
			
		case(2): //torso
			player.inv[slot].item_ID = player.eq[1].item_ID; //copy selected inventory to the body slot			
			setup_item(player.inv[slot].item_ID, slot, 0); //set it up, inventory slot turns into said item
			
			player.eq[1].item_ID = t; //from holding copy that onto the body slot
			setup_item(player.eq[1].item_ID, 1, 1); //set it up, body slot turns into held
			break;
			
		case(3)://weapon
			player.inv[slot].item_ID = player.eq[2].item_ID; //copy selected inventory to the weap slot		 		
			setup_item(player.inv[slot].item_ID, slot, 0); //set it up, inventory slot turns into said item
			
			player.eq[2].item_ID = t; //from holding copy that onto the weap slot
			setup_item(player.eq[2].item_ID, 2, 1); //set it up, weapon slot turns into held
			break;
	}
	
	inventory_check();
}

void render_divide(){ //render the border division
	char image[100];
	strcpy(image,"backgrounds/border.txt");
	render_image(image);
}

void clear_map(){
	int i,j;
	
	for (i=0;i<7;i++){
		for(j=0;j<7;j++){
			map[i][j].room_type = 0;
			map[i][j].is_cleared = 0;
			map[i][j].is_visible = 0;
		}
	}
	
	map[3][3].room_type = 5;	
}

void put_an_exit(){
	int x_rand,y_rand;
	int has_exit = 0;
	
	while(has_exit == 0){
	
	x_rand = (rand() % (6 - 0 + 1)) + 0; //rand 0-6
	y_rand = (rand() % (6 - 0 + 1)) + 0; //rand 0-6
	
	if (map[y_rand][x_rand].room_type == 1){
		map[y_rand][x_rand].room_type = 4;
		has_exit = 1;
		}
	}
}

void set_events(int roomnum){
	int i,num_of_events,x_rand,y_rand;
	int events = 20; //HOW MANY EVENTS THERE ARE
	
	num_of_events = player.floor + ((rand() % (2 - 1 + 1)) + 1); //F1 => [2-3], F2 => [3-4], F3 => [4-5]
	
	if(player.floor >= 19){
		num_of_events = 10;
	}
	
	for(i=0;i<num_of_events;i++){
		randomizer:
		x_rand = (rand() % (6 - 0 + 1)) + 0; //rand 0-6
		y_rand = (rand() % (6 - 0 + 1)) + 0; //rand 0-6
		
		if(map[y_rand][x_rand].room_type == 0 || map[y_rand][x_rand].room_type == 5){
			goto randomizer;	
		}
		
		map[y_rand][x_rand].room_type = 2; 
		
		dice = (rand() % (events - 1 + 1)) + 1;
		map[y_rand][x_rand].event_ID = dice;
		map[y_rand][x_rand].is_cleared = 0;
	}
}

void set_enemies(){
	int i,num_of_enemies,x_rand,y_rand;
	num_of_enemies = player.floor + ((rand() % (2 - 1 + 1)) + 1); //F1 => [1-3], F2 => [2-4], F3 => [3-5]
	
	if(player.floor >= 19){
		num_of_enemies = 20;
	}
	
	for(i=0;i<num_of_enemies;i++){
		randomizer:
		x_rand = (rand() % (6 - 0 + 1)) + 0; //rand 0-6
		y_rand = (rand() % (6 - 0 + 1)) + 0; //rand 0-6
		
		if(map[y_rand][x_rand].room_type == 0 || map[y_rand][x_rand].room_type == 2 || map[y_rand][x_rand].room_type == 5){
			goto randomizer;	
		}
		
		map[y_rand][x_rand].room_type = 3; 
		
		if(player.floor == 1){
				dice = (rand() % (2 - 1 + 1)) + 1; //d2 TO GENERATE ENEMY
			} else
		dice = (rand() % (3 - 1 + 1)) + 1; //d3 TO GENERATE ENEMY
		
		map[y_rand][x_rand].enemy_ID = dice;
		map[y_rand][x_rand].is_cleared = 0;
	}
}

void generate_map(){
	
	int main_x, main_y, i, number_of_rooms;
	
	main_x = 3; main_y = 3;
	
	number_of_rooms = (((rand() % (11 - 8 + 1)) + 8) + (player.floor * 2)); //F1 => [10-13], F2 => [12 - 15], F3 => [14 - 17]
	
	if(player.floor >= 19){
		number_of_rooms = 48;
	}
	
	for (i=0;i<number_of_rooms;i++){
	dice = (rand() % (4 - 1 + 1)) + 1; //rand 1-4
		switch (dice){
			case(1): //gen right
				if (main_x+1 < 7){
					if (map[main_y][main_x+1].room_type != 0){
						main_x++;
						i--;
					} else{
						map[main_y][main_x+1].room_type = 1;
						main_x++;
					}
				} else
				i--;
				break;
				
			case(2): //gen left
				if (main_x-1 > -1){
					if (map[main_y][main_x-1].room_type != 0){
						main_x--;
						i--;
					}else{
						map[main_y][main_x-1].room_type = 1;
						main_x--;
					}
				} else
				i--;
				break;
			
			case(3): //gen up
				if (main_y-1 > -1){
					if (map[main_y-1][main_x].room_type != 0){
						main_y--;
						i--;
					} else{
						map[main_y-1][main_x].room_type = 1;
						main_y--;
					}
				} else
				i--;
				break;
				
			case(4): //gen down
				if (main_y+1 < 7){
					if (map[main_y+1][main_x].room_type != 0){
						main_y++;
						i--;
					} else{
						map[main_y+1][main_x].room_type = 1;
						main_y++;
					}
				} else
				i--;
				break;
		}
	}
	
	set_events(number_of_rooms);
	set_enemies(number_of_rooms);
	put_an_exit();	
	strcpy(map[3][3].flavor_text,"Stepeniste se nalazi iza vas, ali biste trebali da nastavite dalje.\nAko put nazad ne uzimate u obzir, ova prostorija je ista kao i sve ostale ovde.");
}

void buff_calc(int ID){
	switch(ID){
		case(1): //odbrambeni stav
			player.def = player.def + 2;
			player.atk = player.atk - 1;
		break;
		
		case(2): //izbegavanje
			player.dex = player.dex + 30;
			player.def = player.def - 3;
			if(player.def <= 0){
				player.def = 0;
			}
		break;
		
		case(3): //prva pomoc
			player.spd = player.spd - 2;
		break;
		
		case(4): //focus
			player.agi = player.agi + 20;
		break;
		
		case(5): //spremno oruzje
			player.atk = player.atk + 2;	
		break;
		
		case(6): //plan
			player.spd = player.spd + 5;	
		break;
		
		case(8): //steroids
			player.atk = player.atk + 1;
			player.def = player.def + 1;
			player.agi = player.agi + 5;
			player.dex = player.dex + 5;
			player.spd = player.spd + 1;
		break;
		
		case(9):
			player.dex = player.dex + 30;
			player.spd = player.spd + 5;
		break;
		
		case(11):
			player.atk = player.atk - 2;
		break;
		
		case(12):
			player.agi = 0;
		break;
		
		case(13):
			player.def = player.def - 2;
		break;
		
		case(14):
			player.spd = player.spd - 3;
		break;
	}
}

void update_stats(){
	int i;
	
	player.maxhp = bstats.maxhp;
	player.atk = bstats.atk;
	player.def = bstats.def;
	player.agi = bstats.agi;
	player.dex = bstats.dex;
	player.spd = bstats.spd;
	player.luck = bstats.luck;
	
	if(player.xp >= player.reqxp){
		player.xp = player.reqxp;
		printf("! Dostigli ste maksimum XP ovog nivoa, idite u [SPOSOBNOSTI] da otkrijete novi talenat. \n");
	}
	
	for(i=0;i<3;i++){ //add up equipment
			player.maxhp += player.eq[i].hp_boost;
			
			if((player.state == 2 && map[player.location_y][player.location_x].enemy_ID == 2) && (player.eq[i].item_ID == 17 || player.eq[i].item_ID == 18 || player.eq[i].item_ID == 19)){ //if silver vs werewolf
					player.atk = player.atk + (2*player.eq[i].atk_boost);
			}else
			player.atk += player.eq[i].atk_boost;
			
			player.def += player.eq[i].def_boost;
			player.agi += player.eq[i].agi_boost;
			player.dex += player.eq[i].dex_boost;
			player.spd += player.eq[i].spd_boost;
			player.luck += player.eq[i].luck_boost;
	}
	
	for(i=0;i<9;i++){
		if(player.buffs[i].status_ID == 0){
			break;
		} else
		buff_calc(player.buffs[i].status_ID);
	}
	
	if(player.hp > player.maxhp){
		player.hp = player.maxhp;
	}
	
	//PERCS (50mg)
	int hasperk;
	hasperk = check_for_perk(1); // adrenaline
	if(hasperk == 1){
		i=0;
		for(i=0;i<2;i++){
			if(player.maxhp - (player.hp+(5*i)) >= 5){
				player.atk++;
			}
		}
	}
	
	hasperk = check_for_perk(2); //relentless hits
	if(hasperk == 1){
		for(i=0;i<3;i++){
			if(player.perks[i].perk_ID == 2){
				break;
			}
		}
		player.atk = player.atk + player.perks[i].counter;
		player.def = player.def - player.perks[i].counter;
	}
	
	hasperk=check_for_perk(13); //BLOODLUST
	if(hasperk == 1 && player.state == 2){ 
		if(turn>=3){
			player.atk = player.atk + 1;
		}
		if(turn>=6){
			player.atk = player.atk + 1;
		}
	}
}

void setup_player(){
	
	int i;
	for(i=0;i<5;i++){
		setup_item(0,i,0);
		player.inv[i].is_taken = 0;
	}
	
	for(i=0;i<3;i++){
		setup_item(0,i,1);
	}
	
	for(i=0;i<9;i++){
		player.buffs[i].status_ID = 0;
	}
	
	for(i=0;i<3;i++){
		player.perks[i].perk_ID = 0;
	}
	
	switch(player.character_class){
		case(1): //knight
		
			player.level = 1;
			player.xp = 0;
			player.reqxp = 20;
			player.state = 1;
			player.menu_state = 0;
			
			bstats.maxhp = 20;
			bstats.atk = 2;
			bstats.def = 3;
			bstats.agi = 90;
			bstats.dex = 10;
			bstats.spd = 5;
			bstats.luck = 1;
			break;
	}
	
	update_stats();
	player.hp = player.maxhp;
	
	
}

void clear_loot(){
	int i;
	for(i=0;i<5;i++){
		loot[i].item_ID = 0;
		setup_loot(0, i);
	}
}

void victory_calc(){
	dice = (rand() % (7 - 3 + 1)) + 3; //3 - 7
	int xp, hasperk;
	xp = 5 + dice; //min 8 max 12 xp
	
	hasperk = check_for_perk(19); //FAST LEARNER
	if(hasperk == 1){
		xp = xp*2;
	}
	
	player.xp = player.xp + (xp*(player.eq[0].xp_mult)*(player.eq[1].xp_mult)*(player.eq[2].xp_mult)); 
	
	system("cls");
	render_divide(); printf("\n\n");
	printf("Pobeda protiv: %s\n", enemy.enemy_name);
	printf("\t----------\n");
	printf("XP Dobijen: %i\n", (xp*(player.eq[0].xp_mult)*(player.eq[1].xp_mult)*(player.eq[2].xp_mult)));
	printf("\t----------\n");
}

void print_loot(){
	int i;
	
	for(i=0;i<5;i++){
		if(loot[i].item_ID != 0){
			printf("%i - %s", i+1, loot[i].item_name);
	} else{
		printf("%i - ..........", i+1);
	}
		printf("\n");
	}
	printf("\n");
	render_divide(); printf("\n\n");
}

void loot_time(){
	int i, take_item, empty_counter;
	char choice;

	print_loot();
	
	printf("Da li zelite nesto od ovoga da uzmete? (y/n)\n");
	
	retry_choice:
	scanf(" %c", &choice);
	
	if(choice!='y' && choice!='n'){
		printf("Molimo vas da pokusate ponovo. \n");
		goto retry_choice;
	}else
	
	if(choice=='n'){
		clear_loot();
		return;
	}
	
	if(choice == 'y'){
		while(choice == 'y'){
			system("cls");
			render_divide(); printf("\n\n");
			print_loot();
			
			printf("Inventar: \n");
			render_inventory();
			render_divide(); printf("\n\n");
			
			empty_counter = 0;
			printf("Unesite cifru stvari koju zelite da uzmete. (1-5)\n");
			scanf("%i", &take_item);
			take_item--; //user puts on 1-5, it's actually 0-4

			give_item(loot[take_item].item_ID);
			
			loot[take_item].item_ID = 0;
			setup_loot(0, take_item);
			
			for(i=0;i<5;i++){
				if(loot[i].item_ID == 0){
					empty_counter++;
				}
			}
			
			if(empty_counter == 5){
				choice = 'n';
			} else{
				printf("Da li zelite jos nesto da uzmete? (y/n)\n");
				scanf(" %c", &choice);
			}
		}
	}
	
	clear_loot();
}

int generate_loot(int pool_length){
	dice = (rand() % (pool_length - 0 + 1)) + 0; //roll for array slot, 0-pool length
	
	if(player.floor > 3){
		if(map[player.location_y][player.location_x].room_type == 3){ //floor 3 combat
				dice = enemy_pool_f3[dice];
			}else
		dice = event_pool_f3[dice]; //floor 3 loot
	}
	
	switch(player.floor){
		case(1):
			if(map[player.location_y][player.location_x].room_type == 3){ //floor 1 combat
				dice = enemy_pool_f1[dice];
			}else
			dice = event_pool_f1f2[dice]; //floor 1+2 loot
		break;
		
		case(2):
			if(map[player.location_y][player.location_x].room_type == 3){ //floor 2 combat
				dice = enemy_pool_f2[dice]; 
			}else
			dice = event_pool_f1f2[dice]; //floor 1+2 loot
		break;
		
		case(3):
			if(map[player.location_y][player.location_x].room_type == 3){ //floor 3 combat
				dice = enemy_pool_f3[dice];
			}else
			dice = event_pool_f3[dice]; //floor 3 loot
		break;
	}
	
	return dice;
}

void ready_loot(){
	int i, pool_length;
	
	if(player.floor > 3){
		if(map[player.location_y][player.location_x].room_type == 3){ //floor 3 combat
				pool_length = sizeof(enemy_pool_f3) / sizeof(enemy_pool_f3[0]);
			}else
		pool_length = sizeof(event_pool_f3) / sizeof(event_pool_f3[0]); //floor 3 loot
	}
	
	//determine range
	switch(player.floor){
		case(1):
			if(map[player.location_y][player.location_x].room_type == 3){ //floor 1 combat
				pool_length = sizeof(enemy_pool_f1) / sizeof(enemy_pool_f1[0]);
			}else
			pool_length = sizeof(event_pool_f1f2) / sizeof(event_pool_f1f2[0]); //floor 1+2 loot
		break;
		
		case(2):
			if(map[player.location_y][player.location_x].room_type == 3){ //floor 2 combat
				pool_length = sizeof(enemy_pool_f2) / sizeof(enemy_pool_f2[0]);
			}else
			pool_length = sizeof(event_pool_f1f2) / sizeof(event_pool_f1f2[0]); //floor 1+2 loot
		break;
		
		case(3):
			if(map[player.location_y][player.location_x].room_type == 3){ //floor 3 combat
				pool_length = sizeof(enemy_pool_f3) / sizeof(enemy_pool_f3[0]);
			}else
			pool_length = sizeof(event_pool_f3) / sizeof(event_pool_f3[0]); //floor 3 loot
		break;
	}
	
	pool_length--; //pool length 1->n becomes 0->n-1 which is a ok
	
	printf("Dobijene stvari:\n");
	
	for(i=0;i<5;i++){
		dice = (rand() % (100 - 0 + 1)) + 0; //roll for getting loot, 0-100 
		if(dice >= i*30){ //compare (0,30,60,90...), for about 100% 70% 40% 10% chance at loot
			dice = generate_loot(pool_length);
			setup_loot(dice, i);
		}else
		break;
	}
	loot_time();
}

void setup_perks(int slot, struct perks available_perks[3]){
	int i;
	char temp[200];
	char IDstring[5];
	char location[100];
	
	strcpy(location, "perks/");
	
	sprintf(IDstring, "%i", available_perks[slot].perk_ID);
	strcat(location, IDstring);
	
	strcat(location, ".txt");
	
	FILE *perkfile = fopen(("%s", location), "r");
	
	for(i=0;i<2;i++){
		fgets(temp, 200, perkfile);
		strtok(temp, "\n");
		
		switch(i){
			case(0):
				strcpy(available_perks[slot].perk_name, temp);
			break;
			case(1):
				strcpy(available_perks[slot].perk_desc, temp);
			break;
		}
	}
	
}

void print_perks(struct perks available_perks[3]){
	int i;
	for(i=0;i<3;i++){
		if(available_perks[i].perk_ID == 0){
			printf("%i - .................... \n", i+1);
		}else{
			printf("%i - %s | ", i+1, available_perks[i].perk_name);
			printf("[%s]\n", available_perks[i].perk_desc);	
		}
	}
	printf("\n");
}

void calculate_one_time_perks(int ID){
	switch(ID){
		case(3):
			bstats.maxhp = bstats.maxhp - 10;
			if(player.hp > player.maxhp - 10){
				player.hp = (player.maxhp - 10);
			}
		break;
		
		case(4):
			bstats.maxhp = bstats.maxhp + 5;
			heal_player(5);
		break;
		
		case(5):
			bstats.maxhp = bstats.maxhp - 10;
			if(player.hp > player.maxhp - 10){
				player.hp = (player.maxhp - 10);
			}
			bstats.atk = bstats.atk + 3;
		break;
		
		case(9):
			bstats.dex = bstats.dex + 15;
		break;
		
		case(12):
			bstats.agi = bstats.agi + 25;
		break;
		
		case(15):
			bstats.def = bstats.def + 2;
	        bstats.spd = bstats.spd - 5;
	        bstats.dex = bstats.dex - 10;
		break;
		
		case(16):
			bstats.atk = bstats.atk + 1;
			bstats.def = bstats.def + 1;
			bstats.spd = bstats.spd + 1;
			bstats.agi = bstats.agi + 5;
			bstats.dex = bstats.dex + 5;
		break;
		
		case(17):
			bstats.atk = bstats.atk + 1;
		break;
		
		case(20):
			bstats.luck = bstats.luck + 10;
			bstats.agi = bstats.agi - 40;
		break;
		
		case(22):
			bstats.atk = bstats.atk * 2;
       		bstats.def = 0;
		break;
		
		case(23):
			bstats.spd = bstats.spd * 2;
       		bstats.dex = bstats.dex + 5;
		break;
		
		case(24):
			bstats.def = bstats.def + 2;
		break;
		
		case(27):
			bstats.atk = bstats.atk - 2;
			bstats.spd = bstats.spd - 5;
			bstats.dex = bstats.dex + 25;
		break;
		
		case(28):
			bstats.atk = bstats.atk + 3;
			bstats.spd = bstats.spd - 5;
			bstats.agi = bstats.agi - 50;
		break;
	}
}

void start_perks(){
	player.level = player.level + 1;
	player.xp = 0;
	player.reqxp = player.reqxp + 10;
	
	int i,j, perk_choice;
	struct perks available_perks[3];

	for(j=0;j<3;j++){
		
		roll_again:
		dice = (rand() % (28 - 1 + 1)) + 1; //rand 1-28
		
		for(i=0;i<3;i++){
			if(dice == player.perks[i].perk_ID){
				goto roll_again;
			}
			
			if(dice == available_perks[i].perk_ID){
				goto roll_again;
			}
		}
		
		available_perks[j].perk_ID = dice;
		setup_perks(j, available_perks);
	}
	
	print_perks(available_perks);
	
	scanf("%i", &perk_choice); //unosi 1-3, slots su 0-2
	perk_choice--;
	
	switch(player.level){
		case(2):
			player.perks[0].perk_ID = available_perks[perk_choice].perk_ID;
			setup_perks(0, player.perks);
		break;
		
		case(3):
			player.perks[1].perk_ID = available_perks[perk_choice].perk_ID;
			setup_perks(1, player.perks);
		break;
		
		case(4):
			player.perks[2].perk_ID = available_perks[perk_choice].perk_ID;
			setup_perks(2, player.perks);
		break;
	}
	
	calculate_one_time_perks(available_perks[perk_choice].perk_ID);
	
	system("cls");
	player.menu_state = 5;
}

void end_relentless_hits(){
	int i;
	for(i=0;i<3;i++){
		if(player.perks[i].perk_ID == 2){
			break;
		}
	player.perks[i].counter = 0;
}
}

void combat_victory(){
	int hasperk;
	
	int i;
	for(i=0;i<10;i++){ //remove status effects
		remove_buff(i);
	}
	
	//PERCS 50mg
	hasperk = check_for_perk(6); //discipline
	if(hasperk == 1){
		heal_player(3);
		if(player.hp >= player.maxhp){
			player.hp = player.maxhp;
		}
	}
	
	map[player.location_y][player.location_x].is_cleared = 1;
	player.state = 1;
	victory_calc();
	
	clear_loot();
	ready_loot();
}

void damage_player(dmg){
	player.hp = (player.hp - dmg);
	
	//PERCS 50mg
	int hasperk, i;
	hasperk = check_for_perk(11); //activate revenge
	if(hasperk == 1){
		for(i=0;i<3;i++){
			if(player.perks[i].perk_ID == 11){
				break;
			}
			player.perks[i].counter = 1;
		}
	}
	
}

void damage_enemy(dmg){
	int hasperk, i;
	if(dmg<0){
		dmg = 0;
	}
	
	hasperk = check_for_perk(11); //expend revenge
	if(hasperk == 1){
		for(i=0;i<3;i++){
			if(player.perks[i].perk_ID == 11){
				break;
			}
			player.perks[i].counter = 0;
			dmg = dmg + 1;
		}
	}
	
	hasperk = check_for_perk(21); //first blood
	if(hasperk == 1 && turn == 1){
			dmg = dmg * 2;
	}
	
	hasperk = check_for_perk(25); //BREAKTHROUGH
	if(hasperk == 1){
		dmg = dmg + enemy.def;
	}
	
	hasperk = check_for_perk(26); //MOMENT
	if(hasperk == 1){
		int moment;
		moment = (player.spd - enemy.spd);
		
		if(moment>2){
			moment = 2;
		} else
		if(moment<-2){
			moment = -2;
		}
		
		dmg = dmg + moment;
	}
	
	enemy.hp = (enemy.hp - dmg); //HIT EM
	if(enemy.hp <= 0 && player.state == 2){
		combat_victory();
	}
	
	//PERCS 50mg
	hasperk = check_for_perk(2); //relentless hits
	if(hasperk == 1){
		for(i=0;i<3;i++){
			if(player.perks[i].perk_ID == 2){
				break;
			}
			player.perks[i].counter = player.perks[i].counter + 1;
		}
	}
	
	hasperk = check_for_perk(3);
	if(hasperk == 1){
		dice = (rand() % (3 - 1 + 1)) + 1; //rand 1-3
		if(dice == 3){
			heal_player(1);
		}
	}
	
	hasperk = check_for_perk(17); //DOUBLE EDGED SWORD 2 
	if(hasperk == 1){
		damage_player(1);
	}
	
}

void attack_miss(){
	if(turn_priority == 0){
		sprintf(combat_action_1, "Napad je promasio %s!", enemy.enemy_name);
	}else
	sprintf(combat_action_2, "Napad je promasio %s!", enemy.enemy_name);
	
	end_relentless_hits();
}

void use_scroll(mult){
	dice = (rand() % (6 - 1 + 1)) + 1; //d6
	if(dice <= 3 ){
		mult = mult * -1; //turn negative
	}
	
	//PERC
	int hasperk;
	hasperk = check_for_perk(7); //magic knowledge
	if(hasperk == 1 && mult<0){
			mult = 1;
	}

	switch(dice){
		case(1):
			bstats.agi = bstats.agi + (mult*10);
			if(mult>0){
				printf("...Osecate se spretnije. [+%i AGI]\n", mult*10);
			}else
			printf("...Osecate se malo trapavo. [%i AGI]\n", mult*10);
		break;
		
		case(2):
			bstats.atk = bstats.atk + mult;
			if(mult>0){
				printf("...Snazniji ste! [+%i ATK]\n", mult);
			}else
			printf("...Osecate se slabije. [%i ATK]\n", mult);
		break;
		
		case(3):
			bstats.def = bstats.def + mult;
			if(mult>0){
				printf("...Oklop vam sija! [+%i DEF]\n", mult);
			}else
			printf("...Oklop vam je malo prsao? [%i DEF]\n", mult);
		break;
		
		case(4):
			bstats.dex = bstats.dex + (mult*10);
			if(mult>0){
				printf("...Kamen pada sa plafona posle citanja svitka, a vi ste ga sa lakocom izbegli. [+%i DEX]\n", mult*10);
			}else
			printf("...Da li je hodanje uvek bilo tako zamorno? [%i DEX]\n", mult*10);
		break;
		
		case(5):
			bstats.luck = bstats.luck + mult;
			if(mult>0){
				printf("...Pronasli ste detelinu sa cetiri lista na podu, sudbina je na vasoj strani! [+%i LUCK]\n", mult);
			}else
			printf("...Slomili ste ogledalo u blizini dok ste citali svitak. [%i LUCK]\n", mult);
		break;
		
		case(6):
			bstats.spd = bstats.spd + mult;
			if(mult>0){
				printf("...Brze, brze, brze! [+%i SPD]\n", mult);
			}else
			printf("...Usporili ste se malo. [%i SPD]\n", mult);
		break;
	}
}

void use_item(ID, slot){
	int i;
	switch(ID){
		case(62):
			printf("\nPopijete tecnost iz ampulice, rane na vasem telu brzu nestaju! [+5 HP]\n");
			player.hp += 5;
			if(player.hp > player.maxhp){
				player.hp = player.maxhp;
			}
		break;
		
		case(63):
			printf("\nPopijete tecnost iz ampulice, osecate se snaznije. [+1 ALL | 3 POTEZA]\n");
			give_buff(8, 3);
		break;
		
		case(64):
			printf("\nPopijete tecnost iz flasice, rane na vasem telu sasvim nestaju! [+10 HP]\n");
			player.hp += 10;
			if(player.hp > player.maxhp){
				player.hp = player.maxhp;
			}
		break;
		
		case(65):
			printf("\nPopijete tecnost iz flasice, osecate se kao da znate mnogo vise o svetu. [+5 XP]\n");
			player.xp += 5;
		break;
		
		case(66):
			printf("\nPopijete tecnost iz flasice, nista ne moze da vas zaustavi sada. [+1 ALL | 6 POTEZA]\n");
			give_buff(8, 6);
		break;
		
		case(67):
			printf("\nOtvorite vrecicu i iskoristite biljke u njemu. Statusi su vam uklonjeni!\n");
			for(i=0;i<10;i++){
				remove_buff(i);
			}
		break;
		
		case(68):
			printf("\n\nPosle pazljivog citanja, iskoristite neke carolije sa svitka...\n");
			sleep(3);
			use_scroll(1);
		break;
		
		case(69):
			printf("\n\nPosle pazljivog citanja, iskoristite neke carolije sa svitka...\n");
			sleep(3);
			use_scroll(2);
		break;
		
		case(70):
			printf("\n\nOtvarate knjigu i pocinjete malo da citate...\n");
			sleep(3);
			dice = (rand() % (5 - 1 + 1)) + 1; //d6
			if(dice <= 2 ){
				if(dice == 2){
					printf("...Naucili ste nove borilacke vestine! [+1 ATK] \n");
					bstats.atk = bstats.atk+1;
				} else{
					printf("...Naucili ste nove odbrambene tehnike. [+1 DEF] \n");
					bstats.def = bstats.def+1;
				}
			} else{
				printf("...Nista novo niste naucili.\n");
			}
		break;
		
		case(71):
			printf("\n\nUzimate svitak i pocinjete da citate...\n");
			sleep(3);
			dice = (rand() % (4 - 1 + 1)) + 1; //d6
			if(dice <= 2 ){
					printf("...Naucili ste nesto novo o ljudskom telu! [+2 MAX HP] \n");
					bstats.maxhp = bstats.maxhp + 2;
			} else{
				printf("...Nista niste razumeli.\n");
			}
		break;
		
		case(72):
			printf("\n\nBacate dimnu bombu!\n");
			give_buff(9,2);
		break;
		
		case(73):
			printf("\n\nBacate bombu!\n");
			damage_enemy(5);
		break;
	}	
	
	sleep(3);
	player.inv[slot].item_ID = 0;
	setup_item(0, slot, 0);
}

void player_battle_action(int action){
	int will_hit, will_crit, dmg, i, hasperk;
	
	dice = (rand() % (100 - 0 + 1)) + 0; //from 0->100 for hit
	if(dice<=(player.agi - enemy.dex)){
		will_hit = 1;
	} else
	will_hit = 0;

	
	dice = (rand() % (20 - 0 + 1)) + 0; //from 0->100 for crit
	if(dice<=player.luck){
		will_crit = 1;
	} else
	will_crit = 0;
	
	for(i=0;i<10;i++){
		if(player.buffs[i].status_ID == 10){
			
			//special calc for hit chance, 50%
			dice = (rand() % (100 - 0 + 1)) + 0; //from 0->100 for hit
			if(dice<=(50)){
				will_hit = 1;
			} else
			will_hit = 0;
			
			dice = (rand() % (2 - 0 + 1)) + 0; //from 0->2
			dice--; //make that -1 -> 1
			dmg = (player.atk*3 - enemy.def);	
			
			if(will_crit == 1){
				dmg = dmg*2;
			}
					
			dmg = dmg + dice;
			
			if(will_hit == 1){
						if(turn_priority == 0){
						sprintf(combat_action_1, "Snaznim udarcem napadate %s! [%i DMG]", enemy.enemy_name, dmg);
						if(will_crit == 1){
							strcat(combat_action_1, " CRIT!");
						}
					}else
					sprintf(combat_action_2, "Snaznim udarcem napadate %s! [%i DMG]", enemy.enemy_name, dmg);
					if(will_crit == 1){
						strcat(combat_action_2, " CRIT!");
					}

					damage_enemy(dmg);
			goto atk_end;
			}
		}
	}
	
	switch(player.menu_state){
		case(1):
			switch(action){
				case(1): //basic attack, atk damage
					dice = (rand() % (2 - 0 + 1)) + 0; //from 0->2
					dice--; //make that -1 -> 1
					dmg = (player.atk - enemy.def);	
					
					if(will_crit == 1){
						dmg = dmg*2;
					}
					
					dmg = dmg + dice;
					
					if(will_hit == 1){
						if(turn_priority == 0){
						sprintf(combat_action_1, "Napadate %s! [%i DMG]", enemy.enemy_name, dmg);
						if(will_crit == 1){
							strcat(combat_action_1, " CRIT!");
						}
					}else
					sprintf(combat_action_2, "Napadate %s! [%i DMG]", enemy.enemy_name, dmg);
					if(will_crit == 1){
						strcat(combat_action_2, " CRIT!");
					}

					damage_enemy(dmg);
					
					} else
					attack_miss();
				break;
				
				case(2): //slow attack
					if(turn_priority == 0){
						sprintf(combat_action_1, "Spremili ste snazan napad za sledeci potez.");
					}else
					sprintf(combat_action_2, "Spremili ste snazan napad za sledeci potez.");
					
					give_buff(10,1);
				break;
				
				case(3): //2x dmg, self damage
					dice = (rand() % (2 - 0 + 1)) + 0; //from 0->2
					dice--; //make that -1 -> 1
					dmg = (player.atk);	
					
					if(will_crit == 1){
						dmg = dmg*2;
					}
					dmg = dmg + dice;
					
					if(will_hit == 1){
						if(turn_priority == 0){
							sprintf(combat_action_1, "Napadate %s, ali samim tim i povredite sebe. [%i DMG, %i SELF]", enemy.enemy_name, dmg*2, dmg);
							if(will_crit == 1){
								strcat(combat_action_1, " CRIT!");
							}
						}else
						sprintf(combat_action_2, "Napadate %s, ali samim tim i povredite sebe. [%i DMG, %i SELF]", enemy.enemy_name, dmg*2, dmg);
						if(will_crit == 1){
								strcat(combat_action_2, " CRIT!");
						}
						
						damage_enemy(dmg*2);
						damage_player(dmg);
						
						if (player.hp<=0){
							if(turn_priority == 0){
								sprintf(combat_action_1, "Napadate %s, ali samim tim i povredite sebe. Ne odustajete, ovoj povredi niste podlegli! [%i DMG, %i SELF]", enemy.enemy_name, dmg*2, dmg);
							}else
							sprintf(combat_action_2, "Napadate %s, ali samim tim i povredite sebe. Ne odustajete, ovoj povredi niste podlegli! [%i DMG, %i SELF]", enemy.enemy_name, dmg*2, dmg);
							
							player.hp = 1;
						}
					} else
					attack_miss();
				break;
				
				case(4): //aim
					if(turn_priority == 0){
							sprintf(combat_action_1, "Usredsredili ste se na neprijatelja, spremni ste da napadnete. [+20 AGI]");
						}else
						sprintf(combat_action_2, "Usredsredili ste se na neprijatelja, spremni ste da napadnete. [+20 AGI]");
						
					give_buff(4, 2);
				break;
				
				case(5): //ready attack
					if(turn_priority == 0){
							sprintf(combat_action_1, "Drzite oruzje ispred sebe, sa namerom da ovaj udar bude poslednji. [+2 ATK]");
						}else
						sprintf(combat_action_2, "Drzite oruzje ispred sebe, sa namerom da ovaj udar bude poslednji. [+2 ATK]");
						
					give_buff(5, 2);
				break;
								
				case(6): //plan up
					if(turn_priority == 0){
							sprintf(combat_action_1, "Uz plan stize i pobeda, znate sta trebate da uradite. [+5 SPD]");
						}else
						sprintf(combat_action_2, "Uz plan stize i pobeda, znate sta trebate da uradite. [+5 SPD]");
						
					give_buff(6, 2);
				break;
			}
		break;
		
		case(2): //inventory
		
			if(player.inv[action].item_ID != 0){
				if(turn_priority == 0){
					sprintf(combat_action_1, "Iskoristili ste %s.", player.inv[action].item_name);
				}else
				sprintf(combat_action_2, "Iskoristili ste %s.", player.inv[action].item_name);
						
				if(player.inv[action].is_equipment == 0){
					use_item(player.inv[action].item_ID, action);
				} else
					equip_item(action);
				} else{
					printf("Nemam nista u tom slotu, pokusajte ponovo. \n");
					scanf("%i", &action);
					action--;
					player_battle_action(action);
				}
			
			end_relentless_hits();
		break;
		
		case(3): //defending
			switch(action){
				case(1): //dodging
					if(turn_priority == 0){
						sprintf(combat_action_1, "Spremite se da izbegnete neprijateljske napade. [+30 DEX, -3 DEF]");
					}else
					sprintf(combat_action_2, "Spremite se da izbegnete neprijateljske napade. [+30 DEX, -3 DEF]");
					
					give_buff(2, 2);
				break;
				
				case(2): //defending stance
					if(turn_priority == 0){
						sprintf(combat_action_1, "Povucete se nazad, spremni da zaustavite bilo koji udarac. [+2 DEF, -1 ATK]");
					}else
					sprintf(combat_action_2, "Povucete se nazad, spremni da zaustavite bilo koji udarac. [+2 DEF, -1 ATK]");
					
					give_buff(1,2);
				break;
				
				case(3): //first aid
					dice = (rand() % (4 - 2 + 1)) + 2; //from 2->4
					
					hasperk = check_for_perk(10); //medicinal knowledge
					if(hasperk == 1){
						dice = dice + 2;
					}
					
					if(turn_priority == 0){
						sprintf(combat_action_1, "Obmotali ste ranu sa onim sto imate, osecate se malo bolje. [%i HP, -2 SPD]", dice);
					}else
					sprintf(combat_action_2, "Obmotali ste ranu sa onim sto imate, osecate se malo bolje. [%i HP, -2 SPD]", dice);
					
					heal_player(dice);
					
					if(player.hp>=player.maxhp){
						player.hp = player.maxhp;
					}	
					
					give_buff(3, 2);
				break;
			}
			end_relentless_hits();
		break;
	}
	
	atk_end:
		
	hasperk = check_for_perk(18); //REGENERATION 
	if(hasperk == 1){
		if((turn%3==0) && turn<=9){
			heal_player(1);
		}
	}
	
	
	for(i=0;i<10;i++){
		if(player.buffs[i].status_ID == 15){
			player.hp = player.hp - 1; //bloooood
		}
	}
	
	turn++;
	buff_handler();
}

void enemy_basic_attack(int will_hit, int will_crit, int dmg){
	if(will_hit == 1){
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vas napada! [%i DMG]", enemy.enemy_name, dmg);
			if(will_crit == 1){
				strcat(combat_action_1, " CRIT!");
			}
		}else
		sprintf(combat_action_2, "%s vas napada! [%i DMG]", enemy.enemy_name, dmg);
		if(will_crit == 1){
				strcat(combat_action_2, " CRIT!");
			}
		
		damage_player(dmg);
	} else{
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vas je promasio!", enemy.enemy_name);
		}else
		sprintf(combat_action_2, "%s vas je promasio!", enemy.enemy_name);
	}
}

void skeleton_disarm(int will_hit){
	if(will_hit == 1){
		give_buff(11,1);
		
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s izbija oruzje iz vasih ruku! [-2 ATK]", enemy.enemy_name);
		}else
		sprintf(combat_action_2, "%s izbija oruzje iz vasih ruku! [-2 ATK]", enemy.enemy_name);
		
	} else{
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vas je promasio!", enemy.enemy_name);
		}else
		sprintf(combat_action_2, "%s vas je promasio!", enemy.enemy_name);
	}
}

void skeleton_head_toss(int will_hit, int will_crit, int dmg){
	dmg = dmg * 2;
	
	if(will_hit == 1){
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vas gadja kostima! [%i DMG, %i SELF]", enemy.enemy_name, dmg, dmg/2);
			if(will_crit == 1){
				strcat(combat_action_1, " CRIT!");
			}
		}else
		sprintf(combat_action_2, "%s vas gadja kostima! [%i DMG, %i SELF]", enemy.enemy_name, dmg, dmg/2);
		if(will_crit == 1){
				strcat(combat_action_2, " CRIT!");
			}
		
		damage_player(dmg);
		damage_enemy(dmg/2);
	} else{
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vas gadja kostima, ali promasuje! [%i SELF]", enemy.enemy_name, dmg/2);
		}else
		sprintf(combat_action_2, "%s vas gadja kostima, ali promasuje! [%i SELF]", enemy.enemy_name, dmg/2);
		
		damage_enemy(dmg/2);
	}
}

void skeleton_pile(){
	if(turn_priority == 1){
			sprintf(combat_action_1, "%s postaje gomila kostiju, ne mozete ga povrediti sledeceg poteza.", enemy.enemy_name);
		}else
		sprintf(combat_action_2, "%s postaje gomila kostiju, ne mozete ga povrediti sledeceg poteza.", enemy.enemy_name);
		
	give_buff(12, 0);
}

void skeleton_turn(int will_hit, int will_crit, int dmg){
	int in_lead;
	if(enemy.hp > player.hp){
		in_lead = 1;
	} else
	in_lead = 0;
	
	dice = (rand() % (100 - 1 + 1)) + 1; //from 1->100
	
	switch(in_lead){
		case(0):
			 if(dice<=50){
			 	enemy_basic_attack(will_hit, will_crit, dmg);
			 } else{
			 	if(dice>=50 && dice<=75){
			 		skeleton_pile();
				 } else
				 	skeleton_disarm(will_hit);
				 }
		break;
		
		case(1):
			if(dice<=50){
				enemy_basic_attack(will_hit, will_crit, dmg);
			}else{
				if(dice>50 && dice<=65){
			 		skeleton_pile();
				 } else{
				 	if(dice>65 && dice<=75){
				 		skeleton_disarm(will_hit);
					 }else
				 	skeleton_head_toss(will_hit, will_crit, dmg);
				 }
			}
		break;
	}
}

void werewolf_howl(int will_hit){
	if(will_hit == 1){
		give_buff(13,3);
		
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s zavija, uliva strah u vas! [-2 DEF]", enemy.enemy_name);
		}else
		sprintf(combat_action_2, "%s zavija, uliva strah u vas! [-2 DEF]", enemy.enemy_name);
		
	} else{
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s zavija, nema efekta na vama.", enemy.enemy_name);
		}else
		sprintf(combat_action_2, "%s zavija, nema efekta na vama.", enemy.enemy_name);
	}
}

void werewolf_bite(int will_hit, int dmg){
	if(will_hit == 1){
		give_buff(15,2);
		dmg = dmg/2;
		
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vas ujeda za ruku. [%i DMG, +KRVARENJE]", enemy.enemy_name, dmg);
		}else
		sprintf(combat_action_2, "%s vas ujeda za ruku. [%i DMG, +KRVARENJE]", enemy.enemy_name, dmg);
		
		damage_player(dmg);
	} else{
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vas je promasio!", enemy.enemy_name);
		}else
		sprintf(combat_action_2, "%s vas je promasio!", enemy.enemy_name);
	}
	
	enemy.counter = 5;
}

void werewolf_rend(int will_hit, int will_crit, int dmg){
	if(will_hit == 1){
		give_buff(7,2);
		give_buff(14,2);
		
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vam napada noge! [%i DMG, -2 SPD]", enemy.enemy_name, dmg);
			if(will_crit == 1){
				strcat(combat_action_1, " CRIT!");
			}
		}else
		sprintf(combat_action_2, "%s vam napada noge! [%i DMG, -2 SPD]", enemy.enemy_name, dmg);
		if(will_crit == 1){
				strcat(combat_action_2, " CRIT!");
			}
		
		damage_player(dmg);
	} else{
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vam je promasio noge!", enemy.enemy_name);
		}else
		sprintf(combat_action_2, "%s vam je promasio noge!", enemy.enemy_name);
	}
}

void werewolf_turn(int will_hit, int will_crit, int dmg){
	dice = (rand() % (100 - 1 + 1)) + 1; //from 1->100
	
	if(dice<=25){
		werewolf_rend(will_hit, will_crit, dmg);		
	} else
	if(dice<=50 && dice>25){
		werewolf_howl(will_hit);
	}else
	if(dice>50 && dice<=65){
		werewolf_bite(will_hit, dmg);
	}else
	enemy_basic_attack(will_hit, will_crit, dmg);
}

void vampire_bite(int will_hit, int will_crit, int dmg){
	int heal = dmg*2;
	int i;
	
	for(i=0;i<10;i++){
		if(player.buffs[i].status_ID == 15){
			heal = dmg * 2;
		}
	}
	
	if(will_hit == 1){
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vam isisava krv! [-%i HP, +%i SELF]", enemy.enemy_name, dmg, heal);
			if(will_crit == 1){
				strcat(combat_action_1, " CRIT!");
			}
		}else
		sprintf(combat_action_2, "%s vam isisava krv! [-%i HP, +%i SELF]", enemy.enemy_name, dmg, heal);
		if(will_crit == 1){
				strcat(combat_action_2, " CRIT!");
			}
		
		damage_player(dmg);
		enemy.hp = enemy.hp + heal;
		if(enemy.hp>enemy.maxhp){
			enemy.hp = enemy.maxhp;
		}
	} else{
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vas je promasio!", enemy.enemy_name);
		}else
		sprintf(combat_action_2, "%s vas je promasio!", enemy.enemy_name);
	}
}

void vampire_telegraph(){
	if(turn_priority == 1){
			sprintf(combat_action_1, "%s nesto planira, pazite se sledeceg napada.", enemy.enemy_name);
		}else
	sprintf(combat_action_2, "%s nesto planira, pazite se sledeceg napada.", enemy.enemy_name);
}

void vampire_bleed(int will_hit, int dmg){
	if(will_hit == 1){
		give_buff(15,2);
		
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vam sece vrat bodezom! Ako krvarite onda je njegov ujed duplo efektivniji. [%i DMG, +KRVARENJE]", enemy.enemy_name, dmg);
		}else
		sprintf(combat_action_2, "%s vas sece vrat bodezom! Ako krvarite onda je njegov ujed duplo efektivniji. [%i DMG, +KRVARENJE]", enemy.enemy_name, dmg);
		
		damage_player(dmg);
	} else{
		if(turn_priority == 1){
			sprintf(combat_action_1, "%s vas je promasio!", enemy.enemy_name);
		}else
		sprintf(combat_action_2, "%s vas je promasio!", enemy.enemy_name);
	}
}

void vampire_turn(int will_hit, int will_crit, int dmg){
	int in_lead;
	
	if(enemy.hp > player.hp){
		in_lead = 1;
	} else
	in_lead = 0;
	
	dice = (rand() % (100 - 1 + 1)) + 1; //from 1->100
	
	switch(in_lead){
		case(0):
			 if(dice<=50){
			 	vampire_bite(will_hit, will_crit, dmg);
			 } else
			 enemy_basic_attack(will_hit, will_crit, dmg);
		break;
		
		case(1):
			if(dice<=40){
				vampire_bite(will_hit, will_crit, dmg);
			}else{
				if(dice>40 && dice<=75){
			 		if(enemy.counter != 1){
			 			vampire_telegraph();
			 			enemy.counter = 1;
					 } else{
					 	vampire_bleed(will_hit, dmg);
					 	enemy.counter = 0;
					 }
				 } else
				 enemy_basic_attack(will_hit, will_crit, dmg);
			}
		break;
	}
}

void enemy_action(){
	int will_hit, will_crit, dmg, hasperk;
	dmg = enemy.atk - player.def;
	
	if(dmg<=0){ //at least 1 damage
		dmg = 1;
	}
	
	dice = (rand() % (100 - 0 + 1)) + 0; //from 0->100 for hit
	if(dice<=(enemy.agi - player.dex)){
		will_hit = 1;
	} else
	will_hit = 0;

	
	dice = (rand() % (100 - 0 + 1)) + 0; //from 0->100 for crit
	if(dice<=5){
		will_crit = 1;
	} else
	will_crit = 0;
	
	if(will_crit==1){
		hasperk = check_for_perk(24); //ACHILLES HEEL
		if(hasperk == 1){
			dmg = dmg + player.def;
			dmg = dmg*2;
		}
		dmg = dmg*2;
	}
	
	switch(map[player.location_y][player.location_x].enemy_ID){	
		case(1): //skeleton
			skeleton_turn(will_hit, will_crit, dmg);
		break;
		
		case(2):
			werewolf_turn(will_hit, will_crit, dmg);
		break;
		
		case(3):
			vampire_turn(will_hit, will_crit, dmg);
		break;
	}
	
	//enemy_basic_attack(will_hit, will_crit, dmg);
}

void render_calm(char image[100]){
	render_divide();
	
	printf("\n		[F%i - %i, %i]: ", player.floor, player.location_x , player.location_y);
	switch(map[player.location_y][player.location_x].room_type){
		case(1): //hallway
			printf("HODNIK");
			strcpy(map[player.location_y][player.location_x].flavor_text,"Obican hodnik pun paucine i prljavstine. Baklje na zidovima osvetljavaju prostoriju. \nNema niceg od znacaja ovde, bila bi dobra ideja da ovde ne ostajete dugo.");
			break;
			
		case(2):
			printf("DOGADJAJ");
			break;
	
		case(3):
			printf("BORBA");
			strcpy(map[player.location_y][player.location_x].flavor_text,"Ovde se dogodila borba, ako nije bilo ocigledno po stetama u zidovima i podu. \nLes neprijatelja se nalazi ovde, ali ne zelite da ga gledate.");
			break;
			
		case(4):
			printf("STEPENISTE");
			strcpy(map[player.location_y][player.location_x].flavor_text,"Stepeniste koje idu gore ka sledecem spratu. Mozete da se popnete na sledeci sprat koristeci komandu 'idigore'. \nNecete moci da se vratite na sprat ispod, pa biste trebali da budete sasvim sigurni da zelite da idete dalje.");
			break;
			
		case(5):
			printf("POCETNA SOBA");
			break;
	}
	
	printf(" | HP:[%i/%i]", player.hp, player.maxhp);
	printf(" | XP:[%i/%i]", player.xp, player.reqxp);
	printf(" | MAPA");
	printf(" | SPOSOBNOSTI");
	if(player.xp == player.reqxp){
		printf("!");
	}
	printf(" | TALENTI");
	printf(" | INVENTAR");
	
	printf("\n");
	
	render_divide();
	printf("\n\n");
	
	switch(player.menu_state){
		case(0): //no menu
			render_image(image);
			printf("\n\n");
			break;
			
		case(1): //map
			render_map();
			printf("\n\n");
			break;
			
		case(2): //inventory or inventory info
		case(6):
			render_inventory();
			break;
			
		case(3): // levelup/stats
			if(player.xp == player.reqxp){
				player.menu_state = 5;
				system("cls");
				char zxcvbnm;
				printf("Izaberite jednu od tri posebnih talenta, one ce trajati zauvek i ne mogu da nestanu. \nBirajte pazljivo, jer se ovaj izbor ne moze promeniti.\nPretisnite bilo sta da nastavite.\n");
				scanf(" %c", &zxcvbnm);
				system("cls");
				start_perks();
			}else
			render_stats();
			break;
			
		case(4):
			print_perks(player.perks);
		break;
	}
	
	render_divide();
	printf("\n\n");
	
	switch(player.menu_state){
		case(0): //no menu
			printf("%s", map[player.location_y][player.location_x].flavor_text);
			break;
		case(1): //map
			printf("Mapa ovog sprata, moram nastaviti dalje. [Ukucajte komande 'gore', 'dole', 'levo' i 'desno']");
			break;
		case(2): //inventory
			printf("U slucaju da zelite da saznate statistike neke opreme, ukucajte 'oprema'.\nU slucaju da zelite da iskoristite nesto, samo ukucajte slot u kome se nalazi (1-5).");
			break;
		case(3): // levelup/stats
			printf("ATK, DEF = Napad i odbrana\nAGI, DEX = Preciznost i sansa za izbegnut napad, u procentima\nSPD = Brzina, sluzi za biranje prvog u potezu\nLCK = Sreca, sansa da napadi udare slabu tacku protivnika");
			break;
		
		case(4):
			printf("Vasi talenti, mozete ih imati do tri i dobijaju se svakog nivoa igraca.");
		break;
		
		case(5):
			printf("Unesite komande 'nazad', 'mapa', 'inventar' itd. da se vratite gde ste pre bili.");
		break;
		
		case(6): //equipment info
			equipment_info();
		break;
	}
	
	printf("\n\n");
	render_divide();
}

void decide_turn_priority(){
	int enemy_turn_chance, player_turn_chance;
	
			roll_turn_chance: //rolls speed check
			dice = (rand() % (8 - 1 + 1)) + 1; //d8
			player_turn_chance = dice + player.spd; //player's turn chance

			dice = (rand() % (8 - 1 + 1)) + 1; //d8
			enemy_turn_chance = dice + enemy.spd; //enemy's turn chance
			
			if(player_turn_chance >= enemy_turn_chance){ //compare, turn_priority = 0(player first) || 1(enemy first)
				if(player_turn_chance == enemy_turn_chance){
					goto roll_turn_chance;
				} else
				turn_priority = 0;
			} else
			turn_priority = 1;
}

void render_battle(char image[100]){
	int i, j;
	
	if(turn == 1){
		strcpy(combat_action_1, "Ovde ce se nalaziti informacije o borbi.");
		strcpy(combat_action_2, "Izaberite jedan od menija gore i napadnite neprijatelja!");	
	}
	
	
	render_divide();//TOP BAR
	
	printf("\n\t  [%i, %i]: ", player.location_x , player.location_y);
	printf("BORBA VS %s", enemy.enemy_name);
	printf(" | POTEZ %i", turn);

	
	printf(" | HP:[%i/%i]", enemy.hp, enemy.maxhp);
	printf(" | ATK/DEF:[%i|%i]", enemy.atk, enemy.def);
	if(player.hp <= (enemy.atk-player.def) || player.hp == 1){
		printf(" !!!");
	}

	printf(" | NAPAD");
	printf(" | ODBRANA");
	printf(" | INVENTAR");
	printf(" | BEZI!");
	
	printf("\n");
	
	render_divide();//END OF TOP BAR
	printf("\n\n");
	
	render_image(image); //render fight scene
	printf("\n\n");
	
	render_divide(); //bottom bar time
	printf("\n");
	
	printf("HP:["); //print hp
	
	j = player.hp;
	
	for(i=0;i<player.maxhp;i++){
		if(j>0){
			printf("\xFE");
			j--;
		}else
		printf("\x2e");
		
		if(i<player.maxhp-1){
			printf(" ");
		}
		
	}
	printf("]");
	
	int tohit;
	tohit = player.agi - enemy.dex;
	if(tohit<0){
		tohit = 0;
	}
	
	printf(" | ATK/DEF:[%i|%i]", player.atk, player.def);
	printf(" | CHANCE TO HIT:[%i%%]", (tohit));
	printf(" | CHANCE TO DODGE:[%i%%]", (player.dex));
	printf(" | SPD:[%i]\n", player.spd);
	
	render_divide(); //bottom bar end, action bar time
	printf("\nSTATUSI: ");
	
	j = 0;
	for(i=0;i<10;i++){
		if(player.buffs[i].status_ID != 0 && player.buffs[i].status_ID != 12){
			printf(" %s [%i]|", player.buffs[i].status_name, player.buffs[i].turns_left);
			j++;
		}
	}
	
	if(j==0){
		printf("/ |");
	}
	printf("\n");
	
	render_divide();
	switch(player.menu_state){
		case(1): //atk
		printf("\n\n");
		printf("1 | Napad oruzjem [%i DMG]\n", (player.atk - enemy.def));
		printf("2 | Snazan napad oruzjem [%i DMG, DVA POTEZA TRAJE, 50%% HIT CHANCE]\n", (player.atk*3 - enemy.def));
		printf("3 | Nesmotren udarac [%i DMG, %i DMG SEBI]\n", player.atk*2, player.atk);
		printf("		----------\n");
		printf("4 | Naciljaj napad [+20 AGI | 2 POTEZA]\n");
		printf("5 | Spremi oruzje [+2 ATK | 2 POTEZA]\n");
		printf("6 | Planiranje unapred [+5 SPD | 3 POTEZA]\n");
		printf("\n");
		
		render_divide();
		break;
			
		case(2): //inv
		printf("\n\n");
		printf("Inventar: \n");
		printf("1 | %s\n", player.inv[0].item_name);
		printf("2 | %s\n", player.inv[1].item_name);
		printf("3 | %s\n", player.inv[2].item_name);
		printf("4 | %s\n", player.inv[3].item_name);
		printf("5 | %s\n", player.inv[4].item_name);
		
		printf("----------\n");
		
		printf("Oprema: \n");
		printf("A | %s\n", player.eq[0].item_name);
		printf("B | %s\n", player.eq[1].item_name);
		printf("C | %s\n\n", player.eq[2].item_name);
		
		render_divide();
		break;
			
		case(3): //def
		printf("\n\n");
		printf("1 | Izbegavanje\t\t[+30 DEX, -3 DEF | 1 POTEZ]\n");
		printf("2 | Odbrambeni stav\t[+2 DEF, -1 ATK | 2 POTEZA]\n");
		printf("3 | Prva pomoc\t\t[+(2 - 4)HP, -3 SPD | 2 POTEZA COOLDOWN]\n");
		printf("\n");
		
		render_divide();
		break;
	}
	
	printf("\n\n");
	//event bar vvv
	printf("%s \n", combat_action_1);
	printf("%s \n\n", combat_action_2);
	
	render_divide();
}

void setup_event(int ID){
	int i;
	
	FILE *event;
	char IDstring[100];
	
	sprintf(IDstring, "%i", ID);
	
	char temporary[200];
	char filearea[25] = "events/";
	
	strcat(filearea, IDstring);
	strcpy(IDstring, filearea);	
	strcat(IDstring, ".txt");
	
	event = (fopen(IDstring, "r"));
	
	for(i=0;i<7;i++){
		fgets(temporary, 200, event);
		strtok(temporary, "\n"); //delete the trailing newline
		
		switch(i){
			case(0):
				strcpy(ongoing_event.event_name, temporary);
			break;
			
			case(1):
				strcpy(ongoing_event.event_desc1, temporary);
			break;
			
			case(2):
				strcpy(ongoing_event.event_desc2, temporary);
			break;
			
			case(3):
				ongoing_event.num_of_choices = atoi(temporary);
			break;
			
			case(4):
				strcpy(ongoing_event.event_choice1, temporary);
			break;
			
			case(5):
				if(ongoing_event.num_of_choices >= 2){
					strcpy(ongoing_event.event_choice2, temporary);
				}
			break;
			
			case(6):
				if(ongoing_event.num_of_choices >= 3){
					strcpy(ongoing_event.event_choice3, temporary);
				}
			break;
		}
	}
}

void event_trap(){
	int hasperk = 0;
	dice = (rand() % (4 - 1 + 1)) + 1; //d4
	
	hasperk = check_for_perk(9); //REFLEX
	if(hasperk == 1){
			goto special_trap_dodge;
	} else{
		hasperk = check_for_perk(12); //EAGLE EYE
		if(hasperk == 1){
			goto special_trap_seen;	
		}
	}
	strcpy(map[player.location_y][player.location_x].flavor_text,"Ovde se nalazila zamka koju ste nazalost aktivirali.\nPored toga, nema niceg posebnog u ovom hodniku.");
	if(dice > 2){
		dice = (rand() % (3 - 1 + 1)) + 1; //d3
		damage_player(dice);
		printf("Zamka je aktivirana, strela leti i pogadja vas! [-%i HP]\n", dice);
	} else{
		printf("Zamka je aktivirana, strela leti i promasuje vas.\n");
	}
	
	return;
	
	special_trap_dodge:
	printf("[Brzi refleksi] - Sa lakocom izbegavate zamku.\n");
	return;
	
	special_trap_seen:
	printf("[Oko sokolovo] - Uspomoc dobrog vida primecujete mehanizam za zamku, uspesno ste je deaktivirali.\n");
	return;
}

void library_loot_shelves(){
	int hasperk = 0;
	dice = (rand() % (3 - 1 + 1)) + 1; //d3
	
	hasperk = check_for_perk(7); //ARCANE KNOWLEDGE
	if(hasperk == 1){
			goto special_library_loot;
	}
	
	switch(dice){
		case(1):
			printf("Jedna od polica sadrzi cudan svitak, uzimate ga. [+MISTERIOZNI SVITAK]\n");
			give_item(68);
		break;
		
		case(2):
			printf("Kultni simboli prekrivaju vecinu stranica jedne knjige, posle citanja vas boli glava... [-3 HP]\n");
			damage_player(3);
		break;
		
		case(3):
			printf("Knjige su stare i nisu toliko interesantne, vecina njih je u groznom stanju pa se ne mogu citati.\n");
		break;
	}
	
	return;
	
	special_library_loot:
	dice = (rand() % (4 - 1 + 1)) + 1; //d2
	
	if(dice == 1){
		printf("[Poznavanje magije] - Znate sta se verovatno krije iza raznih knjiga, uz malo truda ste to i nasli! [+UKLETI SVITAK]\n");
		give_item(69);
	} else{
		printf("[Poznavanje magije] - Bez problema nalazite svitak i izvlacite ga iz police. [+MISTERIOZNI SVITAK]\n");
		give_item(68);
	}
	
	return;
}

void event_library(int choice){
	switch(choice){
		case(1):
			library_loot_shelves();
		break;
		
		case(2):
			strcpy(map[player.location_y][player.location_x].flavor_text,"Nazalost je vecina knjiga upropasceno ili pretvoreno u neke kultne spise, ali ipak ima neceg sto bi vas zaintrigiralo.");
			dice = (rand() % (10 - 1 + 1)) + 1; //d10
			if(dice == 1){
				printf("Knjiga je o raznim borbama i bojnim tehnikama, naucili ste nesto novo! [+1 ATK]\n");
				bstats.atk = bstats.atk + 1;
			} else
			if(dice == 2){
				printf("Knjiga je puna raznih prica o ratovima i legendarnim borcima. [+5 XP]\n");
				player.xp = player.xp + 5;
			}else
			printf("Dosta strana u knjizi su pocepane ili prezvrljane, nista ovde ne moze da se protumaci.\n");
		break;
		
		case(3):
			printf("Nikada niste voleli da citate, zasto biste odjednom poceli?\n");
			strcpy(map[player.location_y][player.location_x].flavor_text,"Ova prostorija je nekada bila biblioteka, nazalost je skroz napustena i unistena.\nIpak, nalaze se neke knjige i svitci ovde koje jos mogu da se procitaju.");
		break;
	}
}

void open_event_chest(){
	int hasperk = 0;
	hasperk = check_for_perk(12); //EAGLE EYE
	
	dice = (rand() % (4 - 1 + 1)) + 1; //d4
	if(dice <= 3){
		int pool_length;
		
		if(player.floor > 3){
					pool_length = sizeof(event_pool_f3) / sizeof(event_pool_f3[0]);
						pool_length--;
						dice = (rand() % (pool_length - 0 + 1)) + 0; //random from 0 - pool_length
						setup_loot(event_pool_f3[dice], 0);
						printf("Ranac sadrzi blago! [+%s]\n", loot[0].item_name);
						give_item(loot[0].item_ID);	
				}
				
		switch(player.floor){
			case(1): //1 or 2
			case(2):
				pool_length = sizeof(event_pool_f1f2) / sizeof(event_pool_f1f2[0]);
				pool_length--;
				dice = (rand() % (pool_length - 0 + 1)) + 0; //random from 0 - pool_length
				setup_loot(event_pool_f1f2[dice], 0);
				printf("Kovceg sadrzi blago! [+%s]\n", strupr(loot[0].item_name));
				give_item(loot[0].item_ID);		
			break;
				
			case(3):
				pool_length = sizeof(event_pool_f3) / sizeof(event_pool_f3[0]);
				pool_length--;
				dice = (rand() % (pool_length - 0 + 1)) + 0; //random from 0 - pool_length
				setup_loot(event_pool_f3[dice], 0);
				printf("Kovceg sadrzi blago! [+%s]\n", strupr(loot[0].item_name));
				give_item(loot[0].item_ID);	
			break;
		}
	} else{
		if(hasperk == 1){
			printf("[Oko sokolovo] - Uspeli ste da primetite mehanizam zamke koji se nalazi blizu kljucaonice, mozda je bolje da se ovo ne otvara.\n");
		} else{
			printf("Nista se ne nalazi u kovcegu sem bola i kajanja, ovo je bila zamka! [-3 HP]\n");
			damage_player(3);
		}
	}
}

void event_chest(int choice){
	switch(choice){
		case(1):
			open_event_chest();
			strcpy(map[player.location_y][player.location_x].flavor_text,"U hodniku je otvoren kovceg a pored njega baklje.\nPored toga nema niceg posebnog... Sve sobe ionako izgledaju isto.");
		break;
		
		case(2):
			printf("Najbolje je biti pazljiv, verovatno je to bila neka vrsta zamke.\n");
			strcpy(map[player.location_y][player.location_x].flavor_text,"U hodniku je kovceg a pored njega baklje.\nNema niceg posebnog ovde sem toga, ali se ipak pitate sta je u kovcegu.");
		break;
	}
}

void event_dead_guy(int choice){
	switch(choice){
		case(1):
			strcpy(map[player.location_y][player.location_x].flavor_text,"Lose se osecate zbog onoga sto se ovde desilo, ali morate da radite sta god da biste preziveli.");
			printf("Pre nego sto mozete bilo sta da uzmete, ratnik polako ustaje...\n");
			
			map[player.location_y][player.location_x].room_type = 3;
			map[player.location_y][player.location_x].is_cleared = 0;
			map[player.location_y][player.location_x].enemy_ID = 1;
		break;
		
		case(2):
			printf("Nije u redu da se tako nesto uradi, situacija je mozda losa ali nije toliko losa.\n");
			strcpy(map[player.location_y][player.location_x].flavor_text,"Sa vama se u sobi nalazi bezivotno telo nekog nesrecnog ratnika.\nNe osecate se bezbedno ovde, bolje bi bilo da idete negde.");
		break;
	}
}

void event_mushrooms(int choice){
	strcpy(map[player.location_y][player.location_x].flavor_text,"Cak i u pustim prostorijama ovog tornja boze biti zivota.\nPecurke izbijaju iz kamenog poda i rastu visoko.");
	int hasperk = 0;
	dice = (rand() % (4 - 1 + 1)) + 1; //d4
	
	switch(choice){
		case(1):
			hasperk = check_for_perk(14); //PHARMACOLOGY
			if(hasperk == 1){
				printf("[Farmakologija] - Bez problema skupljate pecurke i preradjujete ih u nesto korisno. [+LEKOVITO BILJE]\n");
				give_item(67);
			} else{
				if(dice == 1){
					printf("Jedva ste skupili dovoljno pecuraka, uz malo truda to postaje korisna lekovita smesa. [+LEKOVITO BILJE]\n");
					give_item(67);
				}else{
					printf("Pecurke se raspadaju kada ih skupite, niste dovoljno pazljivi bili.\n");
				}
			}
		break;
		
		case(2):
			if(dice<=2){
				printf("Pecurke su bile lekovite! Osecate se malo bolje. [+2 HP]\n");
				heal_player(2);
			} else{
				hasperk = check_for_perk(10); //MEDICAL KNOWLEDGE
				if(hasperk == 1){
					printf("Pecurke ste prepoznali kao otrovnu vrstu, ne biste trebali ovo da jedete.\n");
				}else{
					printf("Pojedene pecurke su ostale sveukupno par minuta u vasem telu pre nego sto ste ih ispovracali. [-2 HP]\n");
					damage_player(2);
				}
			}
		break;
		
		case(3):
			printf("Nije dobra ideja da se nepoznate stvari jedu, mozda ste trenutno sebi spasili zivot.\n");
		break;
	}
}

void event_backpack(int choice){
	dice = (rand() % (2 - 1 + 1)) + 1; //d2
	int pool_length;
	
	switch(choice){
		case(1):
			if(dice == 1){
				
				if(player.floor > 3){
					pool_length = sizeof(event_pool_f3) / sizeof(event_pool_f3[0]);
						pool_length--;
						dice = (rand() % (pool_length - 0 + 1)) + 0; //random from 0 - pool_length
						setup_loot(event_pool_f3[dice], 0);
						printf("Ranac sadrzi blago! [+%s]\n", loot[0].item_name);
						give_item(loot[0].item_ID);	
				}
				
				strcpy(map[player.location_y][player.location_x].flavor_text,"Jedina stvar od interesa ovde je bio zaboravljen ranac.\nMada, nije ni to interesantno kada znate sta se u njemu nalazilo.");
				switch(player.floor){
					case(1): //1 or 2
					case(2):
						pool_length = sizeof(event_pool_f1f2) / sizeof(event_pool_f1f2[0]);
						pool_length--;
						dice = (rand() % (pool_length - 0 + 1)) + 0; //random from 0 - pool_length
						setup_loot(event_pool_f1f2[dice], 0);
						printf("Ranac sadrzi blago! [+%s]\n", loot[0].item_name);
						give_item(loot[0].item_ID);		
					break;
						
					case(3):
						pool_length = sizeof(event_pool_f3) / sizeof(event_pool_f3[0]);
						pool_length--;
						dice = (rand() % (pool_length - 0 + 1)) + 0; //random from 0 - pool_length
						setup_loot(event_pool_f3[dice], 0);
						printf("Ranac sadrzi blago! [+%s]\n", loot[0].item_name);
						give_item(loot[0].item_ID);	
					break;
				}		
			}else{
				printf("Ranac je sasvim prazan, izgleda kao da je neko bio ovde pre vas.\n");
				strcpy(map[player.location_y][player.location_x].flavor_text,"Jedina stvar od interesa ovde je bio zaboravljen ranac.\nMada, nije ni to interesantno kada znate da je sasvim prazan.");
			}	
		break;
		
		case(2):
			printf("Cak iako je tu ostavljeno, ne biste trebali da gledate po tudjim stvarima!\n");
			strcpy(map[player.location_y][player.location_x].flavor_text,"Jedina stvar od interesa ovde je bio zaboravljen ranac.\nPitate se ko ga je tu ostavio i zasto.");
		break;
	}
}

void event_excalibur(int choice){
	dice = (rand() % (4 - 1 + 1)) + 1; //d4
	
	switch(choice){
		case(1):
			if(dice == 1){
				printf("Posle par pokusaja zakljucujete da je mac vrlo crvsto uboden u kamen, nema sanse da ga izvucete. \n");
				strcpy(map[player.location_y][player.location_x].flavor_text,"Svetlo dopire kroz rupu u plafonu, osvetljava veliki kamen u kome se nalazi velicanstven mac.\nNa kamenu se nalazi natpis koji se ne moze procitati.");
			}else{
				printf("Sa lakocom vadite mac iz kamena, ali je mac na izlasku polomljen. [+SLOMLJEN MAC] \n");
				strcpy(map[player.location_y][player.location_x].flavor_text,"Svetlo dopire kroz rupu u plafonu, osvetljava veliki kamen u kome se jednom nalazio slomljen mac.\nNa kamenu se nalazi natpis koji se ne moze procitati.");
				give_item(10);
			}
		break;
		
		case(2):
			printf("Ne treba vam novo oruzje, ono koje imate je vec dovoljno. \n");
			strcpy(map[player.location_y][player.location_x].flavor_text,"Svetlo dopire kroz rupu u plafonu, osvetljava veliki kamen u kome se nalazi velicanstven mac.\nNa kamenu se nalazi natpis koji se ne moze procitati.");
		break;
	}
}

void cabinet_give_potions(){
	int potion_id;
	dice = (rand() % (3 - 1 + 1)) + 1; //d3
	
	potion_id = dice + 63; //63 + 1 or 2 or 3
	
	setup_loot(potion_id, 0);
	printf("[+%s]\n", strupr(loot[0].item_name));
	give_item(loot[0].item_ID);		
}

void event_potion_cabinet(int choice){
	dice = (rand() % (4 - 1 + 1)) + 1; //d4
	
	switch(choice){
		case(1):
			strcpy(map[player.location_y][player.location_x].flavor_text,"Vitrina sa napicima je i dalje tu ali nemate zelje da vise pokusavate da je otvarate.");
			if(dice<=2){
				printf("Uspesno ste otvorili vitrinu, uzimate jedan od napitka i nastavljate dalje. ");
				cabinet_give_potions();
			}else{
				printf("Kljucaonica sadrzi mehanizam protiv ovoga! Korozivna tecnost lije iz vitrine i pada na vasu kozu. [-4 HP]\n");
				damage_player(4);
			}
		break;
		
		case(2):
			strcpy(map[player.location_y][player.location_x].flavor_text,"Slomljena vitrina se nalazi na podu prostorije.\nPitate se zasto ste to uradili ali nema razloga da se vise brinete oko toga.");
			if(dice == 1){
				printf("Slomili ste vitrinu, vecina napitka je palo na pod ali u njoj. ");
				cabinet_give_potions();
			} else{
				printf("Slomili ste vitrinu a sa njom i napitke koji su se tu nalazili. Sta ste ocekivali? \n");
			}
		break;
		
		case(3):
			strcpy(map[player.location_y][player.location_x].flavor_text,"Vitrina sa napicima se nalazi u prostoriji, nemate zelje da je otvarate niti da degustirate ono sto se u njoj nalazi.");
			printf("Cak iako pise sta je sta, ne bi trebalo da se rizikuje oko ovakvih stvari.\n");
		break;
	}
}

void event_tired(){
	strcpy(map[player.location_y][player.location_x].flavor_text,"Obican hodnik pun paucine i prljavstine. Baklje na zidovima osvetljavaju prostoriju. \nNema niceg od znacaja ovde, bila bi dobra ideja da ovde ne ostajete dugo.");
	int hasperk = 0;
	hasperk = check_for_perk(19); //REGENERATION
	
	if(hasperk == 1){
		strcpy(map[player.location_y][player.location_x].flavor_text,"Cak iako je hladna i mracna, ova prostorija nije toliko losa za odmor.");
		printf("Lepo ste se odmorili, cak iako ne toliko dugo. [+1 HP]\n");
		heal_player(1);
	}else{
		strcpy(map[player.location_y][player.location_x].flavor_text,"Ova prostorija sigurno nije dobra za odmor, a to ste i iskusili.");
		printf("Odmorili ste se malo, ali se ne osecate toliko bolje. [-1 HP]\n");
		damage_player(1);
	}
}

void event_barrels(int choice){
	dice = (rand() % (10 - 1 + 1)) + 1; //d10
	
	switch(choice){
		case(1):
			if(dice==1){
				printf("Slucajno ste izazvali reakciju kod baruta, barut se pali i burad eksplodira! [-5 HP]\n");
				strcpy(map[player.location_y][player.location_x].flavor_text,"U uglu sobe je ogromna rupa oko koje se nalazi dosta komadica drveta koja su nastala od eksplozije.");
				damage_player(5);
			}else{
				strcpy(map[player.location_y][player.location_x].flavor_text,"Soba sadrzi raznu burad koja u sebi imaju barut.\nIskoristili ste sta ste mogli i napravili eksploziv.");
				printf("Uspeli ste da napravite neku varijantu bombice, posluzice vam sigurno. [+EKSPLOZIV]\n");
				give_item(73);
			}
		break;
		
		case(2):
			strcpy(map[player.location_y][player.location_x].flavor_text,"Soba sadrzi raznu burad koja u sebi imaju barut.\nNista niste dirali zbog straha oko slucajne eksplozije.");
			printf("Rizik je previlik za ovako nesto, ionako vam verovatno ne bi posluzilo ovo.\n");
		break;
	}
}

void prisoner_breakout(){
	int pool_length;
	dice = (rand() % (4 - 1 + 1)) + 1; //d4
	if(dice<=2){
		
		if(player.floor > 3){
					pool_length = sizeof(event_pool_f3) / sizeof(event_pool_f3[0]);
						pool_length--;
						dice = (rand() % (pool_length - 0 + 1)) + 0; //random from 0 - pool_length
						setup_loot(event_pool_f3[dice], 0);
						printf("Ranac sadrzi blago! [+%s]\n", loot[0].item_name);
						give_item(loot[0].item_ID);	
		}
		
		strcpy(map[player.location_y][player.location_x].flavor_text,"U sobi je prazna zatvorenicka celija, osecate se dobro sto ste oslobodili onog ranjenog coveka.");
		switch(player.floor){
			case(1): //1 or 2
			case(2):
				pool_length = sizeof(event_pool_f1f2) / sizeof(event_pool_f1f2[0]);
				pool_length--;
				
				dice = (rand() % (pool_length - 0 + 1)) + 0; //random from 0 - pool_length
				setup_loot(event_pool_f1f2[dice], 0);
				printf("Uspesno ste slomili katanac i izbavili ga, u znak zahvalnosti vam daje neku njegovu opremu. [+%s]\n", loot[0].item_name);
				give_item(loot[0].item_ID);		
			break;
						
			case(3):
				pool_length = sizeof(event_pool_f3) / (event_pool_f3[0]);
				pool_length--;
				
				dice = (rand() % (pool_length - 0 + 1)) + 0; //random from 0 - pool_length
				setup_loot(event_pool_f3[dice], 0);
				printf("Uspesno ste slomili katanac i izbavili ga, u znak zahvalnosti vam daje neku njegovu opremu. [+%s]\n", loot[0].item_name);
				give_item(loot[0].item_ID);	
			break;
		}
	}else{
		if(dice == 3){
			strcpy(map[player.location_y][player.location_x].flavor_text,"U sobi je celija sa ranjenim zatvorenikom, probali ste sve sto mozete ali niste uspeli da ga izbavite.");
			printf("Katanac je cvrsci nego sto ste mislili. Niste nista uspeli da postignete.\n");
		}else{
			printf("Kada ponovo pogledate zatvorenika vidite da se promenio... Sada znate zasto je bio zatvoren ovde.\n");
			printf("Pritisnite bilo sta da nastavite.\n");
			
			map[player.location_y][player.location_x].room_type = 3;
			map[player.location_y][player.location_x].is_cleared = 0;
			map[player.location_y][player.location_x].enemy_ID = 2;	
		}
	}
}

void event_prisoner(int choice){
	switch(choice){
		case(1):
			prisoner_breakout();
		break;
		
		case(2):
			printf("Mozda ne mozete da ga izbavite, ali mozete da mu pomognete. Previli ste neke njegove rane preko resetaka u nadi da ce mu to pomoci nekako. [+3 XP]\n");
			strcpy(map[player.location_y][player.location_x].flavor_text,"U sobi je celija sa ranjenim zatvorenikom, probali ste sve sto mozete ali niste uspeli da ga izbavite.");
			player.xp = player.xp + 3;
		break;
		
		case(3):
			strcpy(map[player.location_y][player.location_x].flavor_text,"U sobi je celija sa ranjenim zatvorenikom. Ponekad vam dovikuje nesto kada prodjete pored njega.");
			printf("Pozdravili ste se sa zatvorenikom i otisli, to ipak nije vas problem\n");
		break;
	}
}

void event_cult_ritual(int choice){
	strcpy(map[player.location_y][player.location_x].flavor_text,"U prostoriji je dosta sveca i pribora za ritualni obred.\nNe osecate se skroz sigurno ovde, pa bi bila dobrai deja da napustite ovo mesto.");
	dice = (rand() % (2 - 1 + 1)) + 1; //d2
	
	switch(choice){
		case(1):
			if(dice==1){
				printf("Lobanju ste uspesno uzeli, sta sad? [+LOBANJA]\n");
				give_item(56);
			}else{
				printf("Nije bila dobra ideja da se gaze simboli, osecate ogroman bol u telu i napustate sobu uplaseno. [-5 HP]\n");
				damage_player(5);
			}
		break;
		
		case(2):
			printf("Znate vi sve ovo. Citanjem ovih simbola na podu ste naucili razne nove stvari o onome sto se ovde desava. [+5 XP]\n");
			player.xp = player.xp + 5;
		break;
		
		case(3):
			printf("'Najbolje je ne mesati se u ovakve stvari', mislite dok polako izlazite iz sobe.\n");
		break;
	}
}

void event_safehouse(int choice){
	strcpy(map[player.location_y][player.location_x].flavor_text,"Ova prostorija sadrzi katance i slicne stvari u slucaju da morate da se zatvorite na kratko vreme.\nOsecate se prilicno bezbedno zbog toga.");
	dice = (rand() % (4 - 2 + 1)) + 2; //d4 bottom 2
	
	switch(choice){
		case(1):
			printf("Zakljucali ste sobu i opustili ste se malo. Budite se par sati kasnije osvezeno. [+%i HP]\n", dice);
			heal_player(dice);
		break;
		
		case(2):
			printf("Spavanje u ovakvom mestu nije dobra ideja, nastavljate dalje sa avanturom.\n");
		break;
	}
}

void event_wound(int choice){
	strcpy(map[player.location_y][player.location_x].flavor_text,"Obican hodnik pun paucine i prljavstine. Baklje na zidovima osvetljavaju prostoriju. \nNema niceg od znacaja ovde, bila bi dobra ideja da ovde ne ostajete dugo.");
	int hasperk = 0;
	dice = (rand() % (4 - 1 + 1)) + 1; //d4
	
	if(player.maxhp<15 && dice == 1){
		dice = 2;
	}
	
	hasperk = check_for_perk(10); //medicine knowledge
	if(hasperk == 1){
		goto special_medicine;
	}else{
		hasperk = check_for_perk(19); //regen
		if(hasperk == 1){
			goto special_regen;
		}
	} 
	
	printf("Nadate se da nije nista ozbiljno, ali ipak vas brine malo.\n");
	if(dice==1){
		bstats.maxhp = bstats.maxhp - 1;
	} else
		damage_player(2);
	return;
	
	special_medicine:
	printf("[POZNAVANJE MEDICINE] Brzo ste sterilisali i previli ranu, sigurno ce biti bolje. \n");
	return;
	
	special_regen:
	printf("[REGENERACIJA] Nije to nista, bice bolje.\n");
}

void event_paranoia(int choice){
	dice = (rand() % (4 - 1 + 1)) + 1; //d4
	
	switch(choice){
		case(1):
			if(dice==1){
				printf("Nesto jeste sa vama ovde, i ne deluje vam kao da je novi prijatelj!\n");
				printf("Pritisnite bilo sta da nastavite.\n");
				
				map[player.location_y][player.location_x].room_type = 3;
				map[player.location_y][player.location_x].is_cleared = 0;
				if(player.floor == 1){
					dice = (rand() % (2 - 1 + 1)) + 1; //d2 TO GENERATE ENEMY
				} else
				dice = (rand() % (3 - 1 + 1)) + 1; //d3 TO GENERATE ENEMY
				
				map[player.location_y][player.location_x].enemy_ID = dice;	
			}else{
				printf("Izgleda da nista nije bilo sa vama u sobi.\n");
			}
		break;
		
		case(2):
			printf("Uspesno ste se sakrili. \n");
		break;
	}
}

void event_wall_scroll(int choice, int mult){
	strcpy(map[player.location_y][player.location_x].flavor_text,"Ovaj prilicno obican hodnik sadrzi veliki svitak na zidu koji sigurno krije neke tajne.");
	switch(choice){
		case(1):
			use_scroll(mult);	
		break;
		
		case(2):
			printf("Bolje je da se takve stvari ne citaju, ko zna sta tu pise?\n");
		break;
	}
}

void event_dying_warrior(int choice){
	dice = (rand() % (5 - 1 + 1)) + 1; //d5
	
	switch(choice){
		case(1):
			printf("Ne mozete bas da mu pomognete, pa pricate malo sa njim. [+%i XP]\n", dice);
			strcpy(map[player.location_y][player.location_x].flavor_text,"Pored vas se ovde nalazi ratnik koji je najverovatnije skoro poginuo.");
			player.xp = player.xp + dice;
		break;
		
		case(2):
			printf("Uspeli ste da mu pomognete donekle, zahvaljuje se i prica vam malo o onome sto se desilo [+5 XP].\n");
			strcpy(map[player.location_y][player.location_x].flavor_text,"Pored vas se ovde nalazi ratnik koji je vrlo ranjen.\nNadate se da ste mu spasili zivot.");
			player.xp = player.xp + 5;
		break;
		
		case(3):
			printf("Mozda je zamka ili nesto drugo, bolje je otici odavde sto pre.\n");
			strcpy(map[player.location_y][player.location_x].flavor_text,"Pored vas se ovde nalazi ratnik koji je najverovatnije skoro poginuo.");
		break;
	}
}

void event_old_armor(int choice){
	dice = (rand() % (5 - 1 + 1)) + 1; //d5
	strcpy(map[player.location_y][player.location_x].flavor_text,"U sredini prostorije se nalazi stari oklop.\nOko tog oklopa je tepih i ploca sa natpisom, oklop je stvarno prelep.");
	
	switch(choice){
		case(1):
			printf("Bez osecaja sramote skidate oklop i stavljate ga u ranac. [+STARI OKLOP]\n");
			strcpy(map[player.location_y][player.location_x].flavor_text,"U sredini prostorije se nalazio stari oklop.\nBez njega soba deluje malo prazno.");
			give_item(3);
		break;
		
		case(2):
			printf("Svaka ogrebotina na ovom oklopu sadrzi posebnu pricu. [+%i XP]\n", dice);
			player.xp = player.xp + dice;
		break;
		
		case(3):
			printf("Vrlo je lep oklop, ali ne biste da trosite vreme ovde.\n");
		break;
	}
}

void event_confusion(int choice){
	strcpy(map[player.location_y][player.location_x].flavor_text,"Obican hodnik pun paucine i prljavstine. Baklje na zidovima osvetljavaju prostoriju. \nNema niceg od znacaja ovde, bila bi dobra ideja da ovde ne ostajete dugo.");
	switch(choice){
		case(1):
			if(map[player.last_location_y][player.last_location_x].room_type != 1 && map[player.last_location_y][player.last_location_x].room_type != 4 && map[player.last_location_y][player.last_location_x].room_type != 5){
				printf("Ne mozete da se snadjete u ovom lavirintu. Raspored soba nema smisla! [-5 XP]\n");
				player.xp = player.xp - 5;
				if(player.xp < 0){
					player.xp = 0;
				}
			} else
			printf("U redu je, mozete da se snadjete u ovom mestu.\n");	
			break;
		
		case(2):
			if(map[player.last_location_y][player.last_location_x].room_type != 2){
				printf("Ne mozete da se snadjete u ovom lavirintu. Raspored soba nema smisla! [-5 XP]\n");
				player.xp = player.xp - 5;
				if(player.xp < 0){
					player.xp = 0;
				}
			} else
			printf("U redu je, mozete da se snadjete u ovom mestu.\n");	
		break;
		
		case(3):
			if(map[player.last_location_y][player.last_location_x].room_type != 3){
				printf("Ne mozete da se snadjete u ovom lavirintu. Raspored soba nema smisla! [-5 XP]\n");
				player.xp = player.xp - 5;
				if(player.xp < 0){
					player.xp = 0;
				}
			} else
			printf("U redu je, mozete da se snadjete u ovom mestu.\n");	
		break;
	}
}

void event_handler(){
	int eventchoice, hasperk;
	char eventchoicestring[5];
	
	scanchoice:
	scanf("%s", &eventchoicestring);
	eventchoice = atoi(eventchoicestring);
	
	if(eventchoice > ongoing_event.num_of_choices || eventchoice <= 0){
		printf("Molimo da ponovo unesete izbor. \n");
		goto scanchoice;
	}
	
	map[player.location_y][player.location_x].is_cleared = 1;

	switch(map[player.location_y][player.location_x].event_ID){
		case(1):
			event_trap();	
		break;
		
		case(2):
			event_library(eventchoice);
		break;
		
		case(3):
			event_chest(eventchoice);
		break;
		
		case(4):
			event_dead_guy(eventchoice);
		break;
		
		case(5):
			event_mushrooms(eventchoice);
		break;
		
		case(6):
			event_backpack(eventchoice);
		break;
		
		case(7):
			event_excalibur(eventchoice);
		break;
		
		case(8):
			event_potion_cabinet(eventchoice);	
		break;
		
		case(9):
			event_tired();
		break;
		
		case(10):
			event_barrels(eventchoice);
		break;
		
		case(11):
			hasperk = check_for_perk(10); //MEDICAL KNOWLEDGE
			if(eventchoice == 2 && hasperk!=1){
				printf("Nemate odgovarajuci talenat za ovo, pokusajte nesto drugo. \n");
				goto scanchoice;
			}else
			event_prisoner(eventchoice);
		break;
		
		case(12):
			hasperk = check_for_perk(7); //ARCANE KNOWLEDGE
			if(eventchoice == 2 && hasperk!=1){
				printf("Nemate odgovarajuci talenat za ovo, pokusajte nesto drugo. \n");
				goto scanchoice;
			}else
			event_cult_ritual(eventchoice);
		break;
		
		case(13):
			event_safehouse(eventchoice);
		break;
		
		case(14):
			event_wound(eventchoice);
		break;
		
		case(15):
			hasperk = check_for_perk(8); //CAMO
			if(eventchoice == 2 && hasperk!=1){
				printf("Nemate odgovarajuci talenat za ovo, pokusajte nesto drugo. \n");
				goto scanchoice;
			}else
			event_paranoia(eventchoice);
		break;
		
		case(16):
			event_wall_scroll(eventchoice, 1);
		break;
		
		case(17):
			event_wall_scroll(eventchoice, 2);
		break;
		
		case(18):
			hasperk = check_for_perk(10); //MEDICAL KNOWLEDGE
			if(eventchoice == 2 && hasperk!=1){
				printf("Nemate odgovarajuci talenat za ovo, pokusajte nesto drugo. \n");
				goto scanchoice;
			}else
			event_dying_warrior(eventchoice);
		break;
		
		case(19):
			event_old_armor(eventchoice);
		break;
		
		case(20):
			event_confusion(eventchoice);
		break;
	}
}

void print_event(){
	printf("%s\n", ongoing_event.event_desc1);
	printf("%s\n\n", ongoing_event.event_desc2);

	printf("1 - %s\n", ongoing_event.event_choice1);
	if(ongoing_event.num_of_choices >= 2){
		printf("2 - %s\n", ongoing_event.event_choice2);
		if(ongoing_event.num_of_choices == 3){
			printf("3 - %s\n", ongoing_event.event_choice3);
		}
	}
}

void render_event(char image[100]){
	setup_event(map[player.location_y][player.location_x].event_ID);
	
	render_divide();
	printf("\n		 EVENT: %s", ongoing_event.event_name);
	
	printf(" | HP:[%i/%i]", player.hp, player.maxhp);
	printf(" | XP:[%i/%i]", player.xp, player.reqxp);
	printf(" | MAPA");
	printf(" | SPOSOBNOSTI");
	if(player.xp == player.reqxp){
		printf("!");
	}
	printf(" | TALENTI");
	printf(" | INVENTAR");
	
	printf("\n");
	
	render_divide();
	printf("\n\n");
	
	render_image(image); //render scene
	printf("\n\n");
	render_divide();
	printf("\n\n");
	
	print_event();
	printf("\n");
	render_divide();
	printf("\n\n");
	event_handler();
	
	if(map[player.location_y][player.location_x].room_type == 2 && player.state == 3){
		player.state = 1;
		map[player.location_y][player.location_x].is_cleared = 1;
	
		printf("\n\n");
		printf("Unesite bilo sta da nastavite.\n");
	}	
}

void render(char image[100]){
	switch(player.state){
		case(1): //explore
			render_calm(image);
			break;
			
		case(2): //fight
			render_battle(image);
			break;
			
		case(3): //event
			render_event(image);
			break;
	}
}

void fight_intro(){
	int i;
	system("cls");
	render_divide();printf("\n");
	
	for(i=0;i<5;i++){
		printf("\n");	
	}
	
	dice = (rand() % (6 - 1 + 1)) + 1; //d6
	switch(dice){
		case(1):
			printf("Soba je delovala prazno kada ste usli, ali ubrzo cujete neki zvuk i okrecete se...\n");
		break;
		
		case(2):
			printf("Prostoriju je pre vas okupiralo neko cudoviste, i ne deluje vam kao da ce tek tako izaci.\n");
		break;
		
		case(3):
			printf("Od kako ste usli u prostoriju videli ste nesto u uglu oka, izgleda da je nesto ovde sa vama!\n");
		break;
		
		case(4):
			printf("Okrecete se iza vas zbog iznenadnog zvuka, i ubrzo saznajete sta ga je proizvelo. \n");
		break;
		
		case(5):
			printf("Osecali ste se kao da vas nesto gleda, i vrlo uskoro primecujete zasto.\n");
		break;
		
		case(6):
			printf("Cim udjete u sobu vas nesto napadne! Izbegli ste napad, ali nema mnogo vremena da se porazmisli o ovome. \n");
		break;
	}
	
	printf("\n! - Pretisnite bilo sta da nastavite.\n");
	
	for(i=0;i<5;i++){
		printf("\n");	
	}
	
	render_divide();printf("\n");
	char bilo_sta;
	scanf(" %c", &bilo_sta);
	system("cls");
	
	for(i=0;i<46;i++){
		printf("\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1\xB1");
		Sleep(3);
		printf("\n");
	}
	Sleep(25);
	system("cls");
}

void room_check(char image_to_render[100]){ //TYPES: 0 = nonexistant, 1 = hallway, 2 = event, 3 = encounter, 4 = floor up, 5 = starting room
		int i,j;
		switch(map[player.location_y][player.location_x].room_type){
			case(1):
				strcpy(image_to_render,"backgrounds/dungeon_back.txt");
				break;
			
			case(2):
				if(map[player.location_y][player.location_x].is_cleared != 1){
					strcpy(image_to_render,"backgrounds/event_back.txt");
					player.state = 3;
				}
				break;
				
			case(3): //INITIATE FIGHT
				if(map[player.location_y][player.location_x].is_cleared != 1){
						if(player.state != 2){
							fight_intro();
							setup_enemy(map[player.location_y][player.location_x].enemy_ID);
							player.state = 2;
							turn = 1;
					}
					strcpy(image_to_render, enemy.fight_bg);
				} else
				strcpy(image_to_render,"backgrounds/dungeon_back.txt");
				break;
				
			case(4):
				strcpy(image_to_render,"backgrounds/stairs_back.txt");
				break;
				
			case(5):
				strcpy(image_to_render,"backgrounds/dungeon_back.txt");
				break;
	}
}

void move_rooms(char action[100]){
	
	player.last_location_x = player.location_x;
	player.last_location_y = player.location_y;
	map[player.location_y][player.location_x].is_cleared = 1; //clear room he's going from
	
	if (strcmp(action, "gore") == 0){ //go up
     	if (map[player.location_y-1][player.location_x].room_type == 0 || player.location_y-1 < 0 ){
			printf("Ne mogu tamo da idem. \n");
			sleep(2);
		} else{
			player.location_y--;
		}
	}else 
	
	if(strcmp(action, "dole") == 0){ //go down
		if (map[player.location_y+1][player.location_x].room_type == 0 || player.location_y+1 > 6 ){
			printf("Ne mogu tamo da idem. \n");
			sleep(2);
		} else{
			player.location_y++;
		}	
	}else
	
	if(strcmp(action, "levo") == 0){ //left side take it back this time
		if (map[player.location_y][player.location_x-1].room_type == 0 || player.location_x-1 < 0 ){
			printf("Ne mogu tamo da idem. \n");
			sleep(2);
		} else{
			player.location_x--;
		}	
	}else
	
	if(strcmp(action, "desno") == 0){
		if (map[player.location_y][player.location_x+1].room_type == 0 || player.location_x+1 > 6 ){
			printf("Ne mogu tamo da idem. \n");
			sleep(2);
		} else{
			player.location_x++;
		}
	}else
	printf("INVALID INPUT \n");
}

void swap_to_menu(char menu[100]){ //STATE 1: 0 = main, 1 = map, 2 = inventory, 3 = level up, 4 = status |FLIP ->| STATE 2: 0 = thinking, 1 = attacking, 2 = inventory, 3 = defending
	
	if(player.state == 1){ //if chillin
		if(strcmp(menu, "mapa") == 0){
			player.menu_state = 1;
		}else
		
		if(strcmp(menu, "inventar") == 0){
			player.menu_state = 2;
		}else
		
		if(strcmp(menu, "sposobnosti") == 0){
			player.menu_state = 3;
		}else
		
		if(strcmp(menu, "talenti") == 0){
			player.menu_state = 4;
		}else
		
		if(strcmp(menu, "nazad") == 0){
			player.menu_state = 0;
		}	
	} else
	
	if(player.state == 2){ //if fighting
	
		if(strcmp(menu, "napad") == 0){
			player.menu_state = 1;
		}else
		
		if(strcmp(menu, "odbrana") == 0){
			player.menu_state = 3;
		}else
		
		if(strcmp(menu, "inventar") == 0){
			player.menu_state = 2;
		}else
		
		if(strcmp(menu, "nazad") == 0){
			player.menu_state = 0;
		}
	}
}

void run_from_combat(){
	int i, hasperk;

	hasperk = check_for_perk(8); //camoflauge
	if(hasperk == 1 && turn == 1){
			goto run_away;
	}
	
	dice = (rand() % (100 - 0 + 1)) + 0;	
	
	for(i=0;i<10;i++){
		if(player.buffs[i].status_ID == 7){
			printf("\nTo sam vec probao, nema sanse za sada.\n");
			sleep(3);
			return;
		} else
		if(player.buffs[i].status_ID == 9){
			dice = 0;
		}
	}
	
	if((dice - 20 - (player.luck*2)) < ((player.spd - enemy.spd))*10){ //base chance is 20 + luckx2, check if its smaller than speed diffx10
		run_away:

		for(i=0;i<10;i++){ //remove status effects
			remove_buff(i);
		}
		
		printf("\nPobegao sam! \n");
		player.state = 1;
		player.location_x = player.last_location_x;
		player.location_y = player.last_location_y;
	} else{
		printf("\nNisam uspeo da pobegnem.\n");
		give_buff(7, 2);
	}
	
	sleep(3);
	player.menu_state = 0;
}

void floor_up(){
	
	int hasperk;
	hasperk = check_for_perk(14); //camoflauge
	if(hasperk == 1){
		give_item(64);
	}
	
	clear_map();
	player.floor = player.floor + 1;
	player.location_x = 3;
	player.location_y = 3;
	generate_map();
	player.menu_state = 0;
}

void check_cheats(char action[100]){
	if(strcmp(action, "cheat_giveitem") == 0){ //cheat
			int id_giveitem;
			scanf("%i", &id_giveitem);

			give_item(id_giveitem);
		}
		
		if(strcmp(action, "cheat_givestatus") == 0){ //cheat
			int id_givestatus;
			scanf("%i", &id_givestatus);

			give_buff(id_givestatus, 2);
			}
			
		if(strcmp(action, "cheat_removestatus") == 0){ //cheat 
			int slot;
			scanf("%i", &slot);

			remove_buff(slot);
		}
		
		if(strcmp(action, "cheat_heal") == 0){ //cheat list statuses
			player.hp = player.maxhp;
			printf("DONE");
			sleep(1);
		}
		
		if(strcmp(action, "cheat_givexp") == 0){ //cheat give xp
			int amount;
			scanf("%i", &amount);
			player.xp = player.xp + amount;
			printf("DONE");
			sleep(1);
		}
		
		if(strcmp(action, "cheat_giveperk") == 0){ //cheat give xp
			int perkid;
			scanf("%i", &perkid);
			player.perks[0].perk_ID = perkid;
			setup_perks(0, player.perks);
			printf("DONE");
			sleep(1);
		}
		
		if(strcmp(action, "cheat_skiplevel") == 0){ //cheat go up a floor
			int destination,i;
			scanf("%i", &destination);
			for(i=0;i<destination;i++){
				floor_up();
			}
		}
		
		if(strcmp(action, "cheat_killenemy") == 0){ //cheat KILL
			enemy.hp = 0;
		}
		
		if(strcmp(action, "cheat_victory") == 0){ //cheat win screen
			combat_victory();
		}
}

void action_check(char action[100]){
	if (player.state == 1){ //CALM commands
		if(strcmp(action, "idigore") == 0 && map[player.location_y][player.location_x].room_type == 4){
			floor_up();
		}
		
		if(strcmp(action, "mapa") == 0 || strcmp(action, "inventar") == 0 || strcmp(action, "sposobnosti") == 0 || strcmp(action, "talenti") == 0 || strcmp(action, "nazad") == 0){
			swap_to_menu(action);
		}
		
		if(player.menu_state == 1){ //map commands
			if(strcmp(action, "gore") == 0 || strcmp(action, "dole") == 0 || strcmp(action, "levo") == 0 || strcmp(action, "desno") == 0){
				move_rooms(action);
				if(map[player.location_y][player.location_x].is_cleared == 0){
					player.menu_state = 0;
				}
			}
		}
		
		if(player.menu_state == 2){ //inventory commands
			if(strcmp(action, "1") == 0 || strcmp(action, "2") == 0 || strcmp(action, "3") == 0 || strcmp(action, "4") == 0 || strcmp(action, "5") == 0 || strcmp(action, "6") == 0 || strcmp(action, "a") == 0 || strcmp(action, "b") == 0 || strcmp(action, "c") == 0 ){
				int actionint;
				actionint = atoi(action);
				actionint--; //korisnik ubacuje +1 (retarded)
				
				if(player.inv[actionint].item_ID != 0){ //check if its actually an item
					if (player.inv[actionint].is_taken == 0){
						return;
					}else
					if (player.inv[actionint].is_equipment == 1){
						equip_item(actionint);
					}else{
						use_item(player.inv[actionint].item_ID, actionint);
					}
				}
			} else
			if(strcmp(action, "oprema") == 0){
				player.menu_state = 6;
			}
		}
		
	} else
	if (player.state == 2){ //BATTLE commands
		if(strcmp(action, "napad") == 0 || strcmp(action, "odbrana") == 0 || strcmp(action, "inventar") == 0 || strcmp(action, "nazad") == 0){
			swap_to_menu(action);
		}
		
		if(strcmp(action, "bezi") == 0){
			run_from_combat();	
		}
		
		if(player.menu_state == 1 || player.menu_state == 2 || player.menu_state == 3){ //if in combat
			if(strcmp(action, "1") == 0 || strcmp(action, "2") == 0 || strcmp(action, "3") == 0 || strcmp(action, "4") == 0 || strcmp(action, "5") == 0 || strcmp(action, "6") == 0){
				
				if(player.menu_state == 3 && strcmp(action, "3") == 0){ //if he's tryina heal,
					int i;
					for(i=0;i<10;i++){
						if(player.buffs[i].status_ID == 3){
							printf("\n! - Ne mogu ovo ponovo da uradim.\n");
							sleep(2);
							return;
						}
					}
				} //pull up in the hulk machine
				
				decide_turn_priority();
				int actionint;
				actionint = atoi(action);
				
				if(player.menu_state == 2){
					actionint--;
				}
				
				if(turn_priority==0){
					player_battle_action(actionint);
					enemy_action();
				} else{
					enemy_action();
					player_battle_action(actionint);
				}
					
			}
		}
	}
}

void intro_story(){
	render_image("backgrounds/border.txt");
	printf("\n\n");
	render_image("backgrounds/story_screen.txt");
	printf("\n\n");
	render_image("backgrounds/border.txt");
	printf("\n\n");
	printf("Godina je 11XX, nikada necete da zaboravite dan kada ste bili izbaceni iz vaseg viteskog reda.\n");
	printf("U pokusaju da se iskupite, odlucili ste da napadnete ukleti toranj koji se ne nalazi daleko od vaseg sela.\n");
	printf("Prica se siri da oni koji tamo idu se nikada ne vrate...\n\n");
	printf("Obijate vrata tornja i polako krocite unutra. (Pritisnite bilo sta da nastavite)\n");
	
	char trash;
	scanf(" %s", &trash);
	system("cls");
}

void start_sequence(){
	char top_player_name[100], temp[100];
	int i, top_player_floor;
	
	FILE *highscore = fopen("high_score.txt", "r");
	for(i=0;i<2;i++){
		fgets(temp, 100, highscore);
		strtok(temp, "\n"); //delete the trailing newline
		
		switch(i){
			case(0):
				strcpy(top_player_name, temp);
			break;
			
			case(1):
				top_player_floor = atoi(temp);
			break;
		}
	}
	
	fclose(highscore);
	
	printf("Copyright IT odeljenje Gimnazija u Cupriji 2/4 2020-2021.\n");
	printf("---------------------------------------------------------\n");
	sleep(2);
	printf("Luka Mihajlovic i mentor Dalibor Rajkovic predstavljaju\n");
	sleep(1);
	printf(".\n");
	sleep(1);
	printf(".\n");
	sleep(1);
	printf(".\n");
	sleep(1);
	
	system("cls");
	for(i=0;i<46;i++){
		printf("\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2");
		printf("\n");
	}
	
	Sleep(500);
	
	system("cls");
	for(i=0;i<46;i++){
		printf("\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0");
		printf("\n");
	}
	Sleep(250);
	
	system("cls");
	
	render_image("backgrounds/border.txt");
	printf("\n\n");
	render_image("backgrounds/title_screen.txt");
	printf("\n\n");
	render_image("backgrounds/border.txt");
	printf("\n\n");
	printf("\t\t\t\tLegendarni ratnik na ovom kompjuteru: %s [%i sprat]\n", top_player_name, top_player_floor);
	printf("\t\t\t\tPritisnite bilo sta da zapocnete igru.\n", top_player_name, top_player_floor);
	
	char trash;
	scanf(" %s", &trash);
	
	system("cls");
	intro_story();
}

void player_death(){
	char top_player_name[100], temp[100];
	int i, top_player_floor;
	
	FILE *highscore = fopen("high_score.txt", "r");
	for(i=0;i<2;i++){
		fgets(temp, 100, highscore);
		strtok(temp, "\n"); //delete the trailing newline
		
		switch(i){
			case(0):
				strcpy(top_player_name, temp);
			break;
			
			case(1):
				top_player_floor = atoi(temp);
			break;
		}
	}
	
	fclose(highscore);
	for(i=0;i<46;i++){
		printf("\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2");
		printf("\n");
	}
	sleep(2);
	system("cls");
	for(i=0;i<46;i++){
		printf("\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0");
		printf("\n");
	}
	sleep(2);
	system("cls");
	
	render_image("backgrounds/border.txt");
	printf("\n\n");
	render_image("backgrounds/mortis.txt");
	printf("\n\n");
	render_image("backgrounds/border.txt");
	printf("\n\n");
	
	if(player.floor > top_player_floor){
		printf("Upamceni ste kao legendarni ratnik! Kako ste se bese zvali?\n");
		scanf("%s", &top_player_name);
		top_player_floor = player.floor;
		
		FILE *newscore = fopen("high_score.txt", "w");
		for(i=0;i<2;i++){	
		switch(i){
			case(0):
				fprintf(newscore, "%s", top_player_name);
			break;
			
			case(1):
				fprintf(newscore, "\n%i", top_player_floor);
			break;
		}	
	}
	
	printf("I tako se zavrsava epska prica %s...\n", top_player_name);
		printf("\n\nFin.");	
		
	}else{
		printf("Niste upamceni kao legendarni ratnik, vase ime je davno zaboravljeno.\n");
	}
}

void main() {
	srand(time(0)); //seed random
	
	start_sequence();
	text_color = (rand() % (15 - 1 + 1)) + 1; //get random text color
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), text_color | FOREGROUND_INTENSITY); //color the text :3
	
	char image_to_render[100]; int i,j,k;
	char action[100];
	
	player.character_class = 1; //TEMPORARY, REMOVE LATER !!!
	
	clear_map();
	setup_player();
	
	player.floor = 1;
	player.location_x = 3;
	player.location_y = 3;
	
	generate_map();
	
	while (1==1){ //game loop
		system("cls");
		
		if(player.hp <= 0){
			goto mortis;
		} else
		
		room_check(image_to_render); //set the image to render, also check out and possibly setup an encounter
		update_stats(); //calculate player's stats
		
		render(image_to_render); //main hub, render either calm or battle
		printf("\n\n\n");
		
		inventory_check(); //check for player inventory slots, set up empty ones
			
		scanf("%s", &action); //get action
		
		check_cheats(action); //check for debug cheat codes :o
		action_check(action); //main hub for doing stuff, check all actions and menus	
	}
	
	mortis:
	player_death();
}
