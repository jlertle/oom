//=========================================================
//  OOMidi
//  OpenOctave Midi and Audio Editor
//    $Id: menutitleitem.h,v 1.1.2.1 2009/06/10 00:34:59 terminator356 Exp $
//  (C) Copyright 1999-2001 Werner Schweer (ws@seh.de)
//=========================================================

#ifndef __MENU_TITLE_ITEM_H__
#define __MENU_TITLE_ITEM_H__

#include <QWidgetAction>

//---------------------------------------------------------
//   MenuTitleItem
//---------------------------------------------------------

class MenuTitleItem : public QWidgetAction
{
    QString s;

public:
    MenuTitleItem(const QString&, QWidget* /*parent*/);
    QWidget* createWidget(QWidget* /*parent*/);
};

#endif
