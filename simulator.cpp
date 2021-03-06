#include <set>
#include <omp.h>

#include "simulator.h"
#include "utils.h"

#include <QTime>

Simulator::Simulator(float dt, int iterations, std::vector<ProjectiveBody*> &bodies, float collisionStiffness)
{
//    omp_set_num_threads(1);

    std::cout << "Max num threads: " << omp_get_max_threads() << std::endl;
    Eigen::initParallel();

    initialize(dt, iterations, bodies, collisionStiffness);
}

Simulator::~Simulator()
{

}

void Simulator::initialize(float dt, int iterations, vector<ProjectiveBody*> &bodies, float collisionStiffness)
{
    m_dt = dt;
    m_iterations = iterations;
    m_collisionStiffness = collisionStiffness;
    m_q[0].resize(0);
    m_q[1].resize(0);
    m_q[2].resize(0);
    m_particleMass.resize(0);
    m_constraints.clear();
    m_bodies.clear();

    int totalNumParticles = 0;
    for(int i=0; i<bodies.size(); ++i)
    {
        m_bodies.push_back(bodies[i]);
        m_bodyToIndex[bodies[i]] = totalNumParticles;
        totalNumParticles += bodies[i]->getNumParticles();
    }

    m_q[0].resize(totalNumParticles);
    m_q[1].resize(totalNumParticles);
    m_q[2].resize(totalNumParticles);
    m_v[0].resize(totalNumParticles);
    m_v[1].resize(totalNumParticles);
    m_v[2].resize(totalNumParticles);
    m_v[0].setZero();
    m_v[1].setZero();
    m_v[2].setZero();
    m_particleMass.resize(totalNumParticles);

    for(int i=0; i<bodies.size(); ++i)
    {
        int bodyIndex = m_bodyToIndex[bodies[i]];
        if(bodies[i]->getNumParticles() > 0)
        {
            Eigen::VectorXf pos = bodies[i]->getPositions();
//            Eigen::VectorXf vel = bodies[i]->getVelocities(m_dt);
            for(int j=0; j<pos.rows()/3; ++j)
            {
                m_q[0][bodyIndex+j] = pos[j*3+0];
                m_q[1][bodyIndex+j] = pos[j*3+1];
                m_q[2][bodyIndex+j] = pos[j*3+2];
            }
//            for(int j=0; j<vel.rows()/3; ++j)
//            {
//                m_v[0][bodyIndex+j] = vel[j*3+0];
//                m_v[1][bodyIndex+j] = vel[j*3+1];
//                m_v[2][bodyIndex+j] = vel[j*3+2];
//            }
            m_particleMass.segment(m_bodyToIndex[bodies[i]], bodies[i]->getNumParticles()) = bodies[i]->getParticleMassVector();
            std::vector<ProjectiveConstraint*> constraints = bodies[i]->getConstraints();
            m_constraints.insert(m_constraints.end(), constraints.begin(), constraints.end());
        }

    }

    m_numParticles = totalNumParticles;

    m_massMatrix = m_particleMass.asDiagonal();
    m_massMatrixInv = m_massMatrix.inverse();

    m_Lhs[0] = m_massMatrix * (1.0/(dt*dt));
    m_Lhs[1] = m_massMatrix * (1.0/(dt*dt));
    m_Lhs[2] = m_massMatrix * (1.0/(dt*dt));

    for (int i=0; i < m_constraints.size(); ++i)
    {
        ProjectiveConstraint* c = m_constraints[i];
        Eigen::MatrixXf Ai = c->getAMatrix();

        Eigen::SparseMatrix<float> SiX = c->getSMatrix(m_numParticles, m_bodyToIndex[c->m_body], 0);
        Eigen::SparseMatrix<float> SiY = c->getSMatrix(m_numParticles, m_bodyToIndex[c->m_body], 1);
        Eigen::SparseMatrix<float> SiZ = c->getSMatrix(m_numParticles, m_bodyToIndex[c->m_body], 2);

        m_Lhs[0] = m_Lhs[0] + c->m_stiffness * SiX.transpose() * Ai.transpose() * Ai * SiX;
        m_Lhs[1] = m_Lhs[1] + c->m_stiffness * SiY.transpose() * Ai.transpose() * Ai * SiY;
        m_Lhs[2] = m_Lhs[2] + c->m_stiffness * SiZ.transpose() * Ai.transpose() * Ai * SiZ;
    }

    m_collisionsInPreviousFrame = false;
    m_cholesky[0].compute(m_Lhs[0]);
    m_cholesky[1].compute(m_Lhs[1]);
    m_cholesky[2].compute(m_Lhs[2]);

    m_fext[0].resize(m_numParticles);
    m_fext[0].setZero();
    m_fext[1].resize(m_numParticles);
    for(int i=0;i<totalNumParticles;++i)
        m_fext[1][i] = -9.8 * m_particleMass[i];
    m_fext[2].resize(m_numParticles);
    m_fext[2].setZero();

//    CDQueries::testContinuousAABB = true;
}

void Simulator::advanceTime()
{   
    QTime myTotalTimer, myTimer;
    m_timeLocalSolve = 0;
    m_timeGlobalSolve = 0;
    m_timeSetPositions = 0;

    myTotalTimer.start();

    myTimer.start();

    Eigen::VectorXf qold[3], rhs[3];
    qold[0] = m_q[0];
    qold[1] = m_q[1];
    qold[2] = m_q[2];

    m_sn[0] = m_q[0] + m_dt*m_dt*m_massMatrixInv*m_fext[0];
    m_sn[1] = m_q[1] + m_dt*m_dt*m_massMatrixInv*m_fext[1];
    m_sn[2] = m_q[2] + m_dt*m_dt*m_massMatrixInv*m_fext[2];
    for(int i=0;i<m_bodies.size(); ++i)
    {
        int bodyIndex = m_bodyToIndex[m_bodies[i]];

        for(int j=0; j<m_bodies[i]->getNumParticles(); ++j)
        {
            m_sn[0][bodyIndex+j] += m_dt*m_v[0][bodyIndex+j]*m_bodies[i]->getDamping();
            m_sn[1][bodyIndex+j] += m_dt*m_v[1][bodyIndex+j]*m_bodies[i]->getDamping();
            m_sn[2][bodyIndex+j] += m_dt*m_v[2][bodyIndex+j]*m_bodies[i]->getDamping();
        }
    }

    m_q[0] = m_sn[0];
    m_q[1] = m_sn[1];
    m_q[2] = m_sn[2];

    for(int i=0; i<m_bodies.size(); ++i)
    {
        int numParticles = m_bodies[i]->getNumParticles();
        int bodyIndex = m_bodyToIndex[m_bodies[i]];
        m_bodies[i]->setPositions(m_q[0].segment(bodyIndex,numParticles),
                                    m_q[1].segment(bodyIndex,numParticles), m_q[2].segment(bodyIndex,numParticles));
    }

    rhs[0].resize(m_numParticles);
    rhs[1].resize(m_numParticles);
    rhs[2].resize(m_numParticles);

    m_timeStart = myTimer.elapsed();

    detectCollisions();


    for(int iter=0;iter<m_iterations;++iter)
    {
        myTimer.restart();

        rhs[0].setZero();
        rhs[1].setZero();
        rhs[2].setZero();
        m_projected.clear();
        m_projectedCollisions.clear();
        m_appliedCollisions.clear();

        std::vector<Eigen::VectorXf> indepRhs[3];
        indepRhs[0].resize(m_constraints.size());
        indepRhs[1].resize(m_constraints.size());
        indepRhs[2].resize(m_constraints.size());

        std::vector<std::vector<Eigen::Vector3f> > indepProjected;
        indepProjected.resize(m_constraints.size());

#pragma omp parallel for
        for(int i=0;i<m_constraints.size(); ++ i)
        {
            ProjectiveConstraint *c = m_constraints[i];
            int bodyIndex = m_bodyToIndex[c->m_body];

            std::vector<Eigen::Vector3f> q, p;
            for(int j=0; j<c->m_numParticles; ++j)
            {
                Eigen::Vector3f qpos = toEigenVector3(m_q[0], m_q[1], m_q[2], bodyIndex + c->getVIndex(j));
                q.push_back(qpos);
            }

            c->project(q, p);

            indepProjected[i].insert(indepProjected[i].end(), p.begin(), p.end());

            Eigen::SparseMatrix<float> constraintRhsMatrixX = c->getRhsMatrix(m_numParticles, bodyIndex, 0);
            Eigen::SparseMatrix<float> constraintRhsMatrixY = c->getRhsMatrix(m_numParticles, bodyIndex, 1);
            Eigen::SparseMatrix<float> constraintRhsMatrixZ = c->getRhsMatrix(m_numParticles, bodyIndex, 2);


            std::vector<Eigen::VectorXf> pdim(3);
            pdim[0].resize(c->m_numParticles);
            pdim[1].resize(c->m_numParticles);
            pdim[2].resize(c->m_numParticles);

            for(int j=0; j<c->m_numParticles; ++j)
            {
                for(int k=0; k<3; ++k)
                    pdim[k][j] = p[j][k];
            }

            indepRhs[0][i] = constraintRhsMatrixX * pdim[0];
            indepRhs[1][i] = constraintRhsMatrixY * pdim[1];
            indepRhs[2][i] = constraintRhsMatrixZ * pdim[2];
        }

        for(int i=0;i<m_constraints.size(); ++ i)
        {
            rhs[0] += indepRhs[0][i];
            rhs[1] += indepRhs[1][i];
            rhs[2] += indepRhs[2][i];

            m_projected.insert(m_projected.end(), indepProjected[i].begin(), indepProjected[i].end());
        }

        Eigen::SparseMatrix<float> collisionsLHS[3];
        collisionsLHS[0].resize(m_Lhs[0].rows(), m_Lhs[0].cols());
        collisionsLHS[1].resize(m_Lhs[1].rows(), m_Lhs[1].cols());
        collisionsLHS[2].resize(m_Lhs[2].rows(), m_Lhs[2].cols());

        collisionsLHS[0].setZero();
        collisionsLHS[1].setZero();
        collisionsLHS[2].setZero();

        vector< vector<std::pair<Collision*, int> > > indepAppliedCollisions;
        indepAppliedCollisions.resize(m_collisions.size());

        vector< vector<Eigen::Vector3f> > indepProjectedCollisions;
        indepProjectedCollisions.resize(m_collisions.size());

#pragma omp parallel for
        for(int i=0;i<m_collisions.size(); ++ i)
        {
            CollisionInfoVF* col = (CollisionInfoVF*) m_collisions[i].info;

            LA::Vector3 p_projLA = col->v->Position() - Vector3::dotProd(col->v->Position() - col->p, col->n) * col->n;
            LA::Vector3 pToProj = p_projLA - col->v->Position();
            pToProj.normalize();
            if(LA::Vector3::dotProd(pToProj, col->n) < 0)
                continue;

            Eigen::Vector3f p_proj = toEigenVector3(p_projLA);

            std::vector<PositionConstraint> constraints = m_collisions[i].body1->getPositionConstraints(m_collisionStiffness, col->v->Id(), p_proj);

            for(int j=0; j<constraints.size(); ++j)
            {
                int bodyIndex = m_bodyToIndex[constraints[j].m_body];

                Eigen::Vector3f p_proj_new = constraints[j].getPosition();
                Eigen::Vector3f ne = toEigenVector3(col->n);

                int index = bodyIndex+constraints[j].getVIndex(0);

                Eigen::Vector3f inertiaPos = Eigen::Vector3f(
                            m_sn[0][index],
                        m_sn[1][index],
                        m_sn[2][index]);
                Eigen::Vector3f snToProj = inertiaPos - p_proj_new;
                snToProj.normalize();
                if(snToProj.dot(ne) > 0)
                    continue;

                indepAppliedCollisions[i].push_back(std::make_pair(&m_collisions[i], j));
                indepProjectedCollisions[i].push_back(p_proj_new);

    #pragma omp critical
                {
                    rhs[0][index] += constraints[j].m_stiffness * p_proj_new[0];
                    rhs[1][index] += constraints[j].m_stiffness * p_proj_new[1];
                    rhs[2][index] += constraints[j].m_stiffness * p_proj_new[2];
                }

    #pragma omp critical
                {
                    collisionsLHS[0].coeffRef(index,index) += constraints[j].m_stiffness;
                    collisionsLHS[1].coeffRef(index,index) += constraints[j].m_stiffness;
                    collisionsLHS[2].coeffRef(index,index) += constraints[j].m_stiffness;
                }
            }
        }

        for(int i=0; i<m_collisions.size(); ++i)
        {
            m_appliedCollisions.insert(m_appliedCollisions.end(), indepAppliedCollisions[i].begin(), indepAppliedCollisions[i].end());
            m_projectedCollisions.insert(m_projectedCollisions.end(), indepProjectedCollisions[i].begin(), indepProjectedCollisions[i].end());
        }
        m_numConstraints = m_projected.size() + m_projectedCollisions.size();

        m_Lhs[0] = m_Lhs[0] + collisionsLHS[0];
        m_Lhs[1] = m_Lhs[1] + collisionsLHS[1];
        m_Lhs[2] = m_Lhs[2] + collisionsLHS[2];

        m_timeLocalSolve += myTimer.elapsed();
        myTimer.restart();

        rhs[0] += (1.0/(m_dt*m_dt)) * m_massMatrix * m_sn[0];
        rhs[1] += (1.0/(m_dt*m_dt)) * m_massMatrix * m_sn[1];
        rhs[2] += (1.0/(m_dt*m_dt)) * m_massMatrix * m_sn[2];

        if(m_collisions.size() > 0 || m_collisionsInPreviousFrame)
        {
#pragma omp parallel for
            for(int k=0; k<3; ++k)
                m_cholesky[k].compute(m_Lhs[k]);
            if(m_collisions.size() > 0)
                m_collisionsInPreviousFrame = true;
            else
                m_collisionsInPreviousFrame = false;
        }

#pragma omp parallel for
        for(int k=0; k<3; ++k)
            m_q[k] = m_cholesky[k].solve(rhs[k]);

        m_Lhs[0] = m_Lhs[0] - collisionsLHS[0];
        m_Lhs[1] = m_Lhs[1] - collisionsLHS[1];
        m_Lhs[2] = m_Lhs[2] - collisionsLHS[2];

        m_timeGlobalSolve += myTimer.elapsed();
        myTimer.restart();

        if(m_cholesky[0].info() != Eigen::Success)
            std::cout << "Error solving system x" << std::endl;
        if(m_cholesky[1].info() != Eigen::Success)
            std::cout << "Error solving system y" << std::endl;
        if(m_cholesky[2].info() != Eigen::Success)
            std::cout << "Error solving system z" << std::endl;



        for(int i=0; i<m_bodies.size(); ++i)
        {
            int numParticles = m_bodies[i]->getNumParticles();
            int bodyIndex = m_bodyToIndex[m_bodies[i]];
            m_bodies[i]->setPositions(m_q[0].segment(bodyIndex,numParticles),
                                        m_q[1].segment(bodyIndex,numParticles), m_q[2].segment(bodyIndex,numParticles));
        }
        m_timeSetPositions += myTimer.elapsed();
    }

    myTimer.restart();

    Eigen::VectorXf vOld[3];
    vOld[0] = m_v[0];
    vOld[1] = m_v[1];
    vOld[2] = m_v[2];
    for(int i=0; i<m_v[0].rows(); ++i)
    {
        m_v[0] = (1/m_dt) * (m_q[0] - qold[0]);
        m_v[1] = (1/m_dt) * (m_q[1] - qold[1]);
        m_v[2] = (1/m_dt) * (m_q[2] - qold[2]);
    }

    for(int i=0; i<m_appliedCollisions.size(); ++i)
    {
        Collision *col = m_appliedCollisions[i].first;
        CollisionInfoVF* info = (CollisionInfoVF*) col->info;
        std::vector<int> indices = col->body1->getIndices(info->v->Id());
        int index = m_bodyToIndex[col->body1] + indices[m_appliedCollisions[i].second];
        Eigen::Vector3f n = toEigenVector3(info->n);
        Eigen::Vector3f v(vOld[0][index], vOld[1][index], vOld[2][index]);
        Eigen::Vector3f vReflect = (v - 2*(v.dot(n))*n)*col->body1->getRestitution();
        m_v[0][index] = vReflect[0];
        m_v[1][index] = vReflect[1];
        m_v[2][index] = vReflect[2];
    }

    m_timeFinish = myTimer.elapsed();
    m_timeTotal = myTotalTimer.elapsed();
    //    cout << "Total Time: " << myTotalTimer.elapsed() << endl;
}

void Simulator::detectCollisions()
{
    QTime timer;
    timer.start();
    m_collisions.clear();
    m_normalsCollisions.clear();

    for(int i=0;i<m_bodies.size();++i)
    {
        if(m_bodies[i]->getCCDHandler())
            m_bodies[i]->getCCDHandler()->PreQueryNoUpdatePositions(false);
    }

    for(int i=0; i<m_bodies.size(); ++i)
    {
        if(m_bodies[i]->getType() == ProjectiveBody::STATIC)
            continue;

        for(int j=i+1; j<m_bodies.size(); ++j)
        {
            if(!m_bodies[i]->getCCDHandler() || !m_bodies[j]->getCCDHandler())
                continue;

            vector<CollisionInfo*> newCollisions = CDQueries::HashQuery(m_bodies[i]->getCCDHandler(), m_bodies[j]->getCCDHandler(), false);
//            std::set<std::pair<int, int> > collisionsFound;
            std::set<int > collisionsFound;
            for(int k=0; k<newCollisions.size(); ++k)
            {
                CollisionInfoVF* col = (CollisionInfoVF*) newCollisions[k];
                if(col->objsSwitched)
                    continue;
//                auto it = collisionsFound.find(std::make_pair(col->v->Id(), col->f->Id()));
//                if(it != collisionsFound.end())
//                    continue;
//                collisionsFound.insert(std::make_pair(col->v->Id(), col->f->Id()));
                auto it = collisionsFound.find(col->v->Id());
                if(it != collisionsFound.end())
                    continue;
                collisionsFound.insert(col->v->Id());
                col->n.normalize();
                m_normalsCollisions.push_back(make_pair(toEigenVector3(col->p), toEigenVector3(col->n)));
                m_collisions.push_back(Collision(m_bodies[i], m_bodies[j], col));

            }
        }
    }
//    std::cout << "NumCollisions: " << m_collisions.size() << std::endl;
    for(int i=0;i<m_bodies.size();++i)
    {
        if(m_bodies[i]->getCCDHandler())
            m_bodies[i]->getCCDHandler()->PostQuery();
    }

    m_timeCollisionDetection = timer.elapsed();
}
