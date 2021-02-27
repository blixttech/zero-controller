#include <QDebug>
#include <QApplication>
#include <QHeaderView>
#include <QPushButton>
#include "openclosebuttondelegate.hpp"

namespace zero {

OpenCloseButtonDelegate::OpenCloseButtonDelegate(QObject* parent)
        :QStyledItemDelegate(parent)
    {}

QStyleOptionButton OpenCloseButtonDelegate::prepareButtonOption(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton buttonOption;
    buttonOption.rect = option.rect;
    buttonOption.features = QStyleOptionButton::None;
    buttonOption.direction = option.direction;
    buttonOption.fontMetrics = option.fontMetrics;
    buttonOption.palette = option.palette;
    buttonOption.styleObject = option.styleObject;
    
    // check if zero is closed
    if (index.model()->data(index, Qt::DisplayRole).toBool())
    {
        buttonOption.text = tr("Closed");
    }
    else
    {
        buttonOption.text = tr("Open");
    }
    return buttonOption;
}

void OpenCloseButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, 
                                const QModelIndex &index) const 
{
    if (!index.isValid())
        return;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    QStyleOptionButton btnOpt=prepareButtonOption(option, index);
    style->drawControl(QStyle::CE_PushButton, &btnOpt, painter, widget);
}


QSize OpenCloseButtonDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton btnOpt=prepareButtonOption(option, index);
    const QSize fntSize = btnOpt.fontMetrics.size(Qt::TextShowMnemonic,btnOpt.text);
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    return style->sizeFromContents(QStyle::CT_PushButton,&btnOpt,fntSize,widget);
    /*
    const QSize baseSize = QStyledItemDelegate::sizeHint(option,index);
    return QSize(baseSize.width()+(2*baseSize.height()),baseSize.height());*/
}

QWidget* OpenCloseButtonDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const 
{
/*    QWidget* result = new QWidget(parent);
    result->setGeometry(option.rect);
    QWidget* baseEditor = QStyledItemDelegate::createEditor(result,option,index);
    baseEditor->setObjectName("baseEditor");
    baseEditor->setGeometry(0,0,option.rect.width(),option.rect.height());
    QPushButton* openCloseButton = new QPushButton(result);*/
    QPushButton* openCloseButton = new QPushButton(parent);
    openCloseButton->setObjectName("openCloseButton");
    openCloseButton->setGeometry(0, 0, option.rect.width(),option.rect.height());
//    openCloseButton->setCheckable(true);

    connect(openCloseButton,&QPushButton::clicked,
            this, &OpenCloseButtonDelegate::sendCommitData);

    return openCloseButton;
}

void OpenCloseButtonDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const 
{
    auto openCloseButton = qobject_cast<QPushButton*>(editor);
    if (nullptr == openCloseButton) return;

    if (index.model()->data(index, Qt::DisplayRole).toBool())
    {
        openCloseButton->setText(tr("Closed"));
        openCloseButton->setStyleSheet("QPushButton{ background-color: red }");
    }
    else
    {
        openCloseButton->setText(tr("Open"));
        openCloseButton->setStyleSheet("QPushButton{ background-color: green }");
    }

    //QStyledItemDelegate::setEditorData(baseEditor,index);
}

void OpenCloseButtonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, 
                                            const QModelIndex &index) const 
{
    model->setData(index, QVariant());
}

void OpenCloseButtonDelegate::updateEditorGeometry(QWidget *editor, 
                                                    const QStyleOptionViewItem &option, 
                                                    const QModelIndex &index) const 
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

void OpenCloseButtonDelegate::sendCommitData()
{
    auto *editor = qobject_cast<QPushButton *>(sender());
    emit commitData(editor);
}

} // end namespace 
