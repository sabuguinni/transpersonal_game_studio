#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
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
struct FPerf_LODZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    EPerf_LODLevel LODLevel = EPerf_LODLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxActorsInZone = 100;

    FPerf_LODZone()
    {
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        LODLevel = EPerf_LODLevel::High;
        MaxActorsInZone = 100;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_LODManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_LODManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // LOD Zone Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Zones")
    TArray<FPerf_LODZone> LODZones;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f; // Update every 100ms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsToProcess = 50; // Process max 50 actors per tick

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 10000.0f; // Cull actors beyond this distance

    // LOD Functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_LODLevel GetLODLevelForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateActorLOD(AActor* Actor, EPerf_LODLevel NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterActorForLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterActorFromLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveActorCount() const { return ManagedActors.Num(); }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS) { TargetFrameRate = TargetFPS; }

protected:
    // Internal Management
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ManagedActors;

    UPROPERTY()
    float TargetFrameRate = 60.0f;

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    int32 CurrentProcessIndex = 0;

    // Helper Functions
    void ProcessActorLOD(AActor* Actor);
    void CullDistantActors();
    void InitializeLODZones();
    bool IsActorInZone(AActor* Actor, const FPerf_LODZone& Zone);
    float GetDistanceToPlayer(AActor* Actor);
};