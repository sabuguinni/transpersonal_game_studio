#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSpawnPoint.generated.h"

UENUM(BlueprintType)
enum class ECrowd_SpawnBehavior : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Gather UMETA(DisplayName = "Gather"),
    Flee UMETA(DisplayName = "Flee")
};

UCLASS()
class TRANSPERSONALGAME_API ACrowdSpawnPoint : public AActor
{
    GENERATED_BODY()
    
public:    
    ACrowdSpawnPoint();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxCrowdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    ECrowd_SpawnBehavior DefaultBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bAutoSpawnOnBeginPlay;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdMembers();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllMembers();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedMembers;
};
