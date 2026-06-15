#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Core_TerrainPhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Rocky       UMETA(DisplayName = "Rocky Terrain"),
    Muddy       UMETA(DisplayName = "Muddy Terrain"), 
    Sandy       UMETA(DisplayName = "Sandy Terrain"),
    Grassy      UMETA(DisplayName = "Grassy Terrain"),
    Icy         UMETA(DisplayName = "Icy Terrain"),
    Volcanic    UMETA(DisplayName = "Volcanic Terrain")
};

USTRUCT(BlueprintType)
struct FCore_TerrainPhysicsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    ECore_TerrainType TerrainType = ECore_TerrainType::Grassy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float FrictionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float BounceMultiplier = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bCausesSlipping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bCausesStumbling = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float StaminaDrainMultiplier = 1.0f;
};

/**
 * Core Terrain Physics Component
 * Handles realistic terrain interaction for character movement and object physics
 * Integrates with survival mechanics for stamina and movement penalties
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Terrain physics properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    FCore_TerrainPhysicsData TerrainData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float DetectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableTerrainEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bDebugVisualization = false;

    // Core terrain physics methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainEffects(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsData GetTerrainDataAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsActorOnTerrain(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetTerrainType(ECore_TerrainType NewTerrainType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float CalculateMovementPenalty(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ProcessTerrainCollision(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

    // Terrain effect delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerrainEffectApplied, AActor*, AffectedActor, ECore_TerrainType, TerrainType);
    UPROPERTY(BlueprintAssignable, Category = "Terrain Physics")
    FOnTerrainEffectApplied OnTerrainEffectApplied;

private:
    // Internal terrain processing
    void UpdateTerrainEffects();
    void CalculateTerrainInteraction(AActor* TargetActor);
    bool ValidateTerrainData() const;
    void InitializeTerrainPhysics();

    // Cached references
    UPROPERTY()
    TArray<AActor*> NearbyActors;

    UPROPERTY()
    class UWorld* CachedWorld;

    // Performance optimization
    float LastUpdateTime;
    float UpdateInterval;
};