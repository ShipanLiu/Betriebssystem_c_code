/* 循环队列    一个圈， 首位相连


队首：  front    队尾： rear    刚开始指向同一个元素

插入元素 ：   front 不变， rear  改变  ， 原因 ： 先进先出 ： front 一直指向 最先进的元素

            插入元素后， rear 就会指向下一个

插入最后一个元素：  这时候 front 和rear 就会重合 ： 你无法判断 是 刚刚开始 还是 插满了

    判断 queue为空的时候 ：               front = rear

    当  rear 在插入 最后一个元素 之前 ：  判断 （rear +1）% maxsize == front 就行了



initQueue()

enqueue()  : add an ele

dequeue() :  remove an ele from the front of the queue

peek()  : get the ele(but not remove) at the front of the queue without removing it

isfull() 

isEmpty()

*/

include

