#include "Clm.h"

Clm::Clm(Params *params)
{
    if (params == NULL)
        params = new Params();

    params->constantVelocity = true;
    params->searchWindow = 11;
    params->useWebGL = true;
    params->scoreThreshold = 0.5;
    params->stopOnConvergence = false;
    params->weightPoints = NULL;
    params->sharpenResponse = -1.0;
    m_params = params;

    patches = NULL;
    responses = NULL;
    meanShape = NULL;

    varianceSeq = QVector<int> {10, 5, 1};

    relaxation = 0.1;

    first = true;

    convergenceLimit = 0.01;

    updatePosition = QVector<double>(2);
    vecpos = QVector<double>(2);

    facecheck_count = 0;

    right_eye_position = QVector<double> {0.0, 0.0};
    left_eye_position = QVector<double> {0.0, 0.0};
    nose_position = QVector<double> {0.0, 0.0};

    halfPI = M_PI / 2;
    meanscore = 0;
}


/*
 * load model data, initialize filters, etc.
 * @param <Object> pdm model object
 */
void Clm::init(QJsonObject pdmmodel, QJsonObject left_eye_filter, QJsonObject right_eye_filter, QJsonObject nose_filter)
{
    model = pdmmodel;
    this->left_eye_filter = left_eye_filter;
    this->right_eye_filter = right_eye_filter;
    this->nose_filter = nose_filter;

    // ----- load from model ----- //
    patchType = model["patchModel"].toObject()["patchType"].toString();
    numPatches = model["patchModel"].toObject()["numPatches"].toInt();
    patchSize = model["patchModel"].toObject()["patchSize"].toArray()[0].toInt();
    if (patchType == "MOSSE")
    {
        searchWindow = patchSize;
    } else
    {
        searchWindow = m_params->searchWindow;
    }
    numParameters = model["shapeModel"].toObject()["numEvalues"].toInt();
    modelWidth = model["patchModel"].toObject()["canvasSize"].toArray()[0].toInt();
    modelHeight = model["patchModel"].toObject()["canvasSize"].toArray()[1].toInt();

    // ----- set up canvas to work on ----- //

    sketchW = modelWidth + (searchWindow - 1) + patchSize - 1;
    sketchH = modelHeight + (searchWindow - 1) + patchSize - 1;

    if (model["hints"].toObject().size() != 0 && left_eye_filter.size() != 0 && right_eye_filter.size() != 0 && nose_filter.size() != 0)
    {
        mossef_lefteye = new MosseFilter(m_params);
        mossef_lefteye->load(left_eye_filter);
        mossef_righteye = new MosseFilter(m_params);
        mossef_righteye->load(right_eye_filter);
        mossef_nose = new MosseFilter(m_params);
        mossef_nose->load(nose_filter);
    } else
    {
    }

    // ----- load eigenvectors ----- //
    eigenVectors = numeric::rep(numPatches * 2, numParameters, 0);
    for (int i = 0; i < numPatches * 2; i++)
    {
        for (int j = 0; j < numParameters; j++)
        {
            eigenVectors[i][j] = model["shapeModel"].toObject()["eigenVectors"].toArray()[i].toArray()[j].toDouble();
        }
    }

    // ----- load mean shape ----- //
    meanShape = new QVector<QVector<double>>(numPatches);
    for (int i = 0; i < numPatches; i++)
    {
        QVector<double> tmpModel = QVector<double>
        {
            model["shapeModel"].toObject()["meanShape"].toArray()[i].toArray()[0].toDouble(),
            model["shapeModel"].toObject()["meanShape"].toArray()[i].toArray()[1].toDouble()
        };
        (*meanShape)[i] = tmpModel;
    }

    // ----- get max and mins, width and height of meanshape ----- //
    msxmax = msymax = 0;
    msxmin = msymin = 1000000;
    for (int i = 0; i < numPatches; i++)
    {
        if ( (*meanShape)[i][0] < msxmin )
            msxmin = (*meanShape)[i][0];
        if ( (*meanShape)[i][1] < msymin )
            msymin = (*meanShape)[i][1];
        if ( (*meanShape)[i][0] > msxmax )
            msxmax = (*meanShape)[i][0];
        if ( (*meanShape)[i][1] > msymax )
            msymax = (*meanShape)[i][1];
    }
    msmodelwidth = msxmax - msxmin;
    msmodelheight = msymax - msymin;
    // ----- get scoringweights if they exist ----- //
    if (model["scoring"].toObject().size() != 0)
    {
        QJsonArray jsonArray_coef = model["scoring"].toObject()["coef"].toArray();
        scoringWeights = QVector<double>(jsonArray_coef.size());

        for (int i = 0; i < jsonArray_coef.size(); i++)
            scoringWeights[i] = jsonArray_coef[i].toDouble();

        scoringBias = model["scoring"].toObject()["bias"].toDouble();

        // ----- load eigenvalues ----- //
        QJsonArray jsonArray_eigenV = model["shapeModel"].toObject()["eigenValues"].toArray();
        eigenValues = QVector<double>(jsonArray_eigenV.size());

        for (int i = 0; i < jsonArray_eigenV.size(); i++)
            eigenValues[i] = jsonArray_eigenV[i].toDouble();

        weights = model["patchModel"].toObject()["weights"].toObject();
        biases = model["patchModel"].toObject()["bias"].toObject();

        // ----- precalculate gaussianPriorDiagonal ----- //
        gaussianPD = numeric::rep(numParameters + 4, numParameters + 4, 0);
        // ----- set values and append manual inverse ----- //
        QJsonArray jsonArray_nonRegularizedVectors = model["shapeModel"].toObject()["nonRegularizedVectors"].toArray();
        for (int i = 0; i < numParameters; i++)
        {
            for (int j = 0; j < jsonArray_nonRegularizedVectors.size(); j++)
            {
                if (jsonArray_nonRegularizedVectors[j].toInt() == i)
                {
                    gaussianPD[i + 4][i + 4] = 1 / 10000000;
                } else
                {
                    gaussianPD[i + 4][i + 4] = 1 / eigenValues[i];
                }
            }
        }
        for (int i = 0; i < numParameters + 4; i++)
        {
            currentParameters.push_back(0.0);
        }

    }

    if (patchType == "SVM")
    {
        // ----- use fft convolution if no webGL is available ----- //
        svmFi = new SvmFilter();
        svmFi->init(weights["raw"].toArray(), biases["raw"].toArray(), numPatches, patchSize, searchWindow);

        if (svmFi == NULL)
        {
        }
    }

    if (patchType == "SVM")
    {
        pw = pl = patchSize + searchWindow - 1;
    } else
    {
        pw = pl = searchWindow;
    }

    pdataLength = pw * pl;
    halfSearchWindow = (searchWindow - 1) / 2;
    responsePixels = searchWindow * searchWindow;
    vecProbs = QVector<double>((int) responsePixels);
    patches = new QVector<QVector<double>>(numPatches);
    for (int i = 0; i < numPatches; i++)
    {
        (*patches)[i] = QVector<double>(pdataLength);
    }

    learningRate = QVector<double>(numPatches);
    prevCostFunc = QVector<double>(numPatches);
    for (int i = 0; i < numPatches; i++)
    {
        learningRate[i] = 1.0;
        prevCostFunc[i] = 0.0;
    }
}

/*
 * starts the tracker to run on a regular interval
 */
bool Clm::start(QImage element, QVector<double> box)
{
    // ----- check if model is initalized, else return false ----- //
    if (model.size() == 0)
    {
        return false;
    }
    if (runnerElement.isNull())
    {
        runnerElement = element;        
    }
    runnerBox = box;
    // ----- start named timeout function ----- //
    runnerFunction();
    return true;
}


/*
 * element : canvas or video element
 */
QVector<QVector<double>> Clm::track(QImage element, QVector<double> box)
{
    double scaling, translateX, translateY, rotation;
    double px, py;
    if (first)
    {
        double scaleSize = 500.0 / element.height();
        QTransform transform;
        transform.scale(scaleSize, scaleSize);
        element = element.transformed(transform);
        // ----- do viola-jones on canvas to get initial guess, if we don't have any points ----- //
        QVector<double> gi = getInitialPosition(element, box);
        if (gi.size() == 0)
        {
            return QVector<QVector<double>>(0);
        }
        scaling = gi[0];
        rotation = gi[1];
        translateX = gi[2];
        translateY = gi[3];
        first = false;
    }
    else
    {
        facecheck_count += 1;
        if (m_params->constantVelocity)
        {
            // calculate where to get patches via constant velocity
            // prediction
            if (previousParameters.size() >= 2)
            {
                for (int i = 0; i < currentParameters.size(); i++)
                {
                    currentParameters[i] = (relaxation) * previousParameters[1][i] + (1 - relaxation) * ((2 * previousParameters[1][i]) - previousParameters[0][i]);
                }
            }
        }
        // ----- change translation, rotation and scale parameters ----- //
        rotation = halfPI - atan((currentParameters[0] + 1) / currentParameters[1]);
        if (rotation > halfPI)
        {
            rotation -= M_PI;
        }
        scaling = currentParameters[1] / sin(rotation);
        translateX = currentParameters[2];
        translateY = currentParameters[3];
    }

    //copy the image to candidateImage
    candidateImage = element.copy(0, 0, element.width(), element.height());

    // scale image
    QTransform transform;
    transform.scale(1.0f / (float) scaling, 1.0f / (float) scaling);
    candidateImage = candidateImage.transformed(transform);
    double degree = atan2(candidateImage.height(), (-candidateImage.width())) + rotation;
    double length = sqrt(pow((candidateImage.height() / 2), 2) + pow((candidateImage.width() / 2), 2));

    // rotate image
    transform.reset();
    transform.rotate(-(float) (rotation / M_PI * 180));
    candidateImage = candidateImage.transformed(transform);

    float topX = (float) (length * cos(degree) + candidateImage.width() / 2);
    float topY = (float) (-length * sin(degree) + candidateImage.height() / 2);
    topX = (topX < 0) ? 0 : topX;
    topY = (topY < 0) ? 0 : topY;
    float transX = (float) (fabs(-translateX * cos(rotation) + translateY * cos(M_PI / 2 + rotation)) * (1 / scaling) + topX);
    float transY = (float) (fabs(-translateX * sin(rotation) + translateY * sin(M_PI / 2 + rotation)) * (1 / scaling) + topY);

    QRect rect((int) (transX), (int) (transY), candidateImage.width() - (int) (transX), candidateImage.height() - (int) (transY));
    candidateImage = candidateImage.copy(rect);

    // ----- get cropped images around new points based on model parameters (not scaled and translated) ----- //
    QVector<QVector<double>> patchPositions = calculatePositions(currentParameters, false);

    // ----- check whether tracking is ok ----- //
    if (scoringWeights.size() != 0 && (facecheck_count % 10 == 0))
    {
        if (!checkTracking() || facecheck_count >= 10)
        {
            // ----- reset all parameters ----- //
            facecheck_count = 0;
            first = true;
            scoringHistory.clear();
            for (int i = 0; i < currentParameters.size(); i++)
            {
                currentParameters[i] = 0.0;
                previousParameters.clear();
            }
            return QVector<QVector<double>> (0);
        }
    }
    QVector<double> *pmatrix;
    for (int i = 0; i < numPatches; i++)
    {
        px = patchPositions[i][0] - (pw / 2.0);
        py = patchPositions[i][1] - (pl / 2.0);

        // convert to grayscale
        pmatrix = &(*patches)[i];

        int ipx = (int) round(px);
        int ipy = (int) round(py);

        QVector<int> pdata = QVector<int>(pw * pl * 4);

        for (int j = 0; j < pl; j++)
        {
            for (int k = 0; k < pw; k++)
            {
                if ((ipx + k) >= candidateImage.width() || (ipy + j) >= candidateImage.height())
                {
                    pdata[k * 4 + j * pw * 4] = 0;
                    pdata[k * 4 + j * pw * 4 + 1] = 0;
                    pdata[k * 4 + j * pw * 4 + 2] = 0;
                    pdata[k * 4 + j * pw * 4 + 3] = 0;
                }
                else
                {
                    QColor color = QColor(candidateImage.pixel(ipx + k, ipy + j));
                    pdata[k * 4 + j * pw * 4] = color.red();
                    pdata[k * 4 + j * pw * 4 + 1] = color.green();
                    pdata[k * 4 + j * pw * 4 + 2] = color.blue();
                    pdata[k * 4 + j * pw * 4 + 3] = color.alpha();
                }
            }
        }

        for (int l = 0; l < (*pmatrix).size(); l++)
        {
            (*pmatrix)[l] = pdata[(4 * l)] * 0.3;
            (*pmatrix)[l] += pdata[(4 * l) + 1] * 0.59;
            (*pmatrix)[l] += pdata[(4 * l) + 2] * 0.11;
        }
    }

    if (patchType == "SVM" )
    {
        if (svmFi != NULL)
        {
            responses = svmFi->getResponses(patches);
        } else
        {
            qDebug() << "Clm: SVM-filters do not seem to be initiated properly.";
        }
    }

    // option to increase sharpness of responses
    if (m_params->sharpenResponse > 0)
    {
        for (int i = 0; i < numPatches; i++)
        {
            for (int j = 0; j < responses[i].size(); j++)
            {
                (*responses)[i][j] = pow((*responses)[i][j], m_params->sharpenResponse);
            }
        }
    }

    // iterate until convergence or max 10, 20 iterations?:
    QVector<QVector<double>> originalPositions = currentPositions;
    QVector<QVector<double>> jac;
    QVector<QVector<double>> meanshiftVectors (numPatches);

    for (int aVarianceSeq : varianceSeq)
    {
        // calculate jacobian
        jac = createJacobian(currentParameters, eigenVectors);
        double opj0, opj1;
        for (int j = 0; j < numPatches; j++)
        {
            opj0 = originalPositions[j][0] - ((searchWindow - 1) * scaling / 2);
            opj1 = originalPositions[j][1] - ((searchWindow - 1) * scaling / 2);
            // calculate PI x gaussians
            double vpsum = gpopt(searchWindow, currentPositions[j], updatePosition, vecProbs, responses, opj0, opj1, j, aVarianceSeq, scaling);
            // calculate meanshift-vector
            gpopt2(searchWindow, vecpos, updatePosition, vecProbs, vpsum, opj0, opj1, scaling);
            meanshiftVectors[j] = QVector<double>
            {
                vecpos[0] - currentPositions[j][0], vecpos[1] - currentPositions[j][1]
            };
        }

        QVector<QVector<double>> meanShiftVector = numeric::rep(numPatches * 2, 1, 0);
        for (int k = 0; k < numPatches; k++)
        {
            meanShiftVector[k * 2][0] = meanshiftVectors[k][0];
            meanShiftVector[(k * 2) + 1][0] = meanshiftVectors[k][1];
        }
        // compute pdm parameter update
        QVector<QVector<double>> prior = numeric::mul(gaussianPD, aVarianceSeq);
        QVector<QVector<double>> jtj;
        QVector<QVector<double>> jtv;

        jtj = numeric::dot(numeric::transpose(jac), jac);

        QVector<QVector<double>> cpMatrix = numeric::rep(numParameters + 4, 1, 0);
        for (int l = 0; l < (numParameters + 4); l++)
        {
            cpMatrix[l][0] = currentParameters[l];
        }
        QVector<QVector<double>> priorP = numeric::dot(prior, cpMatrix);

        jtv = numeric::dot(numeric::transpose(jac), meanShiftVector);

        QVector<QVector<double>> paramUpdateLeft = numeric::add(prior, jtj);
        QVector<QVector<double>> paramUpdateRight = numeric::sub(priorP, jtv);
        QVector<QVector<double>> paramUpdate = numeric::dot(numeric::inv(paramUpdateLeft), paramUpdateRight);

        QVector<QVector<double>> oldPositions = currentPositions;
        // update estimated parameters
        for (int k = 0; k < numParameters + 4; k++)
        {
            currentParameters[k] = currentParameters[k] - paramUpdate[k][0];
        }

        // clipping of parameters if they're too high
        double clip;
        for (int k = 0; k < numParameters; k++)
        {
            clip = fabs(3 * sqrt(eigenValues[k]));
            if (fabs(currentParameters[k + 4]) > clip)
            {
                if (currentParameters[k + 4] > 0)
                {
                    currentParameters[k + 4] = clip;
                } else
                {
                    currentParameters[k + 4] = -clip;
                }
            }
        }

        // update current coordinates
        currentPositions = calculatePositions(currentParameters, true);

        // check if converged
        // calculate norm of parameterdifference
        double positionNorm = 0.0;
        double pnsq_x, pnsq_y;
        for (int k = 0; k < currentPositions.size(); k++)
        {
            pnsq_x = (currentPositions[k][0] - oldPositions[k][0]);
            pnsq_y = (currentPositions[k][1] - oldPositions[k][1]);
            positionNorm += ((pnsq_x * pnsq_x) + (pnsq_y * pnsq_y));
        }

        // if norm < limit, then break
        if (positionNorm < convergenceLimit)
        {
            break;
        }

    }

    if (m_params->constantVelocity)
    {
        // add current parameter to array of previous parameters
        previousParameters.push_back(currentParameters);
        if (previousParameters.size() == 3)
            previousParameters.remove(0);
    }

    // store positions, for checking convergence
    if (previousPositions.size() == 10)
        previousPositions.remove(0);
    previousPositions.push_back(currentPositions);

    if (this->getConvergence() < 0.5)
    {
        // we must get a score before we can say we've converged
        if (scoringHistory.size() >= 5)
        {
            if (m_params->stopOnConvergence)
            {
                //this.stop();
            }
        }
    }
    // return new points
    return currentPositions;
}

/*
 * reset tracking, so that track() will start a new detection
 */
void Clm::reset()
{
    first = true;
    scoringHistory.clear();
    for (int i = 0; i < currentParameters.size(); i++)
    {
        currentParameters[i] = 0.0;
        previousParameters.clear();
    }
    runnerElement = QImage();
    runnerBox.clear();
}

/*
 * get coordinates of current model fit
 */
QVector<QVector<double>> Clm::getCurrentPosition()
{
    return currentPositions;
}

/*
 * Get the average of recent model movements Used for checking whether model
 * fit has converged
 */
double Clm::getConvergence()
{
    if (previousPositions.size() < 10)
        return 999999;

    double prevX = 0.0;
    double prevY = 0.0;
    double currX = 0.0;
    double currY = 0.0;

    // average 5 previous positions
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < numPatches; j++)
        {
            prevX += previousPositions[i][j][0];
            prevY += previousPositions[i][j][1];
        }
    }
    prevX /= 5;
    prevY /= 5;

    // average 5 positions before that
    for (int i = 5; i < 10; i++)
    {
        for (int j = 0; j < numPatches; j++)
        {
            currX += previousPositions[i][j][0];
            currY += previousPositions[i][j][1];
        }
    }
    currX /= 5;
    currY /= 5;

    // calculate difference
    double diffX = currX - prevX;
    double diffY = currY - prevY;
    double msavg = ((diffX * diffX) + (diffY * diffY));
    msavg /= previousPositions.size();
    return msavg;
}

void Clm::runnerFunction()
{
    // schedule as many iterations as we can during each request
    QVector<QVector<double>> positionVector;
    positionVector = track(runnerElement, runnerBox);
    int icountTimes = 0;
    while (positionVector.size() != 0)
    {
        positionVector = track(runnerElement, runnerBox);
        icountTimes++;
    }
    positionVector = track(runnerElement, runnerBox);
    icountTimes++;
}


// generates the jacobian matrix used for optimization calculations
QVector<QVector<double>> Clm::createJacobian(QVector<double> parameters, QVector<QVector<double>> eigenVectors)
{
    QVector<QVector<double>> jacobian = numeric::rep(2 * numPatches, numParameters + 4, 0);
    double j0, j1;
    for (int i = 0; i < numPatches; i++)
    {
        // 1
        j0 = (*meanShape)[i][0];
        j1 = (*meanShape)[i][1];
        for (int p = 0; p < numParameters; p++)
        {
            j0 += parameters[p + 4] * eigenVectors[i * 2][p];
            j1 += parameters[p + 4] * eigenVectors[(i * 2) + 1][p];
        }
        jacobian[i * 2][0] = j0;
        jacobian[(i * 2) + 1][0] = j1;
        // 2
        j0 = (*meanShape)[i][1];
        j1 = (*meanShape)[i][0];
        for (int p = 0; p < numParameters; p++)
        {
            j0 += parameters[p + 4] * eigenVectors[(i * 2) + 1][p];
            j1 += parameters[p + 4] * eigenVectors[i * 2][p];
        }
        jacobian[i * 2][1] = -j0;
        jacobian[(i * 2) + 1][1] = j1;
        // 3
        jacobian[i * 2][2] = 1;
        jacobian[(i * 2) + 1][2] = 0;
        // 4
        jacobian[i * 2][3] = 0;
        jacobian[(i * 2) + 1][3] = 1;
        // the rest
        for (int j = 0; j < numParameters; j++)
        {
            j0 = parameters[0] * eigenVectors[i * 2][j] - parameters[1] * eigenVectors[(i * 2) + 1][j] + eigenVectors[i * 2][j];
            j1 = parameters[0] * eigenVectors[(i * 2) + 1][j] + parameters[1] * eigenVectors[i * 2][j] + eigenVectors[(i * 2) + 1][j];
            jacobian[i * 2][j + 4] = j0;
            jacobian[(i * 2) + 1][j + 4] = j1;
        }
    }

    return jacobian;
}

QVector<QVector<double>> Clm::calculatePositions(QVector<double> parameters, bool useTransforms)
{
    double x, y, a, b;
    int numParameters = parameters.size();
    QVector<QVector<double>> positions;
    for (int i = 0; i < numPatches; i++)
    {
        x = (*meanShape)[i][0];
        y = (*meanShape)[i][1];
        for (int j = 0; j < numParameters - 4; j++)
        {
            x += model["shapeModel"].toObject()["eigenVectors"].toArray()[i * 2].toArray()[j].toDouble() * parameters[j + 4];
            y += model["shapeModel"].toObject()["eigenVectors"].toArray()[(i * 2) + 1].toArray()[j].toDouble() * parameters[j + 4];
        }
        if (useTransforms)
        {
            a = parameters[0] * x - parameters[1] * y + parameters[2];
            b = parameters[0] * y + parameters[1] * x + parameters[3];
            x += a;
            y += b;
        }
        QVector<double> temp;
        temp.push_back(x);
        temp.push_back(y);
        positions.push_back(temp);
    }
    return positions;
}

// part one of meanshift calculation
double Clm::gpopt(int responseWidth, QVector<double> currentPositionsj, QVector<double> &updatePosition, QVector<double> &vecProbs,
             QVector<QVector<double>> *responses, double opj0, double opj1, int j, int variance, double scaling)
{
    int pos_idx = 0;
    double vpsum = 0.0;
    double dx, dy;
    for (int k = 0; k < responseWidth; k++)
    {
        updatePosition[1] = opj1 + (k * scaling);
        for (int l = 0; l < responseWidth; l++)
        {
            updatePosition[0] = opj0 + (l * scaling);

            dx = currentPositionsj[0] - updatePosition[0];
            dy = currentPositionsj[1] - updatePosition[1];
            vecProbs[pos_idx] = (*responses)[j][pos_idx] * exp(-0.5 * ((dx * dx) + (dy * dy)) / (variance * scaling));

            vpsum += vecProbs[pos_idx];
            pos_idx++;
        }
    }

    return vpsum;
}

// part two of meanshift calculation
void Clm::gpopt2(int responseWidth, QVector<double> &vecpos, QVector<double> updatePosition, QVector<double> vecProbs, double vpsum, double opj0, double opj1, double scaling)
{
    int pos_idx = 0;
    double vecsum;
    vecpos[0] = 0.0;
    vecpos[1] = 0.0;
    for (int k = 0; k < responseWidth; k++)
    {
        updatePosition[1] = opj1 + (k * scaling);
        for (int l = 0; l < responseWidth; l++)
        {
            updatePosition[0] = opj0 + (l * scaling);
            vecsum = vecProbs[pos_idx] / vpsum;
            vecpos[0] += vecsum * updatePosition[0];
            vecpos[1] += vecsum * updatePosition[1];
            pos_idx++;
        }
    }
}

// calculate score of current fit
bool Clm::checkTracking()
{
    int x = (int) round(msxmin + (msmodelwidth / 4.5));
    int y = (int) round(msymin - (msmodelheight / 12.0));
    int width = (int) round(msmodelwidth - (msmodelwidth * 2 / 4.5));
    int height = (int) round(msmodelheight - (msmodelheight / 12));
    QImage temp(width, height, QImage::Format_RGBA8888);

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            QRgb value;
            value = qRgb(0, 0, 0);
            if ((i + x) >= candidateImage.width() || (j + y) >= candidateImage.height())
                temp.setPixel(i, j, value);
            temp.setPixel(i, j, candidateImage.pixel(i + x, j + y));
        }
    }

    float scaleWidth = 20.0f / width;
    float scaleHeight = 22.0f / height;

    QTransform transform;
    transform.scale(scaleWidth, scaleHeight);
    temp = temp.transformed(transform);

    QVector<int> scdata = QVector<int>(20 * 22 * 4);
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 22; j++)
        {
            QColor color = QColor(temp.pixel(i, j));
            scdata[i * 4 + j * 20 * 4] = color.red();
            scdata[i * 4 + j * 20 * 4 + 1] = color.green();
            scdata[i * 4 + j * 20 * 4 + 2] = color.blue();
            scdata[i * 4 + j * 20 * 4 + 3] = color.alpha();
        }
    }
    // convert data to grayscale
    QVector<double> scoringData = QVector<double>(20 * 22);
    double scmax = 0.0;
    for (int i = 0; i < 20 * 22; i++)
    {
        scoringData[i] = scdata[i * 4] * 0.3 + scdata[(i * 4) + 1] * 0.59 + scdata[(i * 4) + 2] * 0.11;
        scoringData[i] = log(scoringData[i] + 1);
        if (scoringData[i] > scmax)
            scmax = scoringData[i];
    }
    if (scmax > 0)
    {
        // normalize & multiply by svmFilter
        double mean = 0;
        for (int i = 0; i < 20 * 22; i++)
        {
            mean += scoringData[i];
        }
        mean /= (20 * 22);
        double sd = 0;
        for (int i = 0; i < 20 * 22; i++)
        {
            sd += (scoringData[i] - mean) * (scoringData[i] - mean);
        }
        sd /= (20 * 22 - 1);
        sd = sqrt(sd);
        double score = 0;
        for (int i = 0; i < 20 * 22; i++)
        {
            scoringData[i] = (scoringData[i] - mean) / sd;
            score += (scoringData[i]) * scoringWeights[i];
        }
        score += scoringBias;
        score = 1 / (1 + exp(-score));
        if (scoringHistory.size() == 5)
            scoringHistory.remove(0);
        scoringHistory.push_back(score);

        if (scoringHistory.size() > 4)
        {
            // get average
            meanscore = 0;
            for (int i = 0; i < 5; i++)
            {
                meanscore += scoringHistory[i];
            }
            meanscore /= 5;

            // if below threshold, then reset (return false)
            if (meanscore < m_params->scoreThreshold)
                return false;
        }
    }
    return true;
}

QVector<double> Clm::getInitialPosition(QImage element, QVector<double> box)
{
    double translateX = 0, translateY = 0, scaling = 0, rotation = 0;

    if (left_eye_filter.size() != 0 && right_eye_filter.size() != 0 && nose_filter.size() != 0)
    {
        if (model["hints"].toObject().size() != 0)
        {
            double noseFilterWidth = box[2] * 4.5 / 10.0;
            double eyeFilterWidth = box[2] * 6.0 / 10.0;

            QVector<double> nose_result = mossef_nose->track(element, round(box[0] + (box[2] / 2.0) - (noseFilterWidth / 2.0)),
                    round(box[1] + box[3] * (5.0 / 8.0) - (noseFilterWidth / 2.0)), noseFilterWidth, noseFilterWidth, false, false, false);
            QVector<double> right_result = mossef_righteye->track(element, round(box[0] + (box[2] * 3.0 / 4.0) - (eyeFilterWidth / 2.0)),
                    round(box[1] + box[3] * (2.0 / 5.0) - (eyeFilterWidth / 2.0)), eyeFilterWidth, eyeFilterWidth, false, false, false);
            QVector<double> left_result = mossef_lefteye->track(element, round(box[0] + (box[2] / 4.0) - (eyeFilterWidth / 2.0)),
                    round(box[1] + box[3] * (2.0 / 5.0) - (eyeFilterWidth / 2.0)), eyeFilterWidth, eyeFilterWidth, false, false, false);

            right_eye_position[0] = round(box[0] + (box[2] * 3.0 / 4.0) - (eyeFilterWidth / 2.0)) + right_result[0];
            right_eye_position[1] = round(box[1] + box[3] * (2.0 / 5.0) - (eyeFilterWidth / 2.0)) + right_result[1];
            left_eye_position[0] = round(box[0] + (box[2] / 4.0) - (eyeFilterWidth / 2.0)) + left_result[0];
            left_eye_position[1] = round(box[1] + box[3] * (2.0 / 5.0) - (eyeFilterWidth / 2.0)) + left_result[1];
            nose_position[0] = round(box[0] + (box[2] / 2.0) - (noseFilterWidth / 2.0)) + nose_result[0];
            nose_position[1] = round(box[1] + box[3] * (5.0 / 8.0) - (noseFilterWidth / 2.0)) + nose_result[1];

            // get eye and nose positions of model
            QJsonArray lepJsonArray = model["hints"].toObject()["leftEye"].toArray();
            QJsonArray repjJsonArray = model["hints"].toObject()["rightEye"].toArray();
            QJsonArray mepjJsonArray = model["hints"].toObject()["nose"].toArray();

            QVector<double> lep = QVector<double>(lepJsonArray.size());
            QVector<double> rep = QVector<double>(repjJsonArray.size());
            QVector<double> mep = QVector<double>(mepjJsonArray.size());

            for (int i = 0; i < lep.size(); i++)
                lep[i] = lepJsonArray[i].toDouble();
            for (int i = 0; i < lep.size(); i++)
                rep[i] = repjJsonArray[i].toDouble();
            for (int i = 0; i < lep.size(); i++)
                mep[i] = mepjJsonArray[i].toDouble();

            // get scaling, rotation, etc. via procrustes analysis
            QVector<double> procrustes_params = procrustes(
                        QVector<QVector<double>> {left_eye_position, right_eye_position, nose_position},
                        QVector<QVector<double>> {lep, rep, mep}
                        );

            translateX = procrustes_params[0];
            translateY = procrustes_params[1];
            scaling = procrustes_params[2];
            rotation = procrustes_params[3];

            currentParameters[0] = (scaling * cos(rotation)) - 1.0;
            currentParameters[1] = (scaling * sin(rotation));
            currentParameters[2] = translateX;
            currentParameters[3] = translateY;
        }
    }

    currentPositions = calculatePositions(currentParameters, true);

    return QVector<double>{scaling, rotation, translateX, translateY};
}


// procrustes analysis (test ok)
QVector<double> Clm::procrustes(QVector<QVector<double>> temp, QVector<QVector<double>> shape)
{
    // assume temp and shape is a vector of x,y-coordinates
    // i.e. temp = [[x1,y1], [x2,y2], [x3,y3]];
    QVector<QVector<double>> tempClone = QVector<QVector<double>>(temp.size());
    QVector<QVector<double>> shapeClone = QVector<QVector<double>>(shape.size());
    for (int i = 0; i < temp.size(); i++)
    {
        tempClone[i] = QVector<double>{temp[i][0], temp[i][1]};
    }
    for (int i = 0; i < shape.size(); i++)
    {
        shapeClone[i] = QVector<double>{shape[i][0], shape[i][1]};
    }
    shape = shapeClone;
    temp = tempClone;

    // calculate translation
    QVector<double> tempMean = QVector<double>{0.0, 0.0};
    for (QVector<double> atemp : temp)
    {
        tempMean[0] += atemp[0];
        tempMean[1] += atemp[1];
    }
    tempMean[0] /= temp.size();
    tempMean[1] /= temp.size();

    QVector<double> shapeMean = QVector<double>{0.0, 0.0};
    for (QVector<double> aShape : shape)
    {
        shapeMean[0] += aShape[0];
        shapeMean[1] += aShape[1];
    }
    shapeMean[0] /= shape.size();
    shapeMean[1] /= shape.size();

    double translationX = tempMean[0] - shapeMean[0];
    double translationY = tempMean[1] - shapeMean[1];

    // centralize
    for (int i = 0; i < shape.size(); i++)
    {
        shape[i][0] -= shapeMean[0];
        shape[i][1] -= shapeMean[1];
    }
    for (int i = 0; i < temp.size(); i++)
    {
        temp[i][0] -= tempMean[0];
        temp[i][1] -= tempMean[1];
    }

    // scaling
    double scaleS = 0.0;
    for (QVector<double> aShape : shape)
    {
        scaleS += ((aShape[0]) * (aShape[0]));
        scaleS += ((aShape[1]) * (aShape[1]));
    }
    scaleS = sqrt(scaleS / shape.size());

    double scaleT = 0.0;
    for (QVector<double> atemp : temp)
    {
        scaleT += ((atemp[0]) * (atemp[0]));
        scaleT += ((atemp[1]) * (atemp[1]));
    }
    scaleT = sqrt(scaleT / temp.size());

    double scaling = scaleT / scaleS;

    for (int i = 0; i < shape.size(); i++)
    {
        shape[i][0] *= scaling;
        shape[i][1] *= scaling;
    }

    // rotation
    double top = 0.0;
    double bottom = 0.0;
    for (int i = 0; i < shape.size(); i++)
    {
        top += (shape[i][0] * temp[i][1] - shape[i][1] * temp[i][0]);
        bottom += (shape[i][0] * temp[i][0] + shape[i][1] * temp[i][1]);
    }
    double rotation = atan(top / bottom);

    translationX += (shapeMean[0] - (scaling * cos(-rotation) * shapeMean[0]) - (scaling * shapeMean[1] * sin(-rotation)));
    translationY += (shapeMean[1] + (scaling * sin(-rotation) * shapeMean[0]) - (scaling * shapeMean[1] * cos(-rotation)));

    // returns rotation, scaling, transformx and transformx
    return QVector<double>{translationX, translationY, scaling, rotation};
}

void Clm::setRootDirectory(QString rootDirectory)
{
    m_appFileDirectory = rootDirectory;
}
