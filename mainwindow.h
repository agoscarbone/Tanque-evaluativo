#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <Qtimer>

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

//--------------------------------
public slots:
    void Qin(int value);
    void Qout(int value);
    void QoutA(int value);
    void QoutB(int value);
    void dial_max();
    void t_max();
    void tanklevel();

private:
    QTimer timer;
};
#endif // MAINWINDOW_H
