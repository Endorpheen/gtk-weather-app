#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QNetworkReply>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void fetchWeather();
    void onRequestFinished(QNetworkReply *reply);
    void showAboutDialog();

private:
    QLineEdit *cityInput;
    QLabel *weatherDisplay;
};

#endif // MAINWINDOW_H
