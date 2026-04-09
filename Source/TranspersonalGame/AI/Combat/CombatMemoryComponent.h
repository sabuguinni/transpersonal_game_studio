// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameplayTags.h"
#include "CombatMemoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    FVector LastKnownPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    float TotalDamageDealt = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    float TotalDamageReceived = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    int32 EncounterCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    int32 VictoryCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    int32 DefeatCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    float ThreatLevel = 0.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    TArray<FGameplayTag> KnownAbilities;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    TArray<FGameplayTag> SuccessfulTactics;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    TArray<FGameplayTag> FailedTactics;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    bool bIsHostile = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    bool bCanBeFlanked = true;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    bool bUsesRangedAttacks = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Memory")
    float PreferredEngagementDistance = 200.0f;

    FCombatMemory()
    {
        Actor = nullptr;
        LastKnownPosition = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        TotalDamageDealt = 0.0f;
        TotalDamageReceived = 0.0f;
        EncounterCount = 0;
        VictoryCount = 0;
        DefeatCount = 0;
        ThreatLevel = 0.5f;
        bIsHostile = false;
        bCanBeFlanked = true;
        bUsesRangedAttacks = false;
        PreferredEngagementDistance = 200.0f;
    }
};

USTRUCT(BlueprintType)
struct FTacticalMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Tactical Memory")
    FGameplayTag TacticTag;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical Memory")
    int32 UsageCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical Memory")
    int32 SuccessCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical Memory")
    float LastUsedTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical Memory")
    float SuccessRate = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical Memory")
    TWeakObjectPtr<AActor> MostEffectiveAgainst;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical Memory")
    TWeakObjectPtr<AActor> LeastEffectiveAgainst;

    FTacticalMemory()
    {
        UsageCount = 0;
        SuccessCount = 0;
        LastUsedTime = 0.0f;
        SuccessRate = 0.0f;
        MostEffectiveAgainst = nullptr;
        LeastEffectiveAgainst = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FLocationMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Location Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Location Memory")
    FGameplayTag LocationType;

    UPROPERTY(BlueprintReadWrite, Category = "Location Memory")
    float DangerLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Location Memory")
    float LastVisitTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Location Memory")
    int32 CombatEventsCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Location Memory")
    bool bIsAmbushSite = false;

    UPROPERTY(BlueprintReadWrite, Category = "Location Memory")
    bool bHasGoodCover = false;

    UPROPERTY(BlueprintReadWrite, Category = "Location Memory")
    bool bHasEscapeRoutes = false;

    FLocationMemory()
    {
        Location = FVector::ZeroVector;
        DangerLevel = 0.0f;
        LastVisitTime = 0.0f;
        CombatEventsCount = 0;
        bIsAmbushSite = false;
        bHasGoodCover = false;
        bHasEscapeRoutes = false;
    }
};

/**
 * Combat Memory Component that tracks combat experiences, opponent behaviors,
 * tactical effectiveness, and environmental combat data for AI learning
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatMemoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatMemoryComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Memory Storage
    UPROPERTY(BlueprintReadOnly, Category = "Combat Memory")
    TMap<AActor*, FCombatMemory> CombatMemories;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Memory")
    TMap<FGameplayTag, FTacticalMemory> TacticalMemories;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Memory")
    TArray<FLocationMemory> LocationMemories;

    // Memory Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    int32 MaxCombatMemories = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    int32 MaxLocationMemories = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MemoryDecayTime = 300.0f; // 5 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float LocationMemoryRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float ThreatLevelUpdateRate = 0.1f;

public:
    // Combat Memory Management
    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void RecordCombatEncounter(AActor* Opponent, bool bVictorious, float DamageDealt, float DamageReceived);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void UpdateActorPosition(AActor* Actor, FVector Position);

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    bool HasMemoryOf(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    FCombatMemory GetCombatMemory(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    float GetThreatLevel(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    FVector GetLastKnownPosition(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    float GetTimeSinceLastSeen(AActor* Actor) const;

    // Tactical Memory Management
    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void RecordTacticUsage(FGameplayTag TacticTag, bool bSuccessful, AActor* TargetActor = nullptr);

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    float GetTacticSuccessRate(FGameplayTag TacticTag) const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    FGameplayTag GetMostSuccessfulTactic() const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    FGameplayTag GetBestTacticAgainst(AActor* Opponent) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void LearnOpponentAbility(AActor* Opponent, FGameplayTag AbilityTag);

    // Location Memory Management
    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void RecordLocationEvent(FVector Location, FGameplayTag EventType, float DangerLevel = 0.0f);

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    float GetLocationDangerLevel(FVector Location) const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    TArray<FVector> GetSafeLocations(float MaxDangerLevel = 0.3f) const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    TArray<FVector> GetAmbushLocations() const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    FVector GetNearestSafeLocation(FVector FromLocation) const;

    // Memory Analysis
    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    AActor* GetMostDangerousOpponent() const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    AActor* GetWeakestOpponent() const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    float GetOverallCombatExperience() const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    bool ShouldAvoidOpponent(AActor* Opponent) const;

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    bool CanDefeatOpponent(AActor* Opponent) const;

    // Memory Maintenance
    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void CleanupOldMemories();

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void ForgetActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void ClearAllMemories();

    // Learning and Adaptation
    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void AnalyzeCombatPatterns();

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void AdaptToOpponent(AActor* Opponent);

    UFUNCTION(BlueprintPure, Category = "Combat Memory")
    bool IsLearningFromExperience() const;

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintMemoryStats() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawMemoryData() const;

protected:
    // Internal Memory Processing
    void UpdateMemoryDecay(float DeltaTime);
    void UpdateThreatLevels();
    void ProcessLocationLearning();
    void OptimizeMemoryStorage();

    // Memory Utilities
    FCombatMemory& GetOrCreateCombatMemory(AActor* Actor);
    FTacticalMemory& GetOrCreateTacticalMemory(FGameplayTag TacticTag);
    FLocationMemory* FindLocationMemory(FVector Location);
    void RemoveOldestMemory();

    // Learning Algorithms
    float CalculateThreatLevel(const FCombatMemory& Memory) const;
    float CalculateLocationDanger(const FLocationMemory& Memory) const;
    void UpdateTacticEffectiveness(FTacticalMemory& Memory);

    // Timers
    float MemoryUpdateTimer = 0.0f;
    float AnalysisTimer = 0.0f;
    float CleanupTimer = 0.0f;

    // Cached References
    class AAIController* OwnerController;
    class UWorld* CachedWorld;

    // Learning Parameters
    float LearningRate = 0.1f;
    float MemoryStrength = 1.0f;
    bool bEnableLearning = true;
};