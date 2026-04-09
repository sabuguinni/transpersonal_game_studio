// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MetasoundFrontendRegistries.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundDataTypeRegistrationMacro.h"
#include "MetasoundVertex.h"
#include "MetasoundOperatorSettings.h"
#include "MetasoundPrimitives.h"
#include "MetasoundStandardNodesNames.h"
#include "MetasoundFacade.h"
#include "MetasoundStandardNodesCategories.h"

namespace Metasound
{
    namespace DinosaurCallNodeNames
    {
        METASOUND_PARAM(InputTrigger, "Trigger", "Trigger to generate dinosaur call");
        METASOUND_PARAM(InputSpeciesType, "Species Type", "Type of dinosaur species (0=Triceratops, 1=Raptor, 2=TRex, etc.)");
        METASOUND_PARAM(InputEmotionalState, "Emotional State", "Emotional state of the dinosaur (0=Calm, 1=Alert, 2=Aggressive, 3=Fear)");
        METASOUND_PARAM(InputAge, "Age", "Age of the dinosaur (0.0=Baby, 1.0=Adult)");
        METASOUND_PARAM(InputSize, "Size", "Size multiplier for the dinosaur");
        METASOUND_PARAM(InputDistanceFromPlayer, "Distance", "Distance from player for realistic attenuation");
        
        METASOUND_PARAM(OutputAudio, "Audio", "Generated dinosaur call audio");
        METASOUND_PARAM(OutputOnCallGenerated, "On Call Generated", "Trigger when call is generated");
    }

    class TRANSPERSONALGAME_API FDinosaurCallOperator : public TExecutableOperator<FDinosaurCallOperator>
    {
    public:
        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& GetVertexInterface();
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors);

        FDinosaurCallOperator(const FOperatorSettings& InSettings,
                             const FTriggerReadRef& InTrigger,
                             const FInt32ReadRef& InSpeciesType,
                             const FInt32ReadRef& InEmotionalState,
                             const FFloatReadRef& InAge,
                             const FFloatReadRef& InSize,
                             const FFloatReadRef& InDistance);

        virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override;
        virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override;

        void Execute();

    private:
        // Input references
        FTriggerReadRef TriggerInput;
        FInt32ReadRef SpeciesTypeInput;
        FInt32ReadRef EmotionalStateInput;
        FFloatReadRef AgeInput;
        FFloatReadRef SizeInput;
        FFloatReadRef DistanceInput;

        // Output references
        FAudioBufferWriteRef AudioOutput;
        FTriggerWriteRef OnCallGeneratedOutput;

        // Internal state
        FOperatorSettings Settings;
        float Phase;
        float CallDuration;
        float CurrentCallTime;
        bool bIsGeneratingCall;
        
        // Species-specific parameters
        struct FSpeciesParams
        {
            float BaseFrequency;
            float FrequencyRange;
            float CallLength;
            float RoughnessAmount;
            float HarmonicContent;
        };
        
        TArray<FSpeciesParams> SpeciesParameters;
        
        void InitializeSpeciesParameters();
        void GenerateCall(int32 SpeciesType, int32 EmotionalState, float Age, float Size, float Distance);
        float GenerateCallSample(float Time, const FSpeciesParams& Params, int32 EmotionalState, float Age, float Size);
        float ApplyEmotionalModulation(float Sample, int32 EmotionalState, float Time);
        float ApplyAgeModulation(float Sample, float Age);
        float ApplySizeModulation(float Sample, float Size);
    };

    class TRANSPERSONALGAME_API FDinosaurCallNode : public FNodeFacade
    {
    public:
        FDinosaurCallNode(const FNodeInitData& InitData);
    };
}

// Node registration
DECLARE_METASOUND_NODE()