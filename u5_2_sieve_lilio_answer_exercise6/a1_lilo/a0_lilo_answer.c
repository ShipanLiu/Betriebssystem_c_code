// erarbeitet in der Übungsgruppe T02 am 10.05.2022

#include <stdio.h>
#include <stdlib.h>

// 先定义 节点 ， 数值 + 指向下面的指针
typedef struct elem{
    int data;
    struct elem *next;
}elem;

// Globaler Listenkopf, kann dann in insertElement und removeElement verwendet werden.
// static bei globalen Variablen verwenden, um diese modulglobal zu halten.
// 存储在静态数据区的变量会在程序刚开始运行时就完成初始化，也是唯一的一次初始化。
//共有两种变量存储在静态存储区：全局变量和 static 变量，只不过和全局变量比起来，static 可以控制变量的可见范围，
//在全局变量之前加上关键字static，全局变量就被定义成为一个全局静态变量。
// 1）内存中的位置：静态存储区（静态存储区在整个程序运行期间都存在）
// 2）初始化：未经初始化的全局静态变量会被程序自动初始化为0（自动对象的值是任意的，除非他被显示初始化）
// 3）作用域：全局静态变量在声明他的文件之外是不可见的。准确地讲从定义之处开始到文件结尾。

// 好处：
// 定义全局静态变量的好处：
// <1>不会被其他文件所访问，修改
// <2>其他文件中可以使用相同名字的变量，不会发生冲突。
// 静态函数
// 在函数的返回类型前加上关键字static，函数就被定义成为静态函数。
// 函数的定义和声明默认情况下是extern的，但静态函数只是在声明他的文件当中可见，不能被其他文件所用。
// 定义静态函数的好处：
// <1> 其他文件中可以定义相同名字的函数，不会发生冲突

// 定义了一个 elem 类型的 head 指针
// or   elem *head;
static elem *head = NULL;

// Funktion zum Einfügen in die Liste
// 所有未加 static 前缀的全局变量和函数都具有全局可见性，其它的源文件也能访问,
//对于函数来讲，static 的作用仅限于隐藏, 而对于变量而言， 有两个作用
static int insertElement(int value) {
    
    // Nur positive Werte sollen eingefügt werden können.
	if(value < 0)
        return -1;
    
    /* Fall 1: noch kein Element in der Liste
     * - fordere Speicher für ein Listenelement an -> für head, da wir das erste Element erstellen
     * - Fehlerbehandlung mit return -1, falls kein Speicher allokiert werden konnte
     * - calloc initialisiert den angeforderten Speicher automatisch mit 0. Da malloc dies nicht macht, müsste bei Nutzung von malloc hier noch head->next = NULL; hinzugefuegt werden.
     * - Wert in Strukturelement speichern und zurückgeben.
     * -> dies ist der erste Wert, den wir einfuegen, daher ist keine Ueberpruefung auf Duplikate notwendig
     */
    // 假如 list是 空的化， head 指针 指向 NULL， 这时候， 需要 给head 分配自已的一个空间
    if(head == NULL){
        head = calloc(1, sizeof(elem));
        if(head == NULL)
            return -1;
        
        head->data = value;
        return value;
    }
    
    
    /* Fall 2: mindestens ein Element in der Liste
     * - nehme Helfer-Pointer, um ueber die Liste zu iterieren und die Werte zu ueberpruefen
     * - laufe ueber die Liste und pruefe, ob der einzufuegende Wert bereits vorhanden ist => falls ja: return -1;
     */
    // head一直 是 list 的 头， 不会动， 创建一个 傀儡 help， 代替 head 往后动
    elem *help = head;
    
    while(help->next != NULL){
        // value已经存在于 list 里面了
        if(help->data == value)
            return -1;
        
        // 最后 help 就是 最后的那个 节点
        help = help->next;
    }
    
    // Bei dieser Variante muss der letzte Wert noch explizit ueberprueft werden, da wir in der While-Schleife den aktuellen Wert nur pruefen, falls es ein Nachfolge-Element in der Liste gibt.
    // 上面的哪个 while 没有检查最后一个数字的值。 现在检查一下
    if(help->data == value)
        return -1;
    
    /* Alternative:
     *
     * while(help != NULL){
     *      if(help->data == value)
     *          return -1;
     *
     *      // Schleife abbrechen, wenn kein Nachfolge-Element vorhanden ist
     *      if(help->next == NULL)
     *          break;
     *
     *      help = help->next
     * }
     */
    
    /* An dieser Stelle wissen wir, dass der Wert value eingefuegt werden kann, weil er noch nicht in der Liste vorhanden ist.
     * - erstelle ein neues Listenelement und fordere hierfuer Speicher an
     * - entsprechende Fehlerbehandlung calloc
     */
    elem *newElem = calloc(1, sizeof(elem));
    // 假如开辟 新空间 失败
    if(newElem == NULL)
        return -1;
    
    /*
     * - Setze den Wert in das neu erstellte Element ein.
     * - Hänge das neue Element in die Liste ein.
     */
    newElem->data = value;
    help->next = newElem;
    
    // eingefuegten Wert zurueckgeben
    return value;
}





// Funktion zum Entfernen des aeltesten Elements aus edr Liste
static int removeElement(void) {
    // Falls die Liste leer ist, kann nichts entfernt werden. -> Laut Aufgabenstellung als Fehler behandeln.
    // 假如 list 是  空的化， 返回 -1
	if(head == NULL)
        return -1;
    
    /* Mindestens ein Element in der Liste
     * - Speichere das zu loeschende Element (head) zwischen.
     * - Speichere den Wert, der entfernt wird, um diesen danach ohne Speicherzugriffsverletzung zurueckgeben zu koennen.
     * - Setze head auf das naechste Element, dieses ist das neue Kopf-Element der Liste.
     * - Gib den Speicher des alten Kopf-Elements frei und gib den Wert zurueck.
     */
    elem *curr = head;
    int temp = curr->data;
    head = head->next;
    // 因为 head 指向的就是 第一个元素（最老的）， 现在移除 这个 第一个元素
    free(curr);
    
    // 返回 list 最左端的 那个元素（最老的）
    return temp;
}

int main (int argc, char* argv[]) {
	printf("insert 47: %d\n", insertElement(47));
	printf("insert 11: %d\n", insertElement(11));
	printf("insert 23: %d\n", insertElement(23));
	printf("insert 11: %d\n", insertElement(11));

	printf("remove: %d\n", removeElement());
	printf("remove: %d\n", removeElement());


// 输出结果

// insert 47: 47
// insert 11: 11
// insert 23: 23
// insert 11: -1
// remove: 47
// remove: 11

	exit(EXIT_SUCCESS);
}
