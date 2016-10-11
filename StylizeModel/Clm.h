#ifndef CLM_H
#define CLM_H
#include <math.h>
#include <QString>
#include <QJsonArray>
#include <QVector>
#include <QRgb>
#include <QDebug>
#include <QFile>

#include "MosseFilter.h"
#include "Params.h"
#include "SvmFilter.h"
#include "numeric.h"

/*
 * Function name                    test
 * createJacobian()                 OK
 * calculatePositions()
 * gpopt()                          OK
 * gpopt2()                         OK
*/

class Clm
{

public:
    Clm(Params *params);
    /*
     * load model data, initialize filters, etc.
     * @param <Object> pdm model object
     */
    void init(QJsonObject pdmmodel, QJsonObject left_eye_filter, QJsonObject right_eye_filter, QJsonObject nose_filter);

    /*
     * starts the tracker to run on a regular interval
     */
    bool start(QImage element, QVector<double> box);

    /*
     * element : canvas or video element
     */
    QVector<QVector<double>> track(QImage element, QVector<double> box);

    /*
     * reset tracking, so that track() will start a new detection
     */
    void reset();

    /*
     * get coordinates of current model fit
     */
    QVector<QVector<double>> getCurrentPosition();

    void runnerFunction();

    // procrustes analysis (test ok)
    QVector<double> procrustes(QVector<QVector<double>> temp, QVector<QVector<double>> shape);

    void setRootDirectory(QString rootDirectory);

private:
    QString m_appFileDirectory;
    Params *m_params;
    int numPatches, numParameters;
    int patchSize;
    QString patchType;

    QVector<QVector<double>> gaussianPD;
    QVector<QVector<double>> eigenVectors;
    QVector<double> eigenValues;

    double sketchW, sketchH;

    QJsonObject weights;
    QJsonObject model;
    QJsonObject biases;
    QJsonObject left_eye_filter;
    QJsonObject right_eye_filter;
    QJsonObject nose_filter;

    QVector<double> currentParameters ;
    QVector<QVector<double>> currentPositions ;
    QVector<QVector<double>> previousParameters ;
    QVector<QVector<QVector<double>>> previousPositions ;

    QVector<QVector<double>> *patches;
    QVector<QVector<double>> *responses;
    QVector<QVector<double>> *meanShape;

    QVector<int> varianceSeq;

    double relaxation;

    bool first;

    double convergenceLimit;

    QVector<double> learningRate;
    QVector<double> prevCostFunc;

    int searchWindow;
    double modelWidth, modelHeight;

    double halfSearchWindow;
    QVector<double> vecProbs;
    double responsePixels;

    QVector<double> updatePosition;
    QVector<double> vecpos;

    int pw, pl, pdataLength;

    int facecheck_count;

    SvmFilter *svmFi;

    double msxmin, msymin, msxmax, msymax;
    double msmodelwidth, msmodelheight;
    QVector<double> scoringWeights;
    double scoringBias;
    QVector<double> scoringHistory;
    int meanscore;

    MosseFilter *mossef_lefteye, *mossef_righteye, *mossef_nose;
    QVector<double> right_eye_position;
    QVector<double> left_eye_position;
    QVector<double> nose_position;
    QImage runnerElement;
    QImage candidateImage;
    QVector<double> runnerBox;
    QVector<int> pointWeights;

    double halfPI;
    /*
     * Get the average of recent model movements Used for checking whether model
     * fit has converged
     */
    double getConvergence();

    // generates the jacobian matrix used for optimization calculations
    QVector<QVector<double>> createJacobian(QVector<double> parameters, QVector<QVector<double>> eigenVectors);

    QVector<QVector<double>> calculatePositions(QVector<double> parameters, bool useTransforms);

    // part one of meanshift calculation
    double gpopt(int responseWidth, QVector<double> currentPositionsj, QVector<double> &updatePosition, QVector<double> &vecProbs,
                 QVector<QVector<double>> *responses, double opj0, double opj1, int j, int variance, double scaling);

    // part two of meanshift calculation
    void gpopt2(int responseWidth, QVector<double> &vecpos, QVector<double> updatePosition, QVector<double> vecProbs, double vpsum, double opj0, double opj1, double scaling);

    // calculate score of current fit
    bool checkTracking();

    QVector<double> getInitialPosition(QImage element, QVector<double> box);
};

#endif // CLM_H
