#ifndef D3D11WIDGET_H
#define D3D11WIDGET_H

#include <QWidget>
#include <QPoint>

#include "Engine.h"
#include "editorglobal.h"

namespace Ui {
class D3D11Widget;
}

using namespace RENDER_MASTER;

class RenderWidget : public QWidget
{
    Q_OBJECT

    HWND h;

    RENDER_MASTER::ICore *pCore{nullptr};
	RENDER_MASTER::ICoreRender *pCoreRender{nullptr};
	RENDER_MASTER::IRender *pRender{nullptr};
	RENDER_MASTER::ISceneManager *pSceneManager{nullptr};
	RENDER_MASTER::IResourceManager *pResMan{nullptr};

	int rightMouse{};
    QPoint lastMousePos;
    float dx{}, dy{};

	int needCaptureId = 0;
	uint captureX = 0, captureY = 0;

    uint currentMouseX = 0, currentMouseY = 0;

    int key_w{};
    int key_s{};
    int key_a{};
    int key_d{};
    int key_q{};
    int key_e{};

	int isFocusing = 0;
	vec3 focusingTargetPosition;

	const float rotateSpeed = 13.0f;
	const float moveSpeed = 40.0f;

	// resources
	IMesh *_pGridMesh;

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

	void drawManipulator(ICamera *pCamera);
	void drawGrid(const mat4& VP);


private slots:
    void onEngineInited(RENDER_MASTER::ICore *pCore);
    void onEngineClosed(RENDER_MASTER::ICore *pCore);
    void onRender();
	void onUpdate(float dt);
	void onFocusAtSelected(const vec3& worldCeneter, const RENDER_MASTER::AABB& aabb);

private:

	Ui::D3D11Widget *ui;
};

#endif // D3D11WIDGET_H
