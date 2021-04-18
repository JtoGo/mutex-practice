#include<stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define COUNT 1000 // 配列の数
#define LOOP 10000 // busy内でこの数分だけループを回す

struct timespec ts[4][COUNT]; // 各スレッドの測定時間を格納する配列
struct timespec x; // 開始時刻を取得する変数

int rcount = 4; // リソース数

pthread_t th1, th2, th3, th4; // スレッド
pthread_cond_t cvar; 
pthread_mutex_t mtx; 

void *fun(void *arg);
void busy(void);
void get_resource(int res);
void release_resource(int res);

int main(void) {
    // 開始時刻を取得
    clock_gettime(CLOCK_REALTIME, &x);
    long start_time = (x.tv_sec * 1000000000) + x.tv_nsec;
    
    // 各スレッドの生成
    pthread_create(&th1, NULL, &fun, (void *) 1);
    pthread_create(&th2, NULL, &fun, (void *) 2);
    pthread_create(&th3, NULL, &fun, (void *) 3);
    pthread_create(&th4, NULL, &fun, (void *) 4);

    // スレッドの統合
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    pthread_join(th3, NULL);
    pthread_join(th4, NULL);

    // 各スレッドの測定時間の表示
    for (int thn = 0; thn < 4; thn++) {
        for (int i=0; i<COUNT; i++) {
            long current_time = (ts[thn][i].tv_sec * 1000000000) + ts[thn][i].tv_nsec;
        
            printf("%d\t%d\n", current_time-start_time, thn+1);
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
        get_resource(thn); // リソースの取得

        for (int j = 0; j < 10; j++){
            busy();
            clock_gettime(CLOCK_REALTIME, &ts[thn-1][i*10 + j]);
        }

        release_resource(thn); // リソースのリリース
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
// 引数：消費するリソース数
// 戻り値：なし
void get_resource(int res) {
    pthread_mutex_lock(&mtx); // mutex ロック

    // 使用できるリソースがないとき、
    // mutexをアンロックして、リソースが開放されるまで待機
    while (res > rcount) pthread_cond_wait(&cvar, &mtx); 
    
    rcount -= res; // リソース取得
    pthread_mutex_unlock(&mtx); // mutex アンロック
}

// リソースを開放する関数
// 引数：開放するリソース数
// 戻り値：なし
void release_resource(int res) {
    pthread_mutex_lock(&mtx); // mutex ロック

    rcount += res; // リソース開放
    pthread_cond_signal(&cvar);// 待機中のスレッドにシグナルを送る

    pthread_mutex_unlock(&mtx); // mutex アンロック
}