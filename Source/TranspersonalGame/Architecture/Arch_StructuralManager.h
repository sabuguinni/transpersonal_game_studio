#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "Arch_StructuralManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    CaveEntrance,
    StonePlatform,
    RockShelter,
    BoulderFormation,
    CliffOverhang,
    NaturalArch
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsPlayerAccessible;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::RockShelter;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = 1.0f;
        BiomeType = EBiomeType::Savanna;
        bIsPlayerAccessible = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_StructuralManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Structure Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructuresForBiome(EBiomeType BiomeType, int32 StructureCount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetStructuresInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RemoveStructuresInArea(FVector Center, float Radius);

    // Shelter System
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationSheltered(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* FindNearestShelter(FVector PlayerLocation, float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateEmergencyShelter(FVector Location, EBiomeType BiomeType);

    // Structural Integrity
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float CalculateStructuralStability(AActor* Structure);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructureWeathering(AActor* Structure, float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanStructureSupportWeight(AActor* Structure, float Weight);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> ActiveStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructureSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinDistanceBetweenStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoGenerateStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatheringRate;

private:
    void InitializeStructureSystem();
    FVector GetBiomeCenter(EBiomeType BiomeType);
    UStaticMeshComponent* CreateStructureMesh(EArch_StructureType StructureType);
    void ApplyBiomeSpecificMaterials(UStaticMeshComponent* MeshComp, EBiomeType BiomeType);
    bool IsValidStructureLocation(FVector Location, EBiomeType BiomeType);
    void RegisterStructure(AActor* Structure, const FArch_StructureData& StructureData);
};