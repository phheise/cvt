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


#ifndef CVT_COSTFUNCTION_H
#define CVT_COSTFUNCTION_H

#include <vector>
#include <Eigen/Core>

namespace cvt {

    template <class Derived>
    struct CostFuncTrait;
//    {
//        typedef ...   ParameterType; -> parameter (vector) representation
//        typedef ...   JacobianType; -> typedef for Jacobian
//        typedef std::vector<JacobianType,
//                            Eigen::aligned_allocator<JacobianType> > JacobianVectorType;
//        typedef ...   HessianType; -> typedef for Hessian (J^TJ)
//        typedef ... ResidualType;
//        typedef std::vector<ResidualType>       ResidualVectorType;

//        typedef ... ModelType; // internal model representation (e.g. Quaternions, SE3, ...)
//    };

    template<class Derived>
    class CostFunction
    {
        public:
            typedef CostFuncTrait<Derived>                  CFTrait;
            typedef typename CFTrait::DataType              DataType;
            typedef typename CFTrait::ParameterType         ParameterType;
            typedef typename CFTrait::JacobianType          JacobianType;
            typedef typename CFTrait::HessianType           HessianType;
            typedef typename CFTrait::JacobianVectorType    JacobianVectorType;
            typedef typename CFTrait::ResidualType          ResidualType;
            typedef typename CFTrait::ResidualVectorType    ResidualVectorType;
            typedef typename CFTrait::ModelType             ModelType;

            virtual ~CostFunction(){}

            /* for multiscale cost functions */
            virtual size_t              scales() const { return 1; }

            virtual void                evaluate( ResidualVectorType& residuals, JacobianVectorType& jacobians, size_t scale ) = 0;
            virtual void                update( const ParameterType& deltaP ) = 0;

            virtual void                setModel( const ModelType& model ) = 0;
            virtual const ModelType&    model() const = 0;

    };

}

#endif // COSTFUNCTION_H
