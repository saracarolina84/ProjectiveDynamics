#include "springBody.h"
#include "utils.h"

SpringBody::SpringBody(TriMesh *mesh, float particleMass, float springStiffness, bool addExtraSprings, float maxDistance)
    : ProjectiveBody(particleMass), m_mesh(mesh)
{
    m_numParticles = m_mesh->NumVertices();

    m_springConstraints.clear();
    for(int i=0;i<mesh->Edges().size();++i)
    {
        TriEdge *edge = mesh->Edges()[i];
        int v1 = edge->Origin()->Id();
        int v2 = edge->Head()->Id();

        Eigen::Vector3f p1 = toEigenVector3(m_mesh->Vertices()[v1]->Position());
        Eigen::Vector3f p2 = toEigenVector3(m_mesh->Vertices()[v2]->Position());

        SpringConstraint *c = new SpringConstraint(springStiffness, p1, p2, v1, v2);
        m_springConstraints.push_back(c);

        if(addExtraSprings)
        {
            if(edge->IsBoundary())
                continue;

            TriFace* f1 = edge->AdjFace();
            TriFace* f2 = edge->Twin()->AdjFace();

            for(int j=0;j<3;++j)
            {
                if(f1->Vertex(j) != edge->Origin() &&
                   f1->Vertex(j) != edge->Head())
                    v1 = f1->Vertex(j)->Id();
                if(f2->Vertex(j) != edge->Twin()->Origin() &&
                   f2->Vertex(j) != edge->Twin()->Head())
                    v2 = f2->Vertex(j)->Id();
            }
            Vector3 v1p = mesh->Vertices()[v1]->PositionInit();
            Vector3 v2p = mesh->Vertices()[v2]->PositionInit();
//            if(Vector3::squaredDistance(v1p, v2p) <= 1.01*2*meshCellSize*meshCellSize)
            if(Vector3::squaredDistance(v1p, v2p) <= maxDistance)
            {
                Eigen::Vector3f p1 = toEigenVector3(v1p);
                Eigen::Vector3f p2 = toEigenVector3(v2p);

                SpringConstraint *c = new SpringConstraint(springStiffness, p1, p2, v1, v2);
                m_springConstraints.push_back(c);
            }
        }
    }

    m_mesh->DeformWithoutInterpolation();
    m_meshCCDHandler = new HashCCDHandler(m_mesh);
    m_meshCCDHandler->Init();
}

Eigen::VectorXf SpringBody::getPositions()
{
    Eigen::VectorXf q;
    q.resize(m_mesh->NumVertices()*3);
    for(int i=0; i<m_mesh->NumVertices(); ++i)
    {
        q[i*3+0] = m_mesh->Vertices()[i]->Position()[0];
        q[i*3+1] = m_mesh->Vertices()[i]->Position()[1];
        q[i*3+2] = m_mesh->Vertices()[i]->Position()[2];
    }
    return q;
}

void SpringBody::setPositions(const Eigen::VectorXf &qx, const Eigen::VectorXf &qy, const Eigen::VectorXf &qz)
{
    for(int i=0; i<m_mesh->NumVertices(); ++i)
    {
        m_mesh->Vertices()[i]->UpdatePositionOld();
        m_mesh->Vertices()[i]->Position(qx[i], qy[i], qz[i]);
    }
}

std::vector<ProjectiveConstraint *> SpringBody::getConstraints()
{
    std::vector<ProjectiveConstraint *> result;
    result.insert(result.end(), m_springConstraints.begin(), m_springConstraints.end());
    result.insert(result.end(), m_positionConstraints.begin(), m_positionConstraints.end());
    return result;
}

void SpringBody::addPositionConstraint(float stiffness, int vIndex)
{
    Eigen::Vector3f p = toEigenVector3(m_mesh->Vertices()[vIndex]->Position());
    PositionConstraint *c = new PositionConstraint(stiffness, p, vIndex);
    m_positionConstraints.push_back(c);
}

void SpringBody::draw()
{
    m_mesh->Draw();
}