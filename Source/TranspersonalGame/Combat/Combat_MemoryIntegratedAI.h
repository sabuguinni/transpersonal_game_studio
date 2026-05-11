#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/AIController.h"
#include "../SharedTypes.h"
#include "Combat_MemoryIntegratedAI.generated.h"

// Forward declarations
class UNPC_MemorySystem;
class UCombat_TacticalAI;

UENUM(BlueprintType)
enum class ECombat_LearningState : uint8
{
    Observing,
    Learning,
    Adapting,
    Executing,
    Evaluating
};

UENUM(BlueprintType)
enum class ECombat_MemoryType : uint8
{
    SuccessfulAmbush,
    FailedAttack,
    PlayerEscapeRoute,
    EffectiveWeapon,
    TerritorialBoundary,
    PackCoordination,
    ThreatAssessment
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EncounterMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ECombat_MemoryType MemoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float SuccessRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString PlayerBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString CounterStrategy;

    FCombat_EncounterMemory()
    {
        Location = FVector::ZeroVector;
        MemoryType = ECombat_MemoryType::ThreatAssessment;
        SuccessRate = 0.5f;
        EmotionalWeight = 1.0f;
        Timestamp = FDateTime::Now();
        PlayerBehavior = TEXT("");
        CounterStrategy = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_LearningProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float AggresionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float CautionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float AdaptabilityRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float MemoryRetention;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    TArray<FString> LearnedBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    TArray<FString> AvoidedStrategies;

    FCombat_LearningProfile()
    {
        AggresionLevel = 0.7f;
        CautionLevel = 0.5f;
        AdaptabilityRate = 0.8f;
        MemoryRetention = 0.9f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_MemoryIntegratedAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_MemoryIntegratedAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core memory integration functions
    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void RecordEncounter(const FVector& Location, ECombat_MemoryType MemoryType, bool bWasSuccessful, const FString& PlayerBehavior);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    FCombat_EncounterMemory GetRelevantMemory(const FVector& CurrentLocation, float SearchRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void UpdateLearningProfile(bool bEncounterSuccess, const FString& StrategyUsed);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    FString GenerateCounterStrategy(const FString& PlayerBehavior);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    bool ShouldAdaptBehavior(float AdaptationThreshold = 0.6f);

    // Advanced tactical functions
    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void AnalyzePlayerPattern(APawn* PlayerPawn);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    FVector PredictPlayerEscapeRoute(const FVector& PlayerLocation, const FVector& PlayerVelocity);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void CoordinateWithPackMembers(const TArray<APawn*>& PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void SetTerritorialBehavior(bool bIsInTerritory, const FVector& TerritoryCenter, float TerritoryRadius);

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void CleanOldMemories(float MaxAge = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void ShareMemoryWithPack(const TArray<UCombat_MemoryIntegratedAI*>& PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    float CalculateLocationThreat(const FVector& Location);

protected:
    // Memory storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_EncounterMemory> EncounterMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning", meta = (AllowPrivateAccess = "true"))
    FCombat_LearningProfile LearningProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
    ECombat_LearningState CurrentLearningState;

    // Integration with NPC memory system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    UNPC_MemorySystem* NPCMemorySystem;

    // Tactical AI integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    UCombat_TacticalAI* TacticalAI;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float LearningRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float AdaptationThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "true"))
    int32 MaxMemories;

    // Territory and pack behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    bool bIsInTerritory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    TArray<APawn*> PackMembers;

private:
    // Internal helper functions
    void InitializeLearningProfile();
    void ProcessMemoryDecay(float DeltaTime);
    void EvaluateCurrentStrategy();
    FString AnalyzePlayerBehaviorPattern(const TArray<FCombat_EncounterMemory>& RecentMemories);
    void UpdateTacticalKnowledge();
};