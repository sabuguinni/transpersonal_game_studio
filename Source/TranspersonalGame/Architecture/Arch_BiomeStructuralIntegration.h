#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Arch_BiomeStructuralIntegration.generated.h"

UENUM(BlueprintType)
enum class EArch_BiomeStructureType : uint8
{
    None = 0,
    StonePillar,
    LimestoneFormation,
    WeatheredRock,
    PrimitiveFoundation,
    CaveEntrance,
    NaturalArch,
    RockShelter
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_BiomeStructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_BiomeStructureType StructureType = EArch_BiomeStructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType AssociatedBiome = EBiomeType::Forest;

    FArch_BiomeStructureData()
    {
        StructureType = EArch_BiomeStructureType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
        AssociatedBiome = EBiomeType::Forest;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_BiomeStructuralIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_BiomeStructuralIntegration();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    TArray<FArch_BiomeStructureData> BiomeStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    float StructureDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    float MinDistanceBetweenStructures = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    bool bAutoGenerateStructures = true;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void GenerateStructuresForBiome(EBiomeType BiomeType, const FVector& CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    AActor* SpawnStructureAtLocation(const FArch_BiomeStructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void ApplyWeatheringToStructure(AActor* StructureActor, float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void AddMossGrowthToStructure(AActor* StructureActor, bool bEnableMoss);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    TArray<FArch_BiomeStructureData> GetStructuresInRadius(const FVector& Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void ClearAllStructures();

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void RefreshBiomeIntegration();

private:
    UFUNCTION()
    void OnBiomeChanged(EBiomeType NewBiome, const FVector& Location);

    void InitializeBiomeStructures();
    bool IsValidLocationForStructure(const FVector& Location, EArch_BiomeStructureType StructureType) const;
    FArch_BiomeStructureData CreateStructureDataForBiome(EBiomeType BiomeType, const FVector& Location) const;
};

UCLASS()
class TRANSPERSONALGAME_API UArch_BiomeStructuralSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Biome Structures")
    void RegisterStructuralComponent(UArch_BiomeStructuralIntegration* Component);

    UFUNCTION(BlueprintCallable, Category = "Biome Structures")
    void UnregisterStructuralComponent(UArch_BiomeStructuralIntegration* Component);

    UFUNCTION(BlueprintCallable, Category = "Biome Structures")
    void RefreshAllBiomeStructures();

    UFUNCTION(BlueprintCallable, Category = "Biome Structures")
    TArray<AActor*> GetAllStructuresInBiome(EBiomeType BiomeType) const;

protected:
    UPROPERTY()
    TArray<TObjectPtr<UArch_BiomeStructuralIntegration>> RegisteredComponents;

    UPROPERTY()
    TMap<EBiomeType, TArray<TObjectPtr<AActor>>> BiomeStructureMap;

private:
    void UpdateBiomeStructureMapping();
};