#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>


#define ANZAHL 32
#define MAXINDEX 255

typedef union {
    
    unsigned char vek_bytes[64];
    uint64_t chunks[8];
}DATA;

typedef union {
    uint32_t player;
    struct {
        
        uint8_t p;        // Byte 0: Kopf (x:4, y:4)
        uint8_t p_n;      // Byte 1: Aktuell (x:4, y:4)
        uint8_t a;        // Byte 2: Apfel (x:4, y:4)
        uint8_t length;   // Byte 3: Länge
    } bytes; // Zugriff über: 

    struct {
        
        uint32_t posx_kopf : 4; // Die unteren 4 Bits
        uint32_t posy_kopf : 4; // Die oberen 4 Bits
        uint32_t posx_now : 4; // Die unteren 4 Bits
        uint32_t posy_now : 4; // Die oberen 4 Bits
        uint32_t posx_apple : 4; // Die unteren 4 Bits
        uint32_t posy_apple : 4; // Die oberen 4 Bits
        uint32_t player_lenght :8;
    };
}PLAYER;

void push_data_2l(DATA* pdata, uint8_t data) {
    uint64_t carry = (uint64_t)(data & 0x03) << 62;// es gibt falsche loogik wnn ich (uin64_t nicht davor schreibe) 
    
    for (int i = 0; i < 8; i++) {
        // Sichere die untersten 2 Bits für das NÄCHSTE Byte
        uint64_t next_carry = (pdata->chunks[i] & 0x03) << 62;
        
        // Schiebe nach rechts und hole den Carry von links (den hohen Bits)
        pdata->chunks[i] = (pdata->chunks[i] >> 2) | carry;
        
        carry = next_carry;
    }
}

int get_data_index(DATA* pdata,uint16_t index){
    uint16_t byte_index;//2,4,6,6
    byte_index = index%32;
    index = index/32;
    return ((pdata->chunks[index])>>(62-(2*byte_index)))&0x03;// hier war der fehler denn byte_index 1 ist immer an stelle 87
    
}

void draw_bord(DATA *pdata, PLAYER *p,unsigned char direction){
    p->posx_kopf += (direction == 1)?1:
                    (direction ==0)?-1:0;
    p->posy_kopf += (direction == 3)?1:
                    (direction ==2)?-1:0;
    push_data_2l(pdata, direction); //aktuelle kopf
    if (p->bytes.p == p->bytes.a){
        p->player_lenght++;  
        p->posx_apple  = rand()%16;
        p->posy_apple  = rand()%16;
    }

    char grid[16*17+1];// fenster erstellen
    memset(grid, ' ', sizeof(grid));// fenster leeren
    p->posx_now = p->posx_kopf;
    p->posy_now = p->posy_kopf;//jetzige segment ist kof

    for(int y = 1; y <= 16; y++) {
        grid[y * 17 - 1] = '\n';
    }

    grid[16 * 17] = '\0'; // Der Null-Terminator für den Pointer-Zugriff
    grid[p->posy_now*17+p->posx_now] = '@';

    for(int i = 0;i< p->player_lenght;i++){
        uint8_t direction = get_data_index(pdata,i);
        if (direction == 0)p->posx_now += 1;
        if (direction == 1)p->posx_now -= 1;
        if (direction == 2)p->posy_now += 1;
        if (direction == 3)p->posy_now -= 1;
        grid[p->posy_now*17+p->posx_now] = '#';// segment zeichnen
        if (p->bytes.p_n == p->bytes.p) p->bytes.length = 0;// kopf trift segment
        
    }
    // apfel anzeigen
     grid[p->posy_apple*17+p->posx_apple] = 'A';
    // danach updaten dass die logik mit der rekursion richtig ist
    
    printf("\033[H");  // Cursor auf (0,0) ohne zu löschen
    puts(grid);

}

// terminal funktionen
void enable_raw_mode(struct termios *orig_termios) {
    struct termios raw = *orig_termios;
    // ECHO aus, ICANON (Zeilenpuffer) aus
    raw.c_lflag &= ~(ECHO | ICANON);
    // Minimum an Zeichen für read: 0, Timeout: 0 (kein Warten)
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}
int main() {
    
    int vel_kopf = 2;//1,0 = x 2,3 = y kann nur eine vel haben
    unsigned char ch;// tastatur
    // 1. Speicher initialisieren
    DATA snake_body = {0}; // Alles auf 0 setzen
    PLAYER p = {0};

    // einstellungen terminal
    struct termios orig_termios;
    tcgetattr(STDIN_FILENO, &orig_termios); // Backup der alten Einstellungen
    enable_raw_mode(&orig_termios);

    
    // 2. Start-Zustand festlegen
    p.posx_kopf = 5;
    p.posy_kopf = 5;
    p.posx_apple = 10;
    p.posy_apple = 10;
    p.player_lenght = 0; // Kopf + 3 Körperteile

    
    system("clear");
    while (1){
        if (read(STDIN_FILENO, &ch, 1) == 1) {
            if (ch == 'q') break;

            // Escape-Sequenz für Pfeiltasten prüfen (\033 oder 27)
            if (ch == 27) {
                char seq[2];
                // Wir lesen die nächsten zwei Zeichen der Sequenz
                if (read(STDIN_FILENO, &seq[0], 1) == 0) continue;
                if (read(STDIN_FILENO, &seq[1], 1) == 0) continue;

                if (seq[0] == '[') {
                    switch (seq[1]) {
                        case 'A': if (vel_kopf!=3) vel_kopf = 2;
                            printf("HOCH\n"); 
                            break;
                        case 'B': if (vel_kopf!=2) vel_kopf = 3;
                            printf("Runter\n"); 
                            break;
                        case 'C': if (vel_kopf!=0) vel_kopf = 1;
                            printf("Rechts\n"); 
                            break;
                        case 'D': if (vel_kopf!=1) vel_kopf = 0;
                            printf("Links\n"); 
                            break;
                    }
                }
            }
        }	
         
        usleep(100000);
        draw_bord(&snake_body, &p,vel_kopf);

    }
    return 0;
}
