#include<stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define COUNT 1000 // 配列の数
#define LOOP 10000 // busy内でこの数分だけループを回す

struct timespec ts[5][COUNT]; // 各スレッドの測定時間を格納する配列
struct timespec x; // 開始時刻を取得する変数

int rcount = 4; // リソース数：４

pthread_t th0, th1, th2, th3; // スレッド
pthread_cond_t cvar; 
pthread_mutex_t mtx; 

void *fun(void *arg);
void busy(void);
void get_resource(void);
void release_resource(void);

int main(void) {
    // 開始時刻を取得
    clock_gettime(CLOCK_REALTIME, &x);
    long start_time = (x.tv_sec * 1000000000) + x.tv_nsec;
    
    // 各スレッドの生成
    pthread_create(&th0, NULL, &fun, (void *) 0);
    pthread_create(&th1, NULL, &fun, (void *) 1);
    pthread_create(&th2, NULL, &fun, (void *) 2);
    pthread_create(&th3, NULL, &fun, (void *) 3);

    // スレッドの統合
    pthread_join(th0, NULL);
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    pthread_join(th3, NULL);

    // 各スレッドの測定時間の表示
    for (int thn = 0; thn < 4; thn++) {
        for (int i=0; i<COUNT; i++) {
            long current_time = (ts[thn][i].tv_sec * 1000000000) + ts[thn][i].tv_nsec;
        
            printf("%d\t%d\n", current_time-start_time, thn);
        }
    }

    return 0;
}

// 各スレッドで行うプロセスを実行する関数
// 引数：各スレッド番号の汎用ポインタ
// 戻り値：なし
void *fun(void *arg) {
    int thn = (int)arg;
    
    // 各スレッドのプロセスの１単元は、
    // 「"busy()を実行して、時間測定を行う"という処理を10回繰り返す」というものであり
    // 各スレッドは資源を活用して、このプロセスを100回繰り返す。
    for (int i = 0; i < COUNT/10; i++) {
        get_resource(); // リソースの取得

        for (int j = 0; j < 10; j++){
            busy();
            clock_gettime(CLOCK_REALTIME, &ts[thn][i*10 + j]);
        }

        release_resource(); // リソースのリリース
        busy(); // 少し待機

    }

    return 0;
}

// for文をLOOP回繰り返す（時間稼ぎする）関数
// 引数：なし
// 戻り値：なし
void busy (void) {
    for (int i = 0; i < LOOP; i++) {

    }
}

// リソースを取得する関数
// 引数：なし
// 戻り値：なし
void get_resource(void) {
    pthread_mutex_lock(&mtx); // mutex ロック

    // 使用できるリソースがないとき、
    // mutexをアンロックして、リソースが開放されるまで待機
    while (rcount == 0) {
        pthread_cond_wait(&cvar, &mtx); 
    }

    rcount--; // リソース取得
    pthread_mutex_unlock(&mtx); // mutex アンロック
}

// リソースを開放する関数
// 引数：なし
// 戻り値：なし
void release_resource(void) {
    pthread_mutex_lock(&mtx); // mutex ロック

    if (rcount == 0) pthread_cond_signal(&cvar);// 待機中のスレッドにシグナルを送る
    rcount++; // リソース開放

    pthread_mutex_unlock(&mtx); // mutex アンロック
}