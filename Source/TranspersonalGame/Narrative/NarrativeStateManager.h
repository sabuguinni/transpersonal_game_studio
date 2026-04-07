#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "TranspersonalGame/Core/ConsciousnessComponent.h"
#include "NarrativeStateManager.generated.h"

USTRUCT(BlueprintType)
struct FNarrativeNode : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText NodeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EConsciousnessState RequiredConsciousnessState = EConsciousnessState::Ordinary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float MinIntensityRequired = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FGameplayTagContainer TriggerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsTranspersonalExperience = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float EmotionalWeight = 0.0f;
};

USTRUCT(BlueprintType)
struct FNarrativeChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    FString TargetNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    EConsciousnessState ConsciousnessImpact = EConsciousnessState::Ordinary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    float IntensityModifier = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    FGameplayTagContainer RequiredTags;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrativeNodeChanged, const FString&, NodeID, const FNarrativeNode&, NodeData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTranspersonalExperienceTriggered, const FNarrativeNode&, ExperienceNode);

UCLASS()
class TRANSPERSONALGAME_API UNarrativeStateManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Core Narrative Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetCurrentNarrativeNode(const FString& NodeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ProcessNarrativeChoice(const FNarrativeChoice& Choice);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanAccessNode(const FString& NodeID, EConsciousnessState CurrentState, float CurrentIntensity);

    UFUNCTION(BlueprintPure, Category = "Narrative")
    FString GetCurrentNodeID() const { return CurrentNodeID; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarrativeChoice> GetAvailableChoices();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerConsciousnessBasedNarrative(EConsciousnessState State, float Intensity);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarrativeNodeChanged OnNarrativeNodeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnTranspersonalExperienceTriggered OnTranspersonalExperienceTriggered;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TObjectPtr<UDataTable> NarrativeDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString CurrentNodeID = "Start";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> VisitedNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FGameplayTagContainer PlayerNarrativeState;

    // Consciousness integration
    UPROPERTY()
    TWeakObjectPtr<UConsciousnessComponent> PlayerConsciousnessComponent;

    void InitializeNarrativeSystem();
    FNarrativeNode* GetNarrativeNode(const FString& NodeID);
    void UpdatePlayerNarrativeState();
    void CheckForTranspersonalTriggers(EConsciousnessState State, float Intensity);
};