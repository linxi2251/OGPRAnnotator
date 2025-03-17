#ifndef OPERATIONHISTORYMANAGER_H
#define OPERATIONHISTORYMANAGER_H

#include <QObject>
#include <QVariantList>
#include <QStack>
#include <qqmlintegration.h>

// 操作类型枚举
enum class OperationType {
    Add,        // 添加标注
    Delete,     // 删除标注
    Modify      // 修改标注
};

// 操作记录结构
struct Operation {
    OperationType type;     // 操作类型
    QVariantList before;    // 操作前的状态
    QVariantList after;     // 操作后的状态
    int index;              // 操作的索引（用于删除和修改操作）
};

Q_DECLARE_METATYPE(Operation)

class OperationHistoryManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit OperationHistoryManager(QObject *parent = nullptr);

    // 记录添加操作
    Q_INVOKABLE void recordAdd(const QVariantMap &annotation, const QVariantList &allAnnotations);
    
    // 记录删除操作
    Q_INVOKABLE void recordDelete(int index, const QVariantMap &annotation, const QVariantList &allAnnotations);
    
    // 撤销操作
    Q_INVOKABLE QVariantList undo();
    
    // 重做操作
    Q_INVOKABLE QVariantList redo();
    
    // 检查是否可以撤销
    Q_INVOKABLE bool canUndo() const;
    
    // 检查是否可以重做
    Q_INVOKABLE bool canRedo() const;
    
    // 清除所有历史记录
    Q_INVOKABLE void clearHistory();
    
    // 记录修改操作
    Q_INVOKABLE void recordModify(int index, const QVariantList &beforeAnnotations, const QVariantList &afterAnnotations);

signals:
    // 操作完成信号
    void operationCompleted(bool success, const QString &message);

private:
    QStack<Operation> m_undoStack;    // 撤销栈
    QStack<Operation> m_redoStack;    // 重做栈
};

#endif // OPERATIONHISTORYMANAGER_H