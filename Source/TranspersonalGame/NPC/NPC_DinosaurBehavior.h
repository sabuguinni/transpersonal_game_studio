#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor   UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Pteranodon     UMETA(DisplayName = "Pteranodon"),
    Compsognathus  UMETA(DisplayName = "Compsognathus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurBehaviorMode : uint8
{
    Passive        UMETA(DisplayName = "Passive Grazing"),
    Territorial    UMETA(DisplayName = "Territorial Defense"),
    Hunting        UMETA(DisplayName = "Active Hunting"),
    Fleeing        UMETA(DisplayName = "Fleeing Predator"),
    Migrating      UMETA(DisplayName = "Seasonal Migration"),
    Nesting        UMETA(DisplayName = "Nesting Behavior"),
    Aggressive     UMETA(DisplayName = "Aggressive Attack")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsHostile = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    ENPC_DinosaurBehaviorMode CurrentBehaviorMode = ENPC_DinosaurBehaviorMode::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    int32 PackSize = 1;

    // Memory System
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_DinosaurMemory> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate = 0.1f;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorMode(ENPC_DinosaurBehaviorMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AddMemoryEntry(AActor* Actor, float ThreatLevel, bool bHostile);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FNPC_DinosaurMemory* FindMemoryEntry(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateMemoryDecay(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool IsActorInDetectionRange(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ProcessThreatDetection();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ExecuteBehaviorLogic(float DeltaTime);

    // Species-Specific Behaviors
    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteTRexBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteVelociraptorBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteHerbivoreBehavior(float DeltaTime);

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void FormPack(const TArray<UNPC_DinosaurBehavior*>& PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void PackFleeResponse(AActor* Threat);

private:
    // Internal state
    float BehaviorTimer = 0.0f;
    float NextBehaviorChange = 5.0f;
    FVector PatrolCenter;
    FVector CurrentTarget;
    bool bHasPatrolCenter = false;

    // Pack references
    UPROPERTY()
    TArray<UNPC_DinosaurBehavior*> PackMembers;

    UPROPERTY()
    UNPC_DinosaurBehavior* PackLeader;

    bool bIsPackLeader = false;

    // Helper functions
    void InitializeSpeciesDefaults();
    void UpdateStats(float DeltaTime);
    FVector GetRandomPatrolPoint() const;
    bool ShouldChangeBehavior() const;
    void HandlePlayerInteraction(AActor* Player);
};