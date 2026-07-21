#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorManager.generated.h"

class ANPC_BaseNPC;
class UNPC_BehaviorComponent;

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Flee        UMETA(DisplayName = "Flee"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Social      UMETA(DisplayName = "Social"),
    Rest        UMETA(DisplayName = "Rest")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector PatrolCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* TargetActor = nullptr;

    FNPC_BehaviorData()
    {
        CurrentState = ENPC_BehaviorState::Idle;
        StateTimer = 0.0f;
        PatrolCenter = FVector::ZeroVector;
        PatrolRadius = 1000.0f;
        AlertLevel = 0.0f;
        TargetActor = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_BehaviorManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNPC_BehaviorManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // NPC Registration
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RegisterNPC(ANPC_BaseNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UnregisterNPC(ANPC_BaseNPC* NPC);

    // Behavior Management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateNPCBehaviors(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetNPCBehaviorState(ANPC_BaseNPC* NPC, ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetNPCBehaviorState(ANPC_BaseNPC* NPC) const;

    // Alert System
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void TriggerAlert(FVector AlertLocation, float AlertRadius, float AlertIntensity);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ProcessNPCAlert(ANPC_BaseNPC* NPC, FVector AlertLocation, float AlertIntensity);

    // Social Interactions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    TArray<ANPC_BaseNPC*> GetNearbyNPCs(ANPC_BaseNPC* SourceNPC, float SearchRadius) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitiateSocialInteraction(ANPC_BaseNPC* NPC1, ANPC_BaseNPC* NPC2);

protected:
    UPROPERTY()
    TArray<ANPC_BaseNPC*> RegisteredNPCs;

    UPROPERTY()
    TMap<ANPC_BaseNPC*, FNPC_BehaviorData> NPCBehaviorData;

    // Behavior Update Functions
    void UpdateIdleBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime);
    void UpdatePatrolBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime);
    void UpdateAlertBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime);
    void UpdateFleeBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime);
    void UpdateHuntBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime);
    void UpdateSocialBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime);
    void UpdateRestBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime);

    // Utility Functions
    FVector GetRandomPatrolPoint(const FNPC_BehaviorData& BehaviorData) const;
    bool IsLocationSafe(FVector Location) const;
    float CalculateDistanceToPlayer(ANPC_BaseNPC* NPC) const;
};

#include "NPCBehaviorManager.generated.h"