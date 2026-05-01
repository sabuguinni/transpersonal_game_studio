#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    High        UMETA(DisplayName = "High Detail"),
    Medium      UMETA(DisplayName = "Medium Detail"),
    Low         UMETA(DisplayName = "Low Detail"),
    Culled      UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighDetailDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowDetailDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 15000.0f;

    FPerf_LODSettings()
    {
        HighDetailDistance = 2000.0f;
        MediumDetailDistance = 5000.0f;
        LowDetailDistance = 10000.0f;
        CullDistance = 15000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ActorLODData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor LOD")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor LOD")
    EPerf_LODLevel CurrentLODLevel = EPerf_LODLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor LOD")
    float LastUpdateTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor LOD")
    bool bIsImportantActor = false;

    FPerf_ActorLODData()
    {
        CurrentLODLevel = EPerf_LODLevel::High;
        LastUpdateTime = 0.0f;
        bIsImportantActor = false;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Management")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Management")
    TArray<FPerf_ActorLODData> ManagedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Management")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Management")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThreshold = 30.0f;

private:
    float LastUpdateTime = 0.0f;
    int32 CurrentUpdateIndex = 0;
    
    UPROPERTY()
    TWeakObjectPtr<APawn> PlayerPawn;

public:
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RegisterActor(AActor* Actor, bool bIsImportant = false);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateActorLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    EPerf_LODLevel CalculateLODLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODLevel(AActor* Actor, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RefreshManagedActors();

protected:
    void UpdateLODSystem();
    void ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    float GetDistanceToPlayer(AActor* Actor) const;
    bool IsActorInView(AActor* Actor) const;
};