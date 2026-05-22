#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "PrehistoricStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StoneDwelling,
    RockShelter,
    StoneCircle,
    CaveEntrance,
    AncientRuin,
    StonePillar,
    TribalPlatform
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
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StoneDwelling;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        WeatheringLevel = 0.5f;
        BiomeType = EBiomeType::Savanna;
        bIsHabitable = true;
        MaxOccupants = 4;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPrehistoricStructureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPrehistoricStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    TArray<FArch_StructureData> ManagedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    float StructureSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    int32 MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    bool bAutoGenerateStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float WeatheringRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float MinWeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float MaxWeatheringLevel;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void GenerateStructuresForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    AStaticMeshActor* SpawnStructure(const FArch_StructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void RemoveStructure(int32 StructureIndex);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    TArray<FArch_StructureData> GetStructuresInRadius(const FVector& Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    FArch_StructureData GetNearestStructure(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weathering")
    void ApplyWeathering(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weathering")
    void SetStructureWeathering(int32 StructureIndex, float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    bool IsLocationSuitableForStructure(const FVector& Location, EArch_StructureType StructureType) const;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ClearAllStructures();

    UFUNCTION(BlueprintCallable, Category = "Structure Management", CallInEditor)
    void RegenerateAllStructures();

private:
    void InitializeStructureTypes();
    FVector GetRandomLocationInBiome(const FVector& BiomeCenter, float BiomeRadius) const;
    EArch_StructureType GetRandomStructureTypeForBiome(EBiomeType BiomeType) const;
    UStaticMesh* GetMeshForStructureType(EArch_StructureType StructureType) const;
    void ApplyBiomeSpecificMaterials(AStaticMeshActor* StructureActor, EBiomeType BiomeType) const;
};