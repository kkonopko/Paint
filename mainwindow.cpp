/* Paint
 * Copyright (C) 2014 Krzysztof Konopko <krzysztof.konopko@konagma.pl>
 */

#include "mainwindow.h"

#include "document.h"
#include "shape.h"

#include <QtWidgets>

#include <functional>
#include <utility>

namespace Paint {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), brushActionGroup(this), doc(&undoStack)
{
    createMenus();

    setCentralWidget(&doc);

    setWindowTitle(tr("Paint"));
    resize(500, 500);

    brushActionGroup.actions().first()->trigger();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::open()
{
    if (maybeSave()) {
        const QString fileName =
            QFileDialog::getOpenFileName(this, tr("Open File"),
                                         QDir::currentPath());
        if (!fileName.isEmpty()) {
            doc.openImage(fileName);
        }
    }
}

void MainWindow::save()
{
    saveFile("bmp");
}

void MainWindow::penColor()
{
    const QColor newColor = QColorDialog::getColor(doc.getPenColor());
    if (newColor.isValid()) {
        doc.setPenColor(newColor);
    }
}

void MainWindow::penWidth()
{
    bool ok;
    const int newWidth = QInputDialog::getInt(this, tr("Scribble"),
                                              tr("Select pen width:"),
                                              doc.getPenWidth(),
                                              1, 50, 1, &ok);
    if (ok) {
        doc.setPenWidth(newWidth);
    }
}

void MainWindow::createMenus()
{
    using namespace std::placeholders;

    QMenu *const fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Open..."), this, SLOT(open()), QKeySequence::Open);
    fileMenu->addAction("&Save As...", this, SLOT(save()));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"),this, SLOT(close()), QKeySequence::Quit);

    QMenu *const editMenu = menuBar()->addMenu(tr("&Edit"));

    QAction *const undoAct = undoStack.createUndoAction(editMenu, tr("&Undo"));
    undoAct->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undoAct);

    QAction *const redoAct = undoStack.createRedoAction(editMenu, tr("&Redo"));
    redoAct->setShortcut(QKeySequence::Redo);
    editMenu->addAction(redoAct);

    QMenu *const brushMenu = menuBar()->addMenu(tr("&Brush"));
    brushMenu->addAction(tr("&Pen Color..."), this, SLOT(penColor()));
    brushMenu->addAction(tr("Pen &Width..."), this, SLOT(penWidth()));

    brushMenu->addSeparator();

    const std::pair<QString, Document::shape_factory_t> shapeActions[] = {
        std::make_pair(tr("Ellipse"), createEllipse),
        std::make_pair(tr("Eraser"), createEraser),
        std::make_pair(tr("Fill"), std::bind(createFill, &doc, _1, _2, _3)),
        std::make_pair(tr("Rectangle"), createRectangle),
        std::make_pair(tr("Line"), createScribble)
    };

    for (const auto &actionDesc : shapeActions) {
        QAction *const act = brushMenu->addAction(actionDesc.first);

        connect(act, &QAction::triggered,
                std::bind(&Document::setShapeFactory, &doc, actionDesc.second));
        brushActionGroup.addAction(act)->setCheckable(true);
    }

    brushActionGroup.setExclusive(true);

    QMenu *const effectsMenu = menuBar()->addMenu(tr("&Effects"));

    const std::pair<QString, std::function<void()>> effectActions[] = {
        std::make_pair(tr("Flip Horizontally"),
                       std::bind(&Document::flip, &doc, true, false)),
        std::make_pair(tr("Flip Vertically"),
                       std::bind(&Document::flip, &doc, false, true)),
        std::make_pair(tr("Rotate Right 90 deg"),
                       std::bind(&Document::rotate, &doc, 90.0)),
        std::make_pair(tr("Rotate Left 90 deg"),
                       std::bind(&Document::rotate, &doc, -90.0)),
        std::make_pair(tr("Rotate 180 deg"),
                       std::bind(&Document::rotate, &doc, 180.0))
    };

    for (const auto &effectDesc : effectActions) {
        connect(effectsMenu->addAction(effectDesc.first),
                &QAction::triggered, effectDesc.second);
    }
}

bool MainWindow::maybeSave()
{
    if (doc.isModified()) {
       const QMessageBox::StandardButton ret =
               QMessageBox::warning(this, tr("Paint"),
                                    tr("The image has been modified.\n"
                                       "Do you want to save your changes?"),
                                    QMessageBox::Save |
                                    QMessageBox::Discard |
                                    QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            return saveFile("bmp");
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }

    return true;
}

bool MainWindow::saveFile(const QByteArray &fileFormat)
{
    const QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;

    const QString fileName =
            QFileDialog::getSaveFileName(this, tr("Save As"),
                                         initialPath,
                                         tr("%1 Files (*.%2);;All Files (*)")
                                         .arg(QString::fromLatin1(
                                                  fileFormat.toUpper()))
                                         .arg(QString::fromLatin1(fileFormat)));
    return !fileName.isEmpty() &&
            doc.saveImage(fileName, fileFormat.constData());
}

} // namespace Paint
