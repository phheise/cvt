#ifndef CVT_ERRORLOGGER_H
#define CVT_ERRORLOGGER_H

#include <cvt/math/Matrix.h>
#include <cvt/math/Matrix.h>

namespace cvt {
    class ErrorLogger {
        public:
            ErrorLogger() :
                _maxOctave( 0 ),
                _maxIteration( 0 )
            {
            }

            ~ErrorLogger()
            {
                if( _data.size() ) {
                    saveResult();
                }
            }

            void setGTPose( const Matrix4f& mat )
            {
                _gt = mat.inverse();
            }

            void log( size_t octave, size_t iteration, const Matrix4f& cur )
            {
                Matrix4f rel = _gt * cur;

                float tError = Vector3f( rel.col( 3 ) ).length();
                float rError = rotError( rel );
                //std::cout << octave << " - " << iteration << " - " << rError << " - " << tError << std::endl;
                _data.push_back( LogData( octave, iteration, rError, tError ) );
                _maxOctave = Math::max( _maxOctave, octave );
                _maxIteration = Math::max( _maxIteration, iteration );
            }

        private:
            struct LogData {
                    LogData( size_t o, size_t i, float rotE, float tE ):
                        octave( o ), iteration( i ), rError( rotE ), tError( tE )
                    {}

                    size_t  octave;
                    size_t  iteration;
                    float   rError;
                    float   tError;
            };

            struct ScaleResult {
                    ScaleResult( size_t maxIters )
                    {
                        rotErrorInIteration.resize( maxIters, 0.0f );
                        tErrorInIteration.resize( maxIters, 0.0f );
                        samplesForIteration.resize( maxIters, 0 );
                    }

                    void add( size_t iter, float rError, float tError )
                    {
                        rotErrorInIteration[ iter ] += rError;
                        tErrorInIteration[ iter ] += tError;
                        samplesForIteration[ iter ]++;
                    }

                    float avgRotErrorForIteration( size_t i ) const
                    {
                        size_t n = samplesForIteration[ i ];
                        if( n == 0 )
                            return 0.0f;
                        return rotErrorInIteration[ i ] / n;
                    }

                    float avgtErrorForIteration( size_t i ) const
                    {
                        size_t n = samplesForIteration[ i ];
                        if( n == 0 )
                            return 0.0f;
                        return tErrorInIteration[ i ] / n;
                    }

                    std::vector<float>  rotErrorInIteration;
                    std::vector<float>  tErrorInIteration;
                    std::vector<size_t> samplesForIteration;
            };

            std::vector<LogData>    _data;
            size_t                  _maxOctave;
            size_t                  _maxIteration;
            Matrix4f                _gt;

            float rotError( const Matrix4f& m )
            {
                return Math::acos( Math::clamp<float>( 0.5f * ( m.toMatrix3().trace() - 1.0f ), -1.0f, 1.0f ) );
            }

            void saveResult()
            {
                std::ofstream file;
                file.open( "conv_speed.txt" );
                file.precision( 15 );
                file << "# <octave> <error iter0> <...> <error itern>" << std::endl;

                std::vector<ScaleResult> resultForOctave( _maxOctave + 1, ScaleResult( _maxIteration + 1 ) );
                for( size_t i = 0; i < _data.size(); i++ ){
                    const LogData& d = _data[ i ];
                    resultForOctave[ d.octave ].add( d.iteration, d.rError, d.tError );
                }

                for( size_t k = 0; k < _maxIteration + 1; k++ ){
                    file << k << " ";
                    for( size_t i = 0; i < resultForOctave.size(); i++ ){
                        file << std::fixed << resultForOctave[ i ].avgRotErrorForIteration( k ) << " ";
                        file << std::fixed << resultForOctave[ i ].avgtErrorForIteration( k ) << " ";
                    }
                    file << "\n";
                }

                file.close();
            }
    };
}

#endif // ERRORLOGGER_H
