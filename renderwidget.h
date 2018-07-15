#ifndef D3D11WIDGET_H
#define D3D11WIDGET_H

#include <QWidget>
#include <QPoint>

#include "Engine.h"

namespace Ui {
class D3D11Widget;
}

class RenderWidget : public QWidget
{
    Q_OBJECT

    HWND h;

    RENDER_MASTER::ICore *pCore{nullptr};
    RENDER_MASTER::ICoreRender *pCoreRender{nullptr};
    RENDER_MASTER::ISceneManager *pSceneManager{nullptr};
    RENDER_MASTER::ICamera *pCamera{nullptr};

    int mouse{};
    QPoint lastMousePos;
    float dx{}, dy{};

    int key_w{};
    int key_s{};
    int key_a{};
    int key_d{};
    int key_q{};
    int key_e{};

public:
    explicit RenderWidget(QWidget *parent = 0);
    ~RenderWidget();

    virtual QPaintEngine* paintEngine() const { return NULL; }

protected:
    virtual void resizeEvent(QResizeEvent* evt) override;
    virtual void paintEvent(QPaintEvent* evt) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;


private slots:
    void onEngineInited(RENDER_MASTER::ICore *pCore);
    void onEngineClosed(RENDER_MASTER::ICore *pCore);
    void onRender();
    void onUpdate(float dt);

private:
    Ui::D3D11Widget *ui;
};

#endif // D3D11WIDGET_H
