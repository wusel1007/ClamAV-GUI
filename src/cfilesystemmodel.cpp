#include "cfilesystemmodel.h"
#include <QDebug>

CFileSystemModel::CFileSystemModel()
{
}

Qt::ItemFlags CFileSystemModel::flags(const QModelIndex &index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant CFileSystemModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::CheckStateRole)
    {

        return checkedIndexes.contains(index) ? Qt::Checked : Qt::Unchecked;
    }
    else
    {
        return QFileSystemModel::data(index, role);
    }
}

bool CFileSystemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == Qt::CheckStateRole)
    {
        if(value == Qt::Checked)
        {
            checkedIndexes.insert(index);
        }
        else
        {
            checkedIndexes.remove(index);
        }
        emit dataChanged(index, index);
        return true;
    }
    return QFileSystemModel::setData(index, value, role);
}

void CFileSystemModel::setChecked(const QModelIndex &index, bool state){

    if (state == true){
       checkedIndexes.insert(index);
    } else {
        checkedIndexes.remove(index);
    }
}

void CFileSystemModel::setChecked(QString pfad, bool state){
QModelIndex index = this->index(pfad);

    if (state == true){
       checkedIndexes.insert(index);
    } else {
        checkedIndexes.remove(index);
    }
}

void CFileSystemModel::unCheckAll(){
    checkedIndexes.clear();
}
