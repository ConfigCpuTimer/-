#include "stdafx.h"
#include "MyManipulator.h"


MyManipulator::MyManipulator():
m_fSpeed(1.5f), 
m_bLeftButtonDown(false), 
m_fCoordinateX(0.0f), 
m_fAngle(2.5f), 
m_bCollisionDetect(true), 
m_fCoordinateY(0.0f){
	m_ov3Position = osg::Vec3(0.0f, 0.0f, 5.0f);
	m_ov3Rotation = osg::Vec3(osg::PI_2, 0.0f, 0.0f);
}


osg::Matrixd MyManipulator::getMatrix() const {
	osg::Matrix m;
	m.makeRotate(
		m_ov3Rotation._v[0], osg::Vec3(1.0f, 0.0f, 0.0f),
		m_ov3Rotation._v[1], osg::Vec3(0.0f, 1.0f, 0.0f),
		m_ov3Rotation._v[2], osg::Vec3(0.0f, 0.0f, 1.0f)
	);
	return m*osg::Matrix::translate(m_ov3Position);
}


osg::Matrixd MyManipulator::getInverseMatrix() const {
	osg::Matrixd m;
	m.makeRotate(
		m_ov3Rotation._v[0], osg::Vec3(1.0f, 0.0f, 0.0f), 
		m_ov3Rotation._v[1], osg::Vec3(0.0f, 1.0f, 0.0f),
		m_ov3Rotation._v[2], osg::Vec3(0.0f, 0.0f, 1.0f)
	);
	return osg::Matrixd::inverse(m*osg::Matrixd::translate(m_ov3Position));
}


void MyManipulator::changePosition(osg::Vec3& delta) {
	if (m_bCollisionDetect) {
		osg::Vec3 newPos = m_ov3Position + delta;
		osgUtil::IntersectVisitor iv;
		osg::ref_ptr<osg::LineSegment> line = new osg::LineSegment(newPos, m_ov3Position);
		osg::ref_ptr<osg::LineSegment> lineZ = new osg::LineSegment(newPos + osg::Vec3(0.0f, 0.0f, m_fSpeed), newPos - osg::Vec3(0.0f, 0.0f, m_fSpeed));

		iv.addLineSegment(lineZ.get());
		iv.addLineSegment(line.get());
		m_ondNode->accept(iv);

		if (!iv.hits()) {
			m_ov3Position += delta;
		}
	}
	else {
			m_ov3Position += delta;
	}
}


bool MyManipulator::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
	float mouseX = ea.getX();
	float mouseY = ea.getY();

	switch (ea.getEventType()) {
	case(osgGA::GUIEventAdapter::KEYDOWN): {
		if (0x20 == ea.getKey()) {//space
			aa.requestRedraw();
			aa.requestContinuousUpdate(false);
			return true;
		}
		else if (0xff50 == ea.getKey()) {//home
			changePosition(osg::Vec3(0, 0, m_fSpeed));
			return true;
		}
		else if (0xff57 == ea.getKey()) {//end
			changePosition(osg::Vec3(0, 0, -m_fSpeed));
			return true;
		}
		else if (0x2b == ea.getKey()) {//+
			m_fSpeed += 1.0f;
			return true;
		}
		else if (0x2d == ea.getKey()) {//-
			m_fSpeed -= 1.0f;
			if (m_fSpeed < 1.0f) {
				m_fSpeed = 1.0f;
			}
			return true;
		}
		else if (/*0xff52 == ea.getKey() || 0x57 == ea.getKey() || */0x77 == ea.getKey()) {//w up
			changePosition(osg::Vec3(0, m_fSpeed*sinf(osg::PI_2 + m_ov3Rotation._v[2]), 0));
			changePosition(osg::Vec3(m_fSpeed*cosf(osg::PI_2 + m_ov3Rotation._v[2]), 0, 0));
			return true;
		}
		else if (/*ea.getKey() == 0xFF54 || ea.getKey() == 0x53 || */ea.getKey() == 0x73) {//s down
			changePosition(osg::Vec3(0, -m_fSpeed*sinf(osg::PI_2 + m_ov3Rotation._v[2]), 0));
			changePosition(osg::Vec3(-m_fSpeed*cosf(osg::PI_2 + m_ov3Rotation._v[2]), 0, 0));
			return true;
		}
		else if (/*ea.getKey() == 0x41 || */ea.getKey() == 0x61) {//a A
			changePosition(osg::Vec3(0, m_fSpeed*cosf(osg::PI_2 + m_ov3Rotation._v[2]), 0));
			changePosition(osg::Vec3(-m_fSpeed*sinf(osg::PI_2 + m_ov3Rotation._v[2]), 0, 0));
			return true;
		}
		else if (/*ea.getKey() == 0x44 || */ea.getKey() == 0x64) {//d D
			changePosition(osg::Vec3(0, -m_fSpeed*cosf(osg::PI_2 + m_ov3Rotation._v[2]), 0));
			changePosition(osg::Vec3(m_fSpeed*sinf(osg::PI_2 + m_ov3Rotation._v[2]), 0, 0));
			return true;
		}
		else if (0xff53 == ea.getKey()) {//right
			m_ov3Rotation._v[2] -= osg::DegreesToRadians(m_fAngle);
		}
		else if (0xff51 == ea.getKey()) {//left
			m_ov3Rotation._v[2] += osg::DegreesToRadians(m_fAngle);
		}
		else if (0x46 == ea.getKey() || 0x66 == ea.getKey()) {//f F
			computeHomePosition();
			m_fAngle -= 0.2;
			return true;
		}
		else if (0x47 == ea.getKey() || 0x67 == ea.getKey()) {//g G
			m_fAngle += 0.2;
			return true;
		}
		return true;
	}
	case(osgGA::GUIEventAdapter::PUSH): {
		if (1 == ea.getButton()) {
			m_fCoordinateX = mouseX;
			m_fCoordinateY = mouseY;
			m_bLeftButtonDown = true;
		}
		return true;
	}
	case(osgGA::GUIEventAdapter::DRAG): {
		if (m_bLeftButtonDown) {
			m_ov3Rotation._v[2] -= osg::DegreesToRadians(m_fAngle*(mouseX - m_fCoordinateX));
			m_ov3Rotation._v[0] += osg::DegreesToRadians(1.1*(mouseY - m_fCoordinateY));
		}
	}
	case(osgGA::GUIEventAdapter::RELEASE): {
		if (1 == ea.getKey()) {
			m_bLeftButtonDown = false;
		}
		return false;
	}
	default: {
		return false;
	}
	}
}


float MyManipulator::getSpeed() {
	return m_fSpeed;
}


void MyManipulator::setSpeed(float sp) {
	m_fSpeed = sp;
}


void MyManipulator::setViewPoint(osg::Vec3& position) {
	m_ov3Position = position;
}


void MyManipulator::setViewPoint(double* position) {
	for (int i = 0;i < 3;i++) {
		m_ov3Position._v[i] = position[i];
	}
}


osg::Vec3 MyManipulator::getViewPoint() {
	return m_ov3Position;
}


void MyManipulator::setNode(osg::Node* node) {
	m_ondNode = node;
}


void MyManipulator::computeHomePosition() {
	if (m_ondNode.get()) {
		const osg::BoundingSphere& bs = m_ondNode->getBound();
		osg::Vec3 v3 = bs._center;
		setViewPoint(v3);
	}
}


void MyManipulator::setCollisionDetect(bool cd) {
	m_bCollisionDetect = cd;
}


bool MyManipulator::getCollisionDetect() {
	return m_bCollisionDetect;
}


void MyManipulator::setDetectState() {
	m_bCollisionDetect = !m_bCollisionDetect;
}