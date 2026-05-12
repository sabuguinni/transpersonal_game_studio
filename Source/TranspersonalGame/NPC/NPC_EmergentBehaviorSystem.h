#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPCBehaviorTypes.h"
#include "NPC_EmergentBehaviorSystem.generated.h"

class UNPC_MemorySystem;
class UNPC_PackCoordinationSystem;

UENUM(BlueprintType)
enum class ENPC_EmergentBehaviorType : uint8
{
    None = 0,
    Curiosity,          // Investigating unknown objects/sounds
    Opportunistic,      // Taking advantage of situations
    Adaptive,           // Learning from experiences
    Innovative,         // Creating new behaviors
    Social,             // Copying behaviors from pack mates
    Environmental,      // Responding to weather/terrain changes
    Survival            // Emergency survival behaviors
};

USTRUCT(BlueprintType)
struct FNPC_EmergentBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    ENPC_EmergentBehaviorType BehaviorType = ENPC_EmergentBehaviorType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    float Priority = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    TWeakObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    float SuccessRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    int32 TimesAttempted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    float LastAttemptTime = 0.0f;

    FNPC_EmergentBehaviorData()
    {
        BehaviorType = ENPC_EmergentBehaviorType::None;
        Priority = 0.0f;
        Duration = 0.0f;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        SuccessRate = 0.5f;
        TimesAttempted = 0;
        LastAttemptTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_LearningPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    FString PatternName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    TArray<ENPC_EmergentBehaviorType> BehaviorSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float SuccessRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    int32 TimesUsed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float AverageReward = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float LastUsedTime = 0.0f;

    FNPC_LearningPattern()
    {
        PatternName = TEXT("");
        BehaviorSequence.Empty();
        SuccessRate = 0.0f;
        TimesUsed = 0;
        AverageReward = 0.0f;
        LastUsedTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_EmergentBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_EmergentBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core emergent behavior functions
    UFUNCTION(BlueprintCallable, Category = "Emergent Behavior")
    void InitializeEmergentBehavior();

    UFUNCTION(BlueprintCallable, Category = "Emergent Behavior")
    void UpdateEmergentBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Emergent Behavior")
    bool TriggerEmergentBehavior(ENPC_EmergentBehaviorType BehaviorType, AActor* TargetActor = nullptr, FVector TargetLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Emergent Behavior")
    void CompleteEmergentBehavior(bool bSuccess, float Reward = 0.0f);

    // Learning and adaptation
    UFUNCTION(BlueprintCallable, Category = "Learning")
    void LearnFromExperience(ENPC_EmergentBehaviorType BehaviorType, bool bSuccess, float Reward);

    UFUNCTION(BlueprintCallable, Category = "Learning")
    void ObserveBehaviorFromPeer(APawn* PeerPawn, ENPC_EmergentBehaviorType ObservedBehavior);

    UFUNCTION(BlueprintCallable, Category = "Learning")
    FNPC_LearningPattern* FindBestLearningPattern(ENPC_EmergentBehaviorType CurrentSituation);

    UFUNCTION(BlueprintCallable, Category = "Learning")
    void CreateNewLearningPattern(const TArray<ENPC_EmergentBehaviorType>& BehaviorSequence, float InitialReward);

    // Environmental adaptation
    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void AdaptToEnvironmentalChange(ENPC_EnvironmentalFactor Factor, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    bool ShouldAdaptBehaviorToWeather();

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    bool ShouldAdaptBehaviorToTerrain();

    // Curiosity and exploration
    UFUNCTION(BlueprintCallable, Category = "Curiosity")
    void InvestigateUnknownObject(AActor* UnknownObject);

    UFUNCTION(BlueprintCallable, Category = "Curiosity")
    void ExploreNewArea(FVector AreaLocation, float ExplorationRadius);

    UFUNCTION(BlueprintCallable, Category = "Curiosity")
    float CalculateCuriosityLevel(AActor* TargetObject);

    // Innovation and problem solving
    UFUNCTION(BlueprintCallable, Category = "Innovation")
    bool AttemptInnovativeSolution(ENPC_BehaviorState CurrentProblem);

    UFUNCTION(BlueprintCallable, Category = "Innovation")
    void ExperimentWithNewBehavior();

    UFUNCTION(BlueprintCallable, Category = "Innovation")
    bool CanCreateNewBehaviorPattern();

    // Social learning
    UFUNCTION(BlueprintCallable, Category = "Social")
    void ShareLearningWithPack();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ReceiveLearningFromPack(const FNPC_LearningPattern& SharedPattern);

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool ShouldMimicPackLeader();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Emergent Behavior")
    FNPC_EmergentBehaviorData GetCurrentEmergentBehavior() const { return CurrentEmergentBehavior; }

    UFUNCTION(BlueprintPure, Category = "Emergent Behavior")
    bool IsPerformingEmergentBehavior() const { return CurrentEmergentBehavior.BehaviorType != ENPC_EmergentBehaviorType::None; }

    UFUNCTION(BlueprintPure, Category = "Learning")
    int32 GetLearningPatternCount() const { return LearnedPatterns.Num(); }

    UFUNCTION(BlueprintPure, Category = "Learning")
    float GetAdaptabilityLevel() const { return AdaptabilityLevel; }

protected:
    // Current emergent behavior
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emergent Behavior")
    FNPC_EmergentBehaviorData CurrentEmergentBehavior;

    // Learning patterns
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning")
    TArray<FNPC_LearningPattern> LearnedPatterns;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    float CuriosityLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    float AdaptabilityLevel = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    float InnovationLevel = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    float SocialLearningRate = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Behavior")
    float EnvironmentalAdaptationRate = 0.5f;

    // Timing parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float EmergentBehaviorCooldown = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float LearningDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float MaxEmergentBehaviorDuration = 120.0f;

    // Component references
    UPROPERTY()
    TWeakObjectPtr<UNPC_MemorySystem> MemorySystem;

    UPROPERTY()
    TWeakObjectPtr<UNPC_PackCoordinationSystem> PackCoordination;

    UPROPERTY()
    TWeakObjectPtr<APawn> OwnerPawn;

private:
    // Internal helper functions
    void UpdateCuriosityBehavior(float DeltaTime);
    void UpdateAdaptiveBehavior(float DeltaTime);
    void UpdateInnovativeBehavior(float DeltaTime);
    void UpdateSocialLearning(float DeltaTime);

    bool EvaluateEmergentBehaviorTrigger(ENPC_EmergentBehaviorType BehaviorType);
    float CalculateBehaviorPriority(ENPC_EmergentBehaviorType BehaviorType, AActor* TargetActor, FVector TargetLocation);
    void DecayLearningPatterns(float DeltaTime);
    void UpdateBehaviorSuccessRates();

    // Timing variables
    float LastEmergentBehaviorTime = 0.0f;
    float LastLearningUpdateTime = 0.0f;
    float LastEnvironmentalCheckTime = 0.0f;
};