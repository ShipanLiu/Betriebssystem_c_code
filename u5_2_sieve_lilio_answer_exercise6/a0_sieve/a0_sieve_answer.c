//  finished!
// 就是要找 所有的 prime 

#include <stdio.h>
#include <stdbool.h>


int main(int argc, char **argv) {

    const int MAX = 100;
    
    // 因为 要 检查 1~100  一共101 个数字。
    bool prime[MAX+1];
    
    // 因为 0 和 1 不是 prime， 所以直接赋值为 false（需要 #include <stdbool.h>）
    prime[0] = false;
    prime[1] = false;
    
    // 除了 0 和 1 剩下的 初始值 全部设置为 0 和 1
    for(int i=2; i<MAX+1; i++)
        prime[i] = true;
    
    /* Anwenden des Siebs des Eratosthenes
     * Suche eine Primzahl: Falls die angesprochene Zahl im Array prime[] auf true steht, ist diese eine Primzahl.
     * Setze nun alle Vielfachen dieser Zahl auf false, da diese keine Primzahlen sind.
     * Hinweis: Anwendung der in der Aufgabenstellung genannten Abbruchbedingung:
     * Die in der Aufgabenstellung genannte Bedingung 'i * i <= MAX' beendet die Schleife, 
     * sobald klar ist, dass alle Vielfachen von Primzahlen schon "gestrichen" wurden.
     * Am Beispiel: Sobald i=5 ist, sind alle nicht-primen Zahlen kleiner 25 identifiziert, 
     * da sie bereits eine der vorherigen Primzahlen in ihrer Primzahlzerlegung beinhalten. 
     * Die 15 = 3*5 wurde bspw. schon im Durchlauf der 3 gestrichen.
     * Wir koennen die Schleife hier also abbrechen, sobald wir mit i die Wurzel von MAX erreicht haben.
     *  Daher die Abbruchbedingung: i*i > MAX. (Die Schleife laeuft, solange i*i <= MAX)
     */
    // loop 结束的 标志 就是 index^2 > MAX , 这就说明 已经找到了全部的 prime
    for(int i=2; i*i<=MAX; i++){
        // 刚开始 2 是 prime， 
        if(prime[i]){
            /* i ist eine Primzahl
             * starte beim ersten Vielfachen von i: 2*i
             * laufe ueber alle Zahlen
             * rechne immer i dazu, um schnell alle Vielfache zu behandeln
             * setze jedes Vielfache auf false
             */
            // 把 index 为 i 的2 倍， 3倍， 4倍， 5倍  全部 标记为false
            for(int j=2*i; j<=MAX; j+=i)
                prime[j] = false;
        }
    }
    
    // Ausgabe aller Zahlen, die noch als true gekennzeichnet sind -> Primzahlen
    // 现在 输出 全部为 prime 的数
    for(int i=2; i<MAX+1; i++){
        if(prime[i])
            printf("%d\n", i);
    }
    
    return 0;
}
