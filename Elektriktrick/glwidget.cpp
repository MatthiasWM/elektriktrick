/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwidget.h"

#include "ETApp.h"
#include "ETModel.h"
#include "ETTriangle.h"
#include "ETVector.h"

#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QTimer>
#include <QMimeData>

#include <math.h>

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    ET.linkWith(this);
    xRot = 432;
    yRot = 5032;
    zRot = 0;
    gear1Rot = 0;
    zPos = 0.1;

    //QTimer *timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(advanceGears()));
    //timer->start(20);
}

GLWidget::~GLWidget()
{
    ET.linkWith(0L);
    makeCurrent();
}

void GLWidget::setXRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::initializeGL()
{
    static const GLfloat lightPos[4] = { 5.0f, 5.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    static const GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    static const GLfloat mat_shininess[] = { 50.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel (GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glScaled(zPos, zPos, zPos);

    glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(zRot / 16.0, 0.0, 0.0, 1.0);

    // draw my STL model
    if (ET.model())
        ET.model()->Draw(0L, 1024, 1024);

    glPopMatrix();
}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, +1.0, -1.0, 1.0, 5.0, 60.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -40.0);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }
    lastPos = event->pos();
}

bool GLWidget::event(QEvent * e) {
    //static int i = 0;
    //fprintf(stderr, "MainWindow: NativeEvent %d = %d\n", i++, e->type());
    switch (e->type()) {
    case QEvent::Wheel:
        break;
    case Qt::ZoomNativeGesture:
    case 197: { // zoom
        float dz = ((QNativeGestureEvent*)e)->value();
        if (dz<-0.05) dz = 0.0;  // There seem to be glitches on the touch pad that create much larger values
        if (dz> 0.05) dz = 0.0;
        zPos = zPos  * (1.0+(2.0*dz));
        fprintf(stderr, "Gesture: %g (%d, %d, %d, %g)\n", dz, xRot, yRot, zRot, zPos);
        e->accept();
        updateGL();
        return true; }
    default:
        break;
    }
    return QGLWidget::event(e);
}

void GLWidget::advanceGears()
{
    gear1Rot += 2 * 16;
    updateGL();
}


void GLWidget::normalizeAngle(int *angle)
{
    while (*angle < 0)
        *angle += 360 * 16;
    while (*angle > 360 * 16)
        *angle -= 360 * 16;
}


void GLWidget::dragEnterEvent(QDragEnterEvent *event)
{
    printf("dragEnter Event\n"); fflush(stdout);
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
    // formats text html
    //QStringList sl = event->mimeData()->formats();
    //QString t = event->mimeData()->text();
    //QString u = event->mimeData()->html();

}


void GLWidget::dropEvent(QDropEvent *event)
{
    printf("dropEvent Event\n"); fflush(stdout);
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
        QString t = event->mimeData()->text();

        if (t.startsWith("file://")) {
            ET.loadModel(t.toUtf8().data()+7);
        }

    }

    //textBrowser->setPlainText(event->mimeData()->text());
    //mimeTypeCombo->clear();
    //mimeTypeCombo->addItems(event->mimeData()->formats());

}
