#pragma once

#include "histograms_helper.h"
#include "helpers.h"
#include "pointiwise_optimization_subsets.h"
#include <catboost/cuda/gpu_data/compressed_index.h>
#include <catboost/libs/options/oblivious_tree_options.h>

namespace NCatboostCuda {
    template <EFeaturesGroupingPolicy Policy,
              class TLayoutPolicy>
    inline THolder<TScoreHelper<TLayoutPolicy>> CreateScoreHelper(const TCompressedDataSet<TLayoutPolicy>& dataSet,
                                                                  ui32 foldCount,
                                                                  const NCatboostOptions::TObliviousTreeLearnerOptions& treeConfig,
                                                                  bool requestStream = false) {
        using TFeatureScoresHelper = TScoreHelper<TLayoutPolicy>;
        return MakeHolder<TFeatureScoresHelper>(Policy,
                                                dataSet,
                                                foldCount,
                                                treeConfig.MaxDepth,
                                                treeConfig.ScoreFunction,
                                                treeConfig.L2Reg,
                                                treeConfig.FoldSizeLossNormalization,
                                                requestStream);
    };

    template <class TLayoutPolicy = TFeatureParallelLayout>
    class TScoresCalcerOnCompressedDataSet {
    public:
        using TSamplesMapping = typename TLayoutPolicy::TSamplesMapping;

        TScoresCalcerOnCompressedDataSet(const TCompressedDataSet<TLayoutPolicy>& features,
                                         const NCatboostOptions::TObliviousTreeLearnerOptions& treeConfig,
                                         ui32 foldCount,
                                         bool requestStream = false)
            : Features(features)
            , TreeConfig(treeConfig)
            , FoldCount(foldCount)
        {
            if (Features.GetGridSize(EFeaturesGroupingPolicy::BinaryFeatures)) {
                ScoreHelpers[EFeaturesGroupingPolicy::BinaryFeatures] = CreateScoreHelper<EFeaturesGroupingPolicy::BinaryFeatures, TLayoutPolicy>(Features,
                                                                                                                                                  foldCount,
                                                                                                                                                  TreeConfig,
                                                                                                                                                  requestStream);
            }
            if (Features.GetGridSize(EFeaturesGroupingPolicy::HalfByteFeatures)) {
                ScoreHelpers[EFeaturesGroupingPolicy::HalfByteFeatures] = CreateScoreHelper<EFeaturesGroupingPolicy::HalfByteFeatures, TLayoutPolicy>(Features,
                                                                                                                                                      foldCount,
                                                                                                                                                      TreeConfig,
                                                                                                                                                      requestStream);
            }
            if (Features.GetGridSize(EFeaturesGroupingPolicy::OneByteFeatures)) {
                ScoreHelpers[EFeaturesGroupingPolicy::OneByteFeatures] = CreateScoreHelper<EFeaturesGroupingPolicy::OneByteFeatures, TLayoutPolicy>(Features,
                                                                                                                                                    foldCount,
                                                                                                                                                    TreeConfig,
                                                                                                                                                    requestStream);
            }
        }

        const TScoreHelper<TLayoutPolicy>& GetHelperForPolicy(EFeaturesGroupingPolicy policy) const {
            CB_ENSURE(ScoreHelpers.has(policy));
            return *ScoreHelpers.at(policy);
        }

        bool HasHelperForPolicy(EFeaturesGroupingPolicy policy) const {
            return ScoreHelpers.has(policy);
        }

        template <bool IsConst, class TUi32>
        TScoresCalcerOnCompressedDataSet& SubmitCompute(const TOptimizationSubsets<TSamplesMapping, IsConst>& newSubsets,
                                                        const TCudaBuffer<TUi32, TSamplesMapping>& docs) {
            for (auto& helper : ScoreHelpers) {
                helper.second->SubmitCompute(newSubsets, docs);
            }
            return *this;
        }

        TScoresCalcerOnCompressedDataSet& ComputeOptimalSplit(const TCudaBuffer<const TPartitionStatistics, NCudaLib::TMirrorMapping>& partStats,
                                                              double scoreStdDev = 0,
                                                              ui64 seed = 0) {
            TRandom rand(seed);
            for (auto& helper : ScoreHelpers) {
                helper.second->ComputeOptimalSplit(partStats, scoreStdDev, rand.NextUniformL());
            }
            return *this;
        }

        TBestSplitProperties ReadOptimalSplit() {
            TBestSplitProperties best = {static_cast<ui32>(-1),
                                         0,
                                         std::numeric_limits<float>::infinity()};
            for (auto& helper : ScoreHelpers) {
                best = TakeBest(helper.second->ReadOptimalSplit(), best);
            }
            return best;
        }

    private:
        const TCompressedDataSet<TLayoutPolicy>& Features;
        const NCatboostOptions::TObliviousTreeLearnerOptions& TreeConfig;
        ui32 FoldCount;

        using TScoreHelperPtr = THolder<TScoreHelper<TLayoutPolicy>>;
        TMap<EFeaturesGroupingPolicy, TScoreHelperPtr> ScoreHelpers;
    };

}
