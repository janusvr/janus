#include "roomphysics.h"

RoomPhysics::RoomPhysics()
{
    //qDebug() << "RoomPhysics::RoomPhysics()" << this;
    added_room_template = false;
    added_player = false;
    player_on_ground = true;
    jump_vel = 5.0f;
    player_near_portal = false;
    player_near_portal_height = 0.0f;

    collisionConfiguration = new btDefaultCollisionConfiguration();

    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    dispatcher->setDispatcherFlags(btCollisionDispatcher::CD_STATIC_STATIC_REPORTED);

    broadphase = new btDbvtBroadphase(); //initializes Dynamic AABB tree

    solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.8f, 0)); //sensible gravity default
}

RoomPhysics::~RoomPhysics()
{
    //qDebug() << "RoomPhysics::~RoomPhysics()" << this;
    for (btRigidBody * body : rigidBodies) {
        if (body) {
            dynamicsWorld->removeRigidBody(body);
            if (body->getMotionState()) {
                delete body->getMotionState();
            }
            delete body;
        }
    }
    rigidBodies.clear();
    motionStates.clear();

    for (auto it = collisionShapes_sphere.begin(); it != collisionShapes_sphere.end(); ++it) {
        if (it.value()) {
            delete it.value();
        }
    }
    collisionShapes_sphere.clear();

    for (auto it = collisionShapes_multiSphere.begin(); it != collisionShapes_multiSphere.end(); ++it) {
        if (it.value()) {
            delete it.value();
        }
    }
    collisionShapes_multiSphere.clear();

    for (auto it = collisionShapes_box.begin(); it != collisionShapes_box.end(); ++it) {
        if (it.value()) {
            delete it.value();
        }
    }
    collisionShapes_box.clear();

    for (auto it = collisionShapes_cylinder.begin(); it != collisionShapes_cylinder.end(); ++it) {
        if (it.value()) {
            delete it.value();
        }
    }
    collisionShapes_cylinder.clear();

    for (auto it = collisionShapes_capsule.begin(); it != collisionShapes_capsule.end(); ++it) {
        if (it.value()) {
            delete it.value();
        }
    }
    collisionShapes_capsule.clear();

    for (auto it = collisionShapes_staticPlane.begin(); it != collisionShapes_staticPlane.end(); ++it) {
        if (it.value()) {
            delete it.value();
        }
    }
    collisionShapes_staticPlane.clear();

    for (auto it = collisionShapes_BvhTriangleMesh.begin(); it != collisionShapes_BvhTriangleMesh.end(); ++it) {
        if (it.value()) {
            delete it.value();
        }
    }
    collisionShapes_BvhTriangleMesh.clear();

    for (auto it = collisionShapes_triangleMesh.begin(); it != collisionShapes_triangleMesh.end(); ++it) {
        if (it.value()) {
            delete it.value();
        }
    }
    collisionShapes_triangleMesh.clear();

    rigidBodyJSIDs.clear();
    rigidBodyScales.clear();
    rigidBodyMasses.clear();
    rigidBodyCollisions.clear();

    added_room_template = false;
    added_player = false;

    if (dynamicsWorld) {
        delete dynamicsWorld;
        dynamicsWorld = nullptr;
    }
    if (solver) {
        delete solver;
        solver = nullptr;
    }
    if (dispatcher) {
        delete dispatcher;
        dispatcher = nullptr;
    }
    if (collisionConfiguration) {
        delete collisionConfiguration;
        collisionConfiguration = nullptr;
    }
    if (broadphase) {
        delete broadphase;
        broadphase = nullptr;
    }
}

void RoomPhysics::SetGravity(const float f)
{
    if (dynamicsWorld && dynamicsWorld->getGravity() != btVector3(0, f, 0)) {
        dynamicsWorld->setGravity(btVector3(0, f, 0));
    }
}

void RoomPhysics::SetPlayerGravity(const float f)
{
    if (added_player && rigidBodies["__player"]) {
        rigidBodies["__player"]->setGravity(btVector3(0,f,0));
    }
}

void RoomPhysics::SetJumpVelocity(const float f)
{
    jump_vel = f;
}

void RoomPhysics::UpdateSimulation(const double dt)
{
    //qDebug() << "RoomPhysics::UpdateSimulation" << this << dt;
    //update the player
    if (added_player && rigidBodies["__player"]) {
        if (player_near_portal && rigidBodies["__player"]->getBroadphaseHandle()->m_collisionFilterMask & COL_WALL) {
            //add a ground plane so player can't fall            
            AddGroundPlane(player_near_portal_height);
            dynamicsWorld->removeRigidBody(rigidBodies["__player"]);
            dynamicsWorld->addRigidBody(rigidBodies["__player"], COL_PLAYER, COL_BASEPLANE);
        }
        else if (!player_near_portal && !(rigidBodies["__player"]->getBroadphaseHandle()->m_collisionFilterMask & COL_WALL)) {
            //remove ground plane so player can fall
            RemoveGroundPlane();
            dynamicsWorld->removeRigidBody(rigidBodies["__player"]);
            dynamicsWorld->addRigidBody(rigidBodies["__player"], COL_PLAYER, COL_WALL);
        }
    }

    //update physics simulation if timestep is nonzero
    if (dt > 0.0) {
        dynamicsWorld->stepSimulation(dt, 10, dt * 0.1f);
        //dynamicsWorld->stepSimulation(dt, 1); //56.0 - reports of slow physics with v-sync disabled when using this instead

        //update rigid body contacts/collisions
        rigidBodyCollisions.clear();

        const int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
        for (int i = 0; i < numManifolds; i++)
        {
            btPersistentManifold* contactManifold =  dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
            btRigidBody * obA = (btRigidBody *)(contactManifold->getBody0());
            btRigidBody * obB = (btRigidBody *)(contactManifold->getBody1());

            const QString obsA = rigidBodyJSIDs[obA];
            const QString obsB = rigidBodyJSIDs[obB];

            const int numContacts = contactManifold->getNumContacts();
            for (int j = 0; j < numContacts; j++)
            {
                btManifoldPoint& pt = contactManifold->getContactPoint(j);
                if (pt.getDistance() < 0.0f)
                {
                    rigidBodyCollisions[obsA].insert(obsB);
                    rigidBodyCollisions[obsB].insert(obsA);

                    break;
                }
            }
        }
    }
}

void RoomPhysics::AddRoomTemplate(const QPointer <RoomObject> o)
{
    //qDebug() << "RoomPhysics::AddRoomTemplate" << this;
    if (!added_room_template
            && o
            && o->GetAssetObject()            
            && o->GetAssetObject()->GetFinished()) {
        added_room_template = true;
        if (o->GetProperties()->GetJSID() == "__room_plane") {
            AddRoomPlaneTemplate(0.0f);
        }
        else {
            AddMesh(o, COL_WALL, COL_PLAYER | COL_WALL);
        }
    }
}

void RoomPhysics::AddGroundPlane(const float y)
{
    //qDebug() << "RoomPhysics::AddGroundPlane" << this;
    collisionShapes_staticPlane["__plane"] = new btStaticPlaneShape(btVector3(0, 1, 0), y);

    //basis vectors, point (this will be changed to be xdir, ydir, zdir, pos)
    motionStates["__plane"] = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

    //mass (note mass of 0 means infinte, immovable), motionstate, shape, inertia
    btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructInfo(0, motionStates["__plane"], collisionShapes_staticPlane["__plane"]);
    rigidBodyConstructInfo.m_friction = 0.5f; // Roughly concrete
    rigidBodyConstructInfo.m_rollingFriction = 0.01f; // Roughly concrete
    rigidBodyConstructInfo.m_restitution = 0.85f; // Roughly concrete

    btRigidBody* rigidBody = new btRigidBody(rigidBodyConstructInfo);

    rigidBodies["__plane"] = rigidBody;
    rigidBodyJSIDs[rigidBody] = "__plane";
    dynamicsWorld->addRigidBody(rigidBody, COL_BASEPLANE, COL_PLAYER);
}

void RoomPhysics::AddRoomPlaneTemplate(const float y)
{
    //qDebug() << "RoomPhysics::AddRoomPlaneTemplate" << this;
    collisionShapes_staticPlane["__room_plane"] = new btStaticPlaneShape(btVector3(0, 1, 0), y);

    //basis vectors, point (this will be changed to be xdir, ydir, zdir, pos)
    motionStates["__room_plane"] = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

    //mass (note mass of 0 means infinte, immovable), motionstate, shape, inertia
    btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructInfo(0, motionStates["__room_plane"], collisionShapes_staticPlane["__room_plane"]);
    rigidBodyConstructInfo.m_friction = 0.5f; // Roughly concrete
    rigidBodyConstructInfo.m_rollingFriction = 0.01f; // Roughly concrete
    rigidBodyConstructInfo.m_restitution = 0.85f; // Roughly concrete

    btRigidBody* rigidBody = new btRigidBody(rigidBodyConstructInfo);

    rigidBodies["__room_plane"] = rigidBody;
    rigidBodyJSIDs[rigidBody] = "__room_plane";
    dynamicsWorld->addRigidBody(rigidBody, COL_WALL, COL_PLAYER | COL_WALL);
}

void RoomPhysics::RemoveGroundPlane()
{
    if (rigidBodies["__plane"]) {
        rigidBodyJSIDs.remove(rigidBodies["__plane"]);
        dynamicsWorld->removeRigidBody(rigidBodies["__plane"]);
        delete rigidBodies["__plane"];
        delete collisionShapes_staticPlane["__plane"];
        delete motionStates["__plane"];
        rigidBodies.remove("__plane");
        collisionShapes_staticPlane.remove("__plane");
        motionStates.remove("__plane");
    }
}

void RoomPhysics::AddRigidBody(const QPointer <RoomObject> o, short group, short mask)
{
    //qDebug() << "RoomPhysics::AddRigidBody" << this;
    const QString s = o->GetProperties()->GetCollisionID();
    if (s == "sphere") {
        AddSphere(o, group, mask);
    }
    else if (s == "capsule") {
        AddCapsule(o, group, mask);
    }
    else if (s == "cylinder") {
        AddCylinder(o, group, mask);
    }
    else if (s == "cube") {
        AddCube(o, group, mask);
    }
    else {
        AddMesh(o, group, mask);
    }
}

QVector3D RoomPhysics::GetRigidBodyPos(const QString js_id)
{
    if (!rigidBodies.contains(js_id) || rigidBodies[js_id] == NULL) {
        return QVector3D(0,0,0);
    }

    btRigidBody * rb = rigidBodies[js_id];
    btTransform worldTrans;
    rb->getMotionState()->getWorldTransform(worldTrans);
    btVector3 p = worldTrans.getOrigin();

    return QVector3D(p.x(), p.y(), p.z());
}

QVector3D RoomPhysics::GetRigidBodyXDir(const QString js_id)
{
    if (!rigidBodies.contains(js_id) || rigidBodies[js_id] == NULL) {
        return QVector3D(0,0,0);
    }

    btRigidBody * rb = rigidBodies[js_id];
    btTransform worldTrans;
    rb->getMotionState()->getWorldTransform(worldTrans);
    btVector3 p = worldTrans.getBasis().getColumn(0);

    return QVector3D(p.x(), p.y(), p.z());
}

QVector3D RoomPhysics::GetRigidBodyScale(const QString js_id)
{
    if (!rigidBodyScales.contains(js_id)) {
        return QVector3D(0,0,0);
    }

    return rigidBodyScales[js_id];
}

float RoomPhysics::GetRigidBodyMass(const QString js_id)
{
    if (!rigidBodyMasses.contains(js_id)) {
        return 0.0f;
    }

    return rigidBodyMasses[js_id];
}

btRigidBody * RoomPhysics::GetRigidBody(const QPointer <RoomObject> o)
{
    if (o && rigidBodies.contains(o->GetProperties()->GetJSID())) {
        return rigidBodies[o->GetProperties()->GetJSID()];
    }
    else {
        return NULL;
    }
}

QSet <QString> RoomPhysics::GetRigidBodyCollisions(const QPointer <RoomObject> o)
{
    return rigidBodyCollisions[o->GetProperties()->GetJSID()];
}

void RoomPhysics::UpdateToRigidBody(QPointer <Player> player)
{
    //qDebug() << "RoomPhysics::UpdateToRigidBody" << this;
    if (!rigidBodies.contains("__player") || rigidBodies["__player"] == NULL) {
        AddPlayerShape(player);
    }
    else {       
        const QVector3D p = player->GetProperties()->GetPos()->toQVector3D();
        const QVector3D v = player->GetProperties()->GetVel()->toQVector3D();
        const btMatrix3x3 btBasis(1,0,0,
                                  0,1,0,
                                  0,0,1);
        const btVector3 btP(p.x(), p.y(), p.z());

        //basis vectors, point (this will be changed to be xdir, ydir, zdir, pos)
        btRigidBody * rb = rigidBodies["__player"];

        if (rb->getMotionState()) {
            delete rb->getMotionState();
        }
        btDefaultMotionState * motionState = new btDefaultMotionState(btTransform(btBasis, btP));
        rb->setLinearVelocity(btVector3(v.x(), v.y(), v.z()));
        rb->setMotionState(motionState);
    }
}

void RoomPhysics::UpdateFromRigidBody(QPointer <Player> player)
{
    //qDebug() << "RoomPhysics::UpdateFromRigidBody" << this;
    if (!rigidBodies.contains("__player") || rigidBodies["__player"] == NULL) {
        return;
    }

    btRigidBody * rb = rigidBodies["__player"];
    btTransform worldTrans;
    rb->getMotionState()->getWorldTransform(worldTrans);

    const btVector3 p = worldTrans.getOrigin();
    const btVector3 lv = rb->getLinearVelocity();
    const bool flying = player->GetFlying();
    const QVector3D iv = player->GetImpulseVel();

    //stop physics from glitching out plyaer, leading to nan position
    if (!std::isfinite(p.x()) || !std::isfinite(p.y()) || !std::isfinite(p.z())) {        
        UpdateToRigidBody(player);
        return;
    }   

    if (!flying && player_on_ground && player->GetJump()) {
        rb->setLinearVelocity(btVector3(iv.x(), jump_vel, iv.z()));
    }
    else if (!flying && !player->GetJump() && lv.y() > 0.0f) {
        rb->setLinearVelocity(btVector3(iv.x(), lv.y() * 0.25f, iv.z()));
    }
    else if (flying) {
        rb->setLinearVelocity(btVector3(iv.x(), iv.y(), iv.z()));
    }
    else {
        rb->setLinearVelocity(btVector3(iv.x(), lv.y(), iv.z()));
    }
    const btVector3 lv2 = rb->getLinearVelocity();

    player->GetProperties()->SetPos(QVector3D(p.x(), p.y(), p.z()));
    player->GetProperties()->SetVel(QVector3D(lv2.x(), lv2.y(), lv2.z()));
    //    qDebug() << "RoomPhysics::UpdateFromRigidBody" << player_on_ground << iv << QVector3D(lv.x(), lv.y(), lv.z()) << QVector3D(lv2.x(), lv2.y(), lv2.z());
}

void RoomPhysics::UpdateFromRigidBody(const QPointer <RoomObject> o)
{
    //qDebug() << "RoomPhysics::UpdateFromRigidBody2";
    if (o.isNull() || !rigidBodies.contains(o->GetProperties()->GetJSID()) || rigidBodies[o->GetProperties()->GetJSID()] == NULL) {
        return;
    }

    btRigidBody * rb = rigidBodies[o->GetProperties()->GetJSID()];

    btTransform worldTrans;
    rb->getMotionState()->getWorldTransform(worldTrans);

    btVector3 p = worldTrans.getOrigin();
    btVector3 x = worldTrans.getBasis().getColumn(0);
    btVector3 y = worldTrans.getBasis().getColumn(1);
    btVector3 z = worldTrans.getBasis().getColumn(2);
    btVector3 lv = rb->getLinearVelocity();

    o->GetProperties()->SetXDir(QVector3D(x.x(), x.y(), x.z()));
    o->GetProperties()->SetYDir(QVector3D(y.x(), y.y(), y.z()));
    o->GetProperties()->SetZDir(QVector3D(z.x(), z.y(), z.z()));
    o->GetProperties()->SetPos(QVector3D(p.x(), p.y(),p.z()));
    o->GetProperties()->SetVel(QVector3D(lv.x(), lv.y(), lv.z()));
}

void RoomPhysics::UpdateToRigidBody(const QPointer <RoomObject> o)
{
    //qDebug() << "RoomPhysics::UpdateToRigidBody2";
    const QString js_id = o->GetProperties()->GetJSID();
    if (o.isNull() || !rigidBodies.contains(js_id) || rigidBodies[js_id] == NULL) {
        return;
    }

    //need to fix this so it preserves the last filter group/mask
    RemoveRigidBody(o);
    if (o->GetGrabbed()) { //59.3 - note!  We use selection to determine if we should re-add the object to the physics engine with or without player collision
        AddRigidBody(o, COL_WALL, COL_WALL);
    }
    else {
        AddRigidBody(o, COL_WALL, COL_PLAYER | COL_WALL);
    }
}

void RoomPhysics::RemoveRigidBody(const QPointer <RoomObject> o)
{
    //qDebug() << "RoomPhysics::RemoveRigidBody" << o->GetJSID() << o->GetID();
    const QString jsid = o->GetProperties()->GetJSID();
    if (rigidBodies.contains(jsid) && rigidBodies[jsid] != NULL) {
        //qDebug() << "Removing" << o->GetJSID() << o->GetID() << o->GetCollisionID();
        rigidBodyJSIDs.remove(rigidBodies[jsid]);
        dynamicsWorld->removeRigidBody(rigidBodies[jsid]);
        delete rigidBodies[jsid];
        delete motionStates[jsid];
    }
    rigidBodies.remove(jsid);
    motionStates.remove(jsid);

    auto sphere_it = collisionShapes_sphere.find(jsid);
    if (sphere_it != collisionShapes_sphere.end())
    {
        delete *sphere_it;
        collisionShapes_sphere.erase(sphere_it);
    }

    auto multiSphere_it = collisionShapes_multiSphere.find(jsid);
    if (multiSphere_it != collisionShapes_multiSphere.end())
    {
        delete *multiSphere_it;
        collisionShapes_multiSphere.erase(multiSphere_it);
    }

    auto box_it = collisionShapes_box.find(jsid);
    if (box_it != collisionShapes_box.end())
    {
        delete *box_it;
        collisionShapes_box.erase(box_it);
    }

    auto cylinder_it = collisionShapes_cylinder.find(jsid);
    if (cylinder_it != collisionShapes_cylinder.end())
    {
        delete *cylinder_it;
        collisionShapes_cylinder.erase(cylinder_it);
    }

    auto capsule_it = collisionShapes_capsule.find(jsid);
    if (capsule_it != collisionShapes_capsule.end())
    {
        delete *capsule_it;
        collisionShapes_capsule.erase(capsule_it);
    }

    auto staticPlane = collisionShapes_staticPlane.find(jsid);
    if (staticPlane != collisionShapes_staticPlane.end())
    {
        delete *staticPlane;
        collisionShapes_staticPlane.erase(staticPlane);
    }

    auto BvhTriangleMesh_it = collisionShapes_BvhTriangleMesh.find(jsid);
    if (BvhTriangleMesh_it != collisionShapes_BvhTriangleMesh.end())
    {
        delete *BvhTriangleMesh_it;
        collisionShapes_BvhTriangleMesh.erase(BvhTriangleMesh_it);
    }

    auto riangleMesh_it = collisionShapes_triangleMesh.find(jsid);
    if (riangleMesh_it != collisionShapes_triangleMesh.end())
    {
        delete *riangleMesh_it;
        collisionShapes_triangleMesh.erase(riangleMesh_it);
    }

    rigidBodyScales.remove(jsid);
    rigidBodyMasses.remove(jsid);
}

void RoomPhysics::AddSphere(const QPointer <RoomObject> o, short group, short mask)
{
    //qDebug() << "RoomPhysics::AddSphere" << o->GetJSID() << o->GetID() << group << mask;
    const QVector3D scale = o->GetProperties()->GetScale()->toQVector3D();
    const QVector3D cscale = o->GetProperties()->GetCollisionScale()->toQVector3D();
    float btMass = 0.0f;
    if (!o->GetProperties()->GetCollisionStatic() || o->GetProperties()->GetCollisionTrigger()) {
        btMass = 4.0f / 3.0f * MathUtil::_PI * scale.x() * cscale.x() *
                scale.y() * cscale.y() *
                scale.z() * cscale.z(); //volume of sphere (assumes uniform density for mass)
    }

    btCollisionShape * shape;

    if (scale.x() == scale.y() && scale.x() == scale.z()) { //perfect sphere
        const float btRadius = scale.x() * 0.5f;
        shape = new btSphereShape(btRadius);
        collisionShapes_sphere[o->GetProperties()->GetJSID()] = (btSphereShape*)shape;

    }
    else { //ellipsoidal
        btVector3 positions[1];
        positions[0] = btVector3(0,0,0);
        btScalar radii[3];
        radii[0] = scale.x() * cscale.x();
        radii[1] = scale.y() * cscale.y();
        radii[2] = scale.z() * cscale.z();
        shape = new btMultiSphereShape(positions, radii, 1);
        collisionShapes_multiSphere[o->GetProperties()->GetJSID()] = (btMultiSphereShape*)shape;
    }

    btVector3 btInertia;
    shape->calculateLocalInertia(btMass, btInertia);

    AddShape(o, shape, btMass, btInertia, group, mask);
}

void RoomPhysics::AddMesh(const QPointer <RoomObject> o, short group, short mask)
{
    if (o.isNull()) {
        return;
    }

    QPointer <AssetObject> a = o->GetCollisionAssetObject();
    if (a.isNull() || !a->GetFinished()) {
        return;
    }

    const QVector3D s = o->GetProperties()->GetScale()->toQVector3D();
    const QVector3D AABB_size = a->GetBBoxMax() - a->GetBBoxMin();
    const float AABB_volume = AABB_size.x() * AABB_size.y() * AABB_size.z();
    const float AABB_estimated_mass = AABB_volume * 1.0; // Mass of water.
    const float btMass = o->GetProperties()->GetCollisionStatic() ? 0.0f : AABB_estimated_mass; // Infinite mass if static
    const QString jsid = o->GetProperties()->GetJSID();

    collisionShapes_triangleMesh[jsid] = new btTriangleMesh();   

    const QList <QString> materials = a->GetGeom()->GetData().GetMaterialNames();    
    for (int i = 0; i < materials.size(); ++i)
    {
        GeomMaterial& mat = a->GetGeom()->GetData().GetMaterial(materials[i]);

        // For each mesh
        const int mesh_count = mat.vbo_data.size();
        for (int mesh_index = 0; mesh_index < mesh_count; ++mesh_index)
        {
            GeomVBOData & vbo_data = a->GetGeom()->GetData().GetVBOData(materials[i], mesh_index);
            // For each instance
            const int instance_count = vbo_data.m_instance_transforms.size();
            for (int instance_index = 0; instance_index < instance_count; ++instance_index)
            {
                const QVector <GeomTriangle>& tris = a->GetGeom()->GetData().GetTriangles(materials[i], mesh_index);
                QVector3D transformed_position0;
                QVector3D transformed_position1;
                QVector3D transformed_position2;
                for (int j = 0; j < tris.size(); ++j)
                {
                    QVector3D p0 = QVector3D(tris[j].p[0][0],
                                             tris[j].p[0][1],
                                             tris[j].p[0][2]);

                    QVector3D p1 = QVector3D(tris[j].p[1][0],
                                             tris[j].p[1][1],
                                             tris[j].p[1][2]);

                    QVector3D p2 = QVector3D(tris[j].p[2][0],
                                             tris[j].p[2][1],
                                             tris[j].p[2][2]);

                    transformed_position0 = vbo_data.m_instance_transforms[instance_index].map(p0);
                    transformed_position1 = vbo_data.m_instance_transforms[instance_index].map(p1);
                    transformed_position2 = vbo_data.m_instance_transforms[instance_index].map(p2);

                    //56.0 - comment this out - placing limits may make things too conservative in general
                    btVector3 p[3];
                    p[0] = btVector3(transformed_position0.x() * s.x(), transformed_position0.y() * s.y(), transformed_position0.z() * s.z());
                    p[1] = btVector3(transformed_position1.x() * s.x(), transformed_position1.y() * s.y(), transformed_position1.z() * s.z());
                    p[2] = btVector3(transformed_position2.x() * s.x(), transformed_position2.y() * s.y(), transformed_position2.z() * s.z());

                    collisionShapes_triangleMesh[jsid]->addTriangle(p[0], p[1], p[2]);
                }
            }
        }
    }
    //qDebug() << "tris" << collisionShapes_triangleMesh[jsid]->getNumTriangles();

    if (collisionShapes_triangleMesh[jsid]->getNumTriangles() > 0) {
        //qDebug() << "adding" << o->GetID() << o->GetCollisionID();
        btCollisionShape * shape = new btBvhTriangleMeshShape(collisionShapes_triangleMesh[jsid], true); //computes bounding volume hierarchy, is quantized - meant for *static meshes only*
        collisionShapes_BvhTriangleMesh[jsid] = (btBvhTriangleMeshShape*)shape;
        AddShape(o, shape, btMass, btVector3(0,0,0), group, mask);
    }
    else {
        //qDebug() << "CUBE" << o->GetID() << o->GetCollisionID();
        AddCube(o, group, mask);
    }

}

void RoomPhysics::AddCube(const QPointer <RoomObject> o, short group, short mask)
{
    const QVector3D s = o->GetProperties()->GetScale()->toQVector3D();
    const QVector3D cs = o->GetProperties()->GetCollisionScale()->toQVector3D();
    float btMass = 0.0f;
    if (!o->GetProperties()->GetCollisionStatic() || o->GetProperties()->GetCollisionTrigger()) {
        btMass = s.x() * cs.x() *
                s.y() * cs.y() *
                s.z() * cs.z();
    }

    QString jsid = o->GetProperties()->GetJSID();
    collisionShapes_box[jsid] = new btBoxShape(btVector3(s.x() * cs.x() * 0.5f,
                                                        s.y() * cs.y() * 0.5f,
                                                        s.z() * cs.z() * 0.5f));

    btVector3 btInertia;
    collisionShapes_box[jsid]->calculateLocalInertia(btMass, btInertia);

    AddShape(o, collisionShapes_box[jsid], btMass, btInertia, group, mask);
}

void RoomPhysics::AddCylinder(const QPointer <RoomObject> o, short group, short mask)
{
    const QVector3D s = o->GetProperties()->GetScale()->toQVector3D();
    const QVector3D cs = o->GetProperties()->GetCollisionScale()->toQVector3D();
    float btMass = 0.0f;
    if (!o->GetProperties()->GetCollisionStatic() || o->GetProperties()->GetCollisionTrigger()) {
        btMass = o->GetProperties()->GetCollisionStatic() ? 0.0f : s.x() * cs.x() *
                s.y() * cs.y() *
                s.z() * cs.z();
    }

    QString jsid = o->GetProperties()->GetJSID();
    collisionShapes_cylinder[jsid] = new btCylinderShape(btVector3(s.x() * cs.x() * 0.5f,
                                                             s.y() * cs.y() * 0.5f,
                                                             s.z() * cs.z() * 0.5f));

    btVector3 btInertia;
    collisionShapes_cylinder[jsid]->calculateLocalInertia(btMass, btInertia);

    AddShape(o, collisionShapes_cylinder[jsid], btMass, btInertia, group, mask);
}

void RoomPhysics::AddCapsule(const QPointer <RoomObject> o, short group, short mask)
{
    const QVector3D s = o->GetProperties()->GetScale()->toQVector3D();
    const QVector3D cs = o->GetProperties()->GetCollisionScale()->toQVector3D();
    float btMass = 0.0f;
    if (!o->GetProperties()->GetCollisionStatic() || o->GetProperties()->GetCollisionTrigger()) {
        btMass = o->GetProperties()->GetCollisionStatic() ? 0.0f : s.x() * cs.x() *
                s.y() * cs.y() *
                s.z() * cs.z();
    }

    QString jsid = o->GetProperties()->GetJSID();
    collisionShapes_capsule[jsid] = new btCapsuleShape(s.x() * cs.x() * 0.5f,
                                                 s.y() * cs.y()); //radius, height
    btVector3 btInertia;
    collisionShapes_capsule[jsid]->calculateLocalInertia(btMass, btInertia);

    AddShape(o, collisionShapes_capsule[jsid], btMass, btInertia, group, mask);
}

void RoomPhysics::DoPlayerGroundTest(const QPointer <Player> player)
{
    //qDebug() << "RoomPhysics::DoPlayerGroundTest" << this;
    const QVector3D p0 = player->GetProperties()->GetPos()->toQVector3D() + QVector3D(0,0.1f,0);
    const QVector3D p1 = player->GetProperties()->GetPos()->toQVector3D() - QVector3D(0,0.2f,0);

    btCollisionWorld::ClosestRayResultCallback callback(btVector3(p0.x(), p0.y(), p0.z()), btVector3(p1.x(), p1.y(), p1.z()));

    dynamicsWorld->rayTest(btVector3(p0.x(), p0.y(), p0.z()), btVector3(p1.x(), p1.y(), p1.z()), callback);

    player_on_ground = callback.hasHit();
    //player_on_ground = (callback.m_hitPointWorld.size() > 0);
}

bool RoomPhysics::GetPlayerOnGround() const
{
    //qDebug() << "RoomPhysics::GetPlayerOnGround" << this;
    return player_on_ground;
}

void RoomPhysics::SetPlayerNearPortal(const bool b, const float portal_y)
{
    //qDebug() << "RoomPhysics::SetPlayerNearPortal" << this;
    player_near_portal = b;
    player_near_portal_height = portal_y;
}

void RoomPhysics::RemovePlayerShape()
{
    //qDebug() << "RoomPhysics::RemovePlayerShape()" << this;
    added_player = false;
    rigidBodyJSIDs.remove(rigidBodies["__player"]);
    dynamicsWorld->removeRigidBody(rigidBodies["__player"]);
    rigidBodies.remove("__player");
}

void RoomPhysics::AddPlayerShape(const QPointer <Player> player)
{    
    if (!added_player) {
        //qDebug() << "RoomPhysics::AddPlayerShape()" << this;
        added_player = true;

        const QVector3D p = player->GetProperties()->GetPos()->toQVector3D();

        const btMatrix3x3 btBasis(1,0,0,
                                  0,1,0,
                                  0,0,1);
        const btVector3 btP(p.x(), p.y(), p.z());
        const float btMass = 1.0f;

        const float r = player->GetPlayerCollisionRadius();

        btVector3 positions[2];
        positions[0].setX(0.0f);
        positions[0].setY(r);
        positions[0].setZ(0.0f);
        positions[1].setX(0.0f);
        positions[1].setY(1.6f);
        positions[1].setZ(0.0f);

        btScalar radii[6];
        radii[0] = r;
        radii[1] = r;
        radii[2] = r;
        radii[3] = r;
        radii[4] = r;
        radii[5] = r;
        btCollisionShape * shape = new btMultiSphereShape(positions, radii, 2);

        btVector3 btInertia;
        shape->calculateLocalInertia(btMass, btInertia);

        //basis vectors, point (this will be changed to be xdir, ydir, zdir, pos)
        btDefaultMotionState * motionState = new btDefaultMotionState(btTransform(btBasis, btP));
        btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructInfo(btMass, motionState, shape, btInertia);
        rigidBodyConstructInfo.m_friction = 0.0f;
        rigidBodyConstructInfo.m_rollingFriction = 0.00f;
        rigidBodyConstructInfo.m_restitution = 0.55f;

        const QVector3D v = player->GetProperties()->GetVel()->toQVector3D();
        rigidBodies["__player"] = new btRigidBody(rigidBodyConstructInfo);
        rigidBodies["__player"]->setLinearVelocity(btVector3(v.x(), v.y(), v.z()));
        rigidBodies["__player"]->setSleepingThresholds(0.0f, 0.0f); //never sleep the player collision capsule, so it's always hitting stuff
        rigidBodies["__player"]->setAngularFactor(0.0f); //no need for rotations

        rigidBodyJSIDs[rigidBodies["__player"]] = "__player";

        dynamicsWorld->addRigidBody(rigidBodies["__player"], COL_PLAYER, COL_WALL);
    }
}

void RoomPhysics::SetLinearVelocity(const QPointer <RoomObject> o, QVector3D v)
{
    const QString jsid = o->GetProperties()->GetJSID();
    //qDebug() << "RoomPhysics::SetLinearVelocity" << jsid << rigidBodies.contains(jsid) << rigidBodies[jsid];
    if (rigidBodies.contains(jsid) && rigidBodies[jsid]) {
        //qDebug() << "SETTING VEL to" << jsid << v;
        rigidBodies[jsid]->setLinearVelocity(btVector3(v.x(), v.y(), v.z()));
    }
}

void RoomPhysics::AddShape(const QPointer <RoomObject> o, btCollisionShape * shape, const btScalar btMass, const btVector3 btInertia, short group, short mask)
{
    const QString j = o->GetProperties()->GetJSID();
    const QVector3D p = o->GetPos() + o->GetProperties()->GetCollisionPos()->toQVector3D();
    const QVector3D x = o->GetXDir();
    const QVector3D y = o->GetYDir();
    const QVector3D z = o->GetZDir();
    const QVector3D v = o->GetVel();

    const btMatrix3x3 btBasis(x.x(), y.x(), z.x(),
                              x.y(), y.y(), z.y(),
                              x.z(), y.z(), z.z());
    const btVector3 btP(p.x(), p.y(), p.z());

    //basis vectors, point (this will be changed to be xdir, ydir, zdir, pos)
    motionStates[j] = new btDefaultMotionState(btTransform(btBasis, btP));

    //mass (note mass of 0 means infinte, immovable), motionstate, shape, inertia    
    btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructInfo(btMass, motionStates[j], shape, btInertia);
    //rigidBodyConstructInfo.m_additionalDamping = true; //56.0 - prevent jitter
    rigidBodyConstructInfo.m_friction = o->GetProperties()->GetCollisionFriction();
    rigidBodyConstructInfo.m_rollingFriction = o->GetProperties()->GetCollisionRollingFriction();
    rigidBodyConstructInfo.m_restitution = o->GetProperties()->GetCollisionRestitution();
    rigidBodyConstructInfo.m_angularDamping = o->GetProperties()->GetCollisionAngularDamping();
    rigidBodyConstructInfo.m_linearDamping = o->GetProperties()->GetCollisionLinearDamping();

    rigidBodies[j] = new btRigidBody(rigidBodyConstructInfo);
    rigidBodies[j]->setLinearVelocity(btVector3(v.x(), v.y(), v.z()));

    if (o->GetProperties()->GetCollisionTrigger()) {
        rigidBodies[j]->setCollisionFlags(rigidBodies[j]->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }

    rigidBodies[j]->setContactProcessingThreshold(0.0f);
    rigidBodies[j]->setCcdMotionThreshold(o->GetProperties()->GetCollisionCcdMotionThreshold());
    rigidBodies[j]->setCcdSweptSphereRadius(o->GetProperties()->GetCollisionCcdSweptSphereRadius());

    rigidBodyJSIDs[rigidBodies[j]] = j;
    rigidBodyScales[j] = o->GetProperties()->GetScale()->toQVector3D();
    rigidBodyMasses[j] = btMass;

    //qDebug() << "Adding" << o->GetJSID() << o->GetID() << o->GetCollisionID() << group << mask;
    dynamicsWorld->addRigidBody(rigidBodies[j], group, mask);
    //dynamicsWorld->addRigidBody(rigidBodies[o->GetJSID()], COL_WALL, COL_PLAYER | COL_WALL);
}

QVector3D RoomPhysics::GetRigidBodyVel(const QString js_id)
{
    if (!rigidBodies.contains(js_id) || rigidBodies[js_id] == NULL) {
        return QVector3D(0,0,0);
    }

    btVector3 p = rigidBodies[js_id]->getLinearVelocity();
    return QVector3D(p.x(), p.y(), p.z());
}
