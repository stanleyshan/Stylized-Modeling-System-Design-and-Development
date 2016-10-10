#ifndef PARAMS
#define PARAMS
#include <QVector>
class Params
{
public:
    bool constantVelocity;
    int searchWindow;
    bool useWebGL;
    double scoreThreshold;
    bool stopOnConvergence;
    QVector<int> *weightPoints;
    double sharpenResponse;
    bool drawResponse;
    int psrThreshold;
    double eta;
    bool convertToGrayscale;
};

#endif // PARAMS

