#include <stdio.h>
#include <string.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "devices/timer.h"

#include "projects/automated_warehouse/aw_manager.h"

struct robot* robots;
struct message_box boxes_fControl; //control이 보내는 messagebox
struct message_box boxes_fRobot; //robot이 보내는 messagebox
int num_robot;

// test code for central control node thread
void test_cnt(){
        while(1){
                print_map(robots, 4);
                thread_sleep(1000);
                block_thread();
        }
}

void control_thread(){
        while(1){
                print_map(robots, 4);
        }

}

// test code for robot thread
void test_thread(void* aux){
        int idx = *((int *)aux);
        int test = 0;
        while(1){
                printf("thread %d : %d\n", idx, test++);
                thread_sleep(idx * 1000);
        }
}

// entry point of simulator
void run_automated_warehouse(char **argv)
{
        init_automated_warehouse(argv); // do not remove this

        printf("implement automated warehouse!\n");

        // 로봇의 개수 처리
        num_robot = atoi(argv[1]);

        boxes_fControl = malloc(sizeof(struct message_box) * num_robot);        //control이 보내는 messagebox
        boxes_fRobot = malloc(sizeof(struct message_box) * num_robot);          //robot이 보내는 messagebox

        // 로봇 문자 자릿수 계산
        int tempRobotNum = num_robot;
        int robot_num_digit = 0;        //로봇 문자 자릿수
        while (tempRobotNum > 0){
                tempRobotNum = tempRobotNum / 10;
                robot_num_digit++;
        }


        // 로봇 초기화 (로봇 정보)
        robots = malloc(sizeof(struct robot) * num_robot);
        char **name_robot = malloc(sizeof(char) * num_robot); //로봇의 이름 저장
        int *req_payload = malloc(sizeof(int) * num_robot);     //로봇의 payload 저장
        int *goalPoint = malloc(sizeof(int) * num_robot);       //로봇의 goal 저장

        

        char *bufN = malloc(sizeof(char) * (robot_num_digit+1) * (robot_num_digit));
        for (int i=0; i<num_robot; i++){
                name_robot[i] = malloc(sizeof(char) * (robot_num_digit+1));
                snprintf(bufN, robot_num_digit, "%d", i + 1);
                strlcpy(name_robot[i], "R", 3);
                strcat(name_robot[i], bufN);
                setRobot(&robots[i], name_robot[i], 5, 5, req_payload, 0);
        }

        // example of create thread
        tid_t* threads = malloc(sizeof(tid_t) * 4);
        int idxs[4] = {1, 2, 3, 4};
        threads[0] = thread_create("CNT", 0, &test_cnt, NULL);
        threads[1] = thread_create("R1", 0, &test_thread, &idxs[1]);
        threads[2] = thread_create("R2", 0, &test_thread, &idxs[2]);
        threads[3] = thread_create("R3", 0, &test_thread, &idxs[3]);

        // if you want, you can use main thread as a central control node
        
}