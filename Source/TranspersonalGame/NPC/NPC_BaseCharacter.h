#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "../Shared/SharedTypes.h"
#include "NPC_BaseCharacter.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_MemoryType MemoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Importance;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        Actor = nullptr;
        Timestamp = 0.0f;
        MemoryType = ENPC_MemoryType::Neutral;
        Importance = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_RelationshipType RelationshipType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteractionTime;

    FNPC_SocialRelationship()
    {
        TargetActor = nullptr;
        RelationshipType = ENPC_RelationshipType::Neutral;
        TrustLevel = 0.0f;
        LastInteractionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_BaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ANPC_BaseCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core NPC Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float MaxStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float Fear;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    // Perception Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UPawnSensingComponent* PawnSensingComponent;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // Social System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FNPC_SocialRelationship> SocialRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ENPC_PersonalityType PersonalityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRadius;

    // Daily Routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    ENPC_ActivityType CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float ActivityTimer;

public:
    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(FVector Location, AActor* Actor, ENPC_MemoryType MemoryType, float Importance = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemory(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesOfType(ENPC_MemoryType MemoryType);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_MemoryEntry GetMostImportantMemory();

    // Social Functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateRelationship(AActor* TargetActor, ENPC_RelationshipType NewType, float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    FNPC_SocialRelationship GetRelationshipWith(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    TArray<AActor*> GetAlliesInRadius(float Radius);

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetActivity(ENPC_ActivityType NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToThreat(AActor* ThreatActor, ENPC_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ExecuteDailyRoutine(float DeltaTime);

    // Perception Callbacks
    UFUNCTION()
    void OnSeePawn(APawn* SeenPawn);

    UFUNCTION()
    void OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume);

    // Stat Functions
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void RestoreHealth(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ConsumeFood(float FoodValue);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void DrinkWater(float WaterValue);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Stats")
    ENPC_NPCState GetCurrentState() const;

protected:
    // Internal state tracking
    ENPC_NPCState CurrentState;
    float LastActivityTime;
    AActor* CurrentTarget;
    FVector CurrentDestination;

    // Helper functions
    void UpdateStats(float DeltaTime);
    void CleanupOldMemories();
    void UpdateBlackboardValues();
    FVector GetRandomPatrolPoint();
    bool ShouldFlee(AActor* ThreatActor);
};