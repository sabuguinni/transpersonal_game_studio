#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "NPC_MemorySystem.generated.h"

// Forward declarations
class APawn;
class AActor;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* Subject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeSinceLastSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_RelationshipType RelationshipType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FString> AssociatedEvents;

    FNPC_MemoryEntry()
    {
        Subject = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        EmotionalWeight = 0.0f;
        TimeSinceLastSeen = 0.0f;
        RelationshipType = ENPC_RelationshipType::Neutral;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    TMap<AActor*, FNPC_MemoryEntry> KnownIndividuals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    TArray<FVector> ImportantLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    TArray<FString> LearnedBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float OverallTrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    int32 SuccessfulHunts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    int32 FailedHunts;

    FNPC_SocialMemory()
    {
        OverallTrustLevel = 50.0f;
        SuccessfulHunts = 0;
        FailedHunts = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_MemorySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_MemorySystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core memory data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    FNPC_SocialMemory SocialMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    float MaxMemoryDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    int32 MaxStoredMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    float LearningRate;

public:
    // Memory management functions
    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void AddMemoryEntry(AActor* Subject, const FVector& Location, float EmotionalWeight, ENPC_RelationshipType Relationship);

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    bool HasMemoryOf(AActor* Subject) const;

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    FNPC_MemoryEntry GetMemoryOf(AActor* Subject) const;

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void UpdateMemoryEntry(AActor* Subject, const FVector& NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void ForgetActor(AActor* Subject);

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    TArray<AActor*> GetKnownActorsInRange(float Range) const;

    // Learning and adaptation
    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void LearnBehavior(const FString& BehaviorName);

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    bool HasLearnedBehavior(const FString& BehaviorName) const;

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void RecordHuntResult(bool bWasSuccessful);

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    float GetHuntSuccessRate() const;

    // Location memory
    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void RememberLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    FVector GetNearestRememberedLocation(const FVector& CurrentLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    bool IsLocationFamiliar(const FVector& Location, float Tolerance = 500.0f) const;

    // Relationship management
    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void ModifyRelationship(AActor* Subject, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    ENPC_RelationshipType GetRelationshipWith(AActor* Subject) const;

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    TArray<AActor*> GetAlliesInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    TArray<AActor*> GetEnemiesInRange(float Range) const;

protected:
    // Internal memory processing
    void ProcessMemoryDecay(float DeltaTime);
    void CleanupOldMemories();
    void UpdateMemoryLocations();
    
    // Helper functions
    float CalculateEmotionalWeight(AActor* Subject, ENPC_RelationshipType Relationship) const;
    bool ShouldForgetMemory(const FNPC_MemoryEntry& Memory) const;
};