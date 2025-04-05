#include <MyMiniVim.h>

int main(int argc,char *argv[]){
    //对Linux命令行输入作处理
    input_linux_command(argc,argv);

    //对系统界面，文本界面，信息界面，命令界面做初始化处理；对文件读入做初始化处理
    initialize_windows();
    initialize_file();

    //将文本打印在界面上；将行数打印在界面上;
    initialize_print();

    //进入主程序，在三种模式之间切换
    int ch = 0;
    while(!flag_endloop) {
        if(mode=="normal") normal_mode();
        if(mode=="insert") insert_mode();
        if(mode=="command") command_mode();
    }

    //关闭窗口
    endwin();
    return 0;
}

void input_linux_command(int argc,char *argv[]) {
    for (int i = 1; i < argc; i++){
        //开头设置flag_file,用来判断当前字符串是命令还是文件名
        flag_file=true;
        //去掉开头的路径，判断文件的处理形式
        if(!strcmp(argv[i],"-t")) {
            flag_truncate=true;flag_file=false;
        }
        if(!strcmp(argv[i],"-R")){
            flag_readonly=true;flag_file=false;
        }
        if(!strcmp(argv[i],"-W")) {
            flag_wrap=true;flag_file=false;
        }
        if(flag_wrap) {
            if(!strcmp(argv[i],"break")) {
                flag_break=true;flag_scroll=false;flag_wrap=false;flag_file=false;
            }
            if(!strcmp(argv[i],"scroll")) {
                flag_break=false;flag_scroll=true;flag_wrap=false;flag_file=false;
            }
        }
        //处理文件名
        if(flag_file){
            filename = argv[i];
        }
    }
}

void initialize_windows() {
    //ncurses的各种初始化操作
    initscr();
    raw();
    noecho();
    curs_set(2);            //设置光标完全可见

    // 初始化各种界面的颜色
    start_color();
    init_pair(REG_COLOR_NUM, COLOR_WHITE, COLOR_BLACK);
    init_pair(TEXT_COLOR_NUM, COLOR_WHITE, COLOR_BLACK);
    init_pair(INFO_COLOR_NUM, COLOR_WHITE, COLOR_BLACK);
    init_pair(COMMAND_COLOR_NUM, COLOR_RED, COLOR_BLACK);
    init_pair(NUM_COLOR_NUM, COLOR_WHITE, COLOR_BLACK);

    //创建数字界面，文本界面，信息界面与命令界面
    text_win= newwin(LINES-2, COLS-num_col, 0, num_col);
    info_win= newwin(1, COLS, LINES - 2, 0);
    command_win= newwin(1, COLS, LINES - 1, 0);
    num_win = newwin(LINES-2, num_col, 0, 0);

    keypad(stdscr, true);   //初始化stdscr
    keypad(text_win, true);
    keypad(info_win, true);
    keypad(command_win, true);
    keypad(num_win,true);

    //设定初始界面,文本界面，信息界面,命令界面与数字界面的颜色
    wbkgd(stdscr, COLOR_PAIR(REG_COLOR_NUM));
    wbkgd(text_win, COLOR_PAIR(TEXT_COLOR_NUM));
    wbkgd(info_win, COLOR_PAIR(INFO_COLOR_NUM));
    wbkgd(command_win, COLOR_PAIR(COMMAND_COLOR_NUM));
    wbkgd(num_win, COLOR_PAIR(NUM_COLOR_NUM));

    //保持五个界面的刷新
    wrefresh(stdscr);
    wrefresh(text_win);
    wrefresh(info_win);
    wrefresh(command_win);
    wrefresh(num_win);
}

void initialize_file() {
    text.insert(text.begin(), temp_line);
    std::ifstream freader(filename);
    //如果-t或者文件没被打开或者打开失败
    if(flag_truncate || !flag_file || freader.fail()) {
        //std::cout<<"Segmentation fault (core dumped)";
        return;
    }

    while (!freader.eof()){
        freader.get(temp_ch);
        if (!freader)
            break;
        if ((temp_ch == '\n')||(temp_ch == '\t')){
            if(temp_ch == '\n') {
                line_num++;
                text.push_back(temp_line);
            }else{
                //如果读到/t
                for (int i = 1; i <= 4; i++)
                    text[line_num - 1].push_back(' ');
            }
        }else{
            if(temp_ch!='\r'){
                //如果是正常字符
                text[line_num - 1].push_back(temp_ch);
            }else if (!flag_r){
                flag_r=true;
            }


        }
    }
    freader.close();
}

void initialize_print() {
    textprint();
    numprint();
}

void textprint() {
    //折行模式下的打印
    if(flag_break) {
        while (num_start_line >= get_linenum((long int)text[start_line_num].size())){
            num_start_line -= get_linenum((long int)text[start_line_num].size());
            start_line_num++;
        }

        for (long int i = 0; i < LINES - 2; i++){
            wmove(text_win, i, 0);
            for (long int j = 0; j <= COLS - num_col -1; j++){
                wprintw(text_win, "%c", ' ');
            }
        }

        wmove(text_win, 0, 0);
        long int temp_line = start_line_num;
        long int past_line = 0, current_line = 0, rest_line = 0;
        while (past_line < LINES - 2){
            if (temp_line >= line_num){
                mvwprintw(text_win, current_line, 0, "%c", ' ');
                for (long int k = 1; k <= COLS - num_col -1; k++) {
                    wprintw(text_win, "%c", ' ');
                }
                past_line++;current_line++;
                continue;
            }

            //获取剩下行，如果是开头，将其去掉
            rest_line = get_linenum((long int)text[temp_line].size());
            rest_line =((temp_line == start_line_num)?(rest_line-num_start_line):rest_line);
            past_line += rest_line;
            if (texty == temp_line){
                if (textx == 0)
                    temp_texty = current_line, temp_textx = 0;
                else{
                    temp_texty = (textx + COLS - num_col -2) / (COLS - num_col -1) + current_line - 1-((texty == start_line_num)?(num_start_line):(0));
                    temp_textx = (textx - 1) % (COLS - num_col -1) + 1;
                }
            }

            for (long int k = 0; k < rest_line; k++){
                wmove(text_win, current_line, 0);
                if (current_line >= LINES - 2)
                    break;
                for (long int j = 0; j < COLS - num_col -1; j++){
                    if ((k + ((temp_line == start_line_num) ? (num_start_line) : (0))) * (COLS - num_col -1) + j >= text[temp_line].size()){
                        for (long int i = j; i <= COLS - num_col -1; i++)
                            wprintw(text_win, "%c", ' ');
                        break;
                    }
                    wprintw(text_win, "%c", text[temp_line][(k + ((temp_line == start_line_num)?(num_start_line):(0))) * (COLS - num_col -1) + j]);
                }
                current_line++;
            }
            temp_line++;
        }
    }

    //滚动情况下的打印
    if(flag_scroll) {
        for (long int i = 0; i < LINES - 2; i++){
            wmove(text_win, i, 0);//将光标移到numcol处，给numwin留出位置
            templine = start_line_num + i;
            //如果实际行数正常，正常输出
            if (templine < line_num) {
                for (long int j = 0; j <=COLS-num_col-1; j++){
                    tempcol =start_col_num + j;
                    if (textx==tempcol && texty==templine){
                        temp_textx = j;temp_texty = i;
                    }
                    if (tempcol< text[templine].size()){
                        wprintw(text_win, "%c", text[templine][tempcol]);
                    }else{
                        wprintw(text_win, "%c", ' ');
                    }
                }
            }

            //如果实际行数太少，用空格来填补空白免得过于尴尬
            if (templine >= line_num) {
                for (long int j = 0; j <= COLS-num_col-1; j++){
                    wprintw(text_win, "%c", ' ');
                }
            }
        }
    }
    //调整光标，刷新文本界面
    wmove(text_win, temp_texty, temp_textx);
    wrefresh(text_win);
}

void numprint() {

    //对数字窗口进行空白初始化
    for (long int i = 0; i < line_num; i++){
        wmove(num_win, i, 0);
        for (long int k = 0; k < num_col; k++)
            wprintw(num_win, "%c", ' ');
    }

    if(flag_break) {

        long int line_cnt = 0, line_cur = start_line_num, start_line_cur = num_start_line;
        while (line_cnt < LINES - 2){
            if (line_cur >= line_num){
                for (long int i = line_cnt; i < LINES - 2; i++){
                    wmove(num_win, i, 0);
                    wprintw(num_win, "%c", ' ');
                }
                break;
            }

            long int temp_line;
            std::vector<char> num_list({});
            temp_line = line_cur + 1;
            while (temp_line){
                num_list.push_back('0' + temp_line % 10);
                temp_line /= 10;
            }
            wmove(num_win, line_cnt, num_col - 2 - num_list.size());
            for (long int k = num_list.size() - 1; k >= 0; k--)
                wprintw(num_win, "%c", num_list[k]);
            wprintw(num_win, "%c", '|');wprintw(num_win, "%c", ' ');
            start_line_cur = 0;
            line_cnt += get_linenum((long int)text[line_cur++].size()) - start_line_cur;
        }
    }

    //滚动情况下的数字打印
    if (flag_scroll) {
        long int temp_num;

        for (long int i = 0; i < line_num; i++){
            //新建一个numlist来存数
            std::vector<char> num_list({});
            wmove(num_win, i, 0);

            templine = start_line_num + i;

            //将不足显示的数字用空格补上
            if (templine >= line_num){
                for (long int j = i; j < LINES - 2; j++){
                    wmove(num_win, j, 0);
                    wprintw(num_win, "%c", ' ');
                }
                break;
            }

            //类似于进制转换的算法
            temp_num = templine + 1;
            while (temp_num){
                num_list.push_back(int('0') + temp_num % 10);
                temp_num/= 10;
            }

            //打印numlist
            wmove(num_win, i, num_col - 2 - num_list.size());
            for (long int j = num_list.size() - 1; j >= 0; j--) {
                    wprintw(num_win, "%c", num_list[j]);
            }
            wprintw(num_win, "%c", '|');wprintw(num_win, "%c", ' ');
        }
    }

    //刷新数字界面
    wrefresh(num_win);
}

void commandprint() {
    wmove(command_win, 0, 1);
    for (long int k = 0; k < command.size(); k++){
        wprintw(command_win, "%c", command[k]);
    }
    for (long int k = command.size(); k < COLS - 1; k++)
        wprintw(command_win, "%c", ' ');
    wmove(command_win, 0, commandcur + 1);wrefresh(command_win);
}

bool fileprint() {
     COMMAND temp_command = command_history[command_size - 1];
     if (temp_command.num == 0){
        if (!flag_file){
            wmove(command_win, 0, 0);wprintw(command_win, "THERE'S NO FILE EXISTED IN THE WAY,PLEASE INPUT AGAIN");
            return false;
        }else if (flag_readonly && flag_openfile){
            wmove(command_win, 0, 0);wprintw(command_win, "YOU CAN'T DO THIS IN READONLY MODE");
            return false;
        }else{
            flag_openfile = false;
            std::ofstream fwriter(filename);
            for (long int i = 0; i < line_num; i++){
                if (i != 0) {
                    if(flag_r){
                        fwriter << '\r';
                        fwriter << '\n';
                    }else {
                        fwriter << '\n';
                    }
                }
                for (long int k = 0; k < text[i].size(); k++){
                    fwriter << text[i][k];
                }
            }
            fwriter.close();
            wmove(command_win, 0, 0);
            wprintw(command_win, "FILE WAS SAVED:");wprintw(command_win, filename.data());
            return true;
        }
    }else{
        const char *to_filename = temp_command.content[0].c_str();
        if (flag_readonly && flag_openfile){
            wmove(command_win, 0, 0);
            wprintw(command_win, "YOU CAN'T DO THIS IN READONLY MODE");
            return false;
        }else{
            flag_openfile = false;
            std::ofstream fwriter(to_filename);
            for (long int i = 0; i < line_num; i++){
                if (i != 0) {
                    if(flag_r){
                        fwriter << '\r\n';
                    }else {
                        fwriter << '\n';
                    }
                }
                for (long int k = 0; k < text[i].size(); k++){
                    fwriter << text[i][k];
                }
            }
            fwriter.close();
            wmove(command_win, 0, 0);wprintw(command_win, "FILE WAS SAVED:");wprintw(command_win, to_filename);
            return true;
        }
    }
}

void informationprint(std::string mode) {
    //对信息界面作空白化预处理
    wmove(info_win, 0, 0);
    for (long int k = 0; k < COLS; k++)
        wprintw(info_win, "%c", ' ');

    //得到文本当前指针
    getyx(text_win, temp_texty, temp_textx);

    //显示信息并显示
    wmove(info_win, 0, 1);//美观
    wprintw(info_win, "%c", '\"');wprintw(info_win, filename.data());
    wprintw(info_win, "%c", '\"');wprintw(info_win, " LINE:%d COL:%d", texty + 1, textx + 1);
    if(mode=="normal") {
        mvwprintw(info_win, 0, COLS - 13, "NORMAL MODE");
    }
    if(mode=="insert") {
        mvwprintw(info_win, 0, COLS - 13, "INSERT MODE");
    }

    //刷新四个界面
    wrefresh(info_win);wrefresh(command_win);wrefresh(num_win);wrefresh(text_win);
}

void move(int key,std::string mode){
    if (mode!="command"){
        if (key == KEY_UP){
            memory_pos = std::max(textx, memory_pos);
            start_line_num =((texty == start_line_num)?(std::max(start_line_num - 1, 0)):start_line_num),num_start_line=0;
            texty = std::max(texty - 1, 0);
            textx = std::min(memory_pos,int(text[texty].size()));
            start_col_num=(((textx < start_col_num||textx > start_col_num + COLS-num_col-1)&&flag_scroll)?textx:start_col_num);
            if (mode == "normal" && textx == int(text[texty].size())+1 && textx != 0) textx--;
            initialize_print();
        }

        if (key == KEY_DOWN){
            if (texty + 1 < line_num){
                memory_pos = std::max(textx, memory_pos);
                if (flag_break){
                    if (temp_texty + (get_linenum(std::min(memory_pos, (int)text[texty + 1].size()))-get_linenum(textx)+get_linenum((long int)text[texty].size())) >= LINES - 2){ // if it dose goes bottom out of range
                        long int line_cnt_cnt= 0;
                        do{
                            line_cnt_cnt += get_linenum((long int)text[start_line_num++].size());
                        } while (line_cnt_cnt < get_linenum(std::min(memory_pos, (int)text[texty + 1].size()))-get_linenum(textx)+get_linenum((int)text[texty].size()));
                        num_start_line = 0;
                    }
                }
                texty = std::min(texty + 1, line_num);textx = std::min(memory_pos, int(text[texty].size()));
                start_col_num=(((textx < start_col_num||textx > start_col_num + COLS-num_col-1)&&flag_scroll)?textx:start_col_num);
                start_line_num=(((texty == start_line_num + LINES - 2)&&flag_scroll)?(start_line_num+1):start_line_num);
            }
            if (mode == "normal" && textx == int(text[texty].size())+1 && textx != 0)   textx--;
            initialize_print();
        }

        if (key == KEY_LEFT){
            if(flag_break) {
                if (temp_textx <= 1 && temp_texty == 0 && texty == start_line_num && get_linenum(textx) != 1)  num_start_line--;
                textx = std::max(textx - 1, 0);
                if (mode == "normal" && textx == (int)text[texty].size()+1 && textx != 0)  textx--;
            }else {
                if (textx != 0){
                    start_col_num=((temp_textx == 0)?(start_col_num-1):start_col_num);
                    textx=((temp_textx == 0)?textx:(textx-1));
                    if (mode == "normal" && textx == int(text[texty].size())+1 && textx != 0)  textx--;
                }
            }

            initialize_print();
            memory_pos = textx;
            wmove(text_win, temp_texty, temp_textx);
            wrefresh(text_win);
        }

        if (key == KEY_RIGHT){
            if (flag_break){
                if (temp_texty >= COLS -num_col - 1 && temp_texty == LINES - 3 && get_linenum(textx) != get_linenum((long int)text[texty].size() - 1))  num_start_line++;
                textx = std::min(textx + 1, (int)text[texty].size());
                if (mode == "normal" && textx == (int)text[texty].size()+1 && textx != 0)  textx--;
                initialize_print();
                if (temp_textx == COLS - num_col -1 && get_linenum(textx) != get_linenum((long int)text[texty].size()))  temp_texty++, temp_textx = 0;
            }else {
                if (textx != text[texty].size()+1){
                    start_col_num=((temp_textx>= COLS-num_col-1)?(start_col_num+1):start_col_num);
                    textx=((temp_textx>= COLS-num_col-1)?textx:(textx+1));
                }
                if (mode == "normal" && textx==int(text[texty].size())+1 && textx != 0)  textx--;
                initialize_print();
            }
            memory_pos = textx;
            wmove(text_win, temp_texty, temp_textx);
            wrefresh(text_win);
        }

    }else if (mode == "command"){
        if (key == KEY_LEFT){
            commandcur = std::max(0, commandcur - 1);wmove(command_win, 0, commandcur);
        }
        if (key == KEY_RIGHT){
            commandcur = std::min((int)command.size(), commandcur + 1);wmove(command_win, 0, commandcur + 1);
        }
        if (key == KEY_UP){
            command.clear();
            if (command_history.size() == 0)
                return;
            start_command = std::max(0, start_command - 1);
            for (long int k = 0; k < command_history[start_command].cmd.size(); k++){
                command.push_back(command_history[start_command].cmd[k]);
            }
            for (long int i = 0; i < command_history[start_command].num; i++){
                command.push_back(' ');
                for (long int k = 0; k < command_history[start_command].content[i].size(); k++){
                    command.push_back(command_history[start_command].content[i][k]);
                }
            }
            commandcur = command.size();
            commandprint();
        }
        if (key == KEY_DOWN){
            command.clear();
            start_command = std::min(command_size, start_command + 1);
            if (start_command != command_size){
                for (long int k = 0; k < command_history[start_command].cmd.size(); k++){
                    command.push_back(command_history[start_command].cmd[k]);
                }
                for (long int i = 0; i < command_history[start_command].num; i++){
                    command.push_back(' ');
                    for (long int k = 0; k < command_history[start_command].content[i].size(); k++){
                        command.push_back(command_history[start_command].content[i][k]);
                    }
                }
            }else{
                command.clear();
            }
            commandcur = command.size();
            commandprint();
        }
    }
}

void temp_move(int key,std::string mode){
        if (key == KEY_UP){
            start_line_num =((texty == start_line_num)?(std::max(start_line_num - 1, 0)):start_line_num),num_start_line=0;
            texty = std::max(texty - 1, 0);
            textx = std::min(memory_pos,int(text[texty].size()));
            start_col_num=(((textx < start_col_num||textx > start_col_num + COLS-num_col-1)&&flag_scroll)?textx:start_col_num);
            if (mode == "normal" && textx == int(text[texty].size())+1 && textx != 0) textx--;
            initialize_print();
        }

        if (key == KEY_DOWN){
            if (texty + 1 < line_num){
                //memory_pos = std::max(textx, memory_pos);
                if (flag_break){
                    if (temp_texty + (get_linenum(std::min(memory_pos, (int)text[texty + 1].size()))-get_linenum(textx)+get_linenum((long int)text[texty].size())) >= LINES - 2){ // if it dose goes bottom out of range
                        long int line_cnt_cnt= 0;
                        do{
                            line_cnt_cnt += get_linenum((long int)text[start_line_num++].size());
                        } while (line_cnt_cnt < get_linenum(std::min(memory_pos, (int)text[texty + 1].size()))-get_linenum(textx)+get_linenum((int)text[texty].size()));
                        num_start_line = 0;
                    }
                }
                texty = std::min(texty + 1, line_num);textx = std::min(0,int(text[texty].size()));
                start_col_num=(((textx < start_col_num||textx > start_col_num + COLS-num_col-1)&&flag_scroll)?textx:start_col_num);
                start_line_num=(((texty == start_line_num + LINES - 2)&&flag_scroll)?(start_line_num+1):start_line_num);
            }
            if (mode == "normal" && textx == int(text[texty].size())+1 && textx != 0)   textx--;
            initialize_print();
        }
}

int get_linenum(int length){
    return((length!=0)?((length - 1 + COLS - num_col -1)/(COLS - num_col -1)):1);
}

void substitute(std::string string_input, std::string string_output) {
    wmove(command_win, 0, 0);wrefresh(command_win);
    for (long int i = 0; i < line_num; i++){
        long int temp_x = 0;
        while (temp_x <= (long int)text[i].size() - (long int)string_input.size()){
            for (long int j = temp_x; j < temp_x + (long int)string_input.size(); j++){
                if (text[i][j] != string_input[j - temp_x]){
                    temp_x++;
                    break;
                }
                if (j == temp_x + (long int)string_input.size() - 1){
                    for (long int k = 0; k < (long int)string_input.size(); k++)
                        text[i].erase(text[i].begin() + temp_x);
                    for (long int k = 0; k < (long int)string_output.size(); k++)
                        text[i].insert(text[i].begin() + temp_x + k, string_output[k]);
                    temp_x += string_output.size();
                    break;
                }
            }
        }
    }
    num_start_line=0;initialize_print();
}

void normal_mode() {
    bool flag_delete=false;
    textx=((textx == (long int)(text[texty].size()) && textx != 0)?(textx-1):textx);
    initialize_print();

    while (true) {
        //更新信息界面
        informationprint(mode);
        int ch=getch();
        if (ch == ':') {
            flag_delete = false;mode = "command";break;
        }

        if ((ch == 'i' ) && !flag_readonly){
            flag_delete = false;mode="insert";break;
        }

        if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT){
            flag_delete = false;move(ch, "normal");
        }

        if (ch == '0'){
            flag_delete = false;
            while (textx!= 0){
                move(KEY_LEFT, "normal");
            }
            initialize_print();
        }

        if (ch == '$'){
            flag_delete = false;
            while (textx!= std::max((int)text[texty].size(), 0)){
                move(KEY_RIGHT, "normal");
            }
            initialize_print();
        }

        if (ch == 'b'){
            flag_delete = false;
            bool flag_end2=false;
            if (textx == 0 && texty != 0 && flag_break){
                texty--;textx = std::max((int)text[texty].size(), 0);
                start_line_num=((temp_texty == 0)?(start_line_num-1):start_line_num);
                num_start_line=((temp_texty == 0)?(get_linenum((long int)text[start_line_num].size()) - 1):num_start_line);
                initialize_print();
            }else {
                if (textx == 0 && texty != 0 && flag_scroll){
                    textx = text[texty - 1].size();texty--;
                    start_col_num=((textx < start_col_num||textx > start_col_num + COLS-num_col-1)?textx:start_col_num);
                    //temp_move(KEY_UP,"normal");
                    flag_end2=true;
                    initialize_print();
                }
                bool flag_end = false;
                if (!((textx == text[texty].size())&&(flag_end2))) {
                    while (true){
                        if (textx==0)
                            break;
                        if (!flag_end && text[texty][textx - 1] == ' ')
                            flag_end = true;
                        else if (flag_end && text[texty][textx - 1] != ' ')
                            break;
                        move(KEY_LEFT, "normal");
                    }
                    initialize_print();
                }
                initialize_print();
            }
        }

        if (ch == 'w'){
            flag_delete = false;
            bool flag_start = false;

            if (text[texty].size() == 0 && textx == 0){
                    temp_move(KEY_DOWN,"normal");
                    start_line_num=(((temp_texty == LINES -3)&&(flag_scroll))?(start_line_num+1):start_line_num);
                    num_start_line=(((temp_texty == LINES -3)&&(flag_break))?(num_start_line+1):num_start_line);
                    initialize_print();
            }else{
                while (true){
                    if (textx == text[texty].size()){
                        if (texty != line_num - 1){
                            temp_move(KEY_DOWN,"normal");
                            start_col_num=((flag_scroll)?(0):start_col_num);
                        }
                        num_start_line=(((temp_texty == LINES -3)&&(flag_break))?(num_start_line+1):num_start_line);
                        //initialize_print();
                        //wrefresh(text_win);wrefresh(num_win);
                        break;
                    }
                    if (!flag_start && text[texty][textx] == ' ')
                        flag_start = true;
                    else if (flag_start && text[texty][textx] != ' ')
                        break;
                    move(KEY_RIGHT, "normal");
                }
                initialize_print();
            }
        }

        if (ch == 'd' && !flag_readonly){
            if (flag_delete == false)
                flag_delete = true;
            else{
                flag_delete = false;
                if (texty == start_line_num){
                    num_start_line=((flag_break)?0:num_start_line);
                    start_col_num=((flag_scroll)?0:start_col_num);
                    textx = 0;initialize_print();
                }
                if (texty == line_num - 1){
                    start_col_num = ((flag_scroll)?(0):start_col_num);
                    start_line_num=((texty == line_num)?std::max(0, texty - 1):start_line_num);
                    text.erase(text.begin() + texty);
                    line_num--;texty--;
                    if (line_num == 0)  text.push_back(temp_line), texty = 0, textx = 0, line_num++;
                    initialize_print();
                }else{
                    start_col_num = ((flag_scroll)?(0):start_col_num);
                    text.erase(text.begin() + texty);
                    line_num--;textx = 0;
                    initialize_print();
                }
            }
        }
    }
}

void insert_mode() {
    wmove(text_win, temp_texty, temp_textx);
    while (true){
        informationprint(mode);
        int ch = getch();
        if (ch == KEY_ESC){
            mode = "normal";break;
        }else if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT){
            move(ch, "insert");
        }else if (ch == KEY_BACKSPACE){
            if (textx == 0){
                if (texty != 0){
                    flag_openfile = true;
                    start_line_num=((temp_texty==0)?(start_line_num-1):start_line_num);
                    num_start_line=((temp_texty==0)?0:num_start_line);
                    textx = text[texty - 1].size();
                    for (long int i = 0; i < text[texty].size(); i++){
                        text[texty - 1].push_back(text[texty][i]);
                    }
                    for (long int i = 0; i < text[texty].size(); i++){
                        text[texty].pop_back();
                    }
                    text.erase(text.begin() + texty);
                    line_num--;texty--;
                    start_col_num=(((textx < start_col_num||textx > start_col_num + COLS-num_col-1)&&flag_scroll)?textx:start_col_num);
                }
            }
            else if (flag_scroll&&temp_textx == 0)
                start_col_num--;
            else{
                flag_openfile = true;
                text[texty].erase(text[texty].begin() + textx - 1);
                textx--;
            }
            initialize_print();
        }else if (ch == KEY_DC){
            textx=((textx==text[texty].size())?(textx):(textx+1));
            if((textx==text[texty].size())&&(text[texty].size()!=0)) {
                texty++;textx=0;
            }
            if (textx == 0){
                if (texty != 0){
                    flag_openfile = true;
                    start_line_num=((temp_texty==0)?(start_line_num-1):start_line_num);
                    num_start_line=((temp_texty==0)?0:num_start_line);
                    textx = text[texty - 1].size();
                    for (long int i = 0; i < text[texty].size(); i++){
                        text[texty - 1].push_back(text[texty][i]);
                    }
                    for (long int i = 0; i < text[texty].size(); i++){
                        text[texty].pop_back();
                    }
                    text.erase(text.begin() + texty);
                    line_num--;texty--;
                    start_col_num=(((textx < start_col_num||textx > start_col_num + COLS-num_col-1)&&flag_scroll)?textx:start_col_num);
                }
            }else if (flag_scroll&&temp_textx == 0)
                start_col_num--;
            else {
                flag_openfile = true;
                text[texty].erase(text[texty].begin() + textx - 1);
                textx--;
            }
            if(textx==0) {
                if (texty == start_line_num){
                    num_start_line=((flag_break)?0:num_start_line);
                    start_col_num=((flag_scroll)?0:start_col_num);
                    textx = 0;initialize_print();
                }
                if (texty == line_num - 1){
                    start_col_num = ((flag_scroll)?(0):start_col_num);
                    start_line_num=((texty == line_num)?std::max(0, texty - 1):start_line_num);
                    text.erase(text.begin() + texty);
                    line_num--;texty--;
                    if (line_num == 0)  text.push_back(temp_line), texty = 0, textx = 0, line_num++;
                    initialize_print();
                }else{
                    start_col_num = ((flag_scroll)?(0):start_col_num);
                    text.erase(text.begin() + texty);
                    line_num--;textx = 0;
                    initialize_print();
                }
            }
            initialize_print();
        }else if (ch == '\n'){
            flag_openfile = true;
            memory_pos = std::max(textx, memory_pos);
            text.insert(text.begin() + texty + 1, temp_line);
            long int originline_size = text[texty].size();
            for (long int i = textx; i < originline_size; i++){
                text[texty + 1].push_back(text[texty][i]);
            }
            for (long int i = textx; i < originline_size; i++){
                text[texty].pop_back();
            }
            texty++, textx = 0;line_num++;temp_texty++;temp_textx = 0;
            start_line_num=((temp_textx >= LINES - 2 )?(start_line_num+1):start_line_num);
            num_start_line=((temp_textx >= LINES - 2 )?0:num_start_line);
            temp_texty=((temp_textx >= LINES - 2 )?(temp_texty-1):temp_texty);
            start_col_num = ((flag_scroll)?0:start_col_num);
            initialize_print();
            wmove(text_win, temp_texty, temp_textx);
        }else if (ch == '\t'){
            for (long int i = 1; i <= 4; i++) {
                if (flag_break){
                    num_start_line=((temp_textx >= COLS - num_col -1 && temp_texty >= LINES - 3)?(num_start_line+1):num_start_line);
                    initialize_print();
                    if (temp_textx >= COLS - num_col){
                        wmove(text_win, temp_texty + 1 - ((temp_texty >= LINES - 3)?(get_linenum((long int)text[start_line_num - 1].size()) - num_start_line):0), 1);
                    }else
                        wmove(text_win, temp_texty, temp_textx);
                    wrefresh(text_win);
                }else {
                    start_col_num=((temp_textx >= COLS - num_col -1)?(start_col_num+1):start_col_num);
                    initialize_print();
                }
            }
        }else{
            flag_openfile = true;
            text[texty].insert(text[texty].begin() + textx, ch);
            textx++;
            if (flag_break){
                num_start_line=((temp_textx >= COLS - num_col -1 && temp_texty >= LINES - 2)?(num_start_line+1):num_start_line);
                initialize_print();
                if (temp_textx >= COLS - num_col){
                    wmove(text_win, temp_texty + 1 -((temp_texty + 1 >= LINES - 2)?(get_linenum((long int)text[start_line_num - 1].size()) - num_start_line):0),1);
                }
                else
                    wmove(text_win, temp_texty, temp_textx);
                wrefresh(text_win);
            }else {
                start_col_num=((temp_textx >= COLS - num_col -1)?(start_col_num+1):start_col_num);
                initialize_print();
            }
        }
    }
}

void command_mode() {
    commandcur = 0;
    wmove(info_win, 0, 0);
    for (long int k = 0; k < COLS; k++)
        wprintw(info_win, "%c", ' ');
    mvwprintw(command_win, 0, 0, "%c", ':');
    for (long int k = 1; k < COLS; k++)
        wprintw(command_win, "%c", ' ');
    start_command = command_size;command.clear();
    wmove(command_win, 0, 1);
    while (true){
        mvwprintw(info_win, 0, COLS - 13, "COMMAND MODE", mode);
        wmove(command_win, 0, commandcur + 1);
        wrefresh(info_win);wrefresh(command_win);
        int ch = getch();
        if (ch == KEY_ESC){
            mvwprintw(command_win, 0, 0, "%c", ' ');
            for (long int k = 1; k < COLS; k++)
                wprintw(command_win, "%c", ' ');
            mode="normal";
            command.clear();commandprint();
            break;
        }else if (ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_DOWN){
            move(ch, "command");
        }else if (ch == KEY_BACKSPACE){
            if (commandcur > 0){
                command.erase(command.begin() + commandcur - 1);
                commandcur--;commandprint();
                wrefresh(command_win);
            }
        }else if (ch == '\n'){
            mvwprintw(command_win, 0, 0, "%c", ' ');
            for (long int k = 1; k < COLS; k++)
                wprintw(command_win, "%c", ' ');
                if (command.size() != 0) {
                    COMMAND temp_command;
                    bool sub_start = false;
                    long int k = 0;
                    while (command[k] != ' ' && k < command.size()){
                        temp_command.cmd.append(std::string(1, command[k]));
                        k++;
                    }
                    for (; k < command.size(); k++){
                        if (temp_command.cmd == "sub"){
                            if (command[k] == ' ' && !sub_start)
                                continue;
                            if (!sub_start && command[k] == '\"')
                                temp_command.num++, sub_start = true;
                            else if (sub_start && command[k] == '\"')
                                sub_start = false;
                        }else if (command[k] == ' '){
                            temp_command.num++;continue;
                        }
                        temp_command.content[temp_command.num - 1].append(std::string(1, command[k]));
                    }
                    if (command_size == 0 || (!command.empty() && !(temp_command == command_history[command_size - 1]))){
                        command_history.push_back(temp_command);
                        command_size++;start_command = command_size;
                    }
                    command.clear();wmove(command_win, 0, 1);
                    commandprint();

                    if (temp_command.cmd == "w"){
                        fileprint();
                    }else if (temp_command.cmd == "q"){
                        if (!flag_openfile) {
                                wmove(command_win, 0, 0);wprintw(command_win, "DO YOU WANT TO QUIT IT?PLEASE INPUT Y/N:");
                            wrefresh(command_win);
                            bool flag_ch=false;
                            char temp_ch=getch();
                            while (!flag_ch){
                                if (temp_ch == 'Y') {
                                    flag_endloop=true;
                                    flag_ch=true;
                                }else if (temp_ch == 'N') {
                                    flag_endloop=false;
                                    flag_ch=true;
                                    wmove(command_win, 0, 0);
                                    for (long int k = 1; k < COLS; k++)
                                        wprintw(command_win, "%c", ' ');
                                    wrefresh(command_win);
                                }else {
                                    wmove(command_win, 0, 0);
                                    for (long int k = 1; k < COLS; k++)
                                        wprintw(command_win, "%c", ' ');
                                    wmove(command_win, 0, 0);wprintw(command_win, "PLEASE INPUT Y/N AGAIN:");wrefresh(command_win);
                                    temp_ch=getch();
                                }
                            }
                        }else{
                            wmove(command_win, 0, 0);wprintw(command_win, "THE FILE WAS NOT OPENED");
                        }
                    }else if (temp_command.cmd == "q!") {
                        flag_endloop = true;
                    }else if (temp_command.cmd == "wq"){
                        if(fileprint()&&!flag_openfile) flag_endloop=true;
                    }else if (temp_command.cmd == "jmp"){
                        if (temp_command.num >= 1) {
                            long int linenum = 0;
                            for (long int i = 0; i < temp_command.content[0].size(); i++){
                                linenum *= 10;linenum += temp_command.content[0][i] - '0';
                            }
                            linenum--;
                            if (linenum >= 0 && linenum <= line_num - 1){
                                start_line_num = linenum,num_start_line=0;
                                start_col_num = (flag_scroll?0:start_col_num);
                                texty = start_line_num, textx = 0;
                                initialize_print();
                            }else{
                                wmove(command_win, 0, 0);wprintw(command_win, "YOU INPUT AN INVALID LINE NUMBER,PLEASE TRY AGAIN");
                            }
                        }
                    }else if (temp_command.cmd == "sub"){
                        if (temp_command.num >= 2 && temp_command.content[0].size() >= 2 && temp_command.content[1].size() >= 2){
                            if (temp_command.content[0][0] == '\"' && temp_command.content[0][temp_command.content[0].size() - 1] == '\"' && temp_command.content[1][0] == '\"' and temp_command.content[1][temp_command.content[1].size() - 1] == '\"'){
                                std::string string_input, string_output;
                                string_input = temp_command.content[0].substr(1, temp_command.content[0].size() - 2);
                                string_output = temp_command.content[1].substr(1, temp_command.content[1].size() - 2);
                                if (string_input.size() == 0){
                                    wmove(command_win, 0, 0);wprintw(command_win, "NO QUALIFIED WORDS WAS GOT,PLEASE INPUT THE VALID WORDS");
                                }else if (!flag_readonly){
                                    flag_openfile = true;
                                    substitute(string_input, string_output);
                                    wmove(text_win, 0, 0);
                                    wmove(command_win, 0, 0);wprintw(command_win, "SUBSITUTED OPERATION WAS DONE(NOTICE:MAYBE YOUR INPUT DO NOT EXIST)");
                                    wrefresh(command_win);wrefresh(text_win);
                            }else{
                                wmove(command_win, 0, 0);wprintw(command_win, "YOU CAN'T DO THIS IN READONLY MODE");
                            }
                        }else{
                            wmove(command_win, 0, 0);wprintw(command_win, "NO QUALIFIED WORDS WAS GOT,PLEASE INPUT THE VALID WORDS");
                        }
                    }else{
                        wmove(command_win, 0, 0);wprintw(command_win, "NO QUALIFIED WORDS WAS GOT,PLEASE INPUT THE VALID WORDS");
                    }
                }
            }
            mode="normal";
            commandcur = 0;command.clear();wrefresh(command_win);
            break;
        }else{
            command.insert(command.begin() + commandcur, ch);
            commandcur++;commandprint();
        }
    }
}