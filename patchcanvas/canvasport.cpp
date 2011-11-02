#include "canvasport.h"
#include "canvasbox.h"
#include "canvasline.h"
#include "canvaslinemov.h"
#include "canvasbezierline.h"
#include "canvasbezierlinemov.h"

#include <QCursor>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QMenu>
#include <QPainter>
#include <QTimer>

START_NAMESPACE_PATCHCANVAS

        extern Canvas canvas;
extern options_t options;
extern features_t features;

CanvasPort::CanvasPort(int port_id_, QString port_name_, PortMode port_mode_, PortType port_type_, QGraphicsItem* parent) :
        QGraphicsItem(parent, canvas.scene)
{  
    // Save Variables, useful for later
    port_id   = port_id_;
    port_mode = port_mode_;
    port_type = port_type_;
    port_name = port_name_;

    // Base Variables
    port_width  = 15;
    port_height = 15;
    port_font   = QFont(canvas.theme->port_font_name, canvas.theme->port_font_size, canvas.theme->port_font_state);

    mov_line   = 0;
    hover_item = 0;
    last_selected_state = false;

    mouse_down    = false;
    moving_cursor = false;

    setFlags(QGraphicsItem::ItemIsSelectable);
}

int CanvasPort::getPortId()
{
    return port_id;
}

PortMode CanvasPort::getPortMode()
{
    return port_mode;
}

PortType CanvasPort::getPortType()
{
    return port_type;
}

QString CanvasPort::getPortName()
{
    return port_name;
}

QString CanvasPort::getFullPortName()
{
    return ((CanvasBox*)parentItem())->getText()+":"+port_name;
}

int CanvasPort::getPortWidth()
{
    return port_width;
}

int CanvasPort::getPortHeight()
{
    return port_height;
}

void CanvasPort::setPortMode(PortMode port_mode_)
{
    port_mode = port_mode_;
    update();
}

void CanvasPort::setPortType(PortType port_type_)
{
    port_type = port_type_;
    update();
}

void CanvasPort::setPortName(QString port_name_)
{
    if (QFontMetrics(port_font).width(port_name_) < QFontMetrics(port_font).width(port_name))
        QTimer::singleShot(0, canvas.scene, SIGNAL(update()));

    port_name = port_name_;
    update();
}

void CanvasPort::setPortWidth(int port_width_)
{
    if (port_width_ < port_width)
        QTimer::singleShot(0, canvas.scene, SIGNAL(update()));

    port_width = port_width_;
    update();
}

int CanvasPort::type() const
{
    return CanvasPortType;
}

void CanvasPort::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        mouse_down = true;
    else
        mouse_down = false;

    hover_item = 0;
    moving_cursor = false;

    QGraphicsItem::mousePressEvent(event);
}

void CanvasPort::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (mouse_down)
    {
        if (!moving_cursor)
        {
            setCursor(QCursor(Qt::CrossCursor));
            moving_cursor = true;

            for (int i=0; i < canvas.connection_list.count(); i++)
            {
                if (canvas.connection_list[i].port_out_id == port_id || canvas.connection_list[i].port_in_id == port_id)
                {
                    if (options.bezier_lines)
                        ((CanvasBezierLine*)canvas.connection_list[i].widget)->setLocked(true);
                    else
                        ((CanvasLine*)canvas.connection_list[i].widget)->setLocked(true);
                }
            }
        }

        if (!mov_line)
        {
            if (options.bezier_lines)
            {
                CanvasBezierLineMov* new_mov_line = new CanvasBezierLineMov(port_mode, port_type, this);
                new_mov_line->setZValue(canvas.last_z_value);
                mov_line = new_mov_line;
            }
            else
            {
                CanvasLineMov* new_mov_line = new CanvasLineMov(port_mode, port_type, this);
                new_mov_line->setZValue(canvas.last_z_value);
                mov_line = new_mov_line;
            }

            parentItem()->setZValue(canvas.last_z_value+1);
            canvas.last_z_value += 2;
        }

        CanvasPort* item = 0;
        QList<QGraphicsItem*> items = canvas.scene->items(event->scenePos(), Qt::ContainsItemShape, Qt::AscendingOrder);
        for (int i=0; i < items.count(); i++)
        {
            if (items[i]->type() == CanvasPortType)
            {
                if (items[i] != this)
                {
                    if (!item)
                        item = (CanvasPort*)items[i];
                    else if (items[i]->parentItem()->zValue() > item->parentItem()->zValue())
                        item = (CanvasPort*)items[i];
                }
            }
        }

        if (hover_item and hover_item != item)
            hover_item->setSelected(false);

        if (item)
        {
            if (item->getPortMode() != port_mode &&
                (item->getPortType() == port_type || (options.connect_midi2outro &&
                                                      ((item->getPortType() == PORT_TYPE_MIDI  && port_type == PORT_TYPE_OUTRO) ||
                                                       (item->getPortType() == PORT_TYPE_OUTRO && port_type == PORT_TYPE_MIDI))
                                                      )))
            {
                item->setSelected(true);
                hover_item = item;
            }
            else
                hover_item = 0;
        }
        else
            hover_item = 0;

        if (options.bezier_lines)
            ((CanvasBezierLineMov*)mov_line)->updateLinePos(event->scenePos());
        else
            ((CanvasLineMov*)mov_line)->updateLinePos(event->scenePos());
    }

    QGraphicsItem::mouseMoveEvent(event);
}

void CanvasPort::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (mouse_down)
    {
        if (mov_line)
        {
            canvas.scene->removeItem(mov_line);
            delete mov_line;
            mov_line = 0;
        }

        for (int i=0; i < canvas.connection_list.count(); i++)
        {
            if (canvas.connection_list[i].port_out_id == port_id || canvas.connection_list[i].port_in_id == port_id)
            {
                if (options.bezier_lines)
                    ((CanvasBezierLine*)canvas.connection_list[i].widget)->setLocked(false);
                else
                    ((CanvasLine*)canvas.connection_list[i].widget)->setLocked(false);
            }
        }

        if (hover_item)
        {
            bool check = false;
            for (int i=0; i < canvas.connection_list.count(); i++)
            {
                if ( (canvas.connection_list[i].port_out_id == port_id && canvas.connection_list[i].port_in_id == hover_item->getPortId()) ||
                     (canvas.connection_list[i].port_out_id == hover_item->getPortId() && canvas.connection_list[i].port_in_id == port_id) )
                {
                    canvas.callback(ACTION_PORTS_DISCONNECT, canvas.connection_list[i].connection_id, 0, "");
                    check = true;
                    break;
                }
            }

            if (!check)
            {
                if (port_mode == PORT_MODE_OUTPUT)
                    canvas.callback(ACTION_PORTS_CONNECT, port_id, hover_item->getPortId(), "");
                else
                    canvas.callback(ACTION_PORTS_CONNECT, hover_item->getPortId(), port_id, "");
            }

            canvas.scene->clearSelection();
        }
    }

    if (moving_cursor)
        setCursor(QCursor(Qt::ArrowCursor));

    hover_item = 0;
    mouse_down = false;
    moving_cursor = false;

    QGraphicsItem::mouseReleaseEvent(event);
}

void CanvasPort::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    canvas.scene->clearSelection();
    setSelected(true);

    QMenu menu;

    QList<int> port_list; // = canvas.callback(ACTION_REQUEST_PORT_CONNECTION_LIST, port_id, ((CanvasBox*)parentItem())->text+":"+port_name);
    QMenu discMenu("Disconnect", &menu);
    if (port_list.count() > 0)
    {
        for (int i=0; i < port_list.count(); i++)
        {
            //QAction* act_x_disc = discMenu.addAction(port_list[i][1]);
            //QObject::connect(act_x_disc, SIGNAL(triggered()), SLOT(contextMenuDisconnect()));
        }
    }
    else
    {
        QAction* act_x_disc = discMenu.addAction("No connections");
        act_x_disc->setEnabled(false);
    }

    menu.addMenu(&discMenu);
    QAction* act_x_disc_all = menu.addAction("Disconnect &All");
    /*QAction* act_x_sep_1 =*/ menu.addSeparator();
                               QAction* act_x_info = menu.addAction("Get &Info");
                               QAction* act_x_rename = menu.addAction("&Rename");

                               if (!features.port_rename)
                                   act_x_rename->setVisible(false);

                               QAction* act_selected = menu.exec(event->screenPos());

                               //if (act_selected == act_x_info)
                               //  canvas.callback(ACTION_PORT_INFO, port_id);

                               /*else*/ if (act_selected == act_x_rename)
                               {
                                            bool ok_check;
                                            QString new_name = QInputDialog::getText(0, "Rename Port", "New name:", QLineEdit::Normal, port_name, &ok_check);
                                            if (ok_check and !new_name.isEmpty())
                                            {
                                                //canvas.callback(ACTION_PORT_RENAME, port_id, new_name);
                                            }
                                        }
                                        //else if (act_selected == act_x_disc_all)
                                        //    canvas.callback(ACTION_PORT_DISCONNECT_ALL, port_id, ((CanvasBox*)parentItem().text)+":"+port_name);

                                        event->accept();
                                    }

void CanvasPort::contextMenuDisconnect(int port_idx)
{
    for (int i=0; i < canvas.connection_list.count(); i++)
    {
        if ( (canvas.connection_list[i].port_in_id == port_id and canvas.connection_list[i].port_out_id == port_idx) ||
             (canvas.connection_list[i].port_out_id == port_id and canvas.connection_list[i].port_in_id == port_idx) )
        {
            //canvas.callback(ACTION_PORTS_DISCONNECT, canvas.connection_list[i].connection_id);
            break;
        }
    }
}

QRectF CanvasPort::boundingRect() const
{
    return QRectF(0, 0, port_width+12, port_height);
}

void CanvasPort::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setRenderHint(QPainter::Antialiasing, (options.antialiasing == Qt::Checked));

    QPointF text_pos;
    int poly_locx[5] = {0};

    if (port_mode == PORT_MODE_INPUT)
    {
        text_pos = QPointF(3, 12);

        if (canvas.theme->port_mode == Theme::THEME_PORT_POLYGON)
        {
            poly_locx[0] = 0;
            poly_locx[1] = port_width+5;
            poly_locx[2] = port_width+12;
            poly_locx[3] = port_width+5;
            poly_locx[4] = 0;
        }
        else if (canvas.theme->port_mode == Theme::THEME_PORT_SQUARE)
        {
            poly_locx[0] = 0;
            poly_locx[1] = port_width+5;
            poly_locx[2] = port_width+5;
            poly_locx[3] = port_width+5;
            poly_locx[4] = 0;
        }
        else
        {
            qWarning("Invalid THEME_PORT mode");
            return;
        }
    }
    else if (port_mode == PORT_MODE_OUTPUT)
    {
        text_pos = QPointF(9, 12);

        if (canvas.theme->port_mode == Theme::THEME_PORT_POLYGON)
        {
            poly_locx[0] = port_width+12;
            poly_locx[1] = 7;
            poly_locx[2] = 0;
            poly_locx[3] = 7;
            poly_locx[4] = port_width+12;
        }
        else if (canvas.theme->port_mode == Theme::THEME_PORT_SQUARE)
        {
            poly_locx[0] = port_width+12;
            poly_locx[1] = 5;
            poly_locx[2] = 5;
            poly_locx[3] = 5;
            poly_locx[4] = port_width+12;
        }
        else
        {
            qWarning("Invalid THEME_PORT mode");
            return;
        }
    }
    else
    {
        qWarning("Error: Invalid Port Mode!");
        return;
    }

    QColor poly_color;
    QPen poly_pen;

    if (port_type == PORT_TYPE_AUDIO)
    {
        poly_color = isSelected() ? canvas.theme->port_audio_bg_sel : canvas.theme->port_audio_bg;
        poly_pen = isSelected() ? canvas.theme->port_audio_pen_sel : canvas.theme->port_audio_pen;
    }
    else if (port_type == PORT_TYPE_MIDI)
    {
        poly_color = isSelected() ? canvas.theme->port_midi_bg_sel : canvas.theme->port_midi_bg;
        poly_pen = isSelected() ? canvas.theme->port_midi_pen_sel : canvas.theme->port_midi_pen;
    }
    else if (port_type == PORT_TYPE_OUTRO)
    {
        poly_color = isSelected() ? canvas.theme->port_outro_bg_sel : canvas.theme->port_outro_bg;
        poly_pen = isSelected() ? canvas.theme->port_outro_pen_sel : canvas.theme->port_outro_pen;
    }
    else
    {
        qWarning("Error: Invalid Port Type!");
        return;
    }

    QPolygonF polygon;
    polygon += QPointF(poly_locx[0], 0);
    polygon += QPointF(poly_locx[1], 0);
    polygon += QPointF(poly_locx[2], 7.5);
    polygon += QPointF(poly_locx[3], 15);
    polygon += QPointF(poly_locx[4], 15);

    painter->setBrush(poly_color);
    painter->setPen(poly_pen);
    painter->drawPolygon(polygon);

    painter->setPen(canvas.theme->port_text);
    painter->setFont(port_font);
    painter->drawText(text_pos, port_name);

    //if (isSelected() != last_selected_state)
    //  parentItem()->makeItGlow(port_id, isSelected());

    last_selected_state = isSelected();

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

END_NAMESPACE_PATCHCANVAS
