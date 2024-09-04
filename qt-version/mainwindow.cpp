#include "mainwindow.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMenuBar>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Погодное приложение");
    setMinimumSize(400, 300);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    cityInput = new QLineEdit(this);
    cityInput->setPlaceholderText("Введите название города");
    mainLayout->addWidget(cityInput);

    QPushButton *fetchButton = new QPushButton("Узнать погоду", this);
    connect(fetchButton, &QPushButton::clicked, this, &MainWindow::fetchWeather);
    mainLayout->addWidget(fetchButton);

    weatherDisplay = new QLabel("Здесь появится информация о погоде", this);
    weatherDisplay->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(weatherDisplay);

    // Создаем меню
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu *fileMenu = menuBar->addMenu("Файл");
    QAction *exitAction = fileMenu->addAction("Выход");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *helpMenu = menuBar->addMenu("Помощь");
    QAction *aboutAction = helpMenu->addAction("О программе");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // Применяем стили
    setStyleSheet(R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2C3E50, stop:1 #3498DB);
        }
        QLineEdit {
            background-color: #ECF0F1;
            color: #2C3E50;
            padding: 5px;
            border-radius: 3px;
            font-size: 14px;
        }
        QPushButton {
            background-color: #E74C3C;
            color: white;
            font-weight: bold;
            padding: 10px;
            border-radius: 5px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #C0392B;
        }
        QLabel {
            color: #ECF0F1;
            font-size: 16px;
        }
        QMenuBar {
            background-color: rgba(52, 73, 94, 0.8);
            color: #ECF0F1;
        }
        QMenuBar::item:selected {
            background-color: #2980B9;
        }
    )");
}

MainWindow::~MainWindow()
{
}

void MainWindow::fetchWeather()
{
    QString city = cityInput->text();
    if (city.isEmpty()) {
        weatherDisplay->setText("Пожалуйста, введите название города");
        return;
    }

    weatherDisplay->setText("Загрузка...");

    QString apiKey = "YOUR_API_KEY"; // Замените на ваш ключ API
    QString url = QString("http://api.openweathermap.org/data/2.5/weather?q=%1&appid=%2&units=metric").arg(city).arg(apiKey);

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::onRequestFinished);
    manager->get(QNetworkRequest(QUrl(url)));
}

void MainWindow::onRequestFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        weatherDisplay->setText("Ошибка: " + reply->errorString());
    } else {
        QByteArray jsonResponse = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonResponse);
        QJsonObject jsonObject = jsonDoc.object();

        // Проверяем, содержит ли JSON объект поле "main"
        if (jsonObject.contains("main")) {
            // Получаем температуру
            QJsonObject main = jsonObject["main"].toObject();
            double temp = main["temp"].toDouble();

            // Получаем описание погоды (например, "ясно", "облачно" и т.д.)
            QJsonArray weatherArray = jsonObject["weather"].toArray();
            QString weatherDescription;
            if (!weatherArray.isEmpty()) {
                QJsonObject weather = weatherArray.first().toObject();
                weatherDescription = weather["description"].toString();
            }

            // Форматируем и выводим результат
            QString weatherText = QString("Температура: %1 °C\nПогода: %2")
                                      .arg(temp)
                                      .arg(weatherDescription);
            weatherDisplay->setText(weatherText);
        } else {
            weatherDisplay->setText("Ошибка: не удалось получить данные о погоде.");
        }
    }
    reply->deleteLater();
}


void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, "О программе",
                       "Погодное приложение\n"
                       "Версия 1.1\n"
                       "Qt приложение для просмотра погоды");
}
