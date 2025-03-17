#include "operationhistorymanager.h"
#include <QDebug>

OperationHistoryManager::OperationHistoryManager(QObject *parent)
    : QObject(parent)
{
}

void OperationHistoryManager::recordAdd(const QVariantMap &annotation, const QVariantList &allAnnotations)
{
    // 创建操作记录
    Operation op;
    op.type = OperationType::Add;
    op.before = allAnnotations;
    op.after = allAnnotations;
    op.index = allAnnotations.size() - 1; // 新添加的标注在列表末尾
    
    // 添加到撤销栈
    m_undoStack.push(op);
    
    // 清空重做栈
    m_redoStack.clear();
    
    qDebug() << "记录修改操作，当前撤销栈大小:" << m_undoStack.size();
    emit operationCompleted(true, "记录修改操作"); //清空重做栈
    m_redoStack.clear();
    
    qDebug() << "记录添加操作，当前撤销栈大小:" << m_undoStack.size();
    emit operationCompleted(true, "记录添加操作");
}

void OperationHistoryManager::recordDelete(int index, const QVariantMap &annotation, const QVariantList &allAnnotations)
{
    // 创建操作记录
    Operation op;
    op.type = OperationType::Delete;
    op.before = allAnnotations;
    
    // 创建删除后的状态（复制当前状态并移除指定索引的标注）
    QVariantList afterState = allAnnotations;
    if (index >= 0 && index < afterState.size()) {
        afterState.removeAt(index);
    }
    
    op.after = afterState;
    op.index = index;
    
    // 添加到撤销栈
    m_undoStack.push(op);
    
    // 清空重做栈
    m_redoStack.clear();
    
    qDebug() << "记录修改操作，当前撤销栈大小:" << m_undoStack.size();
    emit operationCompleted(true, "记录修改操作"); //清空重做栈
    m_redoStack.clear();
    
    qDebug() << "记录删除操作，当前撤销栈大小:" << m_undoStack.size();
    emit operationCompleted(true, "记录删除操作");
}

QVariantList OperationHistoryManager::undo()
{
    if (m_undoStack.isEmpty()) {
        emit operationCompleted(false, "没有可撤销的操作");
        return QVariantList();
    }
    
    // 从撤销栈中弹出最近的操作
    Operation op = m_undoStack.pop();
    
    // 将操作添加到重做栈
    m_redoStack.push(op);
    
    qDebug() << "撤销操作，当前撤销栈大小:" << m_undoStack.size() << "，重做栈大小:" << m_redoStack.size();
    emit operationCompleted(true, "撤销成功");
    
    // 返回操作前的状态
    return op.before;
}

QVariantList OperationHistoryManager::redo()
{
    if (m_redoStack.isEmpty()) {
        emit operationCompleted(false, "没有可重做的操作");
        return QVariantList();
    }
    
    // 从重做栈中弹出最近的操作
    Operation op = m_redoStack.pop();
    
    // 将操作添加回撤销栈
    m_undoStack.push(op);
    
    qDebug() << "重做操作，当前撤销栈大小:" << m_undoStack.size() << "，重做栈大小:" << m_redoStack.size();
    emit operationCompleted(true, "重做成功");
    
    // 返回操作后的状态
    return op.after;
}

bool OperationHistoryManager::canUndo() const
{
    return !m_undoStack.isEmpty();
}

bool OperationHistoryManager::canRedo() const
{
    return !m_redoStack.isEmpty();
}

void OperationHistoryManager::clearHistory()
{
    m_undoStack.clear();
    m_redoStack.clear();
    
    qDebug() << "清除所有历史记录";
    emit operationCompleted(true, "清除历史记录");
}

// 记录修改操作
void OperationHistoryManager::recordModify(int index, const QVariantList &beforeAnnotations, const QVariantList &afterAnnotations)
{
    // 创建操作记录
    Operation op;
    op.type = OperationType::Modify;
    op.before = beforeAnnotations;
    op.after = afterAnnotations;
    op.index = index;
    
    // 添加到撤销栈
    m_undoStack.push(op);
    
    // 清空重做栈
    m_redoStack.clear();
    
    qDebug() << "记录修改操作，当前撤销栈大小:" << m_undoStack.size();
    emit operationCompleted(true, "记录修改操作"); //清空重做栈
    m_redoStack.clear();
    
    qDebug() << "记录修改操作，当前撤销栈大小:" << m_undoStack.size();
    emit operationCompleted(true, "记录修改操作");
}
