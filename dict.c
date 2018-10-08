/*develop a E-V dictionary with the following requirements:
User Interaction via simple GUI
Basic Dictionary Functionalities
Consult a word
Definition – pronunciation
List of synonyms (Optional)
Auto-complete: Filter list of word based on character entered
Auto-suggestion: When user entered a wrong word – the system find the most suitable word (you may use Levenshtein distance)
Recent words
Add/Delete/Update.*/

// gcc dict.c $(pkg-config --cflags --libs gtk+-3.0) -o dict libfdr.a
// ./dict



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "lib/jrb.h"
#include "lib/dllist.h"

#define LEN 200
#define MAX_WORDS 10
#define DISTANCE 3

typedef struct {
    JRB synonym;
    JRB vertices;
} Graph;

GtkTextBuffer *tb, *tb1, *tb2, *tb3;
char *en, *vie, *pron;

Graph g;
Dllist list;

// Graph - store dict
Graph createGraph();
void  addWord(Graph graph, char* en, char* vie);
char* getWord(Graph graph, char* en);
void  delWord(Graph graph, char* en);
void  updateWord(Graph graph, char* en, char* vie);
void  dropGraph(Graph graph);

// auto suggestions use LevenshteinDistance
int  minimum(int a, int b, int c);
int  LevenshteinDistance(char *s, char *t);
char *suggestions(Graph graph, char* word);

// store recent Words
Dllist create_List();
Dllist findWord_List(Dllist list, char* word);
void insertWord_List(Dllist list, char* word);
void delNode_List(Dllist node);
char* traverse_List(Dllist list);
int  empty_List(Dllist list);
void free_List(Dllist list);

// String
// -> Xu ly string
int is_search_tab(char *str);
char *str_search_tab(Graph graph, char* str);
char *str_n_dup(char *str, int len);
char *str_cat(char *str1, char *str2);
char *str_trim(char* str);

// read and write File
void readFile(char *file, Graph g);
void writeFile(char *file, Graph g);

// GTK - simple GUI
void add();
void update();
void gtk_getWord();
void gtk_search(GtkWidget *widget,gpointer label);
void gtk_add(int argc,char *argv[]);
void gtk_delete(GtkWidget *widget,gpointer label);
void gtk_update(int argc,char *argv[]);
void gtk_recent(GtkWidget *widget,gpointer label);


int main(int argc, char  *argv[])
{
    /* code */
    GtkWidget *button1, *button2, *button3, *button4, *button5;
    GtkWidget *window, *fix, *textview, *label;
    char file[] = "tudien.txt";

    g = createGraph();
    list = create_List();

    readFile(file,g);

    gtk_init(&argc,&argv);
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window),"TỪ ĐIỂN ANH-VIỆT");
    gtk_window_set_default_size(GTK_WINDOW(window),650,500);
    g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);
    
    fix=gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window),fix);
    
    button1=gtk_button_new_with_label("Tra từ");
    gtk_widget_set_size_request(button1,120,50);
    gtk_fixed_put(GTK_FIXED(fix),button1,250,30);
    
    button2=gtk_button_new_with_label("Bổ sung từ mới");
    gtk_widget_set_size_request(button2,120,50);
    gtk_fixed_put(GTK_FIXED(fix),button2,370,30);
    
    button3=gtk_button_new_with_label("Xóa từ");
    gtk_widget_set_size_request(button3,120,50);
    gtk_fixed_put(GTK_FIXED(fix),button3,490,30);
    
    button4=gtk_button_new_with_label("Update");
    gtk_widget_set_size_request(button4,120,50);
    gtk_fixed_put(GTK_FIXED(fix),button4,610,30);

    button5=gtk_button_new_with_label("Recent Words");
    gtk_widget_set_size_request(button5,120,50);
    gtk_fixed_put(GTK_FIXED(fix),button5,730,30);

    textview=gtk_text_view_new();
    gtk_widget_set_size_request(textview,200,40);
    gtk_fixed_put(GTK_FIXED(fix),textview,25,35);
    tb=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

    label=gtk_label_new(NULL);
    gtk_label_set_justify(GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_fixed_put(GTK_FIXED(fix),label,250,100);

    g_signal_connect(button1,"clicked",G_CALLBACK(gtk_search),label);
    g_signal_connect(button2,"clicked",G_CALLBACK(gtk_add),NULL);
    g_signal_connect(button3,"clicked",G_CALLBACK(gtk_delete),label);
    g_signal_connect(button4,"clicked",G_CALLBACK(gtk_update),NULL);
    g_signal_connect(button5,"clicked",G_CALLBACK(gtk_recent),label);

    gtk_widget_show_all(window);
    gtk_main();

    /*char result_file[] = "result.txt";
    writeFile(result_file, g);*/

    printf("Exit\n");
    dropGraph(g);
    free_List(list);
    return 0;
}

/* Graph */

// tao do thi chua tu dien
Graph createGraph() {
    Graph g;
    g.synonym = make_jrb();
    g.vertices = make_jrb();
    return g;
}

// them tu vao tu dien
void addWord(Graph graph, char* en, char* vie) {
    JRB node = jrb_find_str(graph.vertices, en);

    if (node != NULL) {
        printf("Từ [%s] đã có trong từ điển\n",en);
    } else {
        Jval val = new_jval_s(strdup(vie));
        jrb_insert_str(graph.vertices, strdup(en), val);
    }
}

// tim kiem tu tieng anh 
// find -> return vie
// else -> return NULL
char* getWord(Graph graph, char* en) {
    JRB node = jrb_find_str(graph.vertices, en);
    return (node!=NULL) ? jval_s(node->val) : NULL ;
}

// xoa tu
void delWord(Graph graph, char* en) {
    JRB node = jrb_find_str(graph.vertices, en);

    if (node != NULL) {
        jrb_delete_node(node);
    } else {
        printf("Từ [%s] không có trong từ điển\n",en);
    }
}

// update pronucian va nghia tieng viet cua 1 tu da ton tai trong tu dien
void updateWord(Graph graph, char* en, char* vie) {
    JRB node = jrb_find_str(graph.vertices, en);

    if (node != NULL) {
        node->val = new_jval_s(strdup(vie));
    } else {
        printf("Từ [%s] không có trong từ điển\n",en);
    }
}

// free
void dropGraph(Graph graph) {
    JRB node ;

    jrb_traverse(node, graph.synonym)
        jrb_free_tree((JRB) jval_v(node->val));
    jrb_free_tree(graph.synonym);

    jrb_free_tree(graph.vertices);
}

/* Suggestions Words use LevenshteinDistance */
// https://vi.wikipedia.org/wiki/Kho%E1%BA%A3ng_c%C3%A1ch_Levenshtein

// Tra ve danh sach cac tu gan dung nhat duoi dang 1 chuoi
char *suggestions(Graph graph, char* word) {
    JRB node;
    char *result = "Các từ gợi ý:\n";
    int len = strlen(word);
    int i = 0, j = 0;

    jrb_traverse(node, graph.vertices) {
        char *s = jval_s(node->key);
        int   d = LevenshteinDistance(s, word);
        int   cmp = strncmp(word, s, len);

        if( (d < DISTANCE && cmp && (i++ < MAX_WORDS/2)) || (!cmp && (j++ < MAX_WORDS/2)) ) {
            result = str_cat(str_cat(result,s),"\n");
        }            
    }
    return result;
}

// tim so be nhat trong 3 so
int minimum(int a, int b, int c) {
    int min = ( a < b )  ? a : b;
    return ( c < min )  ? c : min;
}

// tim khoang cach Levenshtein giua 2 chuoi s va t
int LevenshteinDistance(char *s, char *t) {
    int d[100][100] = {} ,i ,j ,cost;
    int m = strlen(s);
    int n = strlen(t);

    for (i=0; i<=m ; i++) d[i][0] = i;
    for (j=0; j<=n ; j++) d[0][j] = j;

    for (i=1; i<=m; i++) {
        for (j=1; j<=n; j++) {
            if (s[i-1] == t[j-1]) cost = 0;
            else cost = 1;

            d[i][j] = minimum(d[i-1][j]+1, d[i][j-1]+1, d[i-1][j-1]+cost);
                //  = minimum(  delete   ,    add     ,     replace     )
        }
    }

    return d[m][n];
}



/* List store Recent Words */

// khoi tao list
Dllist create_List() {
    return new_dllist();
}

// tim tu trong list
// find -> tra ve node chua word
// else -> tra ve NULL
Dllist findWord_List(Dllist list, char* word) {
    Dllist node;

    dll_traverse(node, list) {
        int i = strcmp(jval_s(node->val), word);
        if (!i) return node;
    }
    return NULL;
}

//  chen 1 tu vao list
//  neu tu da ton tai trong list 
//  -> xoa di va them vao dau cua list
void insertWord_List(Dllist list, char* word) {
    Dllist node = findWord_List(list, word);
    Jval   val  = new_jval_s(strdup(word));

    if(node != NULL) {
        delNode_List(node);
    }
    dll_prepend(list, val);
}

// xoa 1 node trong list
void delNode_List(Dllist node) {
    dll_delete_node(node);
}

// duyet list 
// -> in ra danh sach cac tu trong list duoi dang 1 chuoi
char* traverse_List(Dllist list) {
    char *result = "Recent Words:\n";
    Dllist node;
    int i=0;

    dll_traverse(node, list) {
        if(++i > MAX_WORDS) break;
        result = str_cat(str_cat(result,jval_s(node->val)),"\n");
    }
    
    return result;
}

// kiem tra xem list co rong hay khong
int empty_List(Dllist list) {
    return dll_empty(list);
}

// giai phong
void free_List(Dllist list) {
    free_dllist(list);
}


/* String */

// check auto complete
int is_search_tab(char *str) {
    return str[strlen(str)-1] == '\t';
}

// chuoi can phai auto comple 
//  -> tra ve tu gan nhat theo thu tu tu dien trong Graph
// chuoi khong can auto comple
//  -> tra ve chinh tu do va bo di ki tu '\t' o cuoi  
// "hello\t" -> "hello"
char *str_search_tab(Graph graph, char* str) {
    JRB node;
    int len = strlen(str);

    jrb_traverse(node, graph.vertices) {
        int i = strncmp(str, jval_s(node->key), len-1);
        if (!i) {
            return strdup(jval_s(node->key));
        }
    }
    return str_n_dup(str,len-1);
}

// rewrite fuction strndup (strndup error in windown ??)
// malloc false -> result = NULL
char *str_n_dup(char *str, int len) {
    char *result;
    int n;

    result = (char *) malloc(len + 1);
    if (result != NULL) {
        for (n = 0; ((n < len) && (str[n] != 0)) ; n++) result[n] = str[n];
        result[n] = '\0';
    }

    return result;
}

// success -> result = str1 + str2
// malloc false -> result = NULL
char *str_cat(char *str1, char *str2) {
    int len1, len2;
    char *result;

    len1 = strlen(str1);
    len2 = strlen(str2);

    result = (char*)malloc((len1 + len2 + 1) *  sizeof(char));

    if (result != NULL) {
        strcpy(result, str1);
        strcpy(result + len1, str2);
    }

    return result;
}

// tra ve 1 chuoi ki tu
// la chuoi ban dau khi da xoa het ki tu space ' ' o dau va cuoi chuoi 
char *str_trim(char* str) { 
    int len = strlen(str);

    int i = 0;
    while (i < len && str[i] == ' ' ) i++;

    int j = len-1;
    while (i < j && str[j] == ' ' ) j--;

    return str_n_dup(str+i, j+1-i);
}


/* File */

void readFile(char *file, Graph g) {
    FILE *fptr = fopen(file,"r");
    char *en, *vie, read[LEN];
    int i, len; 

    if (fptr == NULL) {
        printf("Can't open %s\n",file);
        exit(1);
    }

    fgets(read,LEN,fptr);
    while (fgets(read,LEN,fptr)!=NULL) {
        len = strlen(read);
        if (read[0]=='@') {
            i = 1;
            while(i<len && read[i] != '/') i++;
            en  = str_n_dup(read+1, i-2);
            vie = str_n_dup(read+i, len-i);
        } else if(read[0] == '\n') {
            addWord(g,en,vie);
        } else {
            vie = str_cat(vie,read);
        }
    }
    addWord(g,en,vie);
    fclose(fptr);
}

void writeFile(char *file, Graph g) {
    FILE *fptr = fopen(file,"w");
    JRB node;

    if (fptr == NULL) {
        printf("Can't open %s\n",file);
        exit(1);
    }

    fprintf(fptr,"\n");
    jrb_traverse(node,g.vertices) {
        en  = jval_s(node->key);
        vie = jval_s(node->val);
        fprintf(fptr, "@%s %s\n", en, vie);
    }

    fclose(fptr);
}


/* GTK */

// lay 2 gia tri tieng anh (en) va nghia tieng viet (vie)
void gtk_getWord() {
    GtkTextIter start;
    GtkTextIter end;
    
    gtk_text_buffer_get_start_iter (tb, &start);
    gtk_text_buffer_get_end_iter   (tb, &end);
    en = gtk_text_buffer_get_text (tb, &start, &end, FALSE);

    en = str_trim(en);

    if (is_search_tab(en)) {
        en = str_search_tab(g,en);
    }

    printf("en = [%s]\n",en );

    vie = getWord(g,en);
    insertWord_List(list, en);
}

// lay du lieu khi an button "Bổ sung từ mới"
void add() {
    GtkTextIter start, end;
    char result[1000];

    gtk_text_buffer_get_start_iter (tb1, &start);
    gtk_text_buffer_get_end_iter (tb1, &end);
    pron=gtk_text_buffer_get_text (tb1, &start, &end, FALSE);
    
    gtk_text_buffer_get_start_iter (tb2, &start);
    gtk_text_buffer_get_end_iter (tb2, &end);
    vie =gtk_text_buffer_get_text (tb2, &start, &end, FALSE);
    
    sprintf(result,"%s\n%s",pron,vie);
    addWord(g, en, result);
}

// lay du lieu khi an button "Update"
void update() {
    GtkTextIter start, end;
    char result[1000];

    gtk_text_buffer_get_start_iter (tb1, &start);
    gtk_text_buffer_get_end_iter (tb1, &end);
    pron=gtk_text_buffer_get_text (tb1, &start, &end, FALSE);
    
    gtk_text_buffer_get_start_iter (tb2, &start);
    gtk_text_buffer_get_end_iter (tb2, &end);
    vie =gtk_text_buffer_get_text (tb2, &start, &end, FALSE);

    sprintf(result,"%s\n%s",pron,vie);
    updateWord(g, en, result);
}

// --Cac ham gtk ben duoi la hien thi giao dien khi an cac button tuong ung--
void gtk_search(GtkWidget *widget,gpointer label) {
    char *result;

    gtk_getWord();

    if (en != NULL && vie != NULL)  {
        result = str_cat(str_cat(en," "),vie);  
        gtk_label_set_text(label,result);
    } else {
        result = strdup("Từ không có trong từ điển\n");
        result = str_cat(result,suggestions(g,en));
        gtk_label_set_text(label,result);
    }
}

void gtk_add(int argc,char *argv[]) {
    GtkWidget *w, *fix, *ok;
    GtkWidget *entry1, *entry2;
    GtkWidget *label1, *label2;

    gtk_getWord();

    gtk_init(&argc,&argv);
    w=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(w),GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(w),"Bổ sung từ mới");
    gtk_window_set_default_size(GTK_WINDOW(w),250,400);
    g_signal_connect(w,"destroy",G_CALLBACK(gtk_main_quit),NULL);
            
    fix=gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(w),fix);

    if (en != NULL && vie == NULL) {              
        label1=gtk_label_new("Nhập Pronunciation:");
        gtk_label_set_justify(GTK_LABEL(label1),GTK_JUSTIFY_LEFT);
        gtk_fixed_put(GTK_FIXED(fix),label1,10,30);
        
        entry1=gtk_text_view_new();
        gtk_widget_set_size_request(entry1,200,30);
        gtk_fixed_put(GTK_FIXED(fix),entry1,10,50);
        tb1=gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry1));

        label2=gtk_label_new("Nhập nghĩa Tiếng Việt:");
        gtk_label_set_justify(GTK_LABEL(label2),GTK_JUSTIFY_LEFT);
        gtk_fixed_put(GTK_FIXED(fix),label2,10,90);
        
        entry2=gtk_text_view_new();
        gtk_widget_set_size_request(entry2,200,220);
        gtk_fixed_put(GTK_FIXED(fix),entry2,10,110);
        tb2=gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry2));

        ok=gtk_button_new_with_label("OK");
        gtk_widget_set_size_request(ok,100,30);
        gtk_fixed_put(GTK_FIXED(fix),ok,75,360);
        g_signal_connect(ok,"clicked",G_CALLBACK(add),w);        
    } else {
        label1=gtk_label_new("Từ đã có trong từ điển\n->Bổ sung không thành công");
        gtk_label_set_justify(GTK_LABEL(label1),GTK_JUSTIFY_LEFT);
        gtk_fixed_put(GTK_FIXED(fix),label1,10,30);
    }

    gtk_widget_show_all(w);
    gtk_main();
}

void gtk_delete(GtkWidget *widget,gpointer label) {
    gtk_getWord();

    if (en != NULL && vie != NULL) {
        delWord(g,en);
        gtk_label_set_text(label,"Xóa thành công");
    } else {
        gtk_label_set_text(label,"Từ không có trong từ điển\n->Xóa không thành công");
    }
}

void gtk_update(int argc,char *argv[]) {
    GtkWidget *w, *fix, *ok;
    GtkWidget *entry1, *entry2;
    GtkWidget *label1, *label2;
    
    gtk_getWord();

    gtk_init(&argc,&argv);
    w=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(w),GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(w),"Update Word");
    gtk_window_set_default_size(GTK_WINDOW(w),250,400);
    g_signal_connect(w,"destroy",G_CALLBACK(gtk_main_quit),NULL);
            
    fix=gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(w),fix);

    if (en != NULL && vie != NULL) {
        label1=gtk_label_new("Update Pronunciation:");
        gtk_label_set_justify(GTK_LABEL(label1),GTK_JUSTIFY_LEFT);
        gtk_fixed_put(GTK_FIXED(fix),label1,10,30);
        
        entry1=gtk_text_view_new();
        gtk_widget_set_size_request(entry1,200,30);
        gtk_fixed_put(GTK_FIXED(fix),entry1,10,50);
        tb1=gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry1));

        label2=gtk_label_new("Update nghĩa Tiếng Việt:");
        gtk_label_set_justify(GTK_LABEL(label2),GTK_JUSTIFY_LEFT);
        gtk_fixed_put(GTK_FIXED(fix),label2,10,90);
        
        entry2=gtk_text_view_new();
        gtk_widget_set_size_request(entry2,200,220);
        gtk_fixed_put(GTK_FIXED(fix),entry2,10,110);
        tb2=gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry2));

        ok=gtk_button_new_with_label("OK");
        gtk_widget_set_size_request(ok,100,30);
        gtk_fixed_put(GTK_FIXED(fix),ok,75,360);
        g_signal_connect(ok,"clicked",G_CALLBACK(update),w);        
    } else {
        label1=gtk_label_new("Từ không có trong từ điển\n->Update không thành công");
        gtk_label_set_justify(GTK_LABEL(label1),GTK_JUSTIFY_LEFT);
        gtk_fixed_put(GTK_FIXED(fix),label1,10,30);
    }

    gtk_widget_show_all(w);
    gtk_main();
}

void gtk_recent(GtkWidget *widget,gpointer label) {   
    char *result = traverse_List(list);
    gtk_label_set_text(label,result);
}