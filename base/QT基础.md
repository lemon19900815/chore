# QT基础

## 1. qss

reference：

- [qss语法介绍](https://blog.csdn.net/qq_51762652/article/details/129224126)

- [qss语法详解](https://blog.csdn.net/soldier_d/article/details/118356671?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522170177221916800185890083%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=170177221916800185890083&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduend~default-1-118356671-null-null.142^v96^pc_search_result_base2&utm_term=qt%20setstylesheet%E4%BE%8B%E5%AD%90&spm=1018.2226.3001.4187)



## 2. 基础组件用法

- [QTableWidget用法详解](https://blog.csdn.net/zheng19880607/article/details/132840104)


## 3. 翻译（语言家）

- 源码：
```c++
QLabel *label = new QLabel;
label->setText(tr("Start"));
```

- 生成 ts：
```bash
# 翻译src目录下的所有文件
lupdate src -ts translations/app_zh_CN.ts
```

- Qt Linguist 翻译：
![](img/QT基础.png)

- 生成 qm：
```bash
lrelease translations/app_zh_CN.ts
```

- 程序加载：
```c++
QTranslator translator;
translator.load("app_zh_CN.qm");
app.installTranslator(&translator);
```

- Windows 下推荐写一个 bat：`update_translation.bat`
```bat
@echo off

set QT_BIN=C:\Qt\6.8.0\msvc2022_64\bin

%QT_BIN%\lupdate.exe . -ts translations\app_zh_CN.ts -no-obsolete
%QT_BIN%\lrelease.exe translations\app_zh_CN.ts

pause
```

多个文件时，遍历执行lupdate和lrelease：
```bat
@echo off

set QT_BIN=C:\Qt\Qt5.12.8\5.12.8\msvc2015_64\bin

for /r %%F in (*.ts) do (
    echo update file: %%F
    %QT_BIN%\lupdate.exe . -ts %%F
    %QT_BIN%\lrelease.exe %%F
)

pause
```
## 4. 其他

### 4.1 设置QLabel行间距

由于qlabel本身不支持行间距的样式表属性line-height，所有需要通过html来实现行间距的设置。如下：

```c++
QString tips = u8"<html><body><p style='line-height: 150%'>升级完成！<br>请点击【重启】按钮执行重启。</p></body></html>";
label.setText(tips);
```

### 4.2 UI布局

- 参与布局的元素需要在QWidget上或者QFrame上，这样才可以设置该widget的尺寸（最小或最大尺寸），缩放时才能保证不会变形；
- 窗口左右缩放，需要在Designer上选择水平或垂直分裂布局器；



### 4.3 基础用法

1. 设置窗口风格模式

   ```c++
   static void set_window_style(QWidget *widget)
   {
       // 去掉窗口默认的title栏，并设置背景透明
       widget->setWindowFlags(Qt::FramelessWindowHint);
       widget->setAttribute(Qt::WA_TranslucentBackground);
   }
   ```

   **注意：如果在linux下开发app，直接使用虚拟机背景透明属性会生效，但是如果使用诸如Mobax之类的ssh工具连接虚拟机，则可能造成窗口透明不生效；**

   

2. 设置窗口居中显示

   ```c++
   static void set_window_center(QWidget *widget)
   {
       QScreen *screen = QGuiApplication::primaryScreen();
       QRect screenGeometry = screen->availableGeometry();
       widget->move((screenGeometry.width() - widget->width()) / 2,
                    (screenGeometry.height() - widget->height()) / 2);
   }
   ```

   

3. 设置按钮最大文本宽度，超出宽度使用"..."替代，并设置tooltip显示

   ```c++
   static void set_button_text_max_width(QPushButton &btn, int32_t max_width)
   {
       auto text = btn.text();
       btn.setToolTip(text);
   
       QFontMetrics fm(btn.font());
       int textWidth = fm.width(btn.text()); // 计算文本的宽度
       if (textWidth > max_width)
       {
           // 使用省略号替代超出部分
           QString ellipsizedText = fm.elidedText(text, Qt::ElideRight, max_width);
           btn.setText(ellipsizedText); // 更新QLabel的文本内容
       }
   }
   ```

   

4. 计算文本在字体font下显示需要占用的宽度

   ```c++
   // 计算文本在字体font下显示需要占用的宽度
   static int32_t calc_text_width(const QFont &font, const QString &text)
   {
       return QFontMetrics(font).width(text);
   }
   
   // 计算QPushButton按钮文本显示宽度
   static int32_t calc_btn_text_width(QPushButton *btn)
   {
       return calc_text_width(btn->font(), btn->text());
   }
   ```

   

5. 窗口下的按钮根据按钮内容自适应：***不支持对隐藏按钮进行自适应***；

   ```c++
   // 窗口下的按钮根据翻译内容自适应
   // space: 2个按钮之间的空白区域 [btn1]--space--[btn2]
   // margin: 按钮文本到边缘的留白区域，[--margin--text--margin--]
   static void adjust_btn_width(QWidget *parent,
                                int32_t space, int32_t margin,
                                const std::vector<QPushButton *> &btns)
   {
       if (btns.empty() || !parent)
       {
           return;
       }
   
       auto btn_width = 0;
       for (auto &btn : btns)
       {
           btn->adjustSize();
           btn_width = std::max<int32_t>(btn_width, calc_btn_text_width(btn));
       }
   
       auto btn_count = static_cast<int32_t>(btns.size());
   
       auto space_width = (btn_count - 1) * space;
       auto margin_width = btn_count * margin * 2; // 2侧margin
       auto win_width = btn_width * btn_count + space_width + margin_width;
   
       parent->resize(win_width, parent->height());
   }
   ```

   

6. 