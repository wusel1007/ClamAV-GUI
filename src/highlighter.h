/***************************************************************************
 *   Copyright (C) 2015 by Joerg Zopes                                     *
 *   joerg.zopes@gmx.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H
//
#include <QSyntaxHighlighter>
#include <QRegularExpression>
//
class highlighter : public QSyntaxHighlighter
{
Q_OBJECT
public:
	highlighter(QTextDocument *parent = 0);
 protected:
     void highlightBlock(const QString &text);

private:
     struct HighlightingRule
     {
         QRegularExpression pattern;
         QTextCharFormat format;
     };
     QVector<HighlightingRule> highlightingRules;

     QRegularExpression commentStartExpression;
     QRegularExpression commentEndExpression;

     QTextCharFormat keywordFormat;
     QTextCharFormat classFormat;
     QTextCharFormat quotationFormat;
     QTextCharFormat functionFormat;
     QTextCharFormat multiLineCommentFormat;
     QTextCharFormat singleLineCommentFormat;
     QTextCharFormat xmlFormat;
};
#endif
