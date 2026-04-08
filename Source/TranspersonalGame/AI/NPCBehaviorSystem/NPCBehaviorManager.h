#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "MassEntitySubsystem.h"
#include "NPCBehaviorManager.generated.h"

class ADinosaurNPC;
class UDinosaurSpeciesData;
class UNPCMemoryComponent;

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle,
    Foraging,
    Drinking,
    Resting,
    Socializing,
    Patrolling,
    Fleeing,
    Hunting,
    Territorial,
    Mating,
    Nesting,
    Migrating
};

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Extreme
};

USTRUCT(BlueprintType)
struct FDinosaurPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;
};

USTRUCT(BlueprintType)
struct FDinosaurNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Territory = 50.0f;
};

USTRUCT(BlueprintType)
struct FDinosaurMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Actor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Familiarity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHostile = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPrey = false;
};

USTRUCT(BlueprintType)
struct FDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime = 0.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState Activity = EDinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f;
};

/**
 * Manages NPC behavior for all dinosaurs in the world
 * Handles both individual Behavior Trees and mass simulation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPCBehaviorManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UNPCBehaviorManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Registration
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RegisterDinosaur(ADinosaurNPC* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UnregisterDinosaur(ADinosaurNPC* Dinosaur);

    // Behavior State Management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetDinosaurBehaviorState(ADinosaurNPC* Dinosaur, EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    EDinosaurBehaviorState GetDinosaurBehaviorState(ADinosaurNPC* Dinosaur) const;

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateDinosaurMemory(ADinosaurNPC* Dinosaur, AActor* PerceivedActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FDinosaurMemoryEntry GetMemoryEntry(ADinosaurNPC* Dinosaur, AActor* Actor) const;

    // Ecosystem Simulation
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateEcosystemState(float DeltaTime);

    // Player Interaction
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void NotifyPlayerAction(const FVector& Location, float NoiseLevel, bool bIsAggressive);

    // Domestication System
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float GetDomesticationLevel(ADinosaurNPC* Dinosaur) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateDomesticationLevel(ADinosaurNPC* Dinosaur, float DeltaValue);

protected:
    // Registered dinosaurs
    UPROPERTY()
    TArray<ADinosaurNPC*> RegisteredDinosaurs;

    // Behavior state tracking
    UPROPERTY()
    TMap<ADinosaurNPC*, EDinosaurBehaviorState> DinosaurStates;

    // Memory system
    UPROPERTY()
    TMap<ADinosaurNPC*, TArray<FDinosaurMemoryEntry>> DinosaurMemories;

    // Domestication tracking
    UPROPERTY()
    TMap<ADinosaurNPC*, float> DomesticationLevels;

    // Ecosystem state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float FoodAvailability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float WaterAvailability = 1.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveDetailedBehaviors = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DetailedBehaviorRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUpdateInterval = 0.5f;

private:
    // Internal update functions
    void UpdateDinosaurNeeds(float DeltaTime);
    void UpdateDailyRoutines(float DeltaTime);
    void ProcessDinosaurInteractions(float DeltaTime);
    void ManageBehaviorLOD(float DeltaTime);

    // Memory management
    void CleanupOldMemories(float DeltaTime);
    void UpdateMemoryFamiliarity(float DeltaTime);

    // Timers
    float MemoryUpdateTimer = 0.0f;
    float EcosystemUpdateTimer = 0.0f;
    float InteractionUpdateTimer = 0.0f;
};