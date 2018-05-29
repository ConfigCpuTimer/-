// OSG-FlyingSimulation.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <cmath>

#include <osg/io_utils>
#include <osg/Geometry>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/MatrixTransform>

#include <osgAnimation/Animation>
#include <osgAnimation/AnimationUpdateCallback>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Channel>
#include <osgAnimation/EaseMotion>//冗余标头，但文件定义的Motion类中的getValue()方法可以解释
#include <osgAnimation/Interpolator>
#include <osgAnimation/Sampler>//Sampler中的getValueAt()
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/StackedRotateAxisElement>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/Target>
#include <osgAnimation/UpdateMatrixTransform>

#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>


osg::ref_ptr<osg::Vec3Array> createCubicBezierPath(float delta){
	osg::ref_ptr<osg::Vec3Array> points = new osg::Vec3Array();

	osg::Vec3 startPoint(0.0, 0.0, 0.0); //= *(setPathPoints()->begin());
	osg::Vec3 endPoint(100.0, 0.0, 60.0);// = *(setPathPoints()->end());

	osg::Vec3 ctrlPointA(25.0, 0.0, 40.0);// = *(setControlPoints()->begin());
	osg::Vec3 ctrlPointB(75.0, 0.0, 20.0);// = *(setControlPoints()->end());

	for (float t = 0.0; t < 1.0; t += delta){
		float x =
			powf(1 - t, 3)*startPoint.x() +
			3 * t*powf(1 - t, 2)*ctrlPointA.x() +
			2 * (1 - t)*powf(t, 2)*ctrlPointB.x() +
			powf(t, 3)*endPoint.x();

		float y =
			powf(1 - t, 3)*startPoint.y() +
			3 * t*powf(1 - t, 2)*ctrlPointA.y() +
			2 * (1 - t)*powf(t, 2)*ctrlPointB.y() +
			powf(t, 3)*endPoint.y();
		
		float z =
			powf(1 - t, 3)*startPoint.z() +
			3 * t*powf(1 - t, 2)*ctrlPointA.z() +
			2 * (1 - t)*powf(t, 2)*ctrlPointB.z() +
			powf(t, 3)*endPoint.z();

		osg::Vec3 tempPoint = osg::Vec3(x, y, z);
		points->push_back(tempPoint);
	}

	return points.release();
}


void createKeyframes(
	osg::ref_ptr<osgAnimation::Vec3LinearChannel> ch,
	osg::ref_ptr<osg::Vec3Array> arr,
	float delta
) {
	osg::ref_ptr<osgAnimation::Vec3KeyframeContainer> kfs = ch->getOrCreateSampler()->getOrCreateKeyframeContainer();
	float t = 0.0;

	for (
		osg::Vec3Array::iterator itr = arr->begin();
		itr != arr->end();
		itr++
		) {
		kfs->push_back(osgAnimation::Vec3Keyframe(t, *itr));
		t += delta;
	}
}


osg::ref_ptr<osgAnimation::Vec3LinearChannel> createPathChannel(
	const std::string& channelName,
	const std::string& targetName,
	float delta
){
	osg::ref_ptr<osgAnimation::Vec3LinearChannel> ch = new osgAnimation::Vec3LinearChannel();
	ch->setName(channelName);
	ch->setTargetName(targetName);
	
	createKeyframes(ch, createCubicBezierPath(0.02f), delta);

	return ch.release();
}


int _tmain(int argc, char** argv) {	
	osg::ref_ptr<osgAnimation::Vec3LinearChannel> ch = createPathChannel("position", "PlayCallback", 0.2f);

	osg::ref_ptr<osgAnimation::Animation> anim = new osgAnimation::Animation();
	anim->setPlayMode(osgAnimation::Animation::LOOP);
	anim->addChannel(ch.get());

	osg::ref_ptr<osgAnimation::UpdateMatrixTransform> updater = new osgAnimation::UpdateMatrixTransform("PathCallback");
	updater->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("position"));

	osg::ref_ptr<osg::MatrixTransform> animRoot = new osg::MatrixTransform;
	animRoot->addChild(osgDB::readNodeFile("cessna.osg.0,0,90.rot"));
	animRoot->setDataVariance(osg::Object::DYNAMIC);
	animRoot->setUpdateCallback(updater.get());

	osg::ref_ptr<osgAnimation::BasicAnimationManager> manager = new osgAnimation::BasicAnimationManager;
	manager->registerAnimation(anim.get());

	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(animRoot.get());
	root->addChild(osgDB::readNodeFile("ceep.ive"));
	root->setUpdateCallback(manager.get());

	manager->playAnimation(anim.get());

	//osg::ref_ptr<osgGA::TrackballManipulator> tbm = new osgGA::TrackballManipulator();
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer();
	//viewer->setCameraManipulator(tbm);
	viewer->setSceneData(root.get());
	return viewer->run();
}


//osg::ref_ptr<osg::Geode> createTrace(){
//	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
//	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
//	osg::ref_ptr<osg::Vec3Array> vertices = createPathVertexArray();
//	osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array();//
//
//	color->push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
//
//	geometry->setVertexArray(vertices);
//	geometry->setColorArray(color, osg::Array::BIND_OVERALL);
//	geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, vertices->size()));
//
//	geode->addDrawable(geometry);
//
//	return geode.release();
//}


//int _tmain(int argc, char** argv){
//	osg::ArgumentParser arguments(&argc, argv);
//
//	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer(arguments);
//	//osg::ref_ptr<osgGA::TrackballManipulator> tbm = new osgGA::TrackballManipulator();
//	//viewer->setCameraManipulator(tbm);
//
//	osg::ref_ptr<osg::AnimationPath> ap = createAnimationPath();
//	osg::ref_ptr<osg::AnimationPathCallback> apc = new osg::AnimationPathCallback(ap);
//
//	osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform();
//	osg::ref_ptr<osg::MatrixTransform> cessna = new osg::MatrixTransform();
//	osg::ref_ptr<osg::MatrixTransform> building = new osg::MatrixTransform();
//	
//	//cessna->setMatrix(osg::Matrix::scale(0.1, 0.1, 0.1));
//	cessna->addChild(osgDB::readNodeFile("cessna.osg.0,0,90.rot"));
//	cessna->setUpdateCallback(apc);
//
//	building->setMatrix(osg::Matrix::translate(osg::Vec3(0.0, 0.0, -50.0)));
//	building->addChild(osgDB::readNodeFile("FreeEarth_flat.earth"));
//
//	root->addChild(cessna.get());
//	//root->addChild(createTrace().get());
//	root->addChild(building.get());
//
//	viewer->setSceneData(root.get());
//	return viewer->run();
//}