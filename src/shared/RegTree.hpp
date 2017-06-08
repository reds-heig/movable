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
 * REDS institute, HEIG-VD (Yverdon-les-Bains) - 2016
 */

#ifndef REGTREE_HPP_
#define REGTREE_HPP_

#include "json/json.h"

#include "JSONSerializable.hpp"
#include "JSONSerializer.hpp"
#include "DataTypes.hpp"
#include "logging.hpp"

/**
 * class RegTree - Regression Tree for the MOVABLE project
 *
 * @nodes        : nodes of the regression tree
 * @AVG_TREE_SIZE: average tree size --- used to pre-allocate the nodes
 */
class RegTree : public JSONSerializable {
public:
    /**
     * RegTree() - Train a regression tree for the given features
     *
     * @features       : MxN matrix containing the features, each row
     *                   corresponding to the features of a given sample (i.e.,
     *                   M samples, each with N features)
     * @responses      : Mx1 vector of the responses for the corresponding
     *                   samples
     * @weights        : Mx1 vector of the weights for the corresponding samples
     * @maxDepth       : maximum depth of the tree to explore
     * @updatedFeatIdxs: list of features that have been retained by the
     *                   tree algorithm
     */

    RegTree(const EMat &featuresF,
            const EVec &responsesF,
            const EVec &weightsF,
            const unsigned int maxDepth,
            std::vector< unsigned int > &updatedFeatIdxs);

    /**
     * RegTree() - Build a regression tree starting from its JSON
     *             description (de-serialization)
     *
     * @descr_json: string containing the JSON's description of the tree
     */
    RegTree(std::string &descr_json);

    /**
     * RegTree() - Build a regression tree starting from a JSON node
     *
     * @root: JSON's representation root
     */
    RegTree(Json::Value &root);

    /**
     * RegTree() - Copy constructor, useful for testing purposes
     *
     * @other: source regression tree from which to copy
     */
    RegTree(const RegTree &other);

    /**
     * operator==() - Compare two regression trees for equality
     *
     * @rt1: first regression tree in the comparison
     * @rt2: second regression tree in the comparison
     *
     * Return: true if the two trees are identical, false otherwise
     */
    friend bool
    operator==(const RegTree &rt1, const RegTree &rt2);

    /**
     * operator!=() - Compare two regression trees for difference
     *
     * @rt1: first regression tree in the comparison
     * @rt2: second regression tree in the comparison
     *
     * Return: true if the two trees are different, false otherwise
     */
    friend bool
    operator!=(const RegTree &rt1, const RegTree &rt2);

    /**
     * predict() - Given the learnt regression tree, perform prediction on
     *             the sample set passed as parameter
     *
     * @X      : matrix containing the samples (one for each row) upon which
     *           the prediction will be made
     * @results: predicted values
     */
    void predict(const EMat &X, EVec &results) const;

    /**
     * Serialize() - Serialize a regression tree in JSON format
     *
     * @root: root of the JSON's regression tree description
     */
    virtual void Serialize(Json::Value &root);

private:
    /**
     * struct StumpNode - stump details as learned by the stump classifier
     *
     * isPure   : flag that marks the node as pure
     * threshold: threshold used for the splitting
     * err      : stump's error rate
     * y1       : 1st regressed value
     * y2       : 2nd regressed value
     */
    struct StumpNode {
        bool isPure;
        double threshold;
        double err;
        double y1;
        double y2;

        /* Default constructor -- mark the node as not pure */
        StumpNode()
        {
            isPure = false;
        }
    };

    /**
     * struct NodeStackEntry - stack entry in node processing
     *
     * idxs     : indexes of samples considered in this node
     * nodeIdx  : current node's index
     * isLeaf   : mark the node as leaf
     * n        : value held by the node
     * treeLevel: current splitting level
     */
    struct NodeStackEntry {
        std::vector < unsigned int > idxs;
        unsigned int nodeIdx;
        bool isLeaf;
        double n;
        unsigned int treeLevel;

        /*
         * Default constructor -- initialize the stack's node as not a
         * lead and at the root
         */
        NodeStackEntry()
        {
            isLeaf = false;
            treeLevel = 0;
        }
    };

    /**
     * struct FeatureComparator - get a list of indexes sorted according to
     *                            another vector
     *
     * @X: vector containing the values upon which the sorting is made
     */
    struct FeatureComparator {
        const EVecD &X;

        /**
         * FeatureComparator() - Initialize the vector that is the base
         *                       of the sorting
         *
         * @feat: vector containing the values upon which the sorting is
         *        made
         */
        FeatureComparator(const EVecD &feat) : X(feat) { };

        /**
         * operator() - Compare the vector at two different indexes
         *
         * @idx1: index of the first element
         * @idx2: index of the second element
         *
         * Return: 1 if the value of the vector at idx1 is smaller than
         *         the value at idx2, 0 otherwise
         */
        bool operator()(int idx1, int idx2)
        {
            return X(idx1) < X(idx2);
        }
    };

    /**
     * struct RegTreeNode - node in the regression tree
     *
     * @isLeaf: mark a node as leaf
     * @n     : n is going to represent the *value* if isLeaf == true, the
     *          threshold used in the splitting otherwise
     * featIdx: node's feature index
     * rIdx   : index of the node on the right
     * lIdx   : index of the node on the left
     */
    struct RegTreeNode : public JSONSerializable {
        bool isLeaf;
        double n;
        unsigned int featIdx;
        unsigned int rIdx;
        unsigned int lIdx;

        /**
         * Serialize() - Serialize a regression tree's node
         *
         * @root: root of the produced JSON serialization
         */
        virtual void
        Serialize(Json::Value &root)
        {
            root["isLeaf"] = isLeaf;
            root["n"] = n;
            root["featIdx"] = featIdx;
            root["rIdx"] = rIdx;
            root["lIdx"] = lIdx;
        };

        /**
         * Deserialize() - Deserialize a regression tree's node
         *
         * @root: root of the read JSON serialization
         */
        virtual void
        Deserialize(Json::Value &root)
        {
            isLeaf = root.get("isLeaf", false).asBool();
            n = root.get("n", 0.0).asDouble();
            featIdx = root.get("featIdx", 0).asInt();
            rIdx = root.get("rIdx", 0).asInt();
            lIdx = root.get("lIdx", 0).asInt();
        }

        /**
         * operator==() - Compare two regression tree nodes for equality
         *
         * @rt1: first regression tree node in the comparison
         * @rt2: second regression tree node in the comparison
         *
         * Return: true if the two nodes are identical, false otherwise
         */
        friend bool
        operator==(const RegTreeNode &rtn1, const RegTreeNode &rtn2)
        {
            if (rtn1.isLeaf == rtn2.isLeaf &&
                fabs(rtn1.n-rtn2.n) < 1e-12 &&
                rtn1.featIdx == rtn2.featIdx &&
                rtn1.rIdx == rtn2.rIdx &&
                rtn1.lIdx == rtn2.lIdx) {
                return true;
            }
            return false;
        }

        /**
         * operator!=() - Compare two regression tree nodes for
         *                difference
         *
         * @rt1: first regression tree node in the comparison
         * @rt2: second regression tree node in the comparison
         *
         * Return: true if the two nodes are different, false otherwise
         */
        friend bool
        operator!=(const RegTreeNode &rtn1, const RegTreeNode &rtn2)
        {
            return !(rtn1 == rtn2);
        }

    };

    const unsigned int AVG_TREE_SIZE = 100;

    std::vector < struct RegTreeNode > nodes;

    /**
     * computeError() - Given the responses and the weights, compute the
     *                  error
     *
     * @y1        : left node's value
     * @y2        : right node's value
     * @sumWk1    : sum of the weights for the left node
     * @sumWk2    : sum of the weights for the right node
     * @sumWkRk1  : sum of the weighted responses for the left node
     * @sumWkRk2  : sum of the weighted responses for the right node
     * @sumWkRkSq1: sum of the weighted squared responses for the left node
     * @sumWkRkSq2: sum of the weighted squared responses for the right node
     *
     * Return: error value for the considered split
     */
    static double
    computeError(const double y1, const double y2,
                 const double sumWk1, const double sumWk2,
                 const double sumWkRk1, const double sumWkRk2,
                 const double sumWkRkSq1, const double sumWkRkSq2);

    /**
     * Deserialize() - Deserialize a regression tree in JSON format
     *
     * @root: root of the JSON's regression tree description
     */
    virtual void Deserialize(Json::Value &root);

    /**
     * getIdxSorted() - Consider the elements in the indexes list, and
     *                  return their indexes sorted
     *
     * @X   : vector containing the values that will drive the sorting
     * @idxs: list of considered indexes that has to be sorted
     */
    static void getIdxSorted(const EMatD &X,
                             std::vector< unsigned int > &idxs);

    /**
     * stumpCompare() - Compare two stumps to see which one has the lowest
     *                  error rate
     *
     * @a: first stump to compare
     * @b: second stump to compare
     *
     * Return: True if the first stump has an error rate lower than the
     *         second, false otherwise.
     */
    static bool stumpCompare(struct StumpNode &a,
                             struct StumpNode &b);

    /**
     * trainStump() - Train a single regression stump
     *
     * @features : MxN matrix containing the features, each row
     *             corresponding to the features of a given sample (i.e., M
     *             samples, each with N features)
     * @responses: Mx1 vector of the responses for the corresponding samples
     * @weights  : Mx1 vector of the weights for the corresponding samples
     * @idxs     : indexes of the considered samples
     * @featIdx  : index of the feature under examination
     * @result   : structure where to store the result of the current
     *             operation
     */
    void trainStump(const EMatD &features,
                    const EVecD &responses,
                    const EVecD &weights,
                    const std::vector < unsigned int > &idxs,
                    const unsigned int featIdx,
                    struct StumpNode &result);

    /**
     * updateFeatureIdxs() - Update the indexes of the features in the
     *                       nodes, returning the ordered list of features
     *                       that have been kept (where this list contains
     *                       the feature position before the update)
     *
     * After a tree has been trained, the vast majority of the features
     * proposed are discovered to be useless. This function remaps these
     * feature numbers to the [0,N) interval, modifying accordingly the
     * feature indexes in the nodes and returning the list of the features
     * that have been kept.
     *
     * @updatedFeatIdxs: returned list of updated feature indexes
     */
    void updateFeatureIdxs(std::vector< unsigned int > &updatedFeatIdxs);
};

#endif /* REGTREE_HPP_ */
