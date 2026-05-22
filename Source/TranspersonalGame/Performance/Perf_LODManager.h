#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD0_Ultra     UMETA(DisplayName = "Ultra Detail"),
    LOD1_High      UMETA(DisplayName = "High Detail"), 
    LOD2_Medium    UMETA(DisplayName = "Medium Detail"),
    LOD3_Low       UMETA(DisplayName = "Low Detail"),
    LOD4_Culled    UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float UltraDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 15000.0f;

    FPerf_LODSettings()
    {
        UltraDistance = 1000.0f;
        HighDistance = 2500.0f;
        MediumDistance = 5000.0f;
        LowDistance = 10000.0f;
        CullDistance = 15000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ActorLODData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    AActor* Actor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    EPerf_LODLevel CurrentLOD = EPerf_LODLevel::LOD0_Ultra;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    float LastDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    float LastUpdateTime = 0.0f;

    FPerf_ActorLODData()
    {
        Actor = nullptr;
        CurrentLOD = EPerf_LODLevel::LOD0_Ultra;
        LastDistance = 0.0f;
        LastUpdateTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_LODManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_LODManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Manager")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Manager")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Manager")
    int32 MaxActorsPerFrame = 50;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Manager")
    TArray<FPerf_ActorLODData> ManagedActors;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Manager")
    int32 CurrentUpdateIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Manager")
    float LastUpdateTime = 0.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    EPerf_LODLevel GetActorLODLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    void ForceUpdateAllActors();

    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    int32 GetManagedActorCount() const { return ManagedActors.Num(); }

    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    void SetUpdateFrequency(float NewFrequency);

protected:
    void UpdateActorLOD(FPerf_ActorLODData& ActorData);
    EPerf_LODLevel CalculateLODLevel(float Distance) const;
    void ApplyLODToActor(AActor* Actor, EPerf_LODLevel LODLevel);
    void ApplyStaticMeshLOD(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    void ApplySkeletalMeshLOD(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    float GetDistanceToPlayer(AActor* Actor) const;
    APawn* GetPlayerPawn() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "LOD Manager")
    void OnActorLODChanged(AActor* Actor, EPerf_LODLevel OldLOD, EPerf_LODLevel NewLOD);
};