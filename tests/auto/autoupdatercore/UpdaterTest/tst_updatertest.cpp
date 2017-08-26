#include <updater.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QVector>
#include <functional>
#include "installercontroller.h"
using namespace QtAutoUpdater;

#define TEST_DELAY 1000

//define before QtTest include because of macos
inline bool operator==(const QtAutoUpdater::Updater::UpdateInfo &a, const QtAutoUpdater::Updater::UpdateInfo &b)
{
	return (a.name == b.name &&
			a.version == b.version &&
			a.size == b.size);
}

#include <QtTest>

class UpdaterTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();
	void testUpdaterInitState();

	void testUpdateCheck_data();
	void testUpdateCheck();

private:
	Updater *updater;

	InstallerController *controller;
	QSignalSpy *checkSpy;
	QSignalSpy *stateSpy;
	QSignalSpy *updateInfoSpy;
};

void UpdaterTest::initTestCase()
{
#ifdef Q_OS_LINUX
	Q_ASSERT(qgetenv("LD_PRELOAD").contains("Qt5AutoUpdaterCore"));
#endif

	qputenv("QTAUTOUPDATERCORE_PLUGIN_OVERWRITE",
			OUTDIR + QByteArray("../../../../plugins/updaters/"));

	qRegisterMetaType<Updater::UpdaterState>("UpdaterState");

	controller = new InstallerController(this);
	controller->createRepository();
	controller->createInstaller();
	controller->installLocal();
	controller->setVersion({1, 1, 0});
	controller->createRepository();
}

void UpdaterTest::testUpdaterInitState()
{
	QVERIFY(Updater::supportedUpdaterTypes().contains("qtifw"));

	updater = new Updater(this);

	//error state
	QVERIFY(!updater->isValid());//maintenance tool does not exist
	QCOMPARE(updater->updaterType(), QByteArray("qtifw"));
	QVERIFY(updater->errorString().isEmpty());
	QVERIFY(updater->extendedErrorLog().isEmpty());
	QVERIFY(!updater->willRunOnExit());

	//properties
#ifdef Q_OS_OSX
	QCOMPARE(updater->maintenanceToolPath(), QStringLiteral("../../maintenancetool"));
#else
	QCOMPARE(updater->maintenanceToolPath(), QStringLiteral("./maintenancetool"));
#endif
	QCOMPARE(updater->state(), Updater::HasError);//because of the invalid path
	QVERIFY(updater->updateInfo().isEmpty());

	//deprecated fns
	QT_WARNING_PUSH
	QT_WARNING_DISABLE_DEPRECATED
	QVERIFY(!updater->exitedNormally());
	QVERIFY(updater->errorLog().isEmpty());
	QVERIFY(!updater->isRunning());
	QT_WARNING_POP

	delete updater;
}

void UpdaterTest::testUpdateCheck_data()
{
	QTest::addColumn<QString>("toolPath");
	QTest::addColumn<bool>("hasUpdates");
	QTest::addColumn<QList<Updater::UpdateInfo>>("updates");

	QList<Updater::UpdateInfo> updates;
	updates += {QStringLiteral("QtAutoUpdaterTestInstaller"), QVersionNumber::fromString(QStringLiteral("1.1.0")), 45ull};
	QString path = controller->maintenanceToolPath();
	QTest::newRow("QtAutoUpdaterTestInstaller") << path + QStringLiteral("/maintenancetool")
												<< true
												<< updates;

	updates.clear();

	QTest::newRow("Qt") << QStringLiteral(QTDIR) + QStringLiteral("MaintenanceTool")
						<< false
						<< updates;
}

void UpdaterTest::testUpdateCheck()
{
	QFETCH(QString, toolPath);
	QFETCH(bool, hasUpdates);
	QFETCH(QList<Updater::UpdateInfo>, updates);
	auto tState = hasUpdates ? Updater::HasUpdates : Updater::NoUpdates;

	updater = new Updater(toolPath, this);
	QVERIFY(updater);
	checkSpy = new QSignalSpy(updater, &Updater::updateCheckDone);
	QVERIFY(checkSpy->isValid());
	stateSpy = new QSignalSpy(updater, &Updater::stateChanged);
	QVERIFY(stateSpy->isValid());
	updateInfoSpy = new QSignalSpy(updater, &Updater::updateInfoChanged);
	QVERIFY(updateInfoSpy->isValid());

	//start the check updates
	QCOMPARE(updater->state(), Updater::NoUpdates);
	QVERIFY(updater->checkForUpdates());

	//runnig should have changed to true
	QCOMPARE(stateSpy->size(), 1);
	QCOMPARE(stateSpy->takeFirst()[0].toInt(), (int)Updater::Running);
	QCOMPARE(updater->state(), Updater::Running);
	QVERIFY(updateInfoSpy->takeFirst()[0].value<QList<Updater::UpdateInfo>>().isEmpty());

	//wait max 5 min for the process to finish
	QVERIFY(checkSpy->wait(300000));

	//show error log before continuing checking
	QByteArray log = updater->extendedErrorLog();
	if(!log.isEmpty())
		qWarning() << "Error log:" << log;

	//check if the state changed as expected
	QCOMPARE(stateSpy->size(), 1);
	QCOMPARE(stateSpy->takeFirst()[0].toInt(), (int)tState);
	QCOMPARE(updater->state(), tState);

	//verifiy the "hasUpdates" and "updates" are as expected
	QCOMPARE(checkSpy->size(), 1);
	QCOMPARE(checkSpy->takeFirst()[0].toBool(), hasUpdates);
	QCOMPARE(updater->updateInfo(), updates);
	if(hasUpdates) {
		QCOMPARE(updateInfoSpy->size(), 1);
		QCOMPARE(updateInfoSpy->takeFirst()[0].value<QList<Updater::UpdateInfo>>(), updates);
	}

	//verifiy all signalspies are empty
	QVERIFY(checkSpy->isEmpty());
	QVERIFY(stateSpy->isEmpty());
	QVERIFY(updateInfoSpy->isEmpty());

	//-----------schedule mechanism---------------

	int kId = updater->scheduleUpdate(1, true);//every 1 minute
	QVERIFY(kId);
	updater->cancelScheduledUpdate(kId);

	kId = updater->scheduleUpdate(QDateTime::currentDateTime().addSecs(5));
	QVERIFY(updater->scheduleUpdate(QDateTime::currentDateTime().addSecs(2)));
	updater->cancelScheduledUpdate(kId);

	//wait for the update to start
	QVERIFY(stateSpy->wait(2000 + TEST_DELAY));
	//should be running
	QVERIFY(stateSpy->size() > 0);
	QCOMPARE(stateSpy->takeFirst()[0].toInt(), (int)Updater::Running);
	//wait for it to finish if not running
	if(stateSpy->isEmpty())
		QVERIFY(stateSpy->wait(120000));
	//should have stopped
	QCOMPARE(stateSpy->size(), 1);
	QCOMPARE(stateSpy->takeFirst()[0].toInt(), (int)tState);

	//wait for the canceled one (max 5 secs)
	QVERIFY(!stateSpy->wait(5000 + TEST_DELAY));

	//verifiy the runningSpy is empty
	QVERIFY(stateSpy->isEmpty());
	//clear the rest
	checkSpy->clear();
	updateInfoSpy->clear();

	delete updateInfoSpy;
	delete stateSpy;
	delete checkSpy;
	delete updater;
}

QTEST_GUILESS_MAIN(UpdaterTest)

#include "tst_updatertest.moc"
