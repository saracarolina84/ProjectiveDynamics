#include "springConstraint.h"

using namespace std;

SpringConstraint::SpringConstraint(ProjectiveBody *body, float stiffness, Eigen::Vector3f p1, Eigen::Vector3f p2, int vIndex1, int vIndex2) :
    ProjectiveConstraint(body, stiffness, 2), m_vIndex1(vIndex1), m_vIndex2(vIndex2)
{
    m_restLength = (p1-p2).norm();
    m_fixed1 = false;
    m_fixed2 = false;

}

void SpringConstraint::setFixed(int index)
{
    if(index == m_vIndex1)
        m_fixed1 = true;
    else if(index == m_vIndex2)
        m_fixed2 = true;
}

void SpringConstraint::project(std::vector<Eigen::Vector3f> q, std::vector<Eigen::Vector3f>& p)
{
    assert(q.size() == 2);
    p.resize(2);

    Eigen::Vector3f v = (q[1] - q[0]);
    v.normalize();


    if(m_fixed1 && m_fixed2)
    {
        p[0] = q[0];
        p[1] = q[1];
        return;
    }

    if(m_fixed1)
    {
        p[0] = q[0];
        p[1] = q[0] + m_restLength * v;
        return;
    }
    if(m_fixed2)
    {
        p[1] = q[1];
        p[0] = q[1] - m_restLength * v;
        return;
    }

    float l = (q[0]-q[1]).norm();
    float offset = (l - m_restLength)/2.0;
    p[0] = q[0] + offset * v;
    p[1] = q[1] - offset * v;
}

Eigen::MatrixXf SpringConstraint::getAMatrix()
{
    Eigen::MatrixXf A(2,2);
    A.setOnes();
    A *= 0.5;
    A(0, 1) *= -1.0;
    A(1, 0) *= -1.0;
    return A;
}

Eigen::MatrixXf SpringConstraint::getBMatrix()
{
    return getAMatrix();
}

Eigen::SparseMatrix<float> SpringConstraint::getSMatrix(int numParticles, int bodyIndex, int dim)
{
    Eigen::SparseMatrix<float> Si(2, numParticles);
    vector<Eigen::Triplet<float> > SiData;
    SiData.push_back(Eigen::Triplet<float> (0, bodyIndex + m_vIndex1, 1));
    SiData.push_back(Eigen::Triplet<float> (1, bodyIndex + m_vIndex2, 1));

    Si.setFromTriplets(SiData.begin(), SiData.end());
    return Si;
}

int SpringConstraint::getVIndex(int index)
{
    switch(index)
    {
    case 0: return m_vIndex1; break;
    case 1: return m_vIndex2; break;
    default: return -1; break;
    }
    return -1;
}
