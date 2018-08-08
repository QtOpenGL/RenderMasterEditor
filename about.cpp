#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::About)
{
	ui->setupUi(this);
	setModal(true);
}

About::~About()
{
	delete ui;
}

void About::on_pushButton_clicked()
{
	close();
}
