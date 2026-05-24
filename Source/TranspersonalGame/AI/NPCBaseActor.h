#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "AI/NPCBehaviorComponent.h"
#include "AI/NPCMemorySystem.h"
#include "SharedTypes.h"
#include "NPCBaseActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPCBaseActor : public ACharacter
{
    GENERATED_BODY()

public:
    ANPCBaseActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core NPC Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior", meta = (AllowPrivateAccess = "true"))
    class UNPCBehaviorComponent* BehaviorComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Memory", meta = (AllowPrivateAccess = "true"))
    class UNPCMemorySystem* MemorySystem;

    // NPC Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPCPersonality PersonalityType;

    // NPC Stats
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
    float Fear;

    // Behavior State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPCBehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float FleeRadius;

public:
    // Behavior Interface
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPCBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPCBehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToPlayer(class APawn* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToDinosaur(class APawn* Dinosaur, float Distance);

    // Stats Interface
    UFUNCTION(BlueprintCallable, Category = "NPC Stats")
    void ModifyHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "NPC Stats")
    void ModifyHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = "NPC Stats")
    void ModifyThirst(float Amount);

    UFUNCTION(BlueprintCallable, Category = "NPC Stats")
    void ModifyStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "NPC Stats")
    void ModifyFear(float Amount);

    // Memory Interface
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void RememberLocation(FVector Location, ENPCMemoryType MemoryType);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void RememberActor(AActor* Actor, ENPCMemoryType MemoryType);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    bool IsPlayerInSight() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    bool IsDinosaurNearby() const;

protected:
    // Internal behavior functions
    void UpdateBehaviorState(float DeltaTime);
    void UpdateStats(float DeltaTime);
    void CheckForThreats();
    void PerformIdleBehavior();
    void PerformPatrolBehavior();
    void PerformAlertBehavior();
    void PerformFleeBehavior();

private:
    float LastBehaviorUpdate;
    float LastStatsUpdate;
    float BehaviorUpdateInterval;
    float StatsUpdateInterval;
};