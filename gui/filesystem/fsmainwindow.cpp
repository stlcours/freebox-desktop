#include "fsmainwindow.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <QFileSystemModel>
#include <QFileDialog>
#include "tools.h"
#include "fstaskbutton.h"
#include "authorizemessagebox.h"

FSMainWindow::FSMainWindow(QWidget *parent) :
    AbstractMainWindow(parent)
{
    mTreeView = new FSTreeView;
    mTableView = new FSTableView;
    mHeaderWidget= new FSPathToolBar;
    mSplitter = new QSplitter(Qt::Horizontal);
    mModel = new FSModel(fbx());
    mFolderModel = new QSortFilterProxyModel;
    mTaskWidget = new FSTaskWidget(fbx());
    mTaskDockWidget = new QDockWidget;
    mFolderModel->setFilterKeyColumn(0);
    mFolderModel->setFilterRole(FSModel::IsDirRole);
    mFolderModel->setFilterFixedString("true");

    mFolderModel->setSourceModel(mModel);
    mTreeView->setModel(mFolderModel);
    mTableView->setModel(mModel);
    mHeaderWidget->setModel(mModel);

    mTreeView->setAnimated(true);

    mTableView->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    mTableView->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);
    mTableView->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents);

    mTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableView->verticalHeader()->setDefaultSectionSize(24);
    mTableView->setAlternatingRowColors(true);
    mTableView->verticalHeader()->hide();
    mTreeView->hideColumn(1);
    mTreeView->hideColumn(2);
    mTreeView->hideColumn(3);


    mSplitter->setHandleWidth(4);
    mTreeView->setMaximumWidth(400);




    //construction de la ToolBar
    mToolBar = addToolBar("tool");
    mMkdirAction   = mToolBar->addAction(QIcon(":folder.png"),"Nouveau dossier");
    mUploadAction  = mToolBar->addAction(QIcon(":folder_add.png"),"Uploader ici");
    mRefreshAction = mToolBar->addAction(QIcon(":arrow_refresh.png"),"Rafraîchir");
    mTaskAction    = mToolBar->addAction(QIcon(":progressbar.png"),"");


    mToolBar->setIconSize(QSize(16,16));
    mToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mToolBar->setFloatable(false);
    mToolBar->setMovable(false);



    //construction de l'ensemble
    QVBoxLayout * centerLayout = new QVBoxLayout;
    centerLayout->addWidget(mHeaderWidget);
    centerLayout->addWidget(mTableView);
    centerLayout->setContentsMargins(0,0,0,0);

    QWidget * centerWidget = new QWidget;
    centerWidget->setLayout(centerLayout);

    mSplitter->addWidget(mTreeView);
    mSplitter->addWidget(centerWidget);
    setCentralWidget(mSplitter);

    mSplitter->setStretchFactor(0,1);
    mSplitter->setStretchFactor(1,3);
    //connection


    connect(fbx(),SIGNAL(loginSuccess()),this,SLOT(init()));
    connect(mTreeView,SIGNAL(clicked(QModelIndex)),this,SLOT(setRootIndex(QModelIndex)));
    connect(mTableView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(setRootIndex(QModelIndex)));
    connect(mTableView,SIGNAL(filesAdded(QStringList)),this,SLOT(uploads(QStringList)));
    connect(mHeaderWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(setRootIndex(QModelIndex)));
    connect(mRefreshAction,SIGNAL(triggered()),this,SLOT(refresh()));
    connect(mMkdirAction,SIGNAL(triggered()),this,SLOT(mkdir()));
    connect(mUploadAction,SIGNAL(triggered()),this,SLOT(upload()));
    connect(mTaskAction,SIGNAL(triggered()),mTaskWidget,SLOT(show()));
    connect(mTaskWidget,SIGNAL(countChanged(int)),this,SLOT(setTaskCount(int)));
    resize(800,600);
    setTaskCount(0);
    //    setAttribute(Qt::WA_DeleteOnClose,true);



    setWindowTitle("Explorateur de fichiers");
    setWindowIcon(QIcon(":main_fs.png"));

}

FSMainWindow::~FSMainWindow()
{
    delete mTreeView;
    delete mTableView;
    delete mHeaderWidget;
    delete mSplitter;
    delete mModel;
    delete mFolderModel;
    delete mTaskWidget;

}


void FSMainWindow::refresh()
{
    mModel->refresh(mFolderModel->mapToSource(mTreeView->currentIndex()));

}

void FSMainWindow::mkdir()
{
    QInputDialog dialog;
    dialog.setLabelText("Nom du nouveau dossier:");
    dialog.setWindowTitle("Nouveau dossier");
    dialog.setInputMode(QInputDialog::TextInput);


    if (dialog.exec() == QDialog::Accepted) {
        mModel->mkdir(dialog.textValue(), mTableView->rootIndex());
    }
}

void FSMainWindow::upload()
{
    QFileDialog dialog;
    dialog.setWindowTitle("Uploader");
    QString filename = dialog.getOpenFileName(this,"Uploader");
    qDebug()<<filename;
    mModel->upload(filename, mTableView->rootIndex());


}

void FSMainWindow::uploads(const QStringList &list)
{

    foreach (QString filename, list) {

        mModel->upload(filename, mTableView->rootIndex());

    }

}

void FSMainWindow::init()
{
    mModel->init();
    mTableView->setRootIndex(QModelIndex());
    mHeaderWidget->setCurrentIndex(QModelIndex());
    mTreeView->setCurrentIndex(QModelIndex());
}

void FSMainWindow::setRootIndex(const QModelIndex &index)
{

    if (sender()->metaObject()->className() == QString("FSTreeView") ) {

        QModelIndex fid = mFolderModel->index(index.row(),0,index.parent());
        QModelIndex mid = mFolderModel->mapToSource(fid);

        mTableView->setRootIndex(mid);
        mHeaderWidget->setCurrentIndex(mid);
        mTreeView->setCurrentIndex(fid);
    }

    if (sender()->metaObject()->className() == QString("FSTableView")){

        if (index.data(FSModel::IsDirRole).toBool())
        {
            QModelIndex mid = mModel->index(index.row(),0,index.parent());
            QModelIndex fid = mFolderModel->mapFromSource(mid);

            mTableView->setRootIndex(mid);
            mHeaderWidget->setCurrentIndex(mid);
            mTreeView->setCurrentIndex(fid);
        }
        else {
            QFileDialog dialog;
            dialog.setWindowTitle("Télécharger");
            QString dirPath = dialog.getExistingDirectory(this);
            if (!dirPath.isEmpty()) {
                qDebug()<<dirPath;
                mModel->download(dirPath,index);

            }
        }
    }

    if (sender()->metaObject()->className() == QString("FSPathToolBar"))
    {
        QModelIndex mid = mModel->index(index.row(),0,index.parent());
        QModelIndex fid = mFolderModel->mapFromSource(mid);
        mTableView->setRootIndex(mid);
        mHeaderWidget->setCurrentIndex(mid);
        mTreeView->setCurrentIndex(fid);

    }


}

void FSMainWindow::showTaskWidget(bool show)
{
    mTaskWidget->setVisible(show);

}

void FSMainWindow::setTaskCount(int count)
{

    if (count == 0)
        mTaskAction->setText("Aucune tâche(s)");
    else
        mTaskAction->setText(QString("%3 en cours").arg(count));

}





