#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "NPCBehaviorSystem.h"
#include "DinosaurNPC.generated.h"

// Forward Declarations
class UNPCBehaviorSystem;

/**
 * Dinosaur behavior archetypes
 */
UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    SmallHerbivore     UMETA(DisplayName = "Small Herbivore"),
    LargeHerbivore     UMETA(DisplayName = "Large Herbivore"),
    SmallCarnivore     UMETA(DisplayName = "Small Carnivore"),
    LargeCarnivore     UMETA(DisplayName = "Large Carnivore"),
    Scavenger          UMETA(DisplayName = "Scavenger"),
    Aquatic            UMETA(DisplayName = "Aquatic"),
    Flying             UMETA(DisplayName = "Flying")
};

/**
 * Current behavior state
 */
UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle               UMETA(DisplayName = "Idle"),
    Foraging           UMETA(DisplayName = "Foraging"),
    Hunting            UMETA(DisplayName = "Hunting"),
    Fleeing            UMETA(DisplayName = "Fleeing"),
    Socializing        UMETA(DisplayName = "Socializing"),
    Resting            UMETA(DisplayName = "Resting"),
    Drinking           UMETA(DisplayName = "Drinking"),
    Territorial        UMETA(DisplayName = "Territorial"),
    Mating             UMETA(DisplayName = "Mating"),
    Investigating      UMETA(DisplayName = "Investigating")
};

/**
 * Individual dinosaur NPC with autonomous behavior
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurNPC : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurNPC();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    EDinosaurArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    FString IndividualName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    int32 UniqueID;

    // Physical Variations (for unique identification)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float SizeVariation; // 0.8 to 1.2 multiplier

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor PrimaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor SecondaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float AggressionLevel; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float CuriosityLevel; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float SocialTendency; // 0.0 to 1.0

    // Behavior State
    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    EDinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float StateTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    AActor* CurrentTarget;

    // Daily Routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FVector> PreferredLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float DailyActivityRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Daily Routine")
    float TimeOfDay; // 0.0 to 24.0

    // Memory System
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FNPCMemoryEntry> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // Domestication
    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    TMap<AActor*, FDomesticationData> DomesticationProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    bool bCanBeDomesticated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationDifficulty; // 0.1 to 2.0

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateDailyRoutine(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(AActor* Subject, const FVector& Location, const FString& MemoryType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPCMemoryEntry* GetMemoryOf(AActor* Subject);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessPlayerInteraction(AActor* Player, bool bPositive, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool IsDomesticated(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    float GetTrustLevel(AActor* Player);

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal behavior functions
    void ProcessIdleBehavior(float DeltaTime);
    void ProcessForagingBehavior(float DeltaTime);
    void ProcessHuntingBehavior(float DeltaTime);
    void ProcessFleeingBehavior(float DeltaTime);
    void ProcessSocializingBehavior(float DeltaTime);
    void ProcessRestingBehavior(float DeltaTime);

    // Utility functions
    FVector FindNearestResourceLocation(const FString& ResourceType);
    ADinosaurNPC* FindNearestDinosaur(float MaxDistance);
    bool IsPlayerThreatening(AActor* Player);
    void UpdateBlackboardValues();

private:
    // Reference to behavior system
    UNPCBehaviorSystem* BehaviorSystem;

    // Internal timers
    float RoutineUpdateTimer;
    float PerceptionUpdateTimer;
    float MemoryUpdateTimer;
};