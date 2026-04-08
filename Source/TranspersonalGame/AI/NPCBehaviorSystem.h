#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NPCBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    None                UMETA(DisplayName = "None"),
    SolitaryPredator    UMETA(DisplayName = "Solitary Predator"),
    PackHunter          UMETA(DisplayName = "Pack Hunter"),
    HerdHerbivore       UMETA(DisplayName = "Herd Herbivore"),
    SolitaryHerbivore   UMETA(DisplayName = "Solitary Herbivore"),
    Scavenger           UMETA(DisplayName = "Scavenger"),
    TerritorialPredator UMETA(DisplayName = "Territorial Predator"),
    MigratoryHerbivore  UMETA(DisplayName = "Migratory Herbivore"),
    AmbushPredator      UMETA(DisplayName = "Ambush Predator")
};

UENUM(BlueprintType)
enum class EBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Resting         UMETA(DisplayName = "Resting"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Mating          UMETA(DisplayName = "Mating"),
    Nesting         UMETA(DisplayName = "Nesting"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Investigating   UMETA(DisplayName = "Investigating")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Curious     UMETA(DisplayName = "Curious"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Tired       UMETA(DisplayName = "Tired"),
    Territorial UMETA(DisplayName = "Territorial"),
    Protective  UMETA(DisplayName = "Protective"),
    Playful     UMETA(DisplayName = "Playful")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Importance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer AssociatedTags;

    FMemoryEntry()
    {
        Actor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        Timestamp = 0.0f;
        Importance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime; // 0.0 = Dawn, 0.5 = Noon, 1.0 = Dusk

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBehaviorState RoutineBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LocationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority;

    FDailyRoutine()
    {
        StartTime = 0.0f;
        EndTime = 0.0f;
        RoutineBehavior = EBehaviorState::Idle;
        PreferredLocation = FVector::ZeroVector;
        LocationRadius = 500.0f;
        Priority = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurPersonality
{
    GENERATED_BODY()

    // Core personality traits (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence;

    FDinosaurPersonality()
    {
        Aggression = 0.5f;
        Curiosity = 0.5f;
        Sociability = 0.5f;
        Fearfulness = 0.5f;
        Territoriality = 0.5f;
        Intelligence = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EDinosaurArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FDinosaurPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FDailyRoutine> DailyRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MemoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 MaxMemoryEntries;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EBehaviorState CurrentBehavior;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EEmotionalState CurrentEmotion;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentStress;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentHunger;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentEnergy;

    // Memory System
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FMemoryEntry> ShortTermMemory;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FMemoryEntry> LongTermMemory;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AddMemoryEntry(AActor* Actor, float Importance, const FGameplayTagContainer& Tags);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FMemoryEntry* GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetEmotionalState(EEmotionalState NewEmotion);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FDailyRoutine GetCurrentRoutine();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldReactToPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    float CalculateStressLevel();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateNeeds(float DeltaTime);

private:
    void UpdateMemory(float DeltaTime);
    void ProcessDailyRoutines();
    void UpdateEmotionalState();
    float GetTimeOfDay();
};