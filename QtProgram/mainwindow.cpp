#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainterPath>
#include <QRegion>
#include <QTimer>
#include <QWindow>
#include <QWidget>
#include <windows.h>
#include <QProcess>
#include <iostream>
#include <fstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralwidget->setMouseTracking(true);
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, [=](){Init();});
    t->setSingleShot(true);
    t->start(10);
//    MainWindow::controlWindowScale();
    connect(ui->adjSizeBtn, &QPushButton::clicked, this, [=](){controlWindowScale();});
}

void MainWindow::Init(){



    /* Create main widget and set mask, style sheet and shadow */
    QPainterPath path;
    path.addRoundedRect(ui->mainWidget->rect(), cornerRadius - 1, cornerRadius - 1);
    QRegion mask(path.toFillPolygon().toPolygon());
    ui->mainWidget->setMask(mask);

    QString mainStyle;
    ui->mainWidget->setObjectName("mainWidget");
    mainStyle = "QWidget#mainWidget{background-color:" + mainBackGround.name() + QString::asprintf(";border-radius:%dpx", cornerRadius) + "}";
    ui->mainWidget->setStyleSheet(mainStyle);

    windowShadow = new QGraphicsDropShadowEffect(this);
    windowShadow->setBlurRadius(2);
    windowShadow->setColor(QColor(0, 0, 0));
    windowShadow->setOffset(0, 0);
    ui->mainWidget->setGraphicsEffect(windowShadow);
    /**********************************************************/

    /* Create border in order to cover the zigzag edge of the region */
    border = new QWidget(this);
    border->move(ui->mainWidget->pos() - QPoint(1, 1));
    border->resize(ui->mainWidget->size() + QSize(2, 2));
    QString borderStyle;
    borderStyle = "background-color:#00FFFFFF;border:1.5px solid #686868; border-radius:" + QString::asprintf("%d",cornerRadius) + "px";
    border->setStyleSheet(borderStyle);
    border->setAttribute(Qt::WA_TransparentForMouseEvents);
    border->show();
    /*****************************************************************/

    /* Create about page */
    defaultSettingsPage = new SlidePage(cornerRadius, "关于", ui->mainWidget);
    textInputItem *version = new textInputItem("版本", defaultSettingsPage);
    version->setValue("0.1.1");
    version->setEnabled(false);
    textInputItem *updateDate = new textInputItem("最近更新于", defaultSettingsPage);
    updateDate->setValue("2021/12/12 2:44");
    updateDate->setEnabled(false);
    textInputItem *Author = new textInputItem("作者", defaultSettingsPage);
    Author->setValue("辛统 林程炜 秦世煌 阎琦新");
    Author->setEnabled(false);
    textInputItem *lic = new textInputItem("开源协议", defaultSettingsPage);
    lic->setValue("MIT License");
    lic->setEnabled(false);
    textInputItem *GitHub = new textInputItem("Github仓库地址", defaultSettingsPage);
    GitHub->setValue("https://github.com/xtst/badminton-service-detection");
    GitHub->setEnabled(false);
    defaultSettingsPage->AddContent(GitHub);
    defaultSettingsPage->AddContent(lic);
    defaultSettingsPage->AddContent(Author);
    defaultSettingsPage->AddContent(updateDate);
    defaultSettingsPage->AddContent(version);
    curSettingsPage = defaultSettingsPage;
    defaultSettingsPage->show();
    pageList.push_back(defaultSettingsPage);

    /************************/

    /* Initialize display area */
    QFont titleFont = QFont("Corbel Light", 24);
    QFontMetrics titleFm(titleFont);
    canvasTitle = new QLineEdit(this);
    canvasTitle->setFont(titleFont);
    canvasTitle->setText("「羽毛球发球高度检测系统」");
    canvasTitle->setMaxLength(20);
    canvasTitle->setReadOnly(true);
    canvasTitle->setMinimumHeight(titleFm.height());
    canvasTitle->setMaximumWidth(titleFm.size(Qt::TextSingleLine, "「羽毛球发球高度检测系统」").width() + 10);
    canvasTitle->setStyleSheet("background-color:#00000000;border-style:none;border-width:0px;margin-left:1px;");
    connect(canvasTitle, &QLineEdit::textEdited, canvasTitle, [=](QString text){canvasTitle->setMaximumWidth(titleFm.size(Qt::TextSingleLine, text).width());});

    QFont descFont = QFont("Corbel Light", 12);
    QFontMetrics descFm(descFont);
    canvasDesc = new QLineEdit(this);
    canvasDesc->setFont(descFont);
    canvasDesc->setText("创建一次检测或者选择配置文件");
    canvasDesc->setMaxLength(128);
    canvasDesc->setReadOnly(true);
    canvasDesc->setMinimumHeight(descFm.lineSpacing());
    canvasDesc->setStyleSheet("background-color:#00000000;border-style:none;border-width:0px;");

    settingsIcon = new customIcon(":/icons/icons/settings.svg", "settings", 5, this);
    settingsIcon->setMinimumHeight(canvasTitle->height() * 0.7);
    settingsIcon->setMaximumWidth(canvasTitle->height() * 0.7);
    connect(settingsIcon, &customIcon::clicked, this, [=](){
        QPropertyAnimation *rotate = new QPropertyAnimation(settingsIcon, "rotationAngle", this);
        rotate->setDuration(750);
        rotate->setStartValue(0);
        rotate->setEndValue(90);
        rotate->setEasingCurve(QEasingCurve::InOutExpo);
        rotate->start();
        curSettingsPage->slideIn();
    });
    layersIcon = new customIcon(":/icons/icons/layers.svg", "layers", 5, this);
    layersIcon->setMinimumHeight(canvasTitle->height() * 0.7);
    layersIcon->setMaximumWidth(canvasTitle->height() * 0.7);

    /* create title */

    QWidget *titleInnerWidget = new QWidget(this);
    titleInnerWidget->setFixedHeight(canvasTitle->height());
    QHBoxLayout *innerLayout = new QHBoxLayout(titleInnerWidget);
    titleInnerWidget->setLayout(innerLayout);
    innerLayout->setContentsMargins(0, 0, 0, 0);
    innerLayout->setSpacing(10);
    innerLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    innerLayout->addWidget(canvasTitle);
    innerLayout->addWidget(settingsIcon);
    innerLayout->addWidget(layersIcon);

    QWidget *titleWidget = new QWidget(this);
    titleWidget->setMaximumHeight(canvasTitle->height() + canvasDesc->height());
    QVBoxLayout *outerLayout = new QVBoxLayout(titleWidget);
    titleWidget->setLayout(outerLayout);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);
    outerLayout->addWidget(titleInnerWidget);
    outerLayout->addWidget(canvasDesc);

    /* create default page */

    defaultPage = new QWidget(ui->mainWidget);
    defaultPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    bigIconButton *createNew = new bigIconButton(":/icons/icons/create.png", "开始检测", 10, this);
    createNew->setScale(0.9);
    bigIconButton *openFile = new bigIconButton(":/icons/icons/open.png", "打开配置文件", 10, this);
    connect(openFile, &bigIconButton::clicked, this, [=](){
        QString inputPath = QFileDialog::getOpenFileName(this, tr("Open map"), " ",  tr("羽毛球发球检测系统配置文件(*.bsd)"));
        if(!inputPath.isEmpty()){
            ofstream CVarg("default.bsd");
            // Either this if you use UTF-8 anywhere
            std::string utf8_text = inputPath.toUtf8().constData();
            // or this if you're on Windows :-)
            std::string current_locale_text = inputPath.toLocal8Bit().constData();
            CVarg<<"FilePosition: "<<current_locale_text;
            CVarg.close();

            QProcess process;
            process.start("tasklist");
            process.waitForFinished(); //等待命令执行结束
            QByteArray result = process.readAllStandardOutput();
            if(-1==result.indexOf("badminton-service-detection.exe"))
            {
                process.startDetached("badminton-service-detection.exe");
            }

//            MyCanvas *newCanvas = loadCanvas(inputPath);
//            if(newCanvas != nullptr){
//                canvasList.push_back(newCanvas);
//                selectionItem *newLayer = new selectionItem(newCanvas->name(), newCanvas->description(), layersPage);
//                layerSel->AddItem(newLayer);
//                layerSel->SetSelection(newLayer);
//                pageList.push_back(newCanvas->settingPage());
//                connect(newLayer, &selectionItem::selected, this, [=](){selectCanvas(newCanvas);});
//                selectCanvas(newCanvas);
//                connect(newCanvas, &MyCanvas::nameChanged, this, [=](QString text){
//                    canvasTitle->setText(text);
//                    canvasTitle->setMaximumWidth(QFontMetrics(QFont("Corbel Light", 24)).size(Qt::TextSingleLine, canvasTitle->text()).width() + 10);
//                    newLayer->setTitle(text);
//                });
//                connect(newCanvas, &MyCanvas::descChanged, this, [=](QString text){this->canvasDesc->setText(text);newLayer->setDescription(text);});
//                connect(newCanvas, &MyCanvas::setDel, this, [=](MyCanvas *c){curSettingsPage->slideOut();deleteCanvas(c);layerSel->RemoveItem(newLayer);});
////                windowShadow->setBlurRadius(1);
//                createNewPage->slideOut();
////                windowShadow->setBlurRadius(30);
//            }
        }
    });
    QHBoxLayout *defaultPageLayout = new QHBoxLayout(defaultPage);
    defaultPage->setLayout(defaultPageLayout);
    defaultPageLayout->setContentsMargins(50, 30, 50, 80);
    defaultPageLayout->setSpacing(20);
    defaultPageLayout->addWidget(createNew);
    defaultPageLayout->addWidget(openFile);

    /* create layers page */
    //for add new page
  //  textInputItem *rename = new textInputItem("小贴士1",createNewPage);
  //  rename->setValue("Layer_" + QString::asprintf("%d", canvasList.size()));
  //  textInputItem *redescribe = new textInputItem("Detail:",createNewPage);
  //  redescribe->setValue("No description");

    layersPage = new SlidePage(cornerRadius, "详细信息", ui->mainWidget);
    layersPage->stackUnder(createNewPage);
    connect(layersIcon, &customIcon::clicked, layersPage, &SlidePage::slideIn);
    layerSel = new singleSelectGroup("羽毛球发球高度检测系统", layersPage);
    connect(layerSel, &singleSelectGroup::itemChange, layersPage, [=](){layersPage->UpdateContents();});
    textButton *openFileBtn = new textButton("项目内容详细网站", layersPage);
    connect(openFileBtn, &textButton::clicked, this, [=](){
        /*
        QString inputPath = QFileDialog::getOpenFileName(this, tr("Open map"), " ",  tr("Map File(*.map)"));
        if(!inputPath.isEmpty()){
            MyCanvas *newCanvas = loadCanvas(inputPath);
            if(newCanvas != nullptr){
                canvasList.push_back(newCanvas);
                selectionItem *newLayer = new selectionItem(newCanvas->name(), newCanvas->description(), layersPage);
                layerSel->AddItem(newLayer);
                layerSel->SetSelection(newLayer);
                pageList.push_back(newCanvas->settingPage());
                connect(newLayer, &selectionItem::selected, this, [=](){selectCanvas(newCanvas);});
                selectCanvas(newCanvas);
                connect(newCanvas, &MyCanvas::nameChanged, this, [=](QString text){
                    canvasTitle->setText(text);
                    canvasTitle->setMaximumWidth(QFontMetrics(QFont("Corbel Light", 24)).size(Qt::TextSingleLine, canvasTitle->text()).width() + 10);
                    newLayer->setTitle(text);
                });
                connect(newCanvas, &MyCanvas::descChanged, this, [=](QString text){this->canvasDesc->setText(text);newLayer->setDescription(text);});
                connect(newCanvas, &MyCanvas::setDel, this, [=](MyCanvas *c){curSettingsPage->slideOut();deleteCanvas(c);layerSel->RemoveItem(newLayer);});
                createNewPage->slideOut();
            }
        }*/
        const QUrl regUrl(QLatin1String("https://just-cougar-698.notion.site/24474cf606494b59aa4bf6160f7c6940"));
        QDesktopServices::openUrl(regUrl);
    });
    textButton *addNewBtn = new textButton("演示视频(B站)", layersPage);
    layersPage->AddContent(addNewBtn);
    layersPage->AddContent(openFileBtn);
    layersPage->AddContent(layerSel);
    connect(addNewBtn, &textButton::clicked, this, [=](){
        const QUrl regUrl(QLatin1String("https://www.bilibili.com/video/BV1jR4y147g8"));
        QDesktopServices::openUrl(regUrl);
//        rename->setValue("Layer_" + QString::asprintf("%d", canvasList.size()));
//        redescribe->setValue("No description");createNewPage->slideIn();
    });
    layersPage->show();
    pageList.push_back(layersPage);

    /* create add new slide page */
    createNewPage = new SlidePage(cornerRadius, "开始检测", ui->mainWidget);
    QLineEdit *canvasName = new QLineEdit(this);
    canvasName->setMaximumHeight(20);
    QLineEdit *canvasDesc = new QLineEdit(this);
    canvasDesc->setMaximumHeight(20);

    QWidget *whiteSpace = new QWidget(createNewPage);
    whiteSpace->setFixedHeight(30);
    singleSelectGroup *structureSel = new singleSelectGroup("取帧频率",createNewPage);
    selectionItem *item_1 = new selectionItem("适中（默认）", "常规取帧频率", createNewPage);
    selectionItem *item_2 = new selectionItem("较高", "CPU占用会更高，但会更精确", createNewPage);
    structureSel->AddItem(item_1);
    structureSel->AddItem(item_2);
    singleSelectGroup *dirSel = new singleSelectGroup("动作幅度", createNewPage);
    selectionItem *item_3 = new selectionItem("适中（默认）", "将常见发球动作幅度设为阈值", createNewPage);
    selectionItem *item_4 = new selectionItem("敏锐", "识别更加入微，但会增加误判率", createNewPage);
    dirSel->AddItem(item_3);
    dirSel->AddItem(item_4);
    textButton *submit = new textButton("开始识别!", createNewPage);
    connect(submit, &textButton::clicked, this, [=](){

        QProcess process;
        process.start("tasklist");
        process.waitForFinished(); //等待命令执行结束
        QByteArray result = process.readAllStandardOutput();
        if(-1==result.indexOf("badminton-service-detection.exe"))
        {
            process.startDetached("badminton-service-detection.exe");
        }
     /* MyCanvas *newCanvas = new MyCanvas(cornerRadius,
                                           rename->value(),
                                           redescribe->value(),
                                           structureSel->value() == 0 ? MyCanvas::AL : MyCanvas::AML,
                                           dirSel->value() == 0 ? MyCanvas::DG : MyCanvas::UDG, ui->mainWidget);
        canvasList.push_back(newCanvas);
        selectionItem *newLayer = new selectionItem(newCanvas->name(), newCanvas->description(), layersPage);
        layerSel->AddItem(newLayer);
        layerSel->SetSelection(newLayer);
        pageList.push_back(newCanvas->settingPage());
        connect(newLayer, &selectionItem::selected, this, [=](){selectCanvas(newCanvas);});
        selectCanvas(newCanvas);
        connect(newCanvas, &MyCanvas::nameChanged, this, [=](QString text){
            canvasTitle->setText(text);
            canvasTitle->setMaximumWidth(QFontMetrics(QFont("Corbel Light", 24)).size(Qt::TextSingleLine, canvasTitle->text()).width() + 10);
            newLayer->setTitle(text);
        });
        connect(newCanvas, &MyCanvas::descChanged, this, [=](QString text){this->canvasDesc->setText(text);newLayer->setDescription(text);});
        connect(newCanvas, &MyCanvas::setDel, this, [=](MyCanvas *c){curSettingsPage->slideOut();deleteCanvas(c);layerSel->RemoveItem(newLayer);});
        createNewPage->slideOut();*/
    });
    createNewPage->AddContent(submit);
    createNewPage->AddContent(dirSel);
    createNewPage->AddContent(structureSel);
    createNewPage->AddContent(whiteSpace);
 //   createNewPage->AddContent(redescribe);
 //   createNewPage->AddContent(rename);

    connect(createNew, &bigIconButton::clicked, createNewPage, [=](){

      //  rename->setValue("Layer_" + QString::asprintf("%d", canvasList.size()));redescribe->setValue("No description");
        createNewPage->slideIn();
    });

    createNewPage->show();
    pageList.push_back(createNewPage);

    ui->displayLayout->addWidget(titleWidget);
    ui->displayLayout->addWidget(defaultPage);
    ui->displayLayout->setAlignment(Qt::AlignTop);
}

void MainWindow::selectCanvas(MyCanvas *canvas){
    if(!curCanvas){
        ui->displayLayout->removeWidget(defaultPage);
        defaultPage->hide();
        ui->displayLayout->addWidget(canvas);
        canvas->show();
    }
    else{
        ui->displayLayout->removeWidget(curCanvas);
        curCanvas->hide();
        ui->displayLayout->addWidget(canvas);
        canvas->show();
    }
    curCanvas = canvas;
    canvas->settingPage()->setParent(ui->mainWidget);
    curSettingsPage = canvas->settingPage();
    canvasTitle->setText(curCanvas->name());
    canvasTitle->setMaximumWidth(QFontMetrics(QFont("Corbel Light", 24)).size(Qt::TextSingleLine, canvasTitle->text()).width() + 10);
    canvasDesc->setText(curCanvas->description());
}

void MainWindow::deleteCanvas(MyCanvas *canvas){
    int index = canvasList.indexOf(canvas);
    if(index < 0)
        return;
    canvasList.erase(canvasList.begin() + index);
    ui->displayLayout->removeWidget(curCanvas);
    curCanvas->hide();
    if(canvasList.size() > 0){
        selectCanvas(canvasList[0]);
    }
    else{
        ui->displayLayout->addWidget(defaultPage);
        defaultPage->show();
        curCanvas = nullptr;
        canvasTitle->setText("「羽毛球发球高度检测系统」");
        canvasTitle->setMaximumWidth(QFontMetrics(QFont("Corbel Light", 24)).size(Qt::TextSingleLine, "「羽毛球发球高度检测系统」").width() + 10);
        canvasDesc->setText("创建一次检测或者选择配置文件");
        curSettingsPage = defaultSettingsPage;
    }
    pageList.erase(pageList.begin() + pageList.indexOf(canvas->settingPage()));
    delete canvas;
    ui->mainWidget->update();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        mousePressed = true;
        lastPos = event->globalPosition().toPoint() - this->frameGeometry().topLeft();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if(event->buttons() == Qt::NoButton)
        mousePressed = false;
    if(!mousePressed){
        mouseState = 0;
        if(!maximized && abs(event->pos().x() - ui->mainWidget->pos().x()) < 5)
            mouseState |= AT_LEFT;
        if(!maximized && abs(event->pos().y() - ui->mainWidget->pos().y()) < 5)
            mouseState |= AT_TOP;
        if(!maximized && abs(event->pos().x() - ui->mainWidget->pos().x() - ui->mainWidget->width()) < 5)
            mouseState |= AT_RIGHT;
        if(!maximized && abs(event->pos().y() - ui->mainWidget->pos().y() - ui->mainWidget->height()) < 5)
            mouseState |= AT_BOTTOM;
        if(mouseState == AT_TOP_LEFT  || mouseState == AT_BOTTOM_RIGHT)
            setCursor(Qt::SizeFDiagCursor);
        else if(mouseState == AT_TOP_RIGHT || mouseState == AT_BOTTOM_LEFT)
            setCursor(Qt::SizeBDiagCursor);
        else if(mouseState & (AT_LEFT | AT_RIGHT))
            setCursor(Qt::SizeHorCursor);
        else if(mouseState & (AT_TOP | AT_BOTTOM))
            setCursor(Qt::SizeVerCursor);
        else
            unsetCursor();
    }
    else{
        if(mouseState == 0){
            if(maximized){
                qreal wRatio = (double)event->pos().x() / (double)ui->mainWidget->width();
                controlWindowScale();
                this->move(QPoint(event->globalPosition().x() - ui->mainWidget->width() * wRatio, -30));
                lastPos = QPoint(ui->mainWidget->width() * wRatio, event->pos().y());
            }
            else
                this->move(event->globalPosition().toPoint() - lastPos);
        }
        else{
            QPoint d = event->globalPosition().toPoint() - frameGeometry().topLeft() - lastPos;
            if(mouseState & AT_LEFT){
                this->move(this->frameGeometry().x() + d.x(), this->frameGeometry().y());
                this->resize(this->width() - d.x(), this->height());
            }
            if(mouseState & AT_RIGHT){
                this->resize(this->width() + d.x(), this->height());
            }
            if(mouseState & AT_TOP){
                this->move(this->frameGeometry().x(), this->frameGeometry().y() + d.y());
                this->resize(this->width(), this->height() - d.y());
            }
            if(mouseState & AT_BOTTOM){
                this->resize(this->width(), this->height() + d.y());
            }
        }
        lastPos = event->globalPosition().toPoint() - this->frameGeometry().topLeft();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event){
    //Resize border
    if(border)
        border->resize(ui->mainWidget->size() + QSize(2, 2));

    //Resize mask
    QPainterPath path;
    path.addRoundedRect(ui->mainWidget->rect(), cornerRadius - 1, cornerRadius - 1);
    QRegion mask(path.toFillPolygon().toPolygon());
    ui->mainWidget->setMask(mask);

    //Resize all pages
    for(int i = 0; i < pageList.size(); i++){
        pageList[i]->resize(ui->mainWidget->width() * 0.3 < pageList[i]->preferWidth ? pageList[i]->preferWidth - 1 : ui->mainWidget->width() * 0.3 - 1, ui->mainWidget->height());
        pageList[i]->resize(pageList[i]->width() + 1, pageList[i]->height());
    }
}

void MainWindow::controlWindowScale(){
    if(!maximized){
        lastGeometry = this->frameGeometry();
        windowShadow->setEnabled(false);
        ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
        border->hide();
        QString mainStyle = "QWidget#mainWidget{background-color:" + mainBackGround.name() + ";border-radius:0px;}";
        ui->mainWidget->setStyleSheet(mainStyle);
        this->showMaximized();
        maximized = true;
        QPainterPath path;
        path.addRect(ui->mainWidget->rect());
        QRegion mask(path.toFillPolygon().toPolygon());
        ui->mainWidget->setMask(mask);
    }
    else{
        ui->verticalLayout->setContentsMargins(30, 30, 30, 30);
        this->showNormal();
        QString mainStyle = "QWidget#mainWidget{background-color:" + mainBackGround.name() + QString::asprintf(";border-radius:%dpx", cornerRadius) + "}";
        ui->mainWidget->setStyleSheet(mainStyle);
        QPainterPath path;
        path.addRoundedRect(ui->mainWidget->rect(), cornerRadius - 1, cornerRadius - 1);
        QRegion mask(path.toFillPolygon().toPolygon());
        ui->mainWidget->setMask(mask);
        border->show();
        windowShadow->setEnabled(true);
        this->resize(lastGeometry.width(), lastGeometry.height());
        this->move(lastGeometry.x(), lastGeometry.y());
        maximized = false;
    }
}

MyCanvas* MainWindow::loadCanvas(const QString &path){
    QFile input(path);
    input.open(QIODevice::ReadOnly);
    QTextStream ts(&input);
    QString magicString = ts.readLine();
    if(magicString != "VFdGeWFXUnZaekl3TURJd05ESTE=")   return nullptr;
    MyCanvas *newCanvas = new MyCanvas(ts, cornerRadius, ui->mainWidget);
    input.close();
    return newCanvas;
}
