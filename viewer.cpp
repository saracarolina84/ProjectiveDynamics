#include "viewer.h"
#include <math.h>
#include <set>
#include <stdlib.h> // RAND_MAX


#include "utils.h"
#include <TriCirculator.h>
#include <Tetra.h>
#include <Vertex.h>

using namespace qglviewer;
using namespace std;

///////////////////////   V i e w e r  ///////////////////////
void Viewer::init()
{
    restoreStateFromFile();
    glDisable(GL_LIGHTING);

    m_bodies.clear();

//    testSceneClothOnBunny();
    testSceneClothConstrainedTopCorners();
//    testSceneClothConstrainedCorners();
//    testSceneClothDropping();
//    testSceneSingleTetra();
//    testSceneDeformableSphere();

    m_simulator = new Simulator(m_dt, m_iterations, m_bodies);

    m_play = false;
    m_step = false;

//    help();
    startAnimation();
}

//void Viewer::testSceneClothOnBunny()
//{
//    int rows = m_meshRows;
//    int cols = m_meshColumns;
//    float meshCellSize = m_meshSize/m_meshRows;
//    TriMesh* mesh = TriMesh::CreatePatchMesh(rows, cols, meshCellSize);
//    mesh->Transform(Vector3(0,0.5,0), LA::Quaternion::FromAngleAxis(M_PI_2, Vector3(1,0,0)));
////    m_mesh->Transform(Vector3(0,0,0), LA::Quaternion::FromAngleAxis(M_PI_4, Vector3(0,0,1)));
//    glPointSize(10.0);
//    glBlendFunc(GL_ONE, GL_ONE);

//    m_hardConstraintsIndices.clear();
//    m_hardConstraintsPositions.clear();
//    m_selection.clear();

//    m_q.resize(mesh->NumVertices()*3);
//    m_v.resize(mesh->NumVertices()*3);
//    m_v.setZero();
//    for(int i=0; i<mesh->NumVertices(); ++i)
//    {
//        m_q[i*3+0] = mesh->Vertices()[i]->Position()[0];
//        m_q[i*3+1] = mesh->Vertices()[i]->Position()[1];
//        m_q[i*3+2] = mesh->Vertices()[i]->Position()[2];
//    }

//    m_springConstraints.clear();
//    for(int i=0;i<mesh->Edges().size();++i)
//    {
//        TriEdge *edge = mesh->Edges()[i];
//        int v1 = edge->Origin()->Id();
//        int v2 = edge->Head()->Id();
//        m_springConstraints.push_back(make_pair(v1,v2));

//        if(edge->IsBoundary())
//            continue;

//        TriFace* f1 = edge->AdjFace();
//        TriFace* f2 = edge->Twin()->AdjFace();

//        for(int j=0;j<3;++j)
//        {
//            if(f1->Vertex(j) != edge->Origin() &&
//               f1->Vertex(j) != edge->Head())
//                v1 = f1->Vertex(j)->Id();
//            if(f2->Vertex(j) != edge->Twin()->Origin() &&
//               f2->Vertex(j) != edge->Twin()->Head())
//                v2 = f2->Vertex(j)->Id();
//        }
//        Vector3 v1p = mesh->Vertices()[v1]->PositionInit();
//        Vector3 v2p = mesh->Vertices()[v2]->PositionInit();
//        if(Vector3::squaredDistance(v1p, v2p) <= 1.01*2*meshCellSize*meshCellSize)
//            m_springConstraints.push_back(make_pair(v1,v2));
//    }



//    // static bodies
//    TriMesh *rb = TriMesh::ReadFromFile("/Users/sarac.schvartzman/Dropbox/Code/bunny.obj", TriMesh::OBJ, 2.0f);
//    rb->Transform(Vector3(0,0.0,0), LA::Quaternion::FromAngleAxis(M_PI_2, Vector3(-1,0,0)));
//    m_staticBodies.push_back(rb);
//}

void Viewer::testSceneClothConstrainedTopCorners()
{
    int rows = m_meshRows;
    int cols = m_meshColumns;
    float meshCellSize = m_meshSize/m_meshRows;
    TriMesh* mesh = TriMesh::CreatePatchMesh(rows, cols, meshCellSize);

    glPointSize(10.0);
    glBlendFunc(GL_ONE, GL_ONE);

    vector<TriVertex *> topVertices = *(mesh->getTriVertexOnTop(0.001));

    mesh->Transform(Vector3(0,0.5,0), LA::Quaternion::FromAngleAxis(M_PI_2, Vector3(1,0,0)));
    SpringBody* body = new SpringBody(mesh, m_totalMass, m_springStiffness, true, 1.01*2*meshCellSize*meshCellSize);

    body->addPositionConstraint(m_positionStiffness, topVertices.front()->Id());
    body->addPositionConstraint(m_positionStiffness, topVertices.back()->Id());

    m_bodies.push_back(body);
}

void Viewer::testSceneClothConstrainedCorners()
{
    int rows = m_meshRows;
    int cols = m_meshColumns;
    float meshCellSize = m_meshSize/m_meshRows;
    TriMesh* mesh = TriMesh::CreatePatchMesh(rows, cols, meshCellSize);

    glPointSize(10.0);
    glBlendFunc(GL_ONE, GL_ONE);

    vector<TriVertex *> topVertices = *(mesh->getTriVertexOnTop(0.001));
    vector<TriVertex *> bottomVertices = *(mesh->getTriVertexOnBottom(0.001));

    mesh->Transform(Vector3(0,0.5,0), LA::Quaternion::FromAngleAxis(M_PI_2, Vector3(1,0,0)));
    SpringBody* body = new SpringBody(mesh, m_totalMass, m_springStiffness, true, 1.01*2*meshCellSize*meshCellSize);

    vector<TriVertex *> cornerVertices;
    cornerVertices.push_back(topVertices.front());
    cornerVertices.push_back(topVertices.back());
    cornerVertices.push_back(bottomVertices.front());
    cornerVertices.push_back(bottomVertices.back());
    for(int i=0;i<cornerVertices.size();++i)
    {
        body->addPositionConstraint(m_positionStiffness, cornerVertices[i]->Id());
    }

    m_bodies.push_back(body);
}

//void Viewer::testSceneClothDropping()
//{
//    int rows = m_meshRows;
//    int cols = m_meshColumns;
//    float meshCellSize = m_meshSize/m_meshRows;
//    TriMesh* mesh = TriMesh::CreatePatchMesh(rows, cols, meshCellSize);
//    mesh->Transform(Vector3(0,0,0), LA::Quaternion::FromAngleAxis(0.01, Vector3(1,0,0)));

//    glPointSize(10.0);
//    glBlendFunc(GL_ONE, GL_ONE);

//    m_hardConstraintsIndices.clear();
//    m_hardConstraintsPositions.clear();
//    m_selection.clear();

//    m_q.resize(mesh->NumVertices()*3);
//    m_v.resize(mesh->NumVertices()*3);
//    m_v.setZero();
//    for(int i=0; i<mesh->NumVertices(); ++i)
//    {
//        m_q[i*3+0] = mesh->Vertices()[i]->Position()[0];
//        m_q[i*3+1] = mesh->Vertices()[i]->Position()[1];
//        m_q[i*3+2] = mesh->Vertices()[i]->Position()[2];
//    }

//    m_springConstraints.clear();
//    for(int i=0;i<mesh->Edges().size();++i)
//    {
//        TriEdge *edge = mesh->Edges()[i];
//        int v1 = edge->Origin()->Id();
//        int v2 = edge->Head()->Id();
//        m_springConstraints.push_back(make_pair(v1,v2));

//        if(edge->IsBoundary())
//            continue;

//        TriFace* f1 = edge->AdjFace();
//        TriFace* f2 = edge->Twin()->AdjFace();

//        for(int j=0;j<3;++j)
//        {
//            if(f1->Vertex(j) != edge->Origin() &&
//               f1->Vertex(j) != edge->Head())
//                v1 = f1->Vertex(j)->Id();
//            if(f2->Vertex(j) != edge->Twin()->Origin() &&
//               f2->Vertex(j) != edge->Twin()->Head())
//                v2 = f2->Vertex(j)->Id();
//        }
//        Vector3 v1p = mesh->Vertices()[v1]->PositionInit();
//        Vector3 v2p = mesh->Vertices()[v2]->PositionInit();
//        if(Vector3::squaredDistance(v1p, v2p) <= 1.01*2*meshCellSize*meshCellSize)
//            m_springConstraints.push_back(make_pair(v1,v2));
//    }

//    // static bodies
//    TriMesh *rb = TriMesh::CreateBlockMesh(Vector3(-10,-10,-10), Vector3(10, -1, 10));
//    m_staticBodies.push_back(rb);
//}

//void Viewer::testSceneSingleTetra()
//{
//    glPointSize(10.0);
//    glBlendFunc(GL_ONE, GL_ONE);

//    m_hardConstraintsIndices.clear();
//    m_hardConstraintsPositions.clear();
//    m_selection.clear();
//    m_springConstraints.clear();
//    m_tetraConstraints.clear();

//    TriMesh* mesh = TriMesh::ReadFromFile("/Users/sarac.schvartzman/Dropbox/Code/tetra.obj", TriMesh::OBJ);

//    m_q.resize(mesh->NumVertices()*3);
//    m_v.resize(mesh->NumVertices()*3);
//    m_v.setZero();
//    for(int i=0; i<mesh->NumVertices(); ++i)
//    {
//        m_q[i*3+0] = mesh->Vertices()[i]->Position()[0];
//        m_q[i*3+1] = mesh->Vertices()[i]->Position()[1];
//        m_q[i*3+2] = mesh->Vertices()[i]->Position()[2];
//    }

//    vector<int> tetra;
//    tetra.push_back(0);
//    tetra.push_back(1);
//    tetra.push_back(2);
//    tetra.push_back(3);
//    m_tetraConstraints.push_back(tetra);

//    {
//        addIdToSelection(0);
//        m_hardConstraintsIndices.push_back(0);
//        m_hardConstraintsPositions.push_back(toEigenVector3(mesh->Vertices()[0]->Position()));
//    }

//}

//void Viewer::testSceneDeformableSphere()
//{
//    glPointSize(10.0);
//    glBlendFunc(GL_ONE, GL_ONE);

//    m_hardConstraintsIndices.clear();
//    m_hardConstraintsPositions.clear();
//    m_selection.clear();
//    m_springConstraints.clear();
//    m_tetraConstraints.clear();

//    TriMesh* mesh = TriMesh::ReadFromFile("/Users/sarac.schvartzman/Dropbox/Code/sphere.obj", TriMesh::OBJ);
//    TetraMesh* tetra = TetraMesh::CreateEmbeddingMesh(5, mesh, true);

//    Vertex* vertices = tetra->getVertex();
//    m_q.resize(tetra->numVertices()*3);
//    m_v.resize(tetra->numVertices()*3);
//    m_v.setZero();
//    for(int i=0; i<tetra->numVertices(); ++i)
//    {
//        Vertex* v = vertices+i;
//        m_q[i*3+0] = v->getPosition()[0];
//        m_q[i*3+1] = v->getPosition()[1];
//        m_q[i*3+2] = v->getPosition()[2];
//    }

//    vector<Tetra*> tetras = tetra->getAllTets();

//    for(int i=0;i<tetra->numTetras();++i)
//    {
//        vector<int> tetraIndices;
//        Tetra* t = tetras[i];
//        tetraIndices.push_back(t->_vertex[0]->_id);
//        tetraIndices.push_back(t->_vertex[1]->_id);
//        tetraIndices.push_back(t->_vertex[2]->_id);
//        tetraIndices.push_back(t->_vertex[3]->_id);

//        m_tetraConstraints.push_back(tetraIndices);
//    }

//    {
//        m_hardConstraintsIndices.push_back(0);
//        m_hardConstraintsPositions.push_back(toEigenVector3(mesh->Vertices()[0]->Position()));
//    }


//    // static bodies
//    TriMesh *rb = TriMesh::CreateBlockMesh(Vector3(-10,-10,-10), Vector3(10, -2, 10));
//    m_staticBodies.push_back(rb);
//}

void Viewer::draw()
{
    for(int i=0;i<m_bodies.size();++i)
        m_bodies[i]->draw();

    for(int i=0;i<m_staticBodies.size();++i)
        m_staticBodies[i]->Draw();

    glDisable(GL_LIGHTING);
    glPointSize(10);

    glColor3f(1.0f,0.0,0.0f);
    glBegin(GL_POINTS);

    glColor3f(0,0,1);
    for (unsigned i = 0; i < m_simulator->m_projected.size(); i++)
    {
        Eigen::Vector3f p = m_simulator->m_projected[i];
        glVertex3f(p[0], p[1], p[2]);
    }

    glColor3f(0,1,0);
    for (unsigned i = 0; i < m_simulator->m_sn[0].rows(); i++)
    {
        glVertex3f(m_simulator->m_sn[0][i], m_simulator->m_sn[1][i], m_simulator->m_sn[2][i]);
    }

    glEnd();



    glBegin(GL_LINES);
    glColor3f(1,0,1);
    for (unsigned i = 0; i < m_simulator->m_constraints.size(); i++)
    {
        ProjectiveConstraint* constraint = m_simulator->m_constraints[i];
        for(int j=0; j<constraint->m_numParticles; ++j)
        {
            for(int k=j+1; k<constraint->m_numParticles; ++k)
            {
                glVertex3f(m_simulator->m_q[0][constraint->getVIndex(j)],
                        m_simulator->m_q[1][constraint->getVIndex(j)],
                        m_simulator->m_q[2][constraint->getVIndex(j)]);
                glVertex3f(m_simulator->m_q[0][constraint->getVIndex(k)],
                        m_simulator->m_q[1][constraint->getVIndex(k)],
                        m_simulator->m_q[2][constraint->getVIndex(k)]);
            }
        }
    }
    glEnd();


    glEnable(GL_LIGHTING);

    // Draws rectangular selection area. Could be done in postDraw() instead.
    if (m_selectionMode != NONE)
        drawSelectionRectangle();
}

void Viewer::animate()
{
    if(m_play || m_step)
    {
        m_simulator->advanceTime();
    }

    m_step &= false;
}

QString Viewer::helpString() const {
    QString text("<h2>A n i m a t i o n</h2>");
    text += "Use the <i>animate()</i> function to implement the animation part "
            "of your ";
    text += "application. Once the animation is started, <i>animate()</i> and "
            "<i>draw()</i> ";
    text += "are called in an infinite loop, at a frequency that can be "
            "fixed.<br><br>";
    text += "Press <b>Return</b> to start/stop the animation.";
    return text;
}

void Viewer::keyPressEvent(QKeyEvent *key)
{
    if(key->text() == "o")
    {
        m_step = true;
    }
    else if(key->text() == "p")
    {
        m_play = !m_play;
    }
    else if(key->text() == "d")
    {
//        for(int i=0;i<m_hardConstraintsPositions.size();++i)
//        {
//            m_hardConstraintsPositions[i] += Eigen::Vector3f(0.03, 0, 0);
//        }
    }
    else if(key->text() == "a")
    {
//        for(int i=0;i<m_hardConstraintsPositions.size();++i)
//        {
//            m_hardConstraintsPositions[i] += Eigen::Vector3f(-0.03, 0, 0);
//        }
    }
    else if(key->text() == "s")
    {
//        for(int i=0;i<m_hardConstraintsPositions.size();++i)
//        {
//            m_hardConstraintsPositions[i] += Eigen::Vector3f(0, -0.03, 0);
//        }
    }
    else if(key->text() == "w")
    {
//        for(int i=0;i<m_hardConstraintsPositions.size();++i)
//        {
//            m_hardConstraintsPositions[i] += Eigen::Vector3f(0, 0.03, 0);
//        }
    }
    else if(key->text() == "r")
    {
//        for(int i=0;i<m_hardConstraintsPositions.size();++i)
//        {
//            m_hardConstraintsPositions[i] += Eigen::Vector3f(0, 0, 0.03);
//        }
    }
    else if(key->text() == "f")
    {
//        for(int i=0;i<m_hardConstraintsPositions.size();++i)
//        {
//            m_hardConstraintsPositions[i] += Eigen::Vector3f(0, 0, -0.03);
//        }
    }
//    else if(key->text() == "m")
//    {
//        m_move = !m_move;
//        if(m_move)
//        {
//            QList<int>::iterator it = m_selection.begin();
//            std::vector<TriVertex*> verts = m_mesh->Vertices();

//            m_hardConstraintsIndices.clear();
//            m_hardConstraintsPositions.clear();
//            for(;it!=m_selection.end();++it)
//            {
//                TriVertex* v = verts[*it];
//                Vector3 pos = v->Position();

//                m_hardConstraintsIndices.push_back(v->Id());
//                m_hardConstraintsPositions.push_back(toEigenVector3(pos));
//            }
//            m_simulator->initialize(m_dt, m_hardConstraintsIndices, m_hardConstraintsPositions, m_springConstraints, m_tetraConstraints);
//        }
//    }
    else
        QGLViewer::keyPressEvent(key);
}

//   C u s t o m i z e d   m o u s e   e v e n t s

void Viewer::mousePressEvent(QMouseEvent *e) {
    // Start selection. Mode is ADD with Shift key and TOGGLE with Alt key.
    m_rectangle = QRect(e->pos(), e->pos());

    if ((e->button() == Qt::LeftButton) && (e->modifiers() == Qt::ShiftModifier))
        m_selectionMode = ADD;
    else if ((e->button() == Qt::LeftButton) &&
             (e->modifiers() == Qt::ControlModifier))
        m_selectionMode = REMOVE;
    else if(m_move)
    {
        m_initialPoint = e->pos();
    }
    else {
        //    if (e->modifiers() == Qt::ControlModifier)
        //      startManipulation();

        QGLViewer::mousePressEvent(e);
    }
}

void Viewer::mouseMoveEvent(QMouseEvent *e) {
    if (m_selectionMode != NONE) {
        // Updates m_rectangle coordinates and redraws rectangle
        m_rectangle.setBottomRight(e->pos());
        update();
    }
//    else if(m_move)
//    {
//        QPoint currentPos = e->pos();
//        QPoint displacement = (currentPos - m_initialPoint);
//        qglviewer::Vec up = camera()->upVector();
//        qglviewer::Vec right = camera()->rightVector();
//        int width = camera()->screenWidth();
//        int height = camera()->screenHeight();
//        float realX = (float) displacement.x() / width;
//        float realY = (float) displacement.y() / height;
//        qglviewer::Vec v = up*realY*-1.0 + right*realX;

//        Vector3 u(v[0], v[1], v[2]);
//        QList<int>::iterator it = m_selection.begin();
//        std::vector<TriVertex*> verts = m_mesh->Vertices();

//        for(int i=0;it!=m_selection.end();++it, ++i)
//        {
//            TriVertex* v = verts[*it];
//            Vector3 pos = v->PositionInit();

//            qglviewer::Vec posSP = camera()->projectedCoordinatesOf(qglviewer::Vec(pos));
//            posSP.x += displacement.x();
//            posSP.y += displacement.y();

//            qglviewer::Vec disp = camera()->unprojectedCoordinatesOf(posSP);
//            Vector3 p(disp.x, disp.y, disp.z) ;
//            m_hardConstraintsPositions[i] = toEigenVector3(p);
//        }

//        update();

//    }
    else
    {
        QGLViewer::mouseMoveEvent(e);
    }
}

void Viewer::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_selectionMode != NONE) {
//        bool somethingSelected = false;

//        // Actual selection on the rectangular area.
//        std::vector<TriVertex*> verts = m_mesh->Vertices();
//        for(unsigned i=0;i<verts.size();++i)
//        {
//            TriVertex* v = verts[i];
//            Vector3 p = v->Position();
//            qglviewer::Vec src;
//            src[0] = p[0];
//            src[1] = p[1];
//            src[2] = p[2];

//            qglviewer::Vec w = camera()->projectedCoordinatesOf(src);
//            if (m_rectangle.contains(w.x,w.y))
//            {
//                if(m_selectionMode == ADD)
//                    addIdToSelection(i);
//                else if (m_selectionMode == REMOVE)
//                    removeIdFromSelection(i);
//                somethingSelected = true;
//            }
//        }
//        glEnd();

//        if(!somethingSelected)
//            m_selection.clear();

//        m_selectionMode = NONE;
//        // Update display to show new selected objects
//        update();
    }
    else
        QGLViewer::mouseReleaseEvent(e);
}

//   S e l e c t i o n   t o o l s
void Viewer::addIdToSelection(int id)
{
    if (!m_selection.contains(id))
    {
        m_selection.push_back(id);
    }
}

void Viewer::removeIdFromSelection(int id)
{
    m_selection.removeAll(id);
}


void Viewer::drawSelectionRectangle() const
{
    startScreenCoordinatesSystem();
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);

    glColor4f(0.0, 0.0, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2i(m_rectangle.left(), m_rectangle.top());
    glVertex2i(m_rectangle.right(), m_rectangle.top());
    glVertex2i(m_rectangle.right(), m_rectangle.bottom());
    glVertex2i(m_rectangle.left(), m_rectangle.bottom());
    glEnd();

    glLineWidth(2.0);
    glColor4f(0.4f, 0.4f, 0.5f, 0.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(m_rectangle.left(), m_rectangle.top());
    glVertex2i(m_rectangle.right(), m_rectangle.top());
    glVertex2i(m_rectangle.right(), m_rectangle.bottom());
    glVertex2i(m_rectangle.left(), m_rectangle.bottom());
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    stopScreenCoordinatesSystem();
}


Viewer::Viewer(QWidget *parent) : QGLViewer(parent)
{
    m_selectionMode = NONE;
    m_move = false;

    m_meshRows = 10;
    m_meshColumns = 10;
    m_meshSize = 1;

    m_dt = 0.01;
    m_totalMass = 1.0;
    m_iterations = 1;

    m_springStiffness = 10000;
    m_tetraStiffness = 10000;
    m_collisionStiffness = 100000000;
    m_positionStiffness = 100000000;

}

void Viewer::reset()
{
    m_selectionMode = NONE;
    m_move = false;

    delete(m_simulator);
    for(int i=0;i<m_staticBodies.size();++i)
        delete(m_staticBodies[i]);
    m_staticBodies.clear();
    init();
}

void Viewer::changeMeshRows(int val)
{
    m_meshRows = val;
}

void Viewer::changeMeshCols(int val)
{
    m_meshColumns = val;
}

void Viewer::changeSize(double size)
{
    m_meshSize = size;
}

void Viewer::changeParticleMass(double val)
{
    m_totalMass = val;
}

void Viewer::changeTimestep(double val)
{
    m_dt = val;
}

void Viewer::changeIterations(int val)
{
    m_iterations = val;
}

void Viewer::changeSpringStiffness(double val)
{
    m_springStiffness = val;
}

void Viewer::changeTetraStiffness(double val)
{
    m_tetraStiffness = val;
}

void Viewer::changePositionStiffness(double val)
{
    m_positionStiffness = val;
}

void Viewer::changeCollisionStiffness(double val)
{
    m_collisionStiffness = val;
}

