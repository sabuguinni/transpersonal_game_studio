#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorSystem.generated.h"

// Forward declarations
class UBlackboardComponent;
class UBehaviorTreeComponent;
class AAIController;

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
    Skittish        UMETA(DisplayName = "Skittish")
};

UENUM(BlueprintType)
enum class EDinosaurMood : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Alert           UMETA(DisplayName = "Alert"),
    Hungry          UMETA(DisplayName = "Hungry"),
    Thirsty         UMETA(DisplayName = "Thirsty"),
    Tired           UMETA(DisplayName = "Tired"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Curious         UMETA(DisplayName = "Curious"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Protective      UMETA(DisplayName = "Protective")
};

UENUM(BlueprintType)
enum class EDinosaurActivity : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Nesting         UMETA(DisplayName = "Nesting"),
    Migrating       UMETA(DisplayName = "Migrating")
};

UENUM(BlueprintType)
enum class ETrustLevel : uint8
{
    Hostile         UMETA(DisplayName = "Hostile"),
    Wary            UMETA(DisplayName = "Wary"),
    Neutral         UMETA(DisplayName = "Neutral"),
    Curious         UMETA(DisplayName = "Curious"),
    Accepting       UMETA(DisplayName = "Accepting"),
    Trusting        UMETA(DisplayName = "Trusting"),
    Bonded          UMETA(DisplayName = "Bonded")
};

USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* RememberedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETrustLevel TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositiveInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativeInteractions;

    FDinosaurMemory()
    {
        RememberedActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        TrustLevel = ETrustLevel::Neutral;
        PositiveInteractions = 0;
        NegativeInteractions = 0;
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
    float Safety;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Territory;

    FDinosaurNeeds()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 100.0f;
        Safety = 50.0f;
        Social = 50.0f;
        Territory = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct FDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration; // In hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurActivity Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority; // 0-1, higher = more important

    FDailyRoutine()
    {
        StartTime = 0.0f;
        Duration = 1.0f;
        Activity = EDinosaurActivity::Idle;
        PreferredLocation = FVector::ZeroVector;
        Priority = 0.5f;
    }
};

/**
 * Core NPC Behavior System Component
 * Handles individual dinosaur personality, memory, needs, and daily routines
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core personality and identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    EDinosaurPersonality PrimaryPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    EDinosaurPersonality SecondaryPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PersonalityIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EDinosaurMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EDinosaurActivity CurrentActivity;

    // Needs system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    FDinosaurNeeds CurrentNeeds;

    // Memory system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FDinosaurMemory> MemoryBank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // Daily routine system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FDailyRoutine> DailyRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float CurrentGameHour;

    // Domestication system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    bool bCanBeDomesticated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    ETrustLevel PlayerTrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float TrustDecayRate;

    // Unique physical traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FString UniqueIdentifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    TMap<FString, float> PhysicalVariations;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateMood();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    EDinosaurActivity DetermineNextActivity();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberActor(AActor* Actor, float ThreatLevel = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FDinosaurMemory* GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateActorMemory(AActor* Actor, float ThreatModifier, bool bPositiveInteraction);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessPlayerInteraction(bool bPositive, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanAcceptFood() const;

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool WillFleeFromPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FDailyRoutine GetCurrentRoutineActivity();

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void SetCustomActivity(EDinosaurActivity NewActivity, float Duration = -1.0f);

    // Personality influence functions
    UFUNCTION(BlueprintCallable, Category = "Personality")
    float GetPersonalityModifier(const FString& BehaviorType) const;

    UFUNCTION(BlueprintCallable, Category = "Personality")
    bool ShouldReactToStimulus(AActor* Stimulus, float Distance) const;

private:
    void InitializeUniqueTraits();
    void DecayMemories(float DeltaTime);
    void UpdateTrustLevel(float DeltaTime);
    float CalculateNeedUrgency(const FString& NeedType) const;
    void GenerateDailyRoutine();
};