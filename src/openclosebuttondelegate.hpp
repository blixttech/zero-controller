#pragma once

#include <QStyledItemDelegate>

namespace zero {

// with thanks from 
// https://forum.qt.io/topic/93621/add-buttons-in-tablewidget-s-row/7

class OpenCloseButtonDelegate : public QStyledItemDelegate{
    Q_OBJECT
    Q_DISABLE_COPY(OpenCloseButtonDelegate)
public:
    explicit OpenCloseButtonDelegate(QObject* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override; 
    
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QStyleOptionButton prepareButtonOption(const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const;

    void sendCommitData();
};

} // end namespace
