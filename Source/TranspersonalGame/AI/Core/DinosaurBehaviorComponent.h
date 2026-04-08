#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "DinosaurBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class EDinosaurPersonality : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Curious         UMETA(DisplayName = "Curious"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Social          UMETA(DisplayName = "Social"),
    Solitary        UMETA(DisplayName = "Solitary"),
    Protective      UMETA(DisplayName = "Protective"),
    Playful         UMETA(DisplayName = "Playful")
};

UENUM(BlueprintType)
enum class EDinosaurState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Sleeping        UMETA(DisplayName = "Sleeping")
};

UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild            UMETA(DisplayName = "Wild"),
    Wary            UMETA(DisplayName = "Wary"),
    Neutral         UMETA(DisplayName = "Neutral"),
    Curious         UMETA(DisplayName = "Curious"),
    Friendly        UMETA(DisplayName = "Friendly"),
    Bonded          UMETA(DisplayName = "Bonded"),
    Domesticated    UMETA(DisplayName = "Domesticated")
};

USTRUCT(BlueprintType)
struct FDinosaurMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FamiliaryLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlayer;

    FDinosaurMemoryEntry()
    {
        Actor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        FamiliaryLevel = 0.0f;
        LastSeenTime = 0.0f;
        bIsPlayer = false;
    }
};

USTRUCT(BlueprintType)
struct FDinosaurNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety;

    FDinosaurNeeds()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 100.0f;
        Social = 50.0f;
        Safety = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct FDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime; // 0.0 = midnight, 12.0 = noon, 24.0 = next midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurState Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    FDailyRoutine()
    {
        StartTime = 0.0f;
        EndTime = 24.0f;
        Activity = EDinosaurState::Idle;
        PreferredLocation = FVector::ZeroVector;
        Priority = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CORE PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    FString DinosaurName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    EDinosaurPersonality PrimaryPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    EDinosaurPersonality SecondaryPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    float PersonalityStrength; // 0.0 = balanced, 1.0 = extreme

    // === CURRENT STATE ===
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EDinosaurState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EDomesticationLevel DomesticationLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FDinosaurNeeds CurrentNeeds;

    // === MEMORY SYSTEM ===
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FDinosaurMemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDuration; // How long memories last in seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries;

    // === DAILY ROUTINES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FDailyRoutine> DailyRoutines;

    UPROPERTY(BlueprintReadOnly, Category = "Daily Routine")
    FDailyRoutine CurrentRoutine;

    // === DOMESTICATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    bool bCanBeDomesticated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationProgress; // 0.0 to 100.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationDecayRate; // How fast progress decays without interaction

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    AActor* BondedPlayer;

    // === BEHAVIOR FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateMemory(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateDailyRoutine();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void AddMemoryEntry(AActor* Actor, float ThreatLevel, float FamiliarityLevel);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FDinosaurMemoryEntry* GetMemoryEntry(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InteractWithPlayer(AActor* Player, float InteractionStrength);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool ShouldFleeFrom(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool ShouldInvestigate(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FVector GetPreferredLocation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetCurrentState(EDinosaurState NewState);

    // === BLACKBOARD INTEGRATION ===
    UFUNCTION(BlueprintCallable, Category = "AI Integration")
    void UpdateBlackboard();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Integration")
    class UBlackboardComponent* BlackboardComponent;

private:
    float LastNeedsUpdate;
    float LastMemoryUpdate;
    float LastRoutineUpdate;
    
    void InitializePersonality();
    void InitializeDailyRoutines();
    float GetCurrentTimeOfDay();
    void DecayDomestication(float DeltaTime);
};