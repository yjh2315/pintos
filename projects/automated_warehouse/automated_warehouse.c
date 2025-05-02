#include <stdio.h>
#include <string.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "projects/automated_warehouse/aw_message.h"

#include "devices/timer.h"

#include "projects/automated_warehouse/aw_manager.h"
#include "projects/automated_warehouse/automated_warehouse.h"

struct robot* robots;
struct message_box* boxes_from_central_control_node; //control이 보내는 messagebox
struct message_box* boxes_from_robots; //robot이 보내는 messagebox
int num_robot;

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define WAIT 5
#define STOP 6
#define LOAD 7

void run_Robot_Thread(void *aux){
        int idx = *((int *)aux);
        int robot_row = 6;
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
                                        robot_row--;
                                        break;
                                case DOWN:
                                        robot_row++;
                                        break;
                                
                                case LEFT:
                                        robot_col--;
                                        break;
                                case RIGHT:
                                        robot_col++;
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

#define WALL 100
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
        int cur_payload = robots[idx].required_payload;
        int goal_row, goal_col;
        int col = robots[idx].col;
        int row = robots[idx].row;

        //payload를 load한 상태라면 A,B,C 중 하나를 goal로 set
        if(robots[idx].current_payload == robots[idx].required_payload){
                switch(goal[idx]){
                        case 'A':
                                goal_col = 0;
                                goal_row = 1;
                                break;
                        case 'B':
                                goal_col = 0;
                                goal_row = 3;
                                break;
                        case 'C':
                                goal_col = 0;
                                goal_row = 5;
                                break;
                }
        }

        //payload 위치만 PATH로 설정하고, payload를 load한 상태가 아니라면 goal set
        switch(cur_payload){
                case 1:
                        map[2][2] = PATH;
                        if(robots[idx].current_payload != 1){
                                goal_row = 2;
                                goal_col = 2;
                        }
                        break;
                case 2:
                        map[3][2] = PATH;
                        if(robots[idx].current_payload != 2){
                                goal_row = 3;
                                goal_col = 2;
                        }
                        break;
                case 3:
                        map[4][2] = PATH;
                        if(robots[idx].current_payload != 3){
                                goal_row = 4;
                                goal_col = 2;
                        }
                        break;
                case 4:
                        map[2][4] = PATH;
                        if(robots[idx].current_payload != 4){
                                goal_row = 2;
                                goal_col = 4;
                        }
                        break;
                case 5:
                        map[3][4] = PATH;
                        if(robots[idx].current_payload != 5){
                                goal_row = 3;
                                goal_col = 4;
                        }
                        break;
                case 6:
                        map[4][4] = PATH;
                        if(robots[idx].current_payload != 6){
                                goal_row = 4;
                                goal_col = 4;
                        }
                        break;
                case 7:
                        map[1][3] = PATH;
                        if(robots[idx].current_payload != 7){
                                goal_row = 1;
                                goal_col = 3;
                        }
                        break;
                default:
                        printf("로봇%d의 payload가 이상합니다", idx);

        }

        //map의 goal을 설정합니다.
        map[goal_row][goal_col] = 0;
        
        //로봇이 있는 자리를 WALL 처리합니다.
        for(int i=0; i<num_robot; i++){
                if((robots[i].row == 1 && robots[i].col == 0)||(robots[i].row == 3 && robots[i].col == 0)||(robots[i].row == 5 && robots[i].col == 0)||(robots[i].row == 6 && robots[i].col == 5)){
                        //nothing (중복가능칸)
                }else{
                        map[robots[i].row][robots[i].col] = WALL;
                        //다른 로봇의 위치를 벽처리(넘어갈 수 없음)
                }
        }

        map[row][col] = PATH;

        //목표 칸으로부터 robot까지의 거리를 계산합니다.
        int val = 0 ;
        while(1){
                if(map[row][col] != PATH) break;
                for(int i=0; i<7; i++){
                        for (int j=0; j<7; j++){
                                if(map[i][j] == PATH){
                                        //옆 칸으로 이동할 수 있고, 해당 칸이 val일 경우(PATH의 초기값은 값이 크다. 0인 target 근처를 탐색한다.) -> 반복시 target으로부터 다른 칸까지 이동하는데 걸리는 칸 수를 얻을 수 있다.
                                        if(i!=0 && map[i-1][j] == val){         //순서가 항상 i 비교부터 해야한다.
                                                map[i][j] = val + 1;
                                        }else if (i!=6 && map[i+1][j] == val)
                                        {
                                                map[i][j] = val + 1;
                                        }
                                        else if (j!=0 && map[i][j-1] == val)
                                        {
                                                map[i][j] = val + 1;
                                        }
                                        else if (j!=6 && map[i][j+1] == val)
                                        {
                                                map[i][j] = val + 1;
                                        }
                                }
                        }
                }
                val++;

                if(val >= 16){   //가장 긴 path가 7에서 A로 둘러가는 길인데, 이 길이가 15이기에 val이 16 이상이라면 이는 로봇이 움직일 수 있는 경로가 아님.
                        map[row][col] = ROBT;
                        break;
                }
        }

        int det = map[row][col];
        if((robots[idx].current_payload == cur_payload) && (row == goal_row) && (col == goal_col)){
                return STOP;
        }
        else if((robots[idx].current_payload != cur_payload) && (row == goal_row) && (col == goal_col)){
                return LOAD;
        }
        else if(row > 0 && map[row-1][col] < det){
                //val이 det보다 작다는건 goal까지의 path라는 것이기에
                robots[idx].row--;
                return UP;
        }
        else if(row < 6 && map[row+1][col] < det){
                //val이 det보다 작다는건 goal까지의 path라는 것이기에
                robots[idx].row++;
                return DOWN;
        }
        else if(col > 0 && map[row][col-1] < det){
                //val이 det보다 작다는건 goal까지의 path라는 것이기에
                robots[idx].col--;
                return LEFT;
        }
        else if(col < 6 && map[row][col+1] < det){
                //val이 det보다 작다는건 goal까지의 path라는 것이기에
                robots[idx].col++;
                return RIGHT;
        }
        else{
                return WAIT;
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
        char **name_robot = malloc(sizeof(char*) * num_robot); //로봇의 이름 저장
        int *req_payload = malloc(sizeof(int) * num_robot);     //로봇의 payload 저장
        char *goal = malloc(sizeof(char) * num_robot);       //로봇의 goal 저장
        char *nextPtr;
        char *inStr = strtok_r(argv[2], ":", &nextPtr);

        //input 메시지 파싱
        for (int i=0; i<num_robot; i++){
                initMessageBox(&boxes_from_central_control_node[i]);
                initMessageBox(&boxes_from_robots[i]);
                req_payload[i] = atoi(&inStr[0]);
                goal[i] = inStr[1];
                inStr = strtok_r(NULL, ":", &nextPtr);
        }

        //parsing한 데이터 기반 robot의 이름 설정        
        char *bufN = malloc(sizeof(char) * (robot_num_digit+1));
        for (int i=0; i<num_robot; i++){
                name_robot[i] = malloc(sizeof(char) * (robot_num_digit+1));
                snprintf(bufN, robot_num_digit+1, "%d", i + 1);
                strlcpy(name_robot[i], "R", robot_num_digit+2);
                strlcat(name_robot[i], bufN, robot_num_digit+2);
                setRobot(&robots[i], name_robot[i], 6, 5, req_payload[i], 0);
        }

        // example of create thread
        int *stoppedMachine = malloc(sizeof(int) * num_robot);
        tid_t* threads = malloc(sizeof(tid_t) * num_robot);
        int *idxs = malloc(sizeof(int)*num_robot);
        
        for(int i=0; i<num_robot; i++){
                idxs[i] = i;
                threads[i] = thread_create(name_robot[i], 0, &run_Robot_Thread, &idxs[i]);
                stoppedMachine[i] = 0;
        }

        // print_map(robot, num_robot);

        // if you want, you can use main thread as a central control node
        struct message *centMsg = malloc(sizeof(struct message) * num_robot);
        int num_message;
        int chkOut = 0;
        while(1){
                //메시지 로봇에게 전달
                for(int i=0; i<num_robot; i++){
                        centMsg[i].cmd = findpath(i, goal);
                        //path를 찾아서 이를 centMsg에 담아서 로봇 쓰레드에게 전달
                        if(centMsg[i].cmd == STOP){
                                stoppedMachine[i] = 1;
                        }
                        sendMessage(&boxes_from_central_control_node[i], &centMsg[i]);
                }

                increase_step();
                unblock_threads();

                //메시지 로봇으로부터 수신
                num_message = 0;
                while(num_message < num_robot){
                        for(int i=0; i<num_robot; i++){
                                if(checkMSGBox(&boxes_from_robots[i])){
                                        centMsg[i] = hasReceivedMSG(&boxes_from_robots[i]);
                                        num_message++;
                                        // 로봇으로부터 받은 메시지로 저장
                                }
                        }
                }

                for(int i=0; i<num_robot; i++){
                        robots[i].current_payload = centMsg[i].current_payload;
                }

                print_map(robots, num_robot);

                chkOut = 1;
                for(int i=0; i<num_robot; i++){
                        if(stoppedMachine[i]==0){
                                chkOut = 0;
                                break;
                        }
                }
                if(chkOut == 1){
                        break;
                }
                timer_sleep(100);

        }
        printf("\nProgram Terminate");
        
}