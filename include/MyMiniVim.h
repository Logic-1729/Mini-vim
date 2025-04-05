#include <ncurses.h>
#include <cstring>
#include <string>
#include <iostream>
#include <list>
#include <vector>
#include <fstream>

//主要在intialize_windows里用到的宏
#define REG_COLOR_NUM 1
#define TEXT_COLOR_NUM 2
#define INFO_COLOR_NUM 3
#define COMMAND_COLOR_NUM 4
#define NUM_COLOR_NUM 5
#define KEY_ESC 27

int num_col=9;                      //记录数字窗口占用的列数,数字位数+两位制表符
int line_num=1;                     //记录文本总行数
int start_line_num=0;               //记录实际的第一行的行数,0-base
int start_col_num=0;                //记录实际的第一列的列数,0-base
int templine;                       //记录实际的文本数组行数,0-base
int tempcol;                        //记录实际的文本数组列数,0-base
int textx=0,texty=0;                //记录实际的光标位置
int temp_textx=0,temp_texty=0;      //记录屏幕的光标位置
int num_start_line;                 //记录第一行占用的行数
int memory_pos=0;                   //用来记录记忆位置

char temp_ch;                       //用于读入每个字符
std::string mode="normal";          //用于记录当前系统是模式状态
std::string filename;               //用于保存文件名
std::vector<char> temp_line;        //用来储存每一行
std::vector<std::vector<char>> text;//用来储存文本,行1-base，列0-base
std::ifstream freader(filename);    //用来判断文件打开的各种参数


//在input_linux_command中用于标记文件参数的布尔变量
bool flag_truncate=false;  //用于标记-t模式
bool flag_readonly=false;  //用于标记-R模式
bool flag_wrap=false;      //用于标记-w模式
bool flag_r=false;        //用于标记CRLF文件
bool flag_break=true;     //用于标记-w break模式,默认
bool flag_scroll=false;     //用于标记-w scroll模式
bool flag_file=false;      //用于标记当前正在阅读的是文件名
bool flag_endloop=false;    //用于标记是否结束文件读入
//insert_mode里使用
bool flag_openfile=false;   //用于标记是否插入字符

//命令行相关
struct COMMAND{
    std::string cmd;
    std::string content[100];
    int num = 0;
    bool operator==(const COMMAND &rhs) const{
        if (rhs.cmd != this->cmd)
            return false;
        if (rhs.num != this->num)
            return false;
        for (int i = 0; i < num; i++){
            if (rhs.content[i] != this->content[i])
                return false;
        }
        return true;
    }
};
std::vector<char> command;            //用来储存临时命令
std::vector<COMMAND> command_history; //用来储存历史命令
int command_size = 0;                 //用来储存命令大小
int commandcur = 0;                   //用来储存命令行指针
int start_command;                    //用来储存命令行起始编号

WINDOW *text_win,*info_win,*command_win,*num_win;

//预处理函数
void input_linux_command(int argc,char *argv[]);            //对linux命令行的处理
void initialize_windows();                                  //对窗口的初始化
void initialize_file();                                     //对文件的初始读入

//打印函数
void initialize_print();                                    //对文本界面和数字界面的打印，内含两个子函数
void textprint();
void numprint();
void informationprint(std::string mode);                    //对信息界面的打印
void commandprint();                                        //对命令界面的打印
bool fileprint();                                           //输出文件

//辅助函数
void move(int ch,std::string mode);                         //光标移动
void temp_move(int ch,std::string mode);                    //无记忆化的光标移动
int get_linenum(int key);                                   //获取行号
void substitute(std::string string_input, std::string string_output);     //替换函数

//主函数
void normal_mode();                                         //normal mode的主函数
void command_mode();                                        //command mode的主函数
void insert_mode();                                         //insert mode的主函数


