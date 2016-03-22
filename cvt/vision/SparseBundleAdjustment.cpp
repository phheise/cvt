/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include <cvt/vision/SparseBundleAdjustment.h>

#include <unsupported/Eigen/SparseExtra>

#include <cvt/math/Math.h>
#include <cvt/math/SE3.h>
#include <cvt/vision/Vision.h>

#include <cstring>

namespace cvt {

    SparseBundleAdjustment::SparseBundleAdjustment() :
        _nPts( 0 ),
        _nCams( 0 ),
        _nMeas( 0 ),
        _pointsJTJ( 0 ),
        _invAugPJTJ( 0 ),
        _camsJTJ( 0 ),
        _camResiduals( 0 ),
        _pointResiduals( 0 )
    {
    }

    SparseBundleAdjustment::~SparseBundleAdjustment()
    {
        if( _pointsJTJ )
            delete[] _pointsJTJ;
        if( _invAugPJTJ )
            delete[] _invAugPJTJ;
        if( _camsJTJ )
            delete[] _camsJTJ;
        if( _camResiduals )
            delete[] _camResiduals;
        if( _pointResiduals )
            delete[] _pointResiduals;
    }

    static bool _vectorHasNaNOrInf( Eigen::VectorXd& v )
    {
        for( size_t i = 0; i < v.rows(); ++i ){
            if( Math::isNaN( v[ i ] ) || Math::isInf( v[ i ] ) ) {
                return true;
            }
        }
        return false;
    }

    void SparseBundleAdjustment::optimize( SlamMap & map, const TerminationCriteria<double> & criteria )
    {
        _iterations = 0;
        _costs	    = 0.0;

        // evaluate costs, resize the vectors
        size_t numPoints = map.numFeatures();
        size_t numCams	 = map.numKeyframes();

        // resize internal structures for jacobians etc.
        resize( numCams, numPoints, map.numMeasurements() );

        Eigen::VectorXd	deltaCam( camParamDim * numCams );
        Eigen::VectorXd	deltaPoint( pointParamDim * numPoints );

        // TODO: modularize on the solver:
        // SimplicialCholesky -> LDLt by default, can also set LLt
        Eigen::SimplicialCholesky<Eigen::SparseMatrix<double, Eigen::ColMajor>, Eigen::Lower> solver;

        double lastCosts = 1e20;
        while( true ){
            // build the reduced system: in first iteration, eval costs
            buildReducedCameraSystem( map );

            if( _iterations == 0 ){
                // solve symbolic
                solver.analyzePattern( _sparseReduced );
            }

            solver.factorize( _sparseReduced );
            deltaCam = solver.solve( _reducedRHS );

            // safety check on computed delta
            if( _vectorHasNaNOrInf( deltaCam ) ){
                // increase lambda and try again
                _lambda *= 5.0f;
                continue;
            }

            // try apply the step:
            updateCameras( deltaCam, map );

            //	update the points and re-evaluate costs
            lastCosts = _costs;
            solveStructure( deltaPoint, deltaCam, map );

            std::cout << "Iteration: " << _iterations << " LastCosts: " << lastCosts << ", CurrentCosts: " << _costs << ", lambda: " << _lambda << std::endl;

            if( _costs < lastCosts ){
                // step was good -> update lambda and do next step
                if( _lambda > 1e-8 )
                    _lambda *= 0.1;
                _iterations++;
            } else {
                deltaCam	*= -1.0;
                deltaPoint  *= -1.0;

                // undo the step
                undoStep( deltaCam, deltaPoint, map );

                // reset the step
                _costs = lastCosts;

                // update the lamda
                _lambda *= 5.0;
            }

            if( criteria.finished( _costs, _iterations ) ){
                break;
            }

        }
    }

    void SparseBundleAdjustment::buildReducedCameraSystem( const SlamMap & map )
    {
        evaluateApproxHessians( map );
        updateInverseAugmentedPointHessians();

        if( _iterations == 0 ){
            // prepare the sparse matrix (storage allocation) <- need jointMeasures to be filled
            prepareSparseMatrix( map.numKeyframes() );
        }

        fillSparseMatrix( map );
    }

    void SparseBundleAdjustment::evaluateApproxHessians( const SlamMap & map )
    {
        CamScreenJacType	screenJacCam;
        PointScreenJacType	screenJacPoint;
        Eigen::Matrix<double, 6, 2> jCamTCovInv;
        Eigen::Matrix<double, 3, 2> jPointTCovInv;
        Eigen::Matrix<double, 3, 1> point3d, pCam;
        Eigen::Matrix<double, 2, 1> residual;
        Eigen::Matrix<double, 2, 1> reproj;

        const Eigen::Matrix3d & K = map.intrinsics();

        size_t currMeas = 0;
        double avgDiag = 0.0;
        bool firstIter = ( _iterations == 0 );

        // for all 3D Points
        clear();
        _costs = 0.0;
        for( size_t i = 0; i < _nPts; i++ ){
            _pointsJTJ[ i ].setZero();
            _pointResiduals[ i ].setZero();

            // get the MapFeature:
            const MapFeature & feature = map.featureForId( i );
            const Eigen::Vector4d & ptmp = feature.estimate();
            point3d = ptmp.head<3>() / ptmp[ 3 ];


            // for all point tracks (meas. in the cams) of this point
            MapFeature::ConstPointTrackIterator camIterCurr = feature.pointTrackBegin();
            const MapFeature::ConstPointTrackIterator camIterEnd  = feature.pointTrackEnd();

            /* update the joint meas structure: -> only needed in first iteration! */
            if( firstIter ){
                while( camIterCurr != camIterEnd ){
                    MapFeature::ConstPointTrackIterator camIter = camIterCurr;
                    camIter++;
                    while( camIter != camIterEnd ) {
                        _jointMeasures.addMeasurementForEntity( *camIterCurr, *camIter, i );
                        ++camIter;
                    }
                    ++camIterCurr;
                }
            }

            for( MapFeature::ConstPointTrackIterator camIter = feature.pointTrackBegin();
                 camIter != camIterEnd;
                 camIter++, currMeas++ ){
                // get the keyframe:
                const Keyframe & keyframe = map.keyframeForId( *camIter );

                // screen jacobian for this 3D point in that camera
                const Eigen::Matrix4d & trans = keyframe.pose().transformation();
                const Eigen::Matrix<double, 3, 3> & R = trans.block<3, 3>( 0, 0 );
                pCam = R * point3d + trans.block<3, 1>( 0, 3 );

                // screen jacobian for this camera
                keyframe.pose().screenJacobian( screenJacCam, pCam, K );

                // point jacobian for this point in this cam:
                evalScreenJacWrtPoint( reproj, screenJacPoint, pCam, K, R );

                const MapMeasurement & mm = keyframe.measurementForId( i );
                residual = mm.point - reproj;

                // J^T * Cov^-1
                jCamTCovInv = screenJacCam.transpose() * mm.information;
                jPointTCovInv = screenJacPoint.transpose() * mm.information;

                // accumulate the jacobians
                _pointsJTJ[ i ]			+= jPointTCovInv * screenJacPoint;
                _camsJTJ[ *camIter ]	+= jCamTCovInv * screenJacCam;

                // accumulate the residuals
                _pointResiduals[ i ]		+= ( jPointTCovInv * residual );
                _camResiduals[ *camIter ]   += ( jCamTCovInv   * residual );

                _costs += residual.transpose() * mm.information * residual;

                /* camPointJacobian */
                _camPointJTJ.block( *camIter, i ) = ( jCamTCovInv * screenJacPoint );

            }

            // the pointjac JTJ & pointResidual sums for pt i are complete now
            if( firstIter )
                avgDiag += _pointsJTJ[ i ].diagonal().array().sum();

        }

        _costs /= _nMeas;

        // compute initial lambda on first iteration
        if( firstIter ) {
            // the camJac & JTJ & pointResidual sums are complete now
            for( size_t j = 0; j < _nCams; j++ ){
                avgDiag += _camsJTJ[ j ].diagonal().array().sum();
            }

            // initial lambda
//            _lambda = avgDiag / ( ( _nCams * 6 + _nPts * 3 ) * 100000.0 );
            _lambda = avgDiag / ( ( _nCams * 6 + _nPts * 3 ) * 1000.0 );
        }
    }

    void SparseBundleAdjustment::clear()
    {
        for( size_t i = 0; i < _nCams; i++ ){
            _camsJTJ[ i ].setZero();
            _camResiduals[ i ].setZero();
        }
    }

    void SparseBundleAdjustment::prepareSparseMatrix( size_t numCams )
    {
        // according to the joint Point tracks, we can create the matrix once
        _sparseReduced.reserve( camParamDim * camParamDim * _jointMeasures.numBlocks() );

        size_t c2;
        for( size_t c = 0; c < numCams; c++ ){
            size_t currCamRow = c * camParamDim;
            size_t currCamCol = currCamRow;
            const JointMeasurements::ConstMapIterType iStop  = _jointMeasures.secondEntityIteratorEnd( c );
            JointMeasurements::ConstMapIterType iBeg   = _jointMeasures.secondEntityIteratorBegin( c );

            for( size_t innerCol =  0; innerCol < camParamDim; innerCol++ ){
                size_t col = currCamCol + innerCol;
                _sparseReduced.startVec( col );

                _sparseReduced.insertBack( currCamRow + 0, col ) = 0;
                _sparseReduced.insertBack( currCamRow + 1, col ) = 0;
                _sparseReduced.insertBack( currCamRow + 2, col ) = 0;
                _sparseReduced.insertBack( currCamRow + 3, col ) = 0;
                _sparseReduced.insertBack( currCamRow + 4, col ) = 0;
                _sparseReduced.insertBack( currCamRow + 5, col ) = 0;

                JointMeasurements::ConstMapIterType iStart = iBeg;
                while( iStart != iStop ){
                    c2 = iStart->first;
                    size_t c2Row = c2 * camParamDim;
                    _sparseReduced.insertBack( c2Row + 0, col ) = 0;
                    _sparseReduced.insertBack( c2Row + 1, col ) = 0;
                    _sparseReduced.insertBack( c2Row + 2, col ) = 0;
                    _sparseReduced.insertBack( c2Row + 3, col ) = 0;
                    _sparseReduced.insertBack( c2Row + 4, col ) = 0;
                    _sparseReduced.insertBack( c2Row + 5, col ) = 0;
                    ++iStart;
                }
            }
        }
        _sparseReduced.finalize();
    }

    void SparseBundleAdjustment::fillSparseMatrix( const SlamMap & map )
    {
        // according to the joint Point tracks, we can now fill our matrix:
        CamJTJ tmpBlock;
        Eigen::Matrix<double, camParamDim, pointParamDim> tmpEval;
        CamResidualType tmpRes;

        size_t numCams = map.numKeyframes();
        for( size_t c = 0; c < numCams; c++ ){
            // first create block for this cam:
            tmpBlock = _camsJTJ[ c ];
            tmpRes   = _camResiduals[ c ];

			// augment the jacobian diagonal
            //tmpBlock.diagonal().array() += _lambda;
            tmpBlock.diagonal().array() *= ( 1.0 + _lambda );

            // go over all point measures:
            const Keyframe & k = map.keyframeForId( c );
            Keyframe::MeasurementIterator measIter = k.measurementsBegin();
            Keyframe::MeasurementIterator measEnd  = k.measurementsEnd();
            while( measIter != measEnd ){
                size_t pointId = measIter->first;

                const Eigen::Matrix<double, camParamDim, pointParamDim> & cp = _camPointJTJ.block( c, pointId );
                tmpEval = cp * _invAugPJTJ[ pointId ];
                tmpBlock -= tmpEval * cp.transpose();
                tmpRes   -= tmpEval * _pointResiduals[ pointId ];

                ++measIter;
            }

            // set the block in the sparse matrix:
            setBlockInReducedSparse( tmpBlock, c, c );
            _reducedRHS.segment<camParamDim>( camParamDim * c ) = tmpRes;


            JointMeasurements::ConstMapIterType iter   = _jointMeasures.secondEntityIteratorBegin( c );
            JointMeasurements::ConstMapIterType iStop  = _jointMeasures.secondEntityIteratorEnd( c );
            while( iter != iStop ){
                size_t c2 = iter->first; // id of second cam:

                // iterate over the joint measurements of the two cameras
                std::set<size_t>::const_iterator pIdIter = iter->second.begin();
                std::set<size_t>::const_iterator pEnd    = iter->second.end();
                tmpBlock.setZero();
                while( pIdIter != pEnd ){
                    size_t pId = *pIdIter;
                    tmpBlock -= _camPointJTJ.block( c, pId ) * _invAugPJTJ[ pId ] * _camPointJTJ.block( c2, pId ).transpose();
                    ++pIdIter;
                }
                ++iter;

                setBlockInReducedSparse( tmpBlock.transpose(), c2, c );
            }
        }
    }

    void SparseBundleAdjustment::updateInverseAugmentedPointHessians()
    {
        PointJTJ inv;
        for( size_t i = 0; i < _nPts; i++ ){
            inv = _pointsJTJ[ i ];
            // augment the diagonal:
            //inv.diagonal().array() += _lambda;
            inv.diagonal().array() *= ( 1.0 + _lambda );
            // TODO: is there a way to exploit symmetry when inverting with Eigen?
            _invAugPJTJ[ i ] = inv.inverse();
        }
    }

    void SparseBundleAdjustment::setBlockInReducedSparse( const CamJTJ & m,
                                                          size_t bRow,
                                                          size_t bCol )
    {
        size_t r = camParamDim * bRow;
        size_t c = camParamDim * bCol;

        for( size_t i = 0; i < camParamDim; i++ )
            for( size_t k = 0; k < camParamDim; k++ )
                _sparseReduced.coeffRef( r+k, c+i ) = m( k, i );
    }

    void SparseBundleAdjustment::updateCameras( const Eigen::VectorXd& deltaCam,
                                                SlamMap & map )
    {
        for( size_t i = 0; i < map.numKeyframes(); i++ ){
            map.keyframeForId( i ).updatePose( deltaCam.segment<camParamDim>( camParamDim * i ) );
        }
    }

    void SparseBundleAdjustment::solveStructure( Eigen::VectorXd & deltaStruct,
                                                 const Eigen::VectorXd & deltaCam,
                                                 SlamMap & map )
    {
        size_t nPts = map.numFeatures();
        Eigen::Vector3d res;
        Eigen::Vector3d tmp;
        Eigen::Vector2d pp, r;

        const Eigen::Matrix3d & K = map.intrinsics();

        _costs = 0.0;
        for( size_t i = 0; i < nPts; i++ ){
            MapFeature& f = map.featureForId( i );
            MapFeature::ConstPointTrackIterator camIter = f.pointTrackBegin();
            const MapFeature::ConstPointTrackIterator itEnd = f.pointTrackEnd();

            res = _pointResiduals[ i ];
            while( camIter != itEnd ){
                res -= _camPointJTJ.block( *camIter, i ).transpose() * deltaCam.segment<camParamDim>( *camIter * camParamDim );
                ++camIter;
            }

            tmp = _invAugPJTJ[ i ] * res;
            deltaStruct.segment<pointParamDim>( pointParamDim * i ) = tmp;

            // apply the delta:
            f.estimate().head<pointParamDim>() += tmp;

            // evaluate the current costs again:
            camIter = f.pointTrackBegin();
            while( camIter != itEnd ){
                const Keyframe & kf = map.keyframeForId( *camIter );

                Vision::project( pp,
                                 K,
                                 kf.pose().transformation(),
                                 f.estimate() );

                // get the measurement of point i in keyframe *camIter:
                const MapMeasurement & meas = kf.measurementForId( i );
                r = ( meas.point - pp );
                _costs += ( r.transpose() * meas.information * r );
                ++camIter;
            }
        }
        _costs /= _nMeas;
    }

    void SparseBundleAdjustment::undoStep( const Eigen::VectorXd & dCam,
                                           const Eigen::VectorXd & dPoint,
                                           SlamMap & map )
    {
        for( size_t i = 0; i < map.numKeyframes(); i++ ){
            map.keyframeForId( i ).updatePose( dCam.segment<camParamDim>( camParamDim * i ) );
        }

        for( size_t i = 0; i < map.numFeatures(); i++ ){
            map.featureForId( i ).estimate().head<pointParamDim>() += dPoint.segment<pointParamDim>( pointParamDim * i );
        }
    }

    void SparseBundleAdjustment::resize( size_t numCams, size_t numPoints, size_t numMeas )
    {
        _nMeas = numMeas;

        if( _nPts != numPoints ){
            if( _pointsJTJ )
                delete[] _pointsJTJ;
            _pointsJTJ = new PointJTJ[ numPoints ];
            if( _invAugPJTJ )
                delete[] _invAugPJTJ;
            _invAugPJTJ = new PointJTJ[ numPoints ];
            if( _pointResiduals )
                delete[] _pointResiduals;
            _pointResiduals = new PointResidualType[ numPoints ];

            _camPointJTJ.resize( numCams, numPoints );

            _nPts = numPoints;
        }

        if( _nCams != numCams ){
            if( _camsJTJ )
                delete[] _camsJTJ;
            _camsJTJ = new CamJTJ[ numCams ];
            if( _camResiduals )
                delete[] _camResiduals;
            _camResiduals = new CamResidualType[ numCams ];

            _jointMeasures.resize( numCams );

            _sparseReduced.resize( camParamDim * numCams, camParamDim * numCams );
            _reducedRHS.resize( camParamDim * numCams );

            if( _camPointJTJ.numBlockRows() != numCams )
                _camPointJTJ.resize( numCams, numPoints );

            _nCams = numCams;
        }
    }


    void SparseBundleAdjustment::evalScreenJacWrtPoint( Eigen::Matrix<double, 2, 1> & repr,
                                                        PointScreenJacType & jac,
                                                        const Eigen::Matrix<double, 3, 1> & pInCam,
                                                        const Eigen::Matrix<double, 3, 3> & K,
                                                        const Eigen::Matrix<double, 3, 3> & R ) const
    {
        Eigen::Matrix<double, 3, 1> pCam = K * pInCam;
        double invZ = 1.0 / pCam[ 2 ];
        double invZZ = 1.0 / Math::sqr( pCam[ 2 ] );

        repr[ 0 ] = pCam[ 0 ] * invZ;
        repr[ 1 ] = pCam[ 1 ] * invZ;

        jac( 0, 0 ) = invZ;
        jac( 0, 1 ) = 0.0;
        jac( 0, 2 ) =-invZZ * pCam[ 0 ];
        jac( 1, 0 ) = 0.0;
        jac( 1, 1 ) = invZ;
        jac( 1, 2 ) =-invZZ * pCam[ 1 ];
        jac *= K * R;
    }

}
