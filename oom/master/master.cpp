//=========================================================
//  OOMidi
//  OpenOctave Midi and Audio Editor
//    $Id: master.cpp,v 1.3 2004/04/11 13:03:32 wschweer Exp $
//  (C) Copyright 2000 Werner Schweer (ws@seh.de)
//=========================================================

#include <stdio.h>
#include <values.h>

#include <QCursor>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

#include "globals.h"
#include "gconfig.h"
#include "master.h"
#include "song.h"
#include "scrollscale.h"
#include "midi.h"
#include "AbstractMidiEditor.h"
#include "icons.h"
#include "audio.h"

extern void drawTickRaster(QPainter& p, int x, int y,
		int w, int h, int quant, bool ctrl);

//---------------------------------------------------------
//   Master
//---------------------------------------------------------

Master::Master(AbstractMidiEditor* e, QWidget* parent, int xmag, int ymag)
: View(parent, xmag, ymag)
{
	editor = e;
	//setBg(QColor(195,198,196));
	setBg(QColor(63,63,63));
	vscroll = 0;
	pos[0] = 0;
	pos[1] = 0;
	pos[2] = 0;
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
	connect(song, SIGNAL(posChanged(int, unsigned, bool)), this, SLOT(setPos(int, unsigned, bool)));
	connect(song, SIGNAL(songChanged(int)), this, SLOT(redraw()));
}

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void Master::setPos(int idx, unsigned val, bool adjustScrollbar)
{
	if (pos[idx] == val)
		return;

	int opos = mapx(pos[idx]);
	int npos = mapx(val);

	if (adjustScrollbar && idx == 0)
	{
		switch (song->follow())
		{
			case Song::NO:
				break;
			case Song::JUMP:
				if (npos >= width())
				{
					int ppos = val - rmapxDev(width() / 8);
					if (ppos < 0)
						ppos = 0;
					emit followEvent(ppos);
					opos = mapx(pos[idx]);
					npos = mapx(val);
				}
				else if (npos < 0)
				{
					int ppos = val - rmapxDev(width()*3 / 4);
					if (ppos < 0)
						ppos = 0;
					emit followEvent(ppos);
					opos = mapx(pos[idx]);
					npos = mapx(val);
				}
				break;
			case Song::CONTINUOUS:
				if (npos > (width() / 2))
				{
					int ppos = pos[idx] - rmapxDev(width() / 2);
					if (ppos < 0)
						ppos = 0;
					emit followEvent(ppos);
					opos = mapx(pos[idx]);
					npos = mapx(val);
				}
				else if (npos < (width() / 2))
				{
					int ppos = pos[idx] - rmapxDev(width() / 2);
					if (ppos < 0)
						ppos = 0;
					emit followEvent(ppos);
					opos = mapx(pos[idx]);
					npos = mapx(val);
				}
				break;
		}
	}

	int x;
	int w = 1;
	if (opos > npos)
	{
		w += opos - npos;
		x = npos;
	}
	else
	{
		w += npos - opos;
		x = opos;
	}
	pos[idx] = val;
	redraw(QRect(x - 1, 0, w + 2, height()));
}

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void Master::leaveEvent(QEvent*)
{
	emit tempoChanged(-1);
	emit timeChanged(MAXINT);
}

//---------------------------------------------------------
//   pdraw
//---------------------------------------------------------

void Master::pdraw(QPainter& p, const QRect& rect)
{
	View::pdraw(p, rect); // calls draw()
	p.resetTransform();

	int x = rect.x();
	int y = rect.y();
	int w = rect.width() + 2;
	int h = rect.height();

	int wh = height();
	//---------------------------------------------------
	// draw Canvas Items
	//---------------------------------------------------

	const TempoList* tl = &tempomap;
	for (ciTEvent i = tl->begin(); i != tl->end(); ++i)
	{
		TEvent* e = i->second;
		int etick = mapx(i->first);
		int stick = mapx(i->second->tick);
		int tempo = mapy(280000 - int(60000000000.0 / (e->tempo)));

		if (tempo < 0)
			tempo = 0;
		if (tempo < wh)
		{
			//QColor green = QColor(119, 169, 181, 127);
			//QColor yellow = QColor(41, 130, 140);
			//QColor red = QColor(0, 37, 46, 127);
			//QLinearGradient vuGrad(QPointF(0, 0), QPointF(0, height()));
			QColor green = QColor(config.partWaveColors[1]);
			green.setAlpha(140);
			QColor yellow = QColor(41, 130, 140);
			QColor red = QColor(config.partColors[1]);
			red.setAlpha(140);
			QLinearGradient vuGrad(QPointF(0, 0), QPointF(0, height()));
			vuGrad.setColorAt(1, green);
			vuGrad.setColorAt(0, red);
			QPen myPen = QPen();
			myPen.setBrush(QBrush(vuGrad));
			p.fillRect(stick, tempo, etick - stick, wh, QBrush(vuGrad));
			p.setPen(red);
			p.drawLine(stick, tempo, etick, tempo);
		}
	}

	//---------------------------------------------------
	//    draw marker
	//---------------------------------------------------

	int xp = mapx(pos[0]);
	if (xp >= x && xp < x + w)
	{
		p.setPen(QColor(0, 186, 255));
		p.drawLine(xp, y, xp, y + h);
	}
	xp = mapx(pos[1]);
	if (xp >= x && xp < x + w)
	{
		p.setPen(QColor(139, 225, 69));
		p.drawLine(xp, y, xp, y + h);
	}
	xp = mapx(pos[2]);
	if (xp >= x && xp < x + w)
	{
		p.setPen(QColor(139, 225, 69));
		p.drawLine(xp, y, xp, y + h);
	}
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Master::draw(QPainter& p, const QRect& rect)
{
	drawTickRaster(p, rect.x(), rect.y(), rect.width(), rect.height(), 0, false);
}

//---------------------------------------------------------
//   viewMousePressEvent
//---------------------------------------------------------

void Master::viewMousePressEvent(QMouseEvent* event)
{
	start = event->pos();
	Tool activeTool = tool;
	bool shift = ((QInputEvent*) event)->modifiers() & Qt::ShiftModifier;
	bool ctrl = ((QInputEvent*) event)->modifiers() & Qt::ControlModifier;
	bool alt = ((QInputEvent*) event)->modifiers() & Qt::AltModifier;
	if(shift || ctrl || alt)
		return;

	switch (activeTool)
	{
		case PointerTool:
			drag = DRAG_LASSO_START;
			break;

		case PencilTool:
			drag = DRAG_NEW;
			song->startUndo();
			newVal(start.x(), start.x(), start.y());
			break;

		case RubberTool:
			drag = DRAG_DELETE;
			song->startUndo();
			deleteVal(start.x(), start.x());
			break;

		default:
			break;
	}
}

//---------------------------------------------------------
//   viewMouseMoveEvent
//---------------------------------------------------------

void Master::viewMouseMoveEvent(QMouseEvent* event)
{
	QPoint pos = event->pos();
	bool shift = ((QInputEvent*) event)->modifiers() & Qt::ShiftModifier;
	bool ctrl = ((QInputEvent*) event)->modifiers() & Qt::ControlModifier;
	bool alt = ((QInputEvent*) event)->modifiers() & Qt::AltModifier;
	if(shift || ctrl || alt)
		return;
	//      QPoint dist = pos - start;
	//      bool moving = dist.y() >= 3 || dist.y() <= 3 || dist.x() >= 3 || dist.x() <= 3;

	switch (drag)
	{
		case DRAG_NEW:
			newVal(start.x(), pos.x(), pos.y());
			start = pos;
			break;

		case DRAG_DELETE:
			deleteVal(start.x(), pos.x());
			start = pos;
			break;

		default:
			break;
	}
	emit tempoChanged(280000 - event->y());
	int x = pos.x();
	if (x < 0)
		x = 0;
	emit timeChanged(editor->rasterVal(x));
}

//---------------------------------------------------------
//   viewMouseReleaseEvent
//---------------------------------------------------------

void Master::viewMouseReleaseEvent(QMouseEvent*)
{
	switch (drag)
	{
		case DRAG_RESIZE:
		case DRAG_NEW:
		case DRAG_DELETE:
			song->endUndo(SC_TEMPO);
			break;
		default:
			break;
	}
	drag = DRAG_OFF;
}

//---------------------------------------------------------
//   deleteVal
//---------------------------------------------------------

bool Master::deleteVal1(unsigned int x1, unsigned int x2)
{
	bool songChanged = false;

	TempoList* tl = &tempomap;
	for (iTEvent i = tl->begin(); i != tl->end(); ++i)
	{
		if (i->first < x1)
			continue;
		if (i->first >= x2)
			break;
		iTEvent ii = i;
		++ii;
		if (ii != tl->end())
		{
			int tempo = ii->second->tempo;
			audio->msgDeleteTempo(i->first, tempo, false);
			songChanged = true;
		}
	}
	return songChanged;
}

void Master::deleteVal(int x1, int x2)
{
	//printf("Master::deleteVal\n");
	if (deleteVal1(editor->rasterVal1(x1), x2))
		redraw();
}

//---------------------------------------------------------
//   setTool
//---------------------------------------------------------

void Master::setTool(int t)
{
	if (tool == Tool(t))
		return;
	tool = Tool(t);
	switch (tool)
	{
		case PencilTool:
			setCursor(QCursor(*pencilCursorIcon, 6, 15));
			break;
		default:
			setCursor(QCursor(Qt::ArrowCursor));
			break;
	}
}

//---------------------------------------------------------
//   newVal
//---------------------------------------------------------

void Master::newVal(int x1, int x2, int y)
{
	int xx1 = editor->rasterVal1(x1);
	int xx2 = editor->rasterVal2(x2);

	if (xx1 > xx2)
	{
		int tmp = xx2;
		xx2 = xx1;
		xx1 = tmp;
	}
	deleteVal1(xx1, xx2);
	int yp = (60000000000.0 / (280000 - y));
	//qDebug("Master::newVal: ypos: %d, y: %d", yp, y);
	//audio->msgAddTempo(xx1, int(60000000000.0 / (280000 - y)), false);
	audio->msgAddTempo(xx1, yp, false);
	redraw();
}
