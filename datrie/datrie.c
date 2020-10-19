#define INSERT_MODE 1 /* Indicate the insertion mode */
#define SEARCH_MODE 2 /* Indicate the search, or retrieval, mode */
#define DELETE_MODE 3 / * Indicate the deletion mode */
#define DUMP_MODE 4 /* Indicate the dump mode */
#define END_MODE 5 /* Indicate the end of program */
#define MIN_CODE 1 /* Minimum numerical code */
#define MAX_CODE 255 /* Maximum numerical code */
#define BC_INC 10 /* Increment of the double-array */
#define TAIL_INC 10 /* Increment of TAIL */
#define  KEY_INC 5 /* Increment of the double-array */
#define TEMP_INC 5 /* Increment of TAIL */
#define TRUE -1
#define FALSE 0
#define NILL -1

FILE *KEY_FILE; /* Key dictionary file */
char *KEY; /* Key */
char *TAIL; /* TAIL */
char *TEMP ; /* Buffer */
int *BC; /* BASE and CHECK*/
int MODE ; /* Flag indicating insertion, search,
deletion, dump and end */
int BC_POS ; /* Current maximum index of the double-array */
int TAIL_POS; /* The current maximum index of TAIL */
int BC_MAX ; /* Maximum size of BASE and CHECK */
int TAIL_MAX; /* Maximum size of TAIL */
int KEY_MAX; /* Maximum size of KEY */
int TEMP_MAX; /* Maximum size of TEMP */

void BC_INSERT(), TAIL_INSERT(),
W_BASE() W_CHECK();
char *MEM_STR() ;
void SEPARATE(int s, char *b, int tail_pos);

int BASE (int n) {
    if (n > BC_POS) return(0) ;
    else return (BC[2*n] ) ;
}

int CHECK(int n) {
    if (n > BC_POS) return (0) ;
    else return (BC[2*n+1]) ;
}

void W_BASE (int n, int node) {
    while(n >= BC_MAX) REALLOC_BC() ;
    if(n > BC_POS) BC_POS = n;
    BC[2*n] = node;
}

void W_CHECK(int n, int node) {
    while(n >= BC_MAX) REALLOC_BC() ;
    if(n > BC_POS) BC_POS = n;
    BC[2*n + 1] = node;
}

char *MEM_STR(char *area_name, int *max, int i) {
    char *area;
    *max = i;
    if ((area = (char *)malloc(sizeof(char) * *max)) == NULL) {
        printf("%s malloc error! !\n", area_name) ;
    }

    memset(area, sizeof(area), '\0');
    return(area);
}

void REALLOC_BC() {
    int i, pre_bc;
    AN EFFICIENT IMPLEMENTATION OF TRIE STRUCTURES
        pre_bc = BC_MAX;
    BC_MAX += BC_INC;
    if((BC = (int *)realloc(BC, sizeof(int) *2*BC_MAX) ) == NULL) {
        fprintf(stderr, "BC reallot error! !");
        exit(-1);
    }
    for(i = 2*pret_bc: i < 2*BC_NAX; i++) BC[i] = 0;
    fprintf(stderr, "*** BC realloc ***\n");
}

char *REALLOC_STR(char *area_name, *area, int *max, int inc) {
    int i,pre_size;
    pre_size=*max;
    *max+=int;
    if( (area= (char*) realloc(area, sizeof(char) * *max))==NULL) {
        printf("%s reallot error! !\n",area_name);
        exit(-1) ;
    }
    for(i=pre_size; i<*max; i++) area[i]=='\0';
    fprintf(stderr, "***%S realloc ***\n", area name) ;
    return(area) ;
}

void READ_TAIL(int p){
    int i=0 ;
    while(TAIL[p] != '#') TEMP[i++] = TAIL[P++];
    temp[i++] = '#'; 
    TEMP[i] = '\0';
}

void WRITE_TAIL(char *temp, int p) {
    int i = 0, tail index;
    tail_index = p;
    while((p + strlen(temp)) >= TAIL_MAX-1)
        TAIL = REALLOC_STR("TAIL", TAIL, &TAIL_INC) ;
    while(*(temp+i) != '\0')
        TAIL [tail_index++] = *(temp+i++);
    if( (p + i + 1) > TAIL_POS) TAIL_POS = p + i;
}

main() {
    int c, i, count;

    INITIALIZE() ;
    for(;;) {
        SELECTION();
        count = 1; i=0;
        while( (c = getc(KEY_FILE) ) !=EOF) {
            if(c !='\n'){
                while(i >= KEY_MAX-2) {
                    KEY = REALLOC_STR("KEY",KEY, &KEY_MAX, KEY_INC);
                    TEMP = REALLOC_STR("TEMP", TEMP, &TEMP_MAX, TEMP_INC);
                    KEY[i++] = c;
                    continue;
                    KEY[i] = '\0'; i = 0;
                    switch(MODE) {
                        case INSERT_MODE:
                            if(SEARCH() == FALSE)
                                printf("%d:%s is inserted\n", count++, KEY);
                            else printf("%d:%s is already in your dictionary\n",
                                    count++, KEY) ;
                            break;
                        case SEARCH_MODE:
                            if(SEARCH() == TRUE)
                                printf("%d:%s is searched\n", count++, KEY);
                            else printf("%d:%s is mismatch\n", count++, KEY) ;
                            break;
                        case DELETE MODE:
                            if(SEARCH() == TRUE)
                                printf("%d:%s is deleted\n", count++, KEY);
                            else printf("%d:%s is not in your dictionary\n",
                                    count++, KEY) ;
                            break;
                        case DUMP_MODE: break;
                        default: exit(0);
                    }
                }/*while*/
                INFO(count) ;
                fclose(KEY_FILE) ;
            }/*for*/
        }
    }
}

void SELECTION()
{
    char key_name[30];
    printf(" 1. Insert 2. Search 3. Delete 4. Dump 5. End \n");
    scanf("%d%*c", &MODE) ;
    if(MODE == END_MODE) exit(0) ;
    if(MODE != DUMP_MODE) {
        printf("key_file = ");
        scanf("%s%*c", key_name) ;
        KEY_FILE = fopen(key_name, "r") ;
        if(KEY FILE == NULL){
            printf("\nkey_dic can’t open\n");
            exit(0);
        }
    }
}

void INITIALIZE() {
    int i;
    BC_MAX = BC_INC; BC_POS = 1; TAIL_POS = 1;
    if((BC = (int *)malloc(sizeof (int) *2*BC_MAX)) == NULL) {
        fprint(stderr, "BC malloc error!!") ;
        exit(-1);
    }

    memset(BC, sizeof(BC), 0);
    W_BASE(1,1); BC_POS = 1;
    TAIL = MEM STR("TAIL", &TAIL_MAX, TAIL_INC);
    TAIL_POS =–1; TAIL[0] = '#';
    TEMP = MEM_STR("TEMP", &TEMP_MAX, TEMP_INC);
    KEY = MEM_STR("KEY", &KEY_MAX, KEY_INC) ;
}

void INFO(int count) {
    int it bc_empty = 0;
    for(i=0; i <= BC POS; ++i){
        if(BASE(i) == 0 && CHECK(i) ==0) bc_empty++;
    }
    if(MODE == DUMP_MODE) {
        printf("\n");
        printf("Index | BASE | CHECK\n");
        for(i=0; i <= BC_POS; ++i)
            printf("%7d |%7d | %7d|\n", i, BASE(i), CHECK(i));
        for(i=0; i <= TAIL_POS; ++i) printf("%d%c|", i, TAIL[i]);
        print("\n");
    }
    printf("Total number of keys=%d\n",count-1);
    printf("BC_POS=%d\n", BC_POS);
    printf("bc_empty+%d\n", bc_empty);
    printf("TAIL_POS=%d\n", TAIL_POS);
}

int SEARCH () {
    unsigned char ch;
    int h=-1, s=1, t;
    strcat (KEY, "#") ;
    do {
        ++h ;
        ch = KEY[h];
        t = BASE(s) + ch;
        if (CHECK(t) != s) {
            if(MODE == INSERT_MODE) BC_INSERT(s, KEY+h) ;
            return(FALSE) ;
        }
        if(BASE(t) < 0) break;
        s=t ;
    }while(TRUE);

    if(*(KEY+h) != '#') READ_TAIL ((-1) *BASE(t)) ;
    if(*(KEY+h) == '#' || !strcmp(TEMP, (KEY+h+1))) {
        if(MODE == DELETE_MODE) {
            W_BASE(t, 0); W_CHECK(t, 0);
        }
        return(TRUE) ;
    }else{
        if(MODE == INSERT_MODE && BASE(t) != 0)
            TAIL_INSERT(t, TEMP, KEY+h+1);
        return(FALSE) ;

    }
}

char *SET_LIST (int s) {
    char list [MAX_CODE-MIN_CODE+1 ] ;
    int i,j=0,t ;
    for (i = MIN_CODE; i < MAX_CODE-1; i++) {
        t = BASE (s)+i;
        if (CHECK(t) == s) list [j++] = (char)i;
    }
    list [j] = '\0';
    return (list) ;
}

void BC_INSERT(int s, char *b) {
    int t;
    char list_s[MAX_CODE-MIN_CODE+1], list_t[MAX_CODE-MIN_CODE+1],
    *SET_LIST() ;
    t = BASE(s) + (unsigned char) *b;
    if(CHECK(t) != 0) {
        strcpy(list_s, SET_LIST(s));
        strcpy(list_t, SET_LIST (CHECK(t)));
        if (strlen(list_s) +1<strlen(list_t)) {
            s = CHANGE_BC(s, s, list_s, *b) ;
        } else {
            s = CHANGE_BC(s, CHECK(t), list_t, '\0');
        }
    }
    SEPARATE(s, b, TAIL_POS);
}

void SEPARATE(int s, char *b, int tail_pos) {
    int t;
    t = BASE(S) + (unsigned char) *b; b++;
    W_CHECK (t , s ) ;
    W_BASE (t, (-1) *tail_pos) ;
    WRITE_TAIL (b, tail_pos) ;
}

CHANGE_BC (int current, int s, char *list, char ch) {
    int i, k, old node, new_node, old_base;
    char a_list [MAX_CODE-MIN_CODE ] ;
    old_base = BASE(s);
    if(ch != '\0'){
        strcpy(a_list, list) ; 
        i = strlen (a_list) ;
        a_list [i] = ch; 
        a_list[i+1] = '\0';
    }

    W_BASE (s, X_CHECK(a list) ) ;
    i=0 ;
    do {
        old_node = old_base + (unsigned char) (*list) ;
        new_node = BASE(s) + (unsigned char) (*list) ;
        W_BASE(new_node, BASE(old_node));
        W_CHECK(new_node, s) ;
        if(BASE(old_node) > 0) {
            k = BASE(old_node)+1;
            while (k-BASE(old_node) <MAX_CODE-MIN_CODE||k<BC_POS) {
                if(CHECK(k) == old_node) W_CHECK(k, new_node) ;
                ++k ;
            }
        }
        if(current != s && old_node == current) current = new_node;
        W_BASE(old node, 0); W_CHECK(old_node, 0) ; list++;
    }while(*list != '\0');

    return(current);
}

int X_CHECK(char *list) {
    int i, base_pos = 1, check_pos;
    unsigned char sch;
    i=0 ;

    do{
        ch = list[i++];
        check_pos = base_pos + sch;
        if (CHECK(check_pos) != 0) {
            base_pos++; 
            i= 0;
            continue;
        }
    }while(list[i] != '\0');

    return(base_pos) ;
}

void TAIL_INSERT (int s, char *a, *b) {
    char list [3];
    unsigned char ch;
    int i = 0, length = 0, t, old_tail_pos;
    old_tail_pos = (-1) *BASE(s) ;
    while (a [length] == b [length] ) length++;
    while (i < length) {
        ch = a[i++];
        list [0] = ch; 
        list [l] = '\ 0';
        W_BASE (s, X_CHECK (list, '\0'));
        t–= BASE(s) + ch;
        W_CHECK (t, S) ;
        s=t ;
    }

    list[0] = a [length]; 
    list[1]= b[length]; 
    list[2] = '\0';
    W_BASE(s, X_CHECK(list, '\0'));
    SEPARATE(s, a+length, old_tail_pos);
    SEPARATE(S, b+length, TAIL_POS);
}
