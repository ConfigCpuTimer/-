#include <osg/AnimationPath>
#include <osg/Geode>
#include <osg/Group>
#include <osg/LineSegment>
#include <osg/Node>
#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

#include <osgGA/CameraManipulator>

#include <osgUtil/IntersectVisitor>

#include <osgViewer/Viewer>


class MyManipulator :public osgGA::CameraManipulator{
public:
	MyManipulator();
	~MyManipulator() {}

	void setCollisionDetect(bool);
	bool getCollisionDetect();

	void setDetectState();//如果碰撞检测开启则关闭，如果关闭则开启 
	
	virtual void setNode(osg::Node*);
	
	virtual void setByMatrix(const osg::Matrixd&) {}
	virtual void setByInverseMatrix(const osg::Matrixd&) {}
	virtual osg::Matrixd getMatrix() const;
	virtual osg::Matrixd getInverseMatrix() const;//得到逆矩阵
	
	virtual bool handle(const osgGA::GUIEventAdapter&, osgGA::GUIActionAdapter&);//主要事件控制器

	void setSpeed(float);
	float getSpeed();
	
	void changePosition(osg::Vec3&);//位置变换函数

	void setViewPoint(osg::Vec3&);//设置视点位置 
	void setViewPoint(double*);
	osg::Vec3 getViewPoint();

	void computeHomePosition();
	
private:
	osg::ref_ptr<osg::Node> m_ondNode;
	osg::Vec3 m_ov3Position;
	osg::Vec3f m_ov3Rotation;
	unsigned int m_iID;
	float m_fSpeed;
	float m_fAngle;//角速度
	float m_fCoordinateX;
	float m_fCoordinateY;
	bool m_bLeftButtonDown;
	bool m_bCollisionDetect;
};