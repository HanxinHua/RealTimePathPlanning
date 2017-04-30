#pragma once
#include"simulationParam.h"
#include"nodeStruct.h"
#include"obstacle.h"
#include<list>
#include"SMP.h"
#include"Robot.h"
#include"RRTstar.h"
#include"InformedRRTstar.h"
#include"RT-RRTstar.h"
//--------------------------------------------------------------class
class Enviroment
{
public:
	//--------------------------------------------------------------Function
	// Default constructor  
	Enviroment() { setup(); };
	Enviroment(ofVec2f _start) { setup(_start); };
	// Default destructor  
	~Enviroment() {};
	// Setup method
	void setup();
	void setup(ofVec2f _start);
	void update(Robot * car, list<obstacles*> obst);
	void targetSet(ofVec2f loc);
	// Update method
	void update(Robot *car);
	// Render method draw nodes in enviroment.
	void render();

	void renderGrid();
	//--------------------------------------------------------------Variables
	bool grid = false;
	bool goalin = false;
private:
	//--------------------------------------------------------------Variables
protected:
	//--------------------------------------------------------------Variables
	std::list<Nodes> nodes;
	//std::list<obstacles> obst;
	std::list<Nodes*> path;
	RRTstar rrtstar;
	InformedRRTstar irrtstar;
	RTRRTstar rtrrtstar;
	bool rrtFlag = true;
	bool planner = true;

	ofVec2f goal;
	ofVec2f home;
	//Robot *car;

	// A list or an array of Obstacles should come here

};

inline void Enviroment::setup()
{
	home.set(startx, starty);
	Nodes start(startx, starty, 0);
	this->nodes.push_back(start);
	SMP::start.set(startx, starty);
	SMP::goalFound = false;
}


inline void Enviroment::setup(ofVec2f _start)
{
	home = _start;

	Nodes start(home.x, home.y, 0);
	this->nodes.push_back(start);

	SMP::root = &(this->nodes.front());
	goal.set(goalx, goaly);
	SMP::start.set(startx, starty);
	SMP::goalFound = false;
}

inline void Enviroment::update(Robot *car,list<obstacles*> obst)
{
	//RRTstar - 
	//rrtstar.nextIter(nodes, obst);

	//Informed RRT*-
	//irrtstar.nextIter(nodes, obst);
	//InformedRRTstar::usingInformedRRTstar = true;

	//RTRRTstar-
	if (car->getLocation().distance(SMP::goal) < converge)
		planner = false;

	if (planner)
	{
		car->fillEnviroment(obst, nodes);
		car->controller(SMP::root->location);
		car->update();
	}

	rtrrtstar.nextIter(nodes, obst, car);

	if (planner && SMP::target != NULL)
	{
		path = rtrrtstar.currPath;
		rtrrtstar.currPath.clear();
	}
	
	/* Following is Informed RRT-star stuff
	if (SMP::sampledInGoalRegion)
	SMP::sampledInGoalRegion = false;

	if (SMP::target != NULL && !SMP::moveNow && InformedRRTstar::usingInformedRRTstar)
	{
		path.clear();
		Nodes *pathNode = SMP::target;
		do
		{
			path.push_back(*pathNode);
			pathNode = pathNode->parent;
		} while (pathNode->parent != NULL);
		//planner = !planner;
		path.reverse();
	}*/
	/*
	if (SMP::moveNow && InformedRRTstar::usingInformedRRTstar) {
		std::list<Nodes>::iterator pathIt = path.begin();

		while(pathIt !=path.end()){
			if (pathIt->alive) {
				car->controller(pathIt->location);
				float dist= pathIt->location.distance(car->getLocation());
				if (dist < 2.0) pathIt->alive = false;
				break;
			}
			pathIt++;
		}
		if(pathIt!=path.end())
		car->update();

	}*/
	//TODO: Looking at the car pose and its field of view look for obstacles falling in this field of view 
	//and make the cost to reach for the nodes falling within specified radius as infinity. mark the alive flag for this node as false
	//Mark each children of each of these nodes(till we reach the leaf node) as 'affected'(a boolean on Nodes class)
	//and display these children in a different colour. Possibly, check at the time of drawing - if(alive and affected), then-draw in yellow colour
}

inline void Enviroment::targetSet(ofVec2f loc)
{
	goal = loc;
	SMP::goal = goal;
	RTRRTstar::goalDefined = true;
	
	//TODO: Add Multi-Query Behaviour
	goalin = true;
}

inline void Enviroment::render()
{
	ofEnableAlphaBlending();

	ofSetColor({150, 0, 255});
	if (goalin) {
		ofFill();
		ofDrawCircle(goal.x, goal.y, NODE_RADIUS+2);
		ofNoFill();
		ofSetLineWidth(2);
		ofDrawCircle(goal.x, goal.y, converge);
	}

	for (auto i : this->nodes)
	{
		ofSetColor({ 10,10,10 },150);

		if (i.costToStart == inf) ofSetColor({ 200,0,0 },120);
		
		ofSetLineWidth(2);
		if (i.parent != NULL) {
			ofPoint pt;ofPolyline line;
			pt.set(i.location.x, i.location.y);line.addVertex(pt);
			pt.set(i.parent->location.x, i.parent->location.y);line.addVertex(pt);
			line.draw();
		}
		//ofSetColor({ 10,10,250 },80);
		ofSetLineWidth(1);
		//if (i.prevParent != NULL) {
		//	
		//	ofPoint pt; ofPolyline line;
		//	pt.set(i.location.x, i.location.y); line.addVertex(pt);
		//	pt.set(i.prevParent->location.x, i.prevParent->location.y); line.addVertex(pt);
		//	line.draw();
		//}
		//int hue = i.alive ? 130 : 80;
		//ofSetColor(i.color, hue);
		//ofDrawCircle(i.location.x, i.location.y, NODE_RADIUS);
	}
	if (!path.empty())
	{
		ofSetColor({ 10,250,10 });
		ofSetLineWidth(5);
		for (auto i : path) {
			if (i->parent != NULL) {
				ofPoint pt; ofPolyline line;
				pt.set(i->location.x, i->location.y); line.addVertex(pt);
				pt.set(i->parent->location.x, i->parent->location.y); line.addVertex(pt);
				line.draw();
			}
		}
		ofSetLineWidth(1);
	}
	ofDisableAlphaBlending();
}

inline void Enviroment::renderGrid()
{
	ofEnableAlphaBlending();
	ofSetColor(20, 130, 0, 50);
	for (int i = 0; i < ofGetWindowWidth(); i += 5)
	{
		ofDrawLine(i, 0, i, ofGetWindowHeight());
	}
	for (int j = 0; j < ofGetWindowHeight(); j += 5)
	{
		ofDrawLine(0, j, ofGetWindowWidth(), j);
	}
	ofDisableAlphaBlending();
}