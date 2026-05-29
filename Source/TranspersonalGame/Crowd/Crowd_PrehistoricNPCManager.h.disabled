#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Crowd_PrehistoricNPCManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PrehistoricHuman
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Human")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Human")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Human")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Human")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Human")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Human")
    float ActivityTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Human")
    ECrowd_NPCActivity CurrentActivity;

    FCrowd_PrehistoricHuman()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        MovementSpeed = 150.0f;
        TargetLocation = FVector::ZeroVector;
        bIsMoving = false;
        ActivityTimer = 0.0f;
        CurrentActivity = ECrowd_NPCActivity::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TribalGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Group")
    TArray<FCrowd_PrehistoricHuman> Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Group")
    FVector GroupCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Group")
    float GroupRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Group")
    ECrowd_GroupBehavior GroupBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Group")
    float CohesionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Group")
    float SeparationDistance;

    FCrowd_TribalGroup()
    {
        GroupCenter = FVector::ZeroVector;
        GroupRadius = 500.0f;
        GroupBehavior = ECrowd_GroupBehavior::Gathering;
        CohesionStrength = 1.0f;
        SeparationDistance = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_PrehistoricNPCManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_PrehistoricNPCManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric NPCs")
    int32 MaxNPCCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric NPCs")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric NPCs")
    TArray<FCrowd_TribalGroup> TribalGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric NPCs")
    TArray<UStaticMeshComponent*> NPCMeshComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric NPCs")
    UStaticMesh* HumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric NPCs")
    UMaterialInterface* HumanMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric NPCs")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric NPCs")
    float ActivityChangeChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric NPCs")
    float GroupFormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric NPCs")
    bool bEnableTribalBehavior;

    UFUNCTION(BlueprintCallable, Category = "Prehistoric NPCs")
    void SpawnTribalGroup(FVector CenterLocation, int32 GroupSize);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric NPCs")
    void UpdateNPCBehavior();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric NPCs")
    void MoveNPCToLocation(int32 NPCIndex, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric NPCs")
    void SetNPCActivity(int32 NPCIndex, ECrowd_NPCActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric NPCs")
    void UpdateGroupCohesion(int32 GroupIndex);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric NPCs")
    void HandlePlayerProximity(FVector PlayerLocation, float ProximityRadius);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric NPCs")
    FVector GetRandomLocationInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric NPCs")
    void StartGatheringBehavior(int32 GroupIndex, FVector GatherLocation);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric NPCs")
    void StartHuntingBehavior(int32 GroupIndex, FVector HuntTarget);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric NPCs")
    void UpdateTribalGroupBehavior(int32 GroupIndex, float DeltaTime);

private:
    FTimerHandle UpdateTimer;
    
    void InitializeNPCMeshes();
    void UpdateNPCMovement(FCrowd_PrehistoricHuman& NPC, float DeltaTime);
    void ApplyGroupBehavior(FCrowd_TribalGroup& Group, float DeltaTime);
    FVector CalculateFlockingForce(const FCrowd_PrehistoricHuman& NPC, const FCrowd_TribalGroup& Group);
    void UpdateNPCVisuals();
};