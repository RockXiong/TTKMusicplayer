#include "musicusermanager.h"
#include "musicuiobject.h"
#include "ui_musicusermanager.h"
#include "musicusermodel.h"
#include "musicuserconfigmanager.h"
#include "musicuserrecordwidget.h"

#include <QMenu>

MusicUserManager::MusicUserManager(QWidget *parent)
     : QDialog(parent),
       ui(new Ui::MusicUserManager)
{
    ui->setupUi(this);
    setWindowFlags( Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    createButtonPopMenu();
    m_userModel = new MusicUserModel(this);
    ui->userIcon->setStyleSheet(MusicUIObject::MCustomStyle28);

    connect(ui->logoffButton, SIGNAL(clicked()), SLOT(musicUserLogoff()));
    connect(ui->userIcon, SIGNAL(clicked()), SLOT(popupUserRecordWidget()));
    connect(ui->username, SIGNAL(clicked()), SLOT(popupUserRecordWidget()));
}

MusicUserManager::~MusicUserManager()
{
    delete m_userModel;
    delete ui;
}

void MusicUserManager::setUserUID(const QString &uid)
{
    m_currentUserUID = uid;
    ui->username->setText(m_userModel->getUserName(uid));
    createUserTime();
    m_time.start();
}

void MusicUserManager::createUserTime() const
{
    qlonglong time = m_userModel->getUserLogTime(m_currentUserUID).toLongLong();
    ui->totalTimeLabel->setText(QString::number(time));
}

void MusicUserManager::createButtonPopMenu()
{
    m_popMenu.addAction(tr("Modifies"), this, SLOT(popupUserRecordWidget()));
    m_popMenu.addAction(tr("Switches"), this, SLOT(musicUserLogoff()));
    m_popMenu.addAction(tr("Spacing"));
    m_popMenu.setStyleSheet(MusicUIObject::MMenuStyle02);
    ui->musicSettingButton->setMenu(&m_popMenu);
}

void MusicUserManager::musicUserLogoff()
{
    m_userModel->updateUser(m_currentUserUID, "", "", ui->username->text(),
            QString::number(m_userModel->getUserLogTime(m_currentUserUID)
                     .toLongLong() + m_time.elapsed()/1000 ));

    MusicUserConfigManager xml;
    if(!xml.readUserXMLConfig())
    {
        return;
    }
    MusicUserRecord record;
    xml.readUserConfig( record );
    int index = record.m_names.indexOf(m_currentUserUID);
    record.m_als[index] = "0";  //auto login flag
    xml.writeUserXMLConfig( record );

    m_currentUserUID.clear();
    emit userStateChanged(QString(), QString());
    close();
}

int MusicUserManager::exec()
{
    QWidget *pa = static_cast<QWidget*>(parent());
    QPoint point = pa->mapToGlobal(QPoint(0, 0));
    move(point.x(), point.y() + 27);
    return QDialog::exec();
}

void MusicUserManager::leaveEvent(QEvent *event)
{
    QDialog::leaveEvent(event);
    close();
}

void MusicUserManager::popupUserRecordWidget()
{
    MusicUserRecordWidget record;
    connect(&record, SIGNAL(resetUserName(QString)), SLOT(resetUserName(QString)));
    record.setUserModel(m_userModel, m_currentUserUID);
    record.exec();
}

void MusicUserManager::resetUserName(const QString &name)
{
    ui->username->setText(name);
}