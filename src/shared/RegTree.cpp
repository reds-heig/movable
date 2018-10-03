/**
 * @file MOVABLE definitions for Regression Tree Learning
 *
 * This is a customized version of the SQBlib library made by Carlos Becker
 * http://sites.google.com/site/carlosbecker
 *
 * MOVABLE project: MicrOscopic VisuAlization of BLood cElls for the Detection
 *                  of Malaria and CD4+
 *
 * Alberto Dassatti, Magali Fröhlich, Roberto Rigamonti
 * HES-SO
 * REDS institute, HEIG-VD (Yverdon-les-Bains)
 */

#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>

#include "RegTree.hpp"

RegTree::RegTree(const EMat &featuresF,
                 const EVec &responsesF,
                 const EVec &weightsF,
                 const unsigned int maxDepth,
                 std::vector< unsigned int > &updatedFeatIdxs)
{
    const EMatD features = featuresF.cast< double >();
    const EVecD responses = responsesF.cast< double >();
    const EVecD weights = weightsF.cast< double >();

    assert(features.rows() > 0);
    assert(features.cols() > 0);
    assert(responses.size() == features.rows());
    assert(weights.size() == features.rows());
    assert(maxDepth > 0);

    const unsigned int samplesNo = features.rows();
    const unsigned int featuresNo = features.cols();

    log_trace("Training a new tree (max depth: %d), feature matrix: %dx%d",
              maxDepth, samplesNo, featuresNo);

    nodes.clear();
    nodes.reserve(AVG_TREE_SIZE);

    unsigned int curNodeIdx = 0;
    std::vector < struct NodeStackEntry > stack;
    {
        NodeStackEntry tmp;
        /* Push all samples in the initial sample set */
        tmp.idxs.resize(samplesNo);
        std::iota(tmp.idxs.begin(), tmp.idxs.end(), 0);
        tmp.nodeIdx = curNodeIdx++;
        tmp.isLeaf = false;
        tmp.treeLevel = 0;
        stack.push_back(tmp);
    }

    std::vector < struct StumpNode >::const_iterator bestStump;

    while (!stack.empty()) {
        log_trace("Stack not empty (%d), performing another iteration",
                  (int)stack.size());

        NodeStackEntry top = stack.back();
        stack.pop_back();

        const unsigned int topIdx = top.nodeIdx;

        log_trace("Operating on %d indexes", (int)top.idxs.size());

        /*
         * If needed, resize to prevent a set of continuous
         * reallocations
         */
        if (nodes.size() < topIdx + 1) {
            nodes.resize(2 * topIdx + 1);
        }

        /* If we got to a leaf, store it and skip the rest of the loop */
        if (top.isLeaf) {
            log_trace("Node %d is a leaf, storing it", topIdx);

            nodes[topIdx].isLeaf = true;
            nodes[topIdx].n = top.n;
            continue;
        }

        /* Learn a stump for each possible feature */
        std::vector < struct StumpNode > stumpResults(featuresNo);
#pragma omp parallel for schedule(dynamic)
        for (unsigned int iFeat = 0; iFeat < featuresNo; ++iFeat) {
            trainStump(features, responses, weights,
                       top.idxs, iFeat, stumpResults[iFeat]);
        }

        /* Find the best-performing stump */
        bestStump = std::min_element(stumpResults.begin(),
                                     stumpResults.end(),
                                     stumpCompare);
        const unsigned int bestStumpIdx = bestStump - stumpResults.begin();
        const double bestStumpThreshold = bestStump->threshold;
        log_trace("Best stump found:\n\tfeatIdx %d\n\tisPure %d\n\t"
                  "threshold %f\n\terr %f\n\ty1 %f\n\ty2 %f",
                  bestStumpIdx, bestStump->isPure, bestStumpThreshold,
                  bestStump->err, bestStump->y1, bestStump->y2);

        /*
         * If the best-performing stump is pure, we can store it without
         * further processing
         */
        if (bestStump->isPure) {
            log_trace("The best stump is pure, storing it");

            nodes[topIdx].isLeaf = true;
            nodes[topIdx].n = bestStump->y2;
            continue;
        }

        /*
         * Not at a leaf, split the samples and explore the two
         * descending nodes
         */
        nodes[topIdx].isLeaf = false;
        nodes[topIdx].featIdx = bestStumpIdx;
        nodes[topIdx].n = bestStump->threshold;
        nodes[topIdx].lIdx = curNodeIdx++;
        nodes[topIdx].rIdx = curNodeIdx++;
        /*
         * Create the two corresponding nodes and assign samples
         * to them
         */
        struct NodeStackEntry leftNode;
        struct NodeStackEntry rightNode;

        for (unsigned int iS = 0; iS < top.idxs.size(); ++iS) {
            if (features(top.idxs[iS], bestStumpIdx) < bestStumpThreshold) {
                leftNode.idxs.push_back(top.idxs[iS]);
            } else {
                rightNode.idxs.push_back(top.idxs[iS]);
            }
        }

        log_trace("Samples in LEFT split: %d", (int)leftNode.idxs.size());
        log_trace("Samples in RIGHT split: %d", (int)rightNode.idxs.size());

        leftNode.treeLevel = top.treeLevel + 1;
        leftNode.nodeIdx = nodes[topIdx].lIdx;
        rightNode.treeLevel = top.treeLevel + 1;
        rightNode.nodeIdx = nodes[topIdx].rIdx;

        /*
         * Set the child nodes as leaves if they're too deep or they do
         * not have enough samples
         */
        if (leftNode.treeLevel > maxDepth || leftNode.idxs.size() < 2) {
            leftNode.isLeaf = true;
            leftNode.n = bestStump->y1;
        } else {
            leftNode.isLeaf = false;
        }
        if (rightNode.treeLevel > maxDepth || rightNode.idxs.size() < 2) {
            rightNode.isLeaf = true;
            rightNode.n = bestStump->y2;
        } else {
            rightNode.isLeaf = false;
        }

        /*
         * Push the newly-created nodes in the stack for further
         * processing
         */
        stack.push_back(rightNode);
        stack.push_back(leftNode);
    }

    unsigned int leavesNo = 0;
    for (unsigned int iN = 0; iN < curNodeIdx; ++iN) {
        if (nodes[iN].isLeaf)
            leavesNo++;
    }
    log_trace("Learned %d nodes, %d leaves", curNodeIdx, leavesNo);

    if (nodes.size() != curNodeIdx) {
        nodes.erase(nodes.begin() + curNodeIdx, nodes.end());
    }

    updateFeatureIdxs(updatedFeatIdxs);
}

RegTree::RegTree(std::string &descr_json)
{
    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(descr_json, root)) {
        throw std::runtime_error("invalidJSONDescription");
    }
    Deserialize(root);
}

RegTree::RegTree(Json::Value &root)
{
    Deserialize(root);
}

RegTree::RegTree(const RegTree &other)
{
    this->nodes = other.nodes;
}

bool
operator==(const RegTree &rt1, const RegTree &rt2)
{
    if (rt1.nodes == rt2.nodes) {
        return true;
    }
    return false;
}

bool
operator!=(const RegTree &rt1, const RegTree &rt2)
{
    return !(rt1 == rt2);
}

void
RegTree::predict(const EMat &X_, EVec &results) const
{
    const EMatD X = X_.cast< double >();

    assert(X.rows() > 0);
    assert(X.cols() > 0);
    assert(nodes.size() > 0);

    const unsigned int samplesNo = X.rows();
    results.resize(samplesNo);

#pragma omp parallel for schedule(dynamic)
    for (unsigned int iS = 0; iS < samplesNo; ++iS) {
        unsigned int curNode = 0;
        while (true) {
            if (nodes[curNode].isLeaf) {
                results(iS) = nodes[curNode].n;
                break;
            }

            if (X(iS, nodes[curNode].featIdx) < nodes[curNode].n) {
                curNode = nodes[curNode].lIdx;
            } else {
                curNode = nodes[curNode].rIdx;
            }
        }
    }

}

void
RegTree::Serialize(Json::Value &root)
{
    Json::Value nodes_json(Json::arrayValue);
    for (unsigned int i = 0; i < nodes.size(); ++i) {
        Json::Value node(Json::objectValue);
        nodes[i].Serialize(node);
        nodes_json.append(node);
    }
    root["RT_nodes"] = nodes_json;
}

double
RegTree::computeError(const double y1, const double y2,
                      const double sumWk1, const double sumWk2,
                      const double sumWkRk1, const double sumWkRk2,
                      const double sumWkRkSq1, const double sumWkRkSq2)
{
    return ((y1 * y1 * sumWk1) + sumWkRkSq1 - (2 * y1 * sumWkRk1)) +
        ((y2 * y2 * sumWk2) + sumWkRkSq2 - (2 * y2 * sumWkRk2));
}

void
RegTree::Deserialize(Json::Value &root)
{
    for (Json::Value::iterator it = root["RT_nodes"].begin();
         it != root["RT_nodes"].end();
         ++it) {
        RegTreeNode node;
        node.Deserialize(*it);
        nodes.push_back(node);
    }
}

void
RegTree::getIdxSorted(const EMatD &X,
                      std::vector< unsigned int > &idxs)
{
    assert(X.rows() == 1 || X.cols() == 1);
    assert((unsigned int) X.size() >= idxs.size());
    assert(idxs.size() > 0);

    /* Perform the sorting */
    std::sort(idxs.begin(), idxs.end(), FeatureComparator(X));
}

bool
RegTree::stumpCompare(struct StumpNode &a,
                      struct StumpNode &b)
{
    return a.err < b.err;
}

void
RegTree::trainStump(const EMatD &features,
                    const EVecD &responses,
                    const EVecD &weights,
                    const std::vector < unsigned int > &idxs,
                    const unsigned int featIdx,
                    struct StumpNode &result)
{
    /* X contains the feature we're interested in */
    EVecD X = features.col(featIdx);

    const unsigned int samplesNo = idxs.size();

    std::vector< unsigned int > sortedIdxs = idxs;
    getIdxSorted(X, sortedIdxs);

    double sumWk1 = 0;
    double sumWkRk1 = 0;
    double sumWkRkSq1 = 0;

    double sumWk2 = 0;
    double sumWkRk2 = 0;
    double sumWkRkSq2 = 0;

    for (unsigned int iS = 0; iS < samplesNo; ++iS) {
        const unsigned int sIdx = sortedIdxs[iS];
        const double w = weights(sIdx);
        const double r = responses(sIdx);
        const double rw = r*w;

        sumWk2 += w;
        sumWkRk2 += rw;
        sumWkRkSq2 += rw * r;
    }

    double y1 = 0;
    double y2 = sumWkRk2 / (sumWk2 + 10 * std::numeric_limits< double >::epsilon());
    double err;

    err = computeError(y1, y2, sumWk1, sumWk2, sumWkRk1, sumWkRk2,
                       sumWkRkSq1, sumWkRkSq2);

    /* Check if node is pure */
    bool isPure = false;
    if (fabs(sortedIdxs[samplesNo - 1] - sortedIdxs[0]) <
        10 * std::numeric_limits< double >::epsilon()) {
        isPure = true;
    }

    if (err < samplesNo * std::numeric_limits< double >::epsilon() || isPure) {
        result.isPure = true;
        result.y1 = y2;
        result.y2 = y2;
        result.err = err;
        result.threshold = 0;

        return;
    }

    /* Test the different threshold to see which one fits best */
    double thr = X(sortedIdxs[0]);
    double minErr = err;
    double minThr = thr;
    double prevThr = thr;
    double minY1 = y1;
    double minY2 = y2;
    unsigned int minIdx = 0;

    for (unsigned int iT = 0; iT < samplesNo - 1; ++iT) {
        const double w = weights(sortedIdxs[iT]);
        const double r = responses(sortedIdxs[iT]);
        const double rw = r*w;
        const double r2w = r*rw;

        /* Alter sums */
        sumWk1 += w;
        sumWk2 -= w;

        sumWkRk1 += rw;
        sumWkRk2 -= rw;

        sumWkRkSq1 += r2w;
        sumWkRkSq2 -= r2w;

        thr = X(sortedIdxs[iT + 1]);

        if (prevThr == thr)
            continue;

        y1 = sumWkRk1 / (sumWk1 + 10 * std::numeric_limits< double >::epsilon());
        y2 = sumWkRk2 / (sumWk2 + 10 * std::numeric_limits< double >::epsilon());
        err = computeError(y1, y2, sumWk1, sumWk2, sumWkRk1, sumWkRk2,
                           sumWkRkSq1, sumWkRkSq2);

        if (err < minErr) {
            minErr = err;
            minThr = thr;
            minIdx = iT;
            minY1 = y1;
            minY2 = y2;
        }
    }

    /*
     * Search back for the threshold just below the minimum one, and set the
     * final threshold between these two
     */
    prevThr = minThr;
    for (int iT = (int)minIdx; iT >= 0; --iT) {
        if (X(sortedIdxs[(unsigned int)iT]) != minThr) {
            prevThr = X(sortedIdxs[(unsigned int)iT]);
            break;
        }
    }

    result.y1 = minY1;
    result.y2 = minY2;
    result.err = minErr;
    result.threshold = (minThr + prevThr) / 2;

    log_trace("featIdx: %d, y1: %f y2: %f, err: %f, threshold: %f",
              featIdx, minY1, minY2, minErr, result.threshold);

}

void
RegTree::updateFeatureIdxs(std::vector< unsigned int > &updatedFeatIdxs)
{
    updatedFeatIdxs.clear();
    for (unsigned int n = 0; n < nodes.size(); ++n) {
        updatedFeatIdxs.push_back(nodes[n].featIdx);
    }

    std::sort(updatedFeatIdxs.begin(), updatedFeatIdxs.end());
    auto sf_last = std::unique(updatedFeatIdxs.begin(),
                               updatedFeatIdxs.end());
    updatedFeatIdxs.erase(sf_last, updatedFeatIdxs.end());

    for (unsigned int n = 0; n < nodes.size(); ++n) {
        for (unsigned int f = 0; f < updatedFeatIdxs.size(); ++f) {
            if (nodes[n].featIdx == updatedFeatIdxs[f]) {
                nodes[n].featIdx = f;
                /* Skip remaining features */
                break;
            }
        }
    }
}
