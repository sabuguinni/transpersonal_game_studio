#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NPCBehaviorSystem.generated.h"

// Forward declarations
class UBehaviorTreeComponent;
class UBlackboardComponent;
class ADinosaurNPC;

/**
 * Core behavioral states for all NPCs
 */
UENUM(BlueprintType)
enum class ENPCBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Resting         UMETA(DisplayName = "Resting"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Nesting         UMETA(DisplayName = "Nesting")
};

/**
 * Personality traits that affect behavior decisions
 */
USTRUCT(BlueprintType)
struct FNPCPersonality
{
    GENERATED_BODY()

    // Core personality traits (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Caution = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    FNPCPersonality()
    {
        // Generate random personality on construction
        Aggression = FMath::RandRange(0.0f, 1.0f);
        Curiosity = FMath::RandRange(0.0f, 1.0f);
        Sociability = FMath::RandRange(0.0f, 1.0f);
        Caution = FMath::RandRange(0.0f, 1.0f);
        Territoriality = FMath::RandRange(0.0f, 1.0f);
        Intelligence = FMath::RandRange(0.0f, 1.0f);
    }
};

/**
 * Memory system for NPCs to remember events, locations, and other entities
 */
USTRUCT(BlueprintType)
struct FNPCMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* RememberedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalWeight = 0.0f; // Positive = good memory, negative = bad memory

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventType = TEXT("");

    FNPCMemoryEntry()
    {
        RememberedActor = nullptr;
        Location = FVector::ZeroVector;
        EmotionalWeight = 0.0f;
        Timestamp = 0.0f;
        EventType = TEXT("");
    }
};

/**
 * Daily routine schedule for NPCs
 */
USTRUCT(BlueprintType)
struct FDailyRoutineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime = 0.0f; // Time of day (0.0 = dawn, 0.5 = noon, 1.0 = midnight)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.1f; // Duration as fraction of day

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCBehaviorState Activity = ENPCBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f;

    FDailyRoutineEntry()
    {
        StartTime = 0.0f;
        Duration = 0.1f;
        Activity = ENPCBehaviorState::Idle;
        PreferredLocation = FVector::ZeroVector;
        Priority = 1.0f;
    }
};

/**
 * Core NPC Behavior Component
 * Handles personality, memory, daily routines, and decision making
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core personality
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FNPCPersonality Personality;

    // Current behavioral state
    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    ENPCBehaviorState CurrentState = ENPCBehaviorState::Idle;

    // Memory system
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FNPCMemoryEntry> Memories;

    // Daily routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FDailyRoutineEntry> DailyRoutine;

    // Needs and drives
    UPROPERTY(BlueprintReadWrite, Category = "Needs")
    float Hunger = 0.0f; // 0.0 = not hungry, 1.0 = starving

    UPROPERTY(BlueprintReadWrite, Category = "Needs")
    float Thirst = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Needs")
    float Energy = 1.0f; // 1.0 = fully rested, 0.0 = exhausted

    UPROPERTY(BlueprintReadWrite, Category = "Needs")
    float Social = 0.5f; // Need for social interaction

    UPROPERTY(BlueprintReadWrite, Category = "Needs")
    float Safety = 1.0f; // Feeling of safety

    // Domestication system
    UPROPERTY(BlueprintReadWrite, Category = "Domestication")
    float TrustLevel = 0.0f; // 0.0 = wild, 1.0 = fully domesticated

    UPROPERTY(BlueprintReadWrite, Category = "Domestication")
    bool bCanBeDomesticated = false;

    UPROPERTY(BlueprintReadWrite, Category = "Domestication")
    AActor* TrustedHuman = nullptr;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPCBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(AActor* Actor, FVector Location, float EmotionalWeight, const FString& EventType);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPCMemoryEntry GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    ENPCBehaviorState GetCurrentRoutineActivity();

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessDomestication(AActor* Human, float InteractionQuality, float DeltaTime);

    // Decision making
    UFUNCTION(BlueprintCallable, Category = "AI")
    ENPCBehaviorState DecideNextBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI")
    float CalculateBehaviorPriority(ENPCBehaviorState Behavior);

private:
    float LastNeedsUpdate = 0.0f;
    float NeedsUpdateInterval = 5.0f; // Update needs every 5 seconds

    void GenerateRandomRoutine();
    float GetTimeOfDay();
};