/* Paint
 * Copyright (C) 2014 Krzysztof Konopko <krzysztof.konopko@konagma.pl>
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "document.h"

#include <QActionGroup>
#include <QMainWindow>
#include <QUndoStack>

namespace Paint {

/**
 * @brief The MainWindow class representing the main application's window.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow() = default;

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void open();
    void save();

    void penColor();
    void penWidth();

private:

    void createMenus();
    bool maybeSave();
    bool saveFile(const QByteArray &fileFormat);

    QActionGroup brushActionGroup;
    QUndoStack undoStack;

    Document doc;
};

} // namespace Paint

#endif // MAINWINDOW_H
