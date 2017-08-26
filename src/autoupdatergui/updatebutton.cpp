#include "updatebutton.h"
#include "updatebutton_p.h"
#include "ui_updatebutton.h"
#include "updatecontroller.h"

#include <QtAutoUpdaterCore/updater.h>

using namespace QtAutoUpdater;

UpdateButton::UpdateButton(QWidget *parent, UpdateController *controller) :
	QWidget(parent),
	d(new UpdateButtonPrivate(this, controller))
{}

UpdateButton::~UpdateButton(){}

QString UpdateButton::animationFile() const
{
	return d->loadingGif->fileName();
}

bool UpdateButton::isShowingResult() const
{
	return d->showResult;
}

UpdateController::DisplayLevel UpdateButton::displayLevel() const
{
	return d->level;
}

UpdateController *UpdateButton::controller() const
{
	return d->controller;
}

void UpdateButton::resetState()
{
	d->loadingGif->setPaused(true);
	d->ui->loaderLabel->setVisible(false);
	d->ui->statusLabel->setVisible(false);
	d->ui->checkButton->setEnabled(true);
}

void UpdateButton::setAnimationFile(QString animationFile, int speed)
{
	d->loadingGif->setFileName(animationFile);
	d->loadingGif->setSpeed(speed);
}

void UpdateButton::setAnimationDevice(QIODevice *animationDevice, int speed)
{
	d->loadingGif->setDevice(animationDevice);
	d->loadingGif->setSpeed(speed);
}

void UpdateButton::resetAnimationFile()
{
	d->loadingGif->setFileName(QStringLiteral(":/QtAutoUpdater/icons/updateRunning.gif"));
	d->loadingGif->setSpeed(200);
}

void UpdateButton::setShowResult(bool showResult)
{
	d->showResult = showResult;
}

void UpdateButton::setDisplayLevel(UpdateController::DisplayLevel displayLevel)
{
	d->level = displayLevel;
}

bool UpdateButton::setController(UpdateController *controller)
{
	if(d->loadingGif->state() != QMovie::Running) {//TODO why?
		d->changeController(controller);
		resetState();
		emit controllerChanged(controller);
		return true;
	} else
		return false;
}

void UpdateButton::startUpdate()
{
	if(!d->controller.isNull())
		d->controller->start(d->level);
}

void UpdateButton::changeUpdaterState(int result)
{
	switch (result) {
	case Updater::Running:
		d->loadingGif->start();
		d->ui->loaderLabel->setVisible(true);
		d->ui->checkButton->setEnabled(false);
		d->ui->statusLabel->setText(tr("Checking for updates…"));
		d->ui->statusLabel->setVisible(true);
		break;
	case Updater::NoUpdates:
		d->loadingGif->setPaused(true);
		d->ui->loaderLabel->setVisible(false);
		d->ui->checkButton->setEnabled(true);
		d->ui->statusLabel->setText(tr("No new updates available"));
		d->ui->statusLabel->setVisible(d->showResult);
		break;
	case Updater::HasUpdates:
		d->loadingGif->setPaused(true);
		d->ui->loaderLabel->setVisible(false);
		d->ui->checkButton->setEnabled(false);
		d->ui->statusLabel->setText(tr("New Update!"));
		d->ui->statusLabel->setVisible(d->showResult);
		break;
	case Updater::HasError:
		d->loadingGif->setPaused(true);
		d->ui->loaderLabel->setVisible(false);
		d->ui->checkButton->setEnabled(true);
		d->ui->statusLabel->setText(tr("Update check failed"));
		d->ui->statusLabel->setVisible(d->showResult);
		break;
	default:
		Q_UNREACHABLE();
		break;
	}
}

void UpdateButton::controllerDestroyed()
{
	resetState();
	setDisabled(true);
	emit controllerChanged(nullptr);
}

//-----------------PRIVATE IMPLEMENTATION-----------------

QtAutoUpdater::UpdateButtonPrivate::UpdateButtonPrivate(UpdateButton *q_ptr, UpdateController *controller) :
	q(q_ptr),
	controller(nullptr),
	ui(new Ui::UpdateButton),
	level(UpdateController::ExtendedInfoLevel),
	loadingGif(new QMovie(QStringLiteral(":/QtAutoUpdater/icons/updateRunning.gif"), QByteArray(), q_ptr)),
	showResult(true)
{
	ui->setupUi(q);
	loadingGif->setSpeed(200);

	ui->loaderLabel->setMovie(loadingGif);
	ui->loaderLabel->setVisible(false);
	ui->statusLabel->setVisible(false);

	QObject::connect(ui->checkButton, &QPushButton::clicked,
					 q, &UpdateButton::startUpdate);

	changeController(controller);
}

UpdateButtonPrivate::~UpdateButtonPrivate() {}

void UpdateButtonPrivate::changeController(UpdateController *controller)
{
	this->controller = controller;
	if(controller) {
		QObject::connect(this->controller->updater(), &Updater::stateChanged,
						 q, [this](Updater::UpdaterState state) {
			q->changeUpdaterState(state);
		});
		QObject::connect(this->controller.data(), &UpdateController::destroyed,
						 q, &UpdateButton::controllerDestroyed);
	}
	q->setEnabled(controller);
}
