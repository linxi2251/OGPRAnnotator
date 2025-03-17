#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H
#include <QDir>
#include <qqmlintegration.h>

class ProjectManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
};

#endif // PROJECTMANAGER_H 
