#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "DinosaurAIController.h"
#include "DinosaurMemoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FMemoryLocation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString LocationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 VisitCount;

    FMemoryLocation()
    {
        Location = FVector::ZeroVector;
        LocationType = TEXT("");
        Timestamp = 0.0f;
        Importance = 1.0f;
        VisitCount = 0;
    }

    FMemoryLocation(FVector InLocation, FString InType, float InTimestamp, float InImportance = 1.0f)
        : Location(InLocation), LocationType(InType), Timestamp(InTimestamp), Importance(InImportance), VisitCount(1)
    {}
};

USTRUCT(BlueprintType)
struct FMemoryActor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTimestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Familiarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 EncounterCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsFriendly;

    FMemoryActor()
    {
        Actor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        LastSeenTimestamp = 0.0f;
        Familiarity = 0.0f;
        ThreatLevel = 0.5f;
        EncounterCount = 0;
        bIsFriendly = false;
    }
};

USTRUCT(BlueprintType)
struct FMemoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TWeakObjectPtr<AActor> RelatedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString Description;

    FMemoryEvent()
    {
        EventType = TEXT("");
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EmotionalImpact = 0.0f;
        RelatedActor = nullptr;
        Description = TEXT("");
    }
};

/**
 * Advanced memory system for dinosaur NPCs that tracks locations, actors, events,
 * and behavioral patterns to create realistic long-term memory and learning
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurMemoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurMemoryComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Memory Storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FMemoryLocation> LocationMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FMemoryActor> ActorMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FMemoryEvent> EventMemories;

    // Memory Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxLocationMemories = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxActorMemories = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxEventMemories = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ForgetThreshold = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ShortTermMemoryDuration = 300.0f; // 5 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LongTermMemoryThreshold = 0.8f;

public:
    // Location Memory
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RecordLocation(FVector Location, const FString& LocationType, float Importance = 1.0f);

    UFUNCTION(BlueprintPure, Category = "Memory")
    FVector RecallLocationOfType(const FString& LocationType) const;

    UFUNCTION(BlueprintPure, Category = "Memory")
    TArray<FMemoryLocation> GetLocationMemoriesOfType(const FString& LocationType) const;

    UFUNCTION(BlueprintPure, Category = "Memory")
    FMemoryLocation GetNearestLocationOfType(const FString& LocationType, FVector CurrentLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetLocation(FVector Location, float Radius = 100.0f);

    // Actor Memory
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RecordSighting(AActor* Actor, FVector Location);

    UFUNCTION(BlueprintPure, Category = "Memory")
    bool HasMemoryOf(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "Memory")
    FMemoryActor GetMemoryOf(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateActorRelationship(AActor* Actor, float FamiliarityChange, float ThreatChange);

    UFUNCTION(BlueprintPure, Category = "Memory")
    TArray<FMemoryActor> GetKnownActors() const { return ActorMemories; }

    UFUNCTION(BlueprintPure, Category = "Memory")
    FVector GetLastKnownLocationOf(AActor* Actor) const;

    // Event Memory
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RecordEvent(const FString& EventType, FVector Location, float EmotionalImpact, AActor* RelatedActor = nullptr, const FString& Description = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RecordBehaviorChange(EDinosaurBehaviorState PreviousState, EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "Memory")
    TArray<FMemoryEvent> GetEventsOfType(const FString& EventType) const;

    UFUNCTION(BlueprintPure, Category = "Memory")
    FMemoryEvent GetMostRecentEvent() const;

    UFUNCTION(BlueprintPure, Category = "Memory")
    float GetEmotionalStateFromMemories() const;

    // Memory Analysis
    UFUNCTION(BlueprintPure, Category = "Memory")
    bool IsLocationFamiliar(FVector Location, float Radius = 200.0f) const;

    UFUNCTION(BlueprintPure, Category = "Memory")
    float GetLocationSafety(FVector Location, float Radius = 300.0f) const;

    UFUNCTION(BlueprintPure, Category = "Memory")
    TArray<FVector> GetDangerousAreas() const;

    UFUNCTION(BlueprintPure, Category = "Memory")
    TArray<FVector> GetSafeAreas() const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ConsolidateMemories();

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void DebugPrintMemories() const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ClearAllMemories();

protected:
    // Internal memory management
    void DecayMemories(float DeltaTime);
    void CleanupOldMemories();
    void PruneMemoriesByImportance();
    
    float GetCurrentTimestamp() const;
    bool ShouldForgetMemory(float LastAccessed, float Importance) const;
    void UpdateMemoryImportance(FMemoryLocation& Memory);
    void UpdateMemoryImportance(FMemoryActor& Memory);
    void UpdateMemoryImportance(FMemoryEvent& Memory);

    // Memory timers
    float MemoryUpdateTimer = 0.0f;
    float ConsolidationTimer = 0.0f;
    
    // Cached references
    class UWorld* CachedWorld;
    class ADinosaurAIController* OwnerController;
};