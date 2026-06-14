#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd Entity")
    FVector Position;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd Entity")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd Entity")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd Entity")
    int32 LODLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd Entity")
    bool bIsActive;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 100.0f;
        LODLevel = 0;
        bIsActive = true;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnCrowdEntities(int32 Count, FVector SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateEntityLOD(const FVector& ViewerPosition);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SetEntityMovementTarget(int32 EntityIndex, const FVector& TargetPosition);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    int32 GetActiveEntityCount() const;

private:
    UPROPERTY()
    TArray<FCrowd_EntityData> EntityData;

    UPROPERTY()
    TArray<AActor*> EntityActors;

    void UpdateEntityMovement(float DeltaTime);
    void UpdateEntityLOD();
    void CullDistantEntities(const FVector& ViewerPosition);

    float LODDistance_High = 500.0f;
    float LODDistance_Medium = 1500.0f;
    float LODDistance_Low = 3000.0f;
    float CullDistance = 5000.0f;
};