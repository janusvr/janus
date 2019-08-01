#ifndef ROOMPHYSICS_H
#define ROOMPHYSICS_H

#include <QtCore>

//physical simulation stuff
#include <btBulletDynamicsCommon.h>

#include "player.h"
#include "roomobject.h"

//Various collision group and mask defines
#define BIT(x) (1<<(x))
enum RoomPhysicsColliderType : short
{
    COL_NOTHING = 0,        //Collide with nothing
    COL_PLAYER = BIT(0),    //Collide with player
    COL_WALL = BIT(1),      //Collide with walls
    COL_BASEPLANE = BIT(2)  //Collide with base plane (a special plane used only when near a portal and COL_WALL suppressed)
};

class RoomPhysics : public QObject
{
public:

    RoomPhysics();
    ~RoomPhysics();

    void SetGravity(const float f);
    void SetPlayerGravity(const float f);
    void SetJumpVelocity(const float f);

    void UpdateSimulation(const double dt);

    //a supportive plane on XZ that spots player from falling when near portals and environment collisions are suppressed
    void AddGroundPlane(const float y);
    void RemoveGroundPlane();

    void AddRoomTemplate(const QPointer <RoomObject> o);
    void AddRoomPlaneTemplate(const float y);

    void AddPlayerShape(const QPointer <Player> player);
    void RemovePlayerShape();

    void AddRigidBody(const QPointer <RoomObject> o, short group, short mask);
    btRigidBody * GetRigidBody(const QPointer <RoomObject> o);    
    QVector3D GetRigidBodyPos(const QString js_id);
    QVector3D GetRigidBodyXDir(const QString js_id);
    QVector3D GetRigidBodyScale(const QString js_id);
    QVector3D GetRigidBodyVel(const QString js_id);
    float GetRigidBodyMass(const QString js_id);
    QSet <QString> GetRigidBodyCollisions(const QPointer <RoomObject> o);
    void UpdateFromRigidBody(const QPointer <RoomObject> o); //set the RoomObject to match the physics simulation
    void UpdateToRigidBody(const QPointer <RoomObject> o); //set the physics simulation to match the RoomObject
    void RemoveRigidBody(const QPointer <RoomObject> o);

    void UpdateFromRigidBody(QPointer <Player> player);
    void UpdateToRigidBody(QPointer <Player> player);

    void SetGroundPlane(const float y);

    void DoPlayerGroundTest(const QPointer <Player> player);
    bool GetPlayerOnGround() const;
    void SetPlayerNearPortal(const bool b, const float portal_y);

    void SetLinearVelocity(const QPointer <RoomObject> o, QVector3D v);

    void Clear();

private:

    void AddSphere(const QPointer <RoomObject> o, short group, short mask);
    void AddCube(const QPointer <RoomObject> o, short group, short mask);
    void AddCylinder(const QPointer <RoomObject> o, short group, short mask);
    void AddCapsule(const QPointer <RoomObject> o, short group, short mask);

    void AddMesh(const QPointer <RoomObject> o, short group, short mask);

    //generic, used by above
    void AddShape(const QPointer <RoomObject> o, btCollisionShape * shape, const btScalar btMass, const btVector3 btInertia, short group, short mask);

    btBroadphaseInterface * broadphase;
    btDefaultCollisionConfiguration * collisionConfiguration;
    btCollisionDispatcher * dispatcher;
    btSequentialImpulseConstraintSolver * solver;
    btDiscreteDynamicsWorld * dynamicsWorld;

    bool added_room_template;
    bool added_player;
    bool player_on_ground;
    bool player_near_portal;
    float player_near_portal_height;
    float jump_vel;

    QHash <QString, btRigidBody *> rigidBodies;
    QHash <QString, btDefaultMotionState *> motionStates;

    QHash <QString, btSphereShape *> collisionShapes_sphere;
    QHash <QString, btMultiSphereShape *> collisionShapes_multiSphere;
    QHash <QString, btBoxShape *> collisionShapes_box;
    QHash <QString, btCylinderShape *> collisionShapes_cylinder;
    QHash <QString, btCapsuleShape *> collisionShapes_capsule;
    QHash <QString, btStaticPlaneShape *> collisionShapes_staticPlane;
    QHash <QString, btTriangleMesh *> collisionShapes_triangleMesh;
    QHash <QString, btBvhTriangleMeshShape *> collisionShapes_BvhTriangleMesh;

    QHash <btRigidBody *, QString> rigidBodyJSIDs;
    QHash <QString, QVector3D> rigidBodyScales;
    QHash <QString, float> rigidBodyMasses;    
    QHash <QString, QSet<QString> > rigidBodyCollisions;
};


#endif // ROOMPHYSICS_H
