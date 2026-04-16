#pragma once

#include "CoreMinimal.h"
#include "Engine/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "CrowdSpawnPoint.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSpawnPoint : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSpawnPoint();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Visual representation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* SpawnRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    // Crowd spawn configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    int32 MaxCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float SpawnInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    ECrowd_NPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    ECrowd_ActivityType DefaultActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float WanderRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    bool bIsActive;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentCrowdCount;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float LastSpawnTime;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedNPCs;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Spawn")
    void SpawnCrowdMember();

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawn")
    void DespawnCrowdMember(AActor* NPCActor);

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawn")
    void SetSpawnActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawn")
    FVector GetRandomSpawnLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawn")
    void CleanupInvalidNPCs();

private:
    void UpdateCrowdSpawning(float DeltaTime);
    bool CanSpawnNewNPC() const;
    void InitializeVisualization();
};