#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    TribeMember     UMETA(DisplayName = "Tribe Member"),
    RaptorPack      UMETA(DisplayName = "Raptor Pack"),
    HerbivoreHerd   UMETA(DisplayName = "Herbivore Herd"),
    Scavenger       UMETA(DisplayName = "Scavenger"),
    Predator        UMETA(DisplayName = "Predator")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    AActor* AgentActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::TribeMember;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 120.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterCrowdAgent(AActor* Agent, ECrowd_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterCrowdAgent(AActor* Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetHerdMigrationTarget(const FVector& Destination);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ActivateRaptorPackHunt(const FVector& PreyLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_AgentData> GetAgentsByType(ECrowd_AgentType AgentType) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdMigrationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TribeMemberWanderRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 RaptorPackSize;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bSimulationActive;

private:
    UPROPERTY()
    TArray<FCrowd_AgentData> CrowdAgents;
};
