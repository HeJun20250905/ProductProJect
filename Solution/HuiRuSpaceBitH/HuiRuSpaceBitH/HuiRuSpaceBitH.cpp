#include "HuiRuSpaceBitH.h"
#include <QMessageBox>
#include <QDebug>
#include <QMouseEvent>

#include "opencv2\core.hpp"
#include "opencv2\opencv.hpp"

#ifdef _DEBUG
#define  LIB_PATH     "..\\OpenCV341_Win32\\Library\\Debug"
#define  LIB_SUFFIX   "d.lib" 
#else
#define  LIB_PATH     "..\\OpenCV341_Win32\\Library\\Release"
#define  LIB_SUFFIX   ".lib"  
#endif

#pragma comment(lib,  LIB_PATH   "\\opencv_aruco341"           LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_bgsegm341"          LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_bioinspired341"     LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_calib3d341"         LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_ccalib341"          LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_core341"            LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_datasets341"        LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_dnn341"             LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_dnn_objdetect341"   LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_dpm341"             LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_face341"            LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_features2d341"      LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_flann341"           LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_fuzzy341"           LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_hfs341"             LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_highgui341"         LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_imgcodecs341"       LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_imgproc341"         LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_img_hash341"        LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_line_descriptor341" LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_ml341"              LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_objdetect341"       LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_optflow341"         LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_phase_unwrapping341" LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_photo341"           LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_plot341"            LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_reg341"             LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_rgbd341"            LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_saliency341"        LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_shape341"           LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_stereo341"          LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_stitching341"       LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_structured_light341" LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_superres341"        LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_surface_matching341" LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_text341"            LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_tracking341"        LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_video341"           LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_videoio341"         LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_videostab341"       LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_xfeatures2d341"     LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_ximgproc341"        LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_xobjdetect341"      LIB_SUFFIX)
#pragma comment(lib,  LIB_PATH   "\\opencv_xphoto341"          LIB_SUFFIX)

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif                                                //默认使用utf-8的编码格式

HuiRuSpaceBitH::HuiRuSpaceBitH(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::HuiRuSpaceBitHClass())
{
    ui->setupUi(this);

    // 设置mainWindow标题
    this->setWindowTitle("HuiRuSpaceBitH");

    // 设置dialog标题风格
    this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    //this->setWindowIcon(QIcon(":/icon/HuiRuSpaceBitH.ico"));

    ui->label_cvMat0->installEventFilter(this);
    ui->label_cvMat1->installEventFilter(this);
    ui->label_cvMat2->installEventFilter(this);
    ui->label_cvMat3->installEventFilter(this);
    ui->label_cvMat4->installEventFilter(this);
    ui->label_cvMat5->installEventFilter(this);
    
    ui->label_Window0->installEventFilter(this);
    ui->label_Window1->installEventFilter(this);
    ui->label_Window2->installEventFilter(this);
    ui->label_Window3->installEventFilter(this);

    m_cvMatLabels.push_back(ui->label_cvMat0);
    m_cvMatLabels.push_back(ui->label_cvMat1);
    m_cvMatLabels.push_back(ui->label_cvMat2);
    m_cvMatLabels.push_back(ui->label_cvMat3);
    m_cvMatLabels.push_back(ui->label_cvMat4);
    m_cvMatLabels.push_back(ui->label_cvMat5);
    
    m_cvMatIndex = 0;

    m_windowLabels.push_back(ui->label_Window0);
    m_windowLabels.push_back(ui->label_Window1);
    m_windowLabels.push_back(ui->label_Window2);
    m_windowLabels.push_back(ui->label_Window3);
    
    m_windowIndex = 0;

    m_recordIndex = 0;
}

HuiRuSpaceBitH::~HuiRuSpaceBitH()
{
    delete ui;
}

bool HuiRuSpaceBitH::eventFilter(QObject* obj, QEvent* e)
{
    // 判断事件是否来自目标控件
    if (obj == ui->label_cvMat0 ||
        obj == ui->label_cvMat1 ||
        obj == ui->label_cvMat2 ||
        obj == ui->label_cvMat3 ||
        obj == ui->label_cvMat4 ||
        obj == ui->label_cvMat5 ||
        obj == ui->label_Window0 ||
        obj == ui->label_Window1 ||
        obj == ui->label_Window2 || 
        obj == ui->label_Window3 )
    {
        // 判断事件类型是否为鼠标按下
        if (e->type() == QEvent::MouseButtonPress)
        {
            if (obj == ui->label_cvMat0)
            {
                m_cvMatIndex = 0;
                m_recordIndex = 0;
                ui->textEdit_cvMat->setText("cvMat0");
                ui->textEdit_Record->setText(m_recordString[0].join("\n"));
            }
            else if (obj == ui->label_cvMat1)
            {
                m_cvMatIndex = 1;
                m_recordIndex = 1;
                ui->textEdit_cvMat->setText("cvMat1");
                ui->textEdit_Record->setText(m_recordString[1].join("\n"));
            }
            else if (obj == ui->label_cvMat2)
            {
                m_cvMatIndex = 2;
                m_recordIndex = 2;
                ui->textEdit_cvMat->setText("cvMat2");
                ui->textEdit_Record->setText(m_recordString[2].join("\n"));
            }
            else if (obj == ui->label_cvMat3)
            {
                m_cvMatIndex = 3;
                m_recordIndex = 3;
                ui->textEdit_cvMat->setText("cvMat3");
                ui->textEdit_Record->setText(m_recordString[3].join("\n"));
            }
            else if (obj == ui->label_cvMat4)
            {
                m_cvMatIndex = 4;
                m_recordIndex = 4;
                ui->textEdit_cvMat->setText("cvMat4");
                ui->textEdit_Record->setText(m_recordString[4].join("\n"));
            }
            else if (obj == ui->label_cvMat5)
            {
                m_cvMatIndex = 5;
                m_recordIndex = 5;
                ui->textEdit_cvMat->setText("cvMat5");
                ui->textEdit_Record->setText(m_recordString[5].join("\n"));
            }
            else if (obj == ui->label_Window0)
            {
                m_windowIndex = 0;
                ui->textEdit_Window->setText("Window0");
            }
            else if (obj == ui->label_Window1)
            {
                m_windowIndex = 1;
                ui->textEdit_Window->setText("Window1");
            }
            else if (obj == ui->label_Window2)
            {
                m_windowIndex = 2;
                ui->textEdit_Window->setText("Window2");
            }
            else if (obj == ui->label_Window3)
            {
                m_windowIndex = 3;
                ui->textEdit_Window->setText("Window3");
            }

            // 在这里编写你的点击响应逻辑
            qDebug() << "Label 被点击了！";

            // 返回true表示该事件已被处理，不再向下传递
            return true;
        }
    }

    // 对于其他控件或其他类型的事件，交由父类默认处理
    return QWidget::eventFilter(obj, e);
}

void HuiRuSpaceBitH::on_pushButton_LoadSource_clicked()
{
    QString text = ui->pushButton_LoadSource->text();
    m_recordString[m_recordIndex].append(text);
    ui->textEdit_Record->setText(m_recordString[m_recordIndex].join("\n"));
}


