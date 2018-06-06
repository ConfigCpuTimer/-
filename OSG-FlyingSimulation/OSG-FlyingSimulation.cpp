// OSG-FlyingSimulation.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "MyManipulator.h"

#include <iostream>
#include <cmath>

#include <osg/Geometry>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/Shape>
#include <osg/ShapeDrawable>

#include <osgAnimation/Animation>
#include <osgAnimation/AnimationUpdateCallback>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Channel>
#include <osgAnimation/EaseMotion>//冗余标头，但文件定义的Motion类中的getValue()方法可以解释
#include <osgAnimation/Interpolator>
#include <osgAnimation/Sampler>//Sampler中的getValueAt()
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/Target>
#include <osgAnimation/UpdateMatrixTransform>

#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgWidget/Label>
#include <osgWidget/Table>
#include <osgWidget/WindowManager>


class MyLabel :public osgWidget::Label {
protected:
	osg::ref_ptr<osgAnimation::Animation> m_anim;
	osg::ref_ptr<osgAnimation::BasicAnimationManager> m_bam;

public:
	MyLabel(const std::string& label) :osgWidget::Label(label, "") {
		setFont("fonts/VeraMono.ttf");
		
		setFontSize(14);
		setFontColor(1.0f, 1.0f, 1.0f, 1.0f);

		setColor(0.3f, 0.3f, 0.3f, 1.0f);
		setPadding(2.0f);
		setCanFill(true);

		addSize(150.0f, 25.0f);

		setLabel(label);
		setEventMask(osgWidget::EVENT_MOUSE_PUSH | osgWidget::EVENT_MASK_MOUSE_MOVE);
	}

	/*void setAnimationAndManager(
		osg::ref_ptr<osgAnimation::BasicAnimationManager> bam,
		osg::ref_ptr<osgAnimation::Animation> anim) {
		
		m_bam = bam;
		m_anim = anim;
		bam->registerAnimation(m_anim);
	}*/


	bool mousePush(double, double, const osgWidget::WindowManager*) {
		osg::ref_ptr<osgWidget::Table> p = dynamic_cast<osgWidget::Table*> (_parent);

		if (!p)
			return false;
			
		p->hide();

		const std::string& name = getName();

		if (!m_bam->findAnimation(m_anim))
			return false;

		if (!name.compare("Start")) {
			m_bam->playAnimation(m_anim);
		}
		else if (!name.compare("Stop")) {
			m_bam->stopAnimation(m_anim);
		}

		return true;
	}

	bool mouseEnter(double, double, const osgWidget::WindowManager*) {
		setColor(0.9f, 0.6f, 0.1f, 1.0f);

		return true;
	}

	bool mouseLeave(double, double, const osgWidget::WindowManager*) {
		setColor(0.3f, 0.3f, 0.3f, 1.0f);

		return true;
	}
};


class MyLabelMenu :public MyLabel {
private:
	osg::ref_ptr<osgWidget::Table> m_window;

public:
	MyLabelMenu(const std::string& label) :MyLabel(label) {
		m_window = new osgWidget::Table(std::string("Menu" + label, 6, 5));

		m_window->addWidget(new MyLabel("Start"), 0, 0);
		m_window->addWidget(new MyLabel("Stop"), 1, 0);

		m_window->resize();
	}

	void managed(osgWidget::WindowManager* wm) {
		osgWidget::Label::managed(wm);

		wm->addChild(m_window.get());

		m_window->hide();
	}

	void positioned() {
		osgWidget::Label::positioned();

		m_window->setOrigin(_parent->getX(), _parent->getY() + _parent->getHeight());
	}

	bool mousePush(double, double, const osgWidget::WindowManager*) {
		if (!m_window->isVisible()) m_window->show();

		else m_window->hide();

		return true;
	}
};


osg::ref_ptr<osg::Vec3Array> cubicBezier(
	osg::Vec3 Start,
	osg::Vec3 CtrlA,
	osg::Vec3 CtrlB,
	osg::Vec3 End,
	float delta) {

	osg::ref_ptr<osg::Vec3Array> Points = new osg::Vec3Array();

	for (float i = 0.0; i < 1.0; i += delta) {
		float x =
			powf(1 - i, 3)*Start.x() +
			3 * i*powf(1 - i, 2)*CtrlA.x() +
			2 * (1 - i)*powf(i, 2)*CtrlB.x() +
			powf(i, 3)*End.x();

		float y =
			powf(1 - i, 3)*Start.y() +
			3 * i*powf(1 - i, 2)*CtrlA.y() +
			2 * (1 - i)*powf(i, 2)*CtrlB.y() +
			powf(i, 3)*End.y();

		float z =
			powf(1 - i, 3)*Start.z() +
			3 * i*powf(1 - i, 2)*CtrlA.z() +
			2 * (1 - i)*powf(i, 2)*CtrlB.z() +
			powf(i, 3)*End.z();

		osg::Vec3 TempPoint = osg::Vec3(x, y, z);
		Points->push_back(TempPoint);
	}

	return Points.release();
}


osg::ref_ptr<osg::Vec3Array> createPathArray(/*float delta*/) {
	osg::ref_ptr<osg::Vec3Array> points = new osg::Vec3Array();

	osg::Vec3 PointA(100.0, -250.0, 100.0);
	osg::Vec3 PointB(100.0, -1.0, 150.0);
	osg::Vec3 CtrlPointA(100.0, -100.0, 100.0);
	osg::Vec3 CtrlPointB(100.0, -180.0, 200.0);

	osg::ref_ptr<osg::Vec3Array> StripeA = 
		cubicBezier(PointA, CtrlPointA, CtrlPointB, PointB, 0.05f).get();

	for (osg::Vec3Array::iterator itr = StripeA->begin();
		itr != StripeA->end();
		itr++) {
		points->push_back(*itr);
	}//t==4.0

	for (float i = 0.0;i < 1.225;i += 0.025) {
		float yaw = (float)i*2.0f*osg::PI;
		osg::Vec3 TempPoint(cosf(yaw) * 100.0, sinf(yaw) * 100.0, 150.0f);
		points->push_back(TempPoint);
	}//t==11.8

	osg::Vec3 PointC(0.0, 101.0, 150.0);
	osg::Vec3 PointD(-250.0, 180.0, 230.0);
	osg::Vec3 CtrlPointC(-120.0, 105.0, 165.0);
	osg::Vec3 CtrlPointD(-180.0, 108.0, 180.0);

	osg::ref_ptr<osg::Vec3Array> StripeB =
		cubicBezier(PointC, CtrlPointC, CtrlPointD, PointD, 0.05f).get();

	for (osg::Vec3Array::iterator itr = StripeB->begin();
		itr != StripeB->end();
		itr++) {
		points->push_back(*itr);
	}

	return points.release();
}


osg::ref_ptr<osg::Geode> createTrace(/*float delta*/) {
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> vertices = createPathArray(/*delta*/);
	osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array();//

	color->push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));

	geometry->setVertexArray(vertices);
	geometry->setColorArray(color, osg::Array::BIND_OVERALL);
	geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, vertices->size()));

	geode->addDrawable(geometry);

	return geode.release();
}


void createVec3Keyframes(
	osg::ref_ptr<osgAnimation::Vec3LinearChannel> ch,
	osg::ref_ptr<osg::Vec3Array> arr,
	float delta) {
	
	osg::ref_ptr<osgAnimation::Vec3KeyframeContainer> kfs = ch->getOrCreateSampler()->getOrCreateKeyframeContainer();
	float t = 0.0;

	for (osg::Vec3Array::iterator itr = arr->begin();
		itr != arr->end();
		itr++) {
		kfs->push_back(osgAnimation::Vec3Keyframe(t, *itr));
		t += delta;
	}

	return;
}


void createQuatKeyframes(
	osg::ref_ptr<osgAnimation::QuatSphericalLinearChannel> ch,
	float delta) {

	osg::ref_ptr<osgAnimation::QuatKeyframeContainer> kfs = ch->getOrCreateSampler()->getOrCreateKeyframeContainer();

	kfs->push_back(osgAnimation::QuatKeyframe(0.0, osg::Quat(0, osg::X_AXIS)));
	kfs->push_back(osgAnimation::QuatKeyframe(2.0, osg::Quat(osg::PI_4, osg::X_AXIS)));
	kfs->push_back(osgAnimation::QuatKeyframe(4.0, osg::Quat(0, osg::X_AXIS)));
	kfs->push_back(osgAnimation::QuatKeyframe(6.0, osg::Quat(osg::PI_2, osg::Z_AXIS)));
	kfs->push_back(osgAnimation::QuatKeyframe(8.0, osg::Quat(osg::PI, osg::Z_AXIS)));
	kfs->push_back(osgAnimation::QuatKeyframe(10.0, osg::Quat(osg::PI_2*3, osg::Z_AXIS)));
	kfs->push_back(osgAnimation::QuatKeyframe(12.0, osg::Quat(osg::PI*2, osg::Z_AXIS)));
	kfs->push_back(osgAnimation::QuatKeyframe(14.0, osg::Quat(osg::PI_2*5, osg::Z_AXIS)));
    //kfs->push_back(osgAnimation::QuatKeyframe(16.0,osg::Quat())

	return;
}


osg::ref_ptr<osgAnimation::Vec3LinearChannel> createPathChannel(
	const std::string& channelName,
	const std::string& targetName,
	float delta) {

	osg::ref_ptr<osgAnimation::Vec3LinearChannel> ch = new osgAnimation::Vec3LinearChannel();
	ch->setName(channelName);
	ch->setTargetName(targetName);

	osg::ref_ptr<osg::Vec3Array> arr = createPathArray(/*0.02f*/);
	createVec3Keyframes(ch.get(), arr.get(), delta);

	return ch.release();
}


osg::ref_ptr<osgAnimation::QuatSphericalLinearChannel> createQuatChannel(
	const std::string& channelName,
	const std::string& targetName,
	float delta) {

	osg::ref_ptr<osgAnimation::QuatSphericalLinearChannel> ch = new osgAnimation::QuatSphericalLinearChannel();
	ch->setName(channelName);
	ch->setTargetName(targetName);

	createQuatKeyframes(ch.get(), delta);

	return ch.release();
}


int _tmain(int argc, char** argv) {
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer();

	osg::ref_ptr<osgWidget::Window> menu = new osgWidget::Box("menu", osgWidget::Box::HORIZONTAL);
	menu->addWidget(new MyLabelMenu("choose action"));
	menu->getBackground()->setColor(1.0f, 1.0f, 1.0f, 1.0f);
	menu->setPosition(15.0f, 15.0f, 15.0f);
	menu->attachMoveCallback();
	
	//const unsigned int MASK_2D = 0xf0000000;
	osg::ref_ptr<osgWidget::WindowManager> wm =
		new osgWidget::WindowManager(
			viewer.get(),
			800,
			600,
			1,
			osgWidget::WindowManager::WM_USE_RENDERBINS);
	wm->addChild(menu);

	osg::ref_ptr<osgAnimation::Vec3LinearChannel> position = 
		createPathChannel("position", "PlayCallback", 0.2f);
	osg::ref_ptr<osgAnimation::QuatSphericalLinearChannel> euler = 
		createQuatChannel("euler", "PlayCallback", 0.2f);

	osg::ref_ptr<osgAnimation::Animation> anim = new osgAnimation::Animation();
	anim->setPlayMode(osgAnimation::Animation::LOOP);
	anim->addChannel(position.get());
	anim->addChannel(euler.get());

	osg::ref_ptr<osgAnimation::UpdateMatrixTransform> updater = new osgAnimation::UpdateMatrixTransform("PlayCallback");
	updater->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("position"));
	updater->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("euler"));

	osg::ref_ptr<osg::MatrixTransform> animRoot = new osg::MatrixTransform();
	animRoot->addChild(osgDB::readNodeFile("B737.ive"));
	animRoot->setDataVariance(osg::Object::DYNAMIC);
	animRoot->setUpdateCallback(updater.get());

	osg::ref_ptr<osgAnimation::BasicAnimationManager> manager = 
		new osgAnimation::BasicAnimationManager;
	manager->registerAnimation(anim.get());

	osg::ref_ptr<osg::Geode> path = createTrace(/*0.02f*/);

	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(animRoot.get());
	root->addChild(path.get());
	root->addChild(osgDB::readNodeFile("lz.osg"));
	root->setUpdateCallback(manager.get());

	manager->playAnimation(anim.get());
	
	viewer->setCameraManipulator(new MyManipulator);
	/*viewer->getCameraManipulator()->setHomePosition(
		osg::Y_AXIS,
		osg::Vec3d(100.0, -250.0, 200.0),
		osg::Z_AXIS,
		true
	);*/
	viewer->setSceneData(root.get());
	//viewer->home();

	return osgWidget::createExample(*viewer, wm.get(), root.get());
}
//QWidget
//GUIEventHandler
//应先注册动画
//w s