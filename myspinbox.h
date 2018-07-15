#ifndef MYSPINBOX_H
#define MYSPINBOX_H
#include <QDoubleSpinBox>
#include <QDebug>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QCursor>

class MySpinBox : public QDoubleSpinBox
{
	Q_OBJECT

public:
	MySpinBox(QWidget *parent = 0, const char *name = 0)
	{
		installEventFilter(this);
	}
	~MySpinBox(){}

protected:
	virtual void mousePressEvent(QMouseEvent* event) override
	{
		//qDebug() << "mousePressEvent";
		QCursor c = cursor();
		mouseStartPosY = c.pos().y();
		startValue = value();
		screens = 0;
		QDoubleSpinBox::mousePressEvent(event);
	}


	virtual void mouseMoveEvent(QMouseEvent* event) override
	{
		QRect rec = QApplication::desktop()->screenGeometry();
		int height = rec.height();
		int width = rec.width();

		setCursor(Qt::SizeVerCursor);
		const float multiplier = .3f;

		QCursor c = cursor();

		int valueOffset = float ((
				int(screens == 0) * int(mouseStartPosY - c.pos().y()) +
				int(screens > 0) * (-1) * ((height - mouseStartPosY) + (screens - 1) * height + c.pos().y()) +
				int(screens < 0) * (mouseStartPosY + (-screens - 1) * height + (height - c.pos().y()))
				 )* multiplier);

		setValue(startValue + valueOffset);

		// up -> down
		if (c.pos().y() <= 0)
		{
			c.setPos(c.pos().x(), height - 1);
			setCursor(c);
			screens--;
		}

		// down -> up
		else if (c.pos().y() >= height - 1)
		{
			c.setPos(c.pos().x(), 1);
			setCursor(c);
			screens++;
		}
		QDoubleSpinBox::mouseMoveEvent(event);
	}


	virtual void mouseReleaseEvent(QMouseEvent* event) override
	{
		//qDebug() << "mouseReleaseEvent";
		unsetCursor();
		QDoubleSpinBox::mouseReleaseEvent(event);
	}

	// clear focus on enter pressed
	bool virtual eventFilter(QObject* obj, QEvent* event)
	{
		if (event->type()==QEvent::KeyPress) {
			QKeyEvent* key = static_cast<QKeyEvent*>(event);
			if ( (key->key()==Qt::Key_Enter) || (key->key()==Qt::Key_Return) ) {
				clearFocus();
			} else {
				return QObject::eventFilter(obj, event);
			}
			return true;
		} else {
			return QObject::eventFilter(obj, event);
		}
		return false;
	}

private:
	int mouseStartPosY{0};
	int startValue{0};
	int screens{0};
};

#endif // MYSPINBOX_H
