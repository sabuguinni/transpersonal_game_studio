#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EnvArt_ProceduralFoliage.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FoliageType
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float MaxSlope = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    int32 MaxInstancesPerBiome = 200;

    FEnvArt_FoliageType()
    {
        Mesh = nullptr;
        Density = 1.0f;
        ScaleRange = FVector2D(0.8f, 1.2f);
        MaxSlope = 45.0f;
        MaxInstancesPerBiome = 200;
    }
};

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_ProceduralFoliage : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_ProceduralFoliage();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Generation")
    TMap<EEnvArt_BiomeType, TArray<FEnvArt_FoliageType>> BiomeFoliageTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Generation")
    float GenerationRadius = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Generation")
    int32 MaxTotalInstances = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Generation")
    bool bAutoGenerate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance = 15000.0f;

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void GenerateFoliageForBiome(EEnvArt_BiomeType BiomeType, FVector BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void ClearAllFoliage();

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void RegenerateFoliage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevels();

private:
    UPROPERTY()
    TArray<UInstancedStaticMeshComponent*> FoliageComponents;

    UPROPERTY()
    TMap<EEnvArt_BiomeType, int32> BiomeInstanceCounts;

    void CreateFoliageInstance(const FEnvArt_FoliageType& FoliageType, const FVector& Location, const FRotator& Rotation, const FVector& Scale, EEnvArt_BiomeType BiomeType);
    
    bool IsValidPlacementLocation(const FVector& Location, float MaxSlope) const;
    
    FVector GetRandomScaleFromRange(const FVector2D& ScaleRange) const;
    
    FRotator GetRandomRotation() const;
    
    EEnvArt_BiomeType GetBiomeTypeAtLocation(const FVector& Location) const;
    
    void InitializeDefaultFoliageTypes();
};