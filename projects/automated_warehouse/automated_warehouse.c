#include <stdio.h>
#include <string.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "devices/timer.h"

#include "projects/automated_warehouse/aw_manager.h"

struct robot* robots;
struct message_box boxes_from_central_control_node; //control이 보내는 messagebox
struct message_box boxes_from_robots; //robot이 보내는 messagebox
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

#define UP 1;
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define WAIT 5
#define STOP 6
#define LOAD 7

void run_Robot_Thread(void *aux){
        int idx = *((int *)aux);
        int robot_row = 5;
        int robot_col = 5;
        int cur_payload = 0;
        while(1){
                struct message message;

                //메시지 수신
                while(1){
                        if(checkMSGBox(&boxes_from_central_control_node[idx])){
                                message = hasReceivedMSG(&boxes_from_central_control_node[idx]);
                                break;
                        }
                }
                //메시지가 이동일 때 처리.
                if(message.cmd < 5){
                        switch(message.cmd){
                                case UP:
                                        row--;
                                        break;
                                case DOWN:
                                        row++;
                                        break;
                                
                                case LEFT:
                                        col--;
                                        break;
                                case RIGHT:
                                        col++;
                                        break;
                        }
                }
                //메시지가 LOAD 일 때 payload 갱신
                else if(message.cmd == LOAD){
                        if(robot_col == 2){
                                switch(robot_row){
                                        case 2:
                                                cur_payload = 1;
                                                break;
                                        case 3:
                                                cur_payload = 2;
                                                break;
                                        case 4: 
                                                cur_payload = 3;
                                                break;
                                }
                        }else if(robot_col == 4){
                                switch(robot_row){
                                        case 2:
                                                cur_payload = 4;
                                                break;
                                        case 3:
                                                cur_payload = 5;
                                                break;
                                        case 4: 
                                                cur_payload = 6;
                                                break;
                                }
                        }else if(robot_col == 3 && robot_row == 1){
                                cur_payload = 7;
                        }
                }

                message.row = robot_row;
                message.col = robot_col;
                message.current_payload = cur_payload;
                sendMessage(&boxes_from_robots[idx], &message);
                block_thread();
        }
}

#define WALL 99
#define PATH 98
#define ROBT -1

int findpath(int idx,char *goal){
        //로봇은 goal, payload만 도달할 수 있기에 실제 길 외에는 다 wall 처리해둠
        //map은 [row][col]의 정보를 임시 저장
        int map[7][7] = {
                {WALL, WALL, WALL, WALL, WALL, WALL, WALL},
                {WALL, PATH, PATH, WALL, PATH, PATH, WALL},
                {WALL, PATH, WALL, WALL, WALL, PATH, WALL},
                {WALL, PATH, WALL, WALL, WALL, PATH, WALL},
                {WALL, PATH, WALL, WALL, WALL, PATH, WALL},
                {WALL, PATH, PATH, PATH, PATH, PATH, WALL},
                {WALL, WALL, WALL, WALL, WALL, PATH, WALL}
        };
        int cur_payload = robot[idx].required_payload;
        int goal_row, goal_col;
        int col = robot[idx].col;
        int row = robot[idx].row;

        //payload를 load한 상태라면 A,B,C 중 하나를 goal로 set
        if(robot[idx].cur_payload == robot[idx].required_payload){
                switch(goal[idx]){
                        case 'A':
                                gol_col = 0;
                                gol_row = 1;
                                break;
                        case 'B':
                                gol_col = 0;
                                gol_row = 3;
                                break;
                        case 'C':
                                gol_col = 0;
                                gol_row = 5;
                                break;
                }
        }

        //payload 위치만 PATH로 설정하고, payload를 load한 상태가 아니라면 goal set
        switch(cur_payload){
                case 1:
                        map[2][2] = PATH;
                        if(robot[idx].current_payload != 1){
                                goal_row = 2
                                goal_col = 2
                        }
                        break;
                case 2:
                        map[3][2] = PATH;
                        if(robot[idx].current_payload != 2){
                                goal_row = 3
                                goal_col = 2
                        }
                        break;
                case 3:
                        map[4][2] = PATH;
                        if(robot[idx].current_payload != 3){
                                goal_row = 4
                                goal_col = 2
                        }
                        break;
                case 4:
                        map[2][4] = PATH;
                        if(robot[idx].current_payload != 4){
                                goal_row = 2
                                goal_col = 4
                        }
                        break;
                case 5:
                        map[3][4] = PATH;
                        if(robot[idx].current_payload != 5){
                                goal_row = 3
                                goal_col = 4
                        }
                        break;
                case 6:
                        map[4][4] = PATH;
                        if(robot[idx].current_payload != 6){
                                goal_row = 4
                                goal_col = 4
                        }
                        break;
                case 7:
                        map[1][3] = PATH;
                        if(robot[idx].current_payload != 7){
                                goal_row = 1
                                goal_col = 3
                        }
                        break;
                default:
                        printf("로봇%d의 payload가 이상합니다", idx);

        }

        //map의 goal을 설정합니다.
        map[goal_row][goal_col] = 0;
        
        //로봇이 있는 자리를 WALL 처리합니다.
        for(int i=0; i<num_robot; i++){
                if((robot[i].row == 1 && robot[i].col == 0)||(robot[i].row == 3 && robot[i].col == 0)||(robot[i].row == 5 && robot[i].col == 0)||(robot[i].row == 6 && robot[i].col == 5)){
                        //nothing (중복가능칸)
                }else{
                        map[robot[i].row][robot[i].col] = WALL;
                        //다른 로봇의 위치를 벽처리(넘어갈 수 없음)
                }
        }

        //목표 칸으로부터 robot까지의 거리를 계산합니다. BFS 알고리즘 사용
        while(1){
                if(map[row][col] != PATH) break;
                for(int i=0; i<7; i++){
                        for (int j=0; j<7; j++){
                                if(map[i][j] == PATH){
                                        
                                }
                        }
                }
        }
}



// entry point of simulator
void run_automated_warehouse(char **argv)
{
        init_automated_warehouse(argv); // do not remove this

        printf("implement automated warehouse!\n");

        // 로봇의 개수 처리
        num_robot = atoi(argv[1]);

        boxes_from_central_control_node = malloc(sizeof(struct message_box) * num_robot);        //control이 보내는 messagebox
        boxes_from_robots = malloc(sizeof(struct message_box) * num_robot);          //robot이 보내는 messagebox

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
        char *goal = malloc(sizeof(int) * num_robot);       //로봇의 goal 저장
        char *nextPtr
        char *inStr = strtok_r(argv[2], ":", &nextPtr);

        //
        for (int i=0; i<num_robot; i++){
                initMessageBox(&boxes_from_central_control_node[i]);
                initMessageBox(&boxes_from_robots[i]);
                req_payload[i] = atoi(&inStr[0]);
                goal[i] = inStr[1];
                inStr = strtok_r(NULL, ":", &nextPtr);
        }


        char *bufN = malloc(sizeof(char) * (robot_num_digit+1) * (robot_num_digit));
        for (int i=0; i<num_robot; i++){
                name_robot[i] = malloc(sizeof(char) * (robot_num_digit+1));
                snprintf(bufN, robot_num_digit, "%d", i + 1);
                strlcpy(name_robot[i], "R", 3);
                strcat(name_robot[i], bufN);
                setRobot(&robots[i], name_robot[i], 5, 5, req_payload, 0);
        }

        // example of create thread
        tid_t* threads = malloc(sizeof(tid_t) * num_robot);
        int *idxs = malloc(sizeof(int)*num_robot);
        for(int i=0; i<num_robot; i++){
                idxs[i] = i;
                threads[i] = thread_create(name_robot[i], 0, &robot_thread, &idxs[i]);
        }

        // if you want, you can use main thread as a central control node
        
}