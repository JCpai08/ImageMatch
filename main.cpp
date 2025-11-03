// main.cpp
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Progress.H>
#include <fstream>
#include "src/ImageProcessor.hpp"

// 全局变量
ImageProcessor processor;
Fl_Input *file1_input, *file2_input;
Fl_Choice *window_size_choice;
Fl_Multiline_Output *result_box;
Fl_Progress *progress_bar;      // 进度条

// 回调函数声明
void browse_file1_cb(Fl_Widget*, void*);
void browse_file2_cb(Fl_Widget*, void*);
void calculate_cb(Fl_Widget*, void*);

// 回调函数实现保持不变
void browse_file1_cb(Fl_Widget*, void*) {
    const char *filename = fl_file_chooser("选择第一个文件", "*", "");
    if (filename) {
        file1_input->value(filename);
        processor.setFile1(filename);
    }
}

void browse_file2_cb(Fl_Widget*, void*) {
    const char *filename = fl_file_chooser("选择第二个文件", "*", "");
    if (filename) {
        file2_input->value(filename);
        processor.setFile2(filename);
    }
}

void window_size_cb(Fl_Widget*, void*) {
    int selected = window_size_choice->value();
    switch(selected) {
        case 0: processor.windowSize = 3; break;
        case 1: processor.windowSize = 5; break;
        case 2: processor.windowSize = 7; break;
        default: processor.windowSize = 5; break;
    }
}

void calculate_cb(Fl_Widget*, void*) {
    // 重置进度
    progress_bar->value(0.0);
    progress_bar->redraw();    
    Fl::check();

    if (processor.processImages(progress_bar)) {
        std::ofstream out("result.txt");
        out << processor.result;
        out.close();
        result_box->value((processor.output+"结果已保存到 result.txt").c_str());
    } else {
        fl_alert("%s", processor.output.c_str());
    }
    result_box->redraw();
}

int main(int argc, char **argv) {
    Fl_Window *window = new Fl_Window(400, 300, "图像匹配工具");
    
    // UI组件创建代码保持不变
    Fl_Box *file1_label = new Fl_Box(20, 20, 80, 25, "图像1:");
    file1_label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    
    file1_input = new Fl_Input(100, 20, 140, 25);
    file1_input->readonly(true);
    
    Fl_Button *browse1_btn = new Fl_Button(250, 20, 80, 25, "浏览...");
    browse1_btn->callback(browse_file1_cb);
    
    Fl_Box *file2_label = new Fl_Box(20, 60, 80, 25, "图像2:");
    file2_label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    
    file2_input = new Fl_Input(100, 60, 140, 25);
    file2_input->readonly(true);
    
    Fl_Button *browse2_btn = new Fl_Button(250, 60, 80, 25, "浏览...");
    browse2_btn->callback(browse_file2_cb);
    
    Fl_Button *calc_btn = new Fl_Button(20, 100, 100, 30, "开始匹配");
    calc_btn->callback(calculate_cb);

    Fl_Box *window_size_label = new Fl_Box(140, 100, 120, 25, "匹配窗口大小:");
    window_size_label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

    window_size_choice = new Fl_Choice(260, 100, 60, 25);
    window_size_choice->add("3");
    window_size_choice->add("5");
    window_size_choice->add("7");
    window_size_choice->value(1); // 默认选择第一个选项
    window_size_choice->callback(window_size_cb);
    processor.windowSize = 5; // 设置默认值

    // 添加进度条
    progress_bar = new Fl_Progress(20, 135, 300, 20);
    progress_bar->minimum(0.0);
    progress_bar->maximum(100.0);
    progress_bar->value(0.0);
    progress_bar->label("进度显示");
    
    result_box = new Fl_Multiline_Output(20, 160, 300, 110);
    result_box->box(FL_DOWN_BOX);
    result_box->value("请选择待匹配图像");

    window->end();
    window->show(argc, argv);
    
    return Fl::run();
}