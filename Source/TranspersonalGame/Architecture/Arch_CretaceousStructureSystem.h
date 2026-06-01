#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Arch_CretaceousStructureSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StoneArch       UMETA(DisplayName = "Stone Archway"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    Cairn           UMETA(DisplayName = "Stone Cairn"),
    NaturalBridge   UMETA(DisplayName = "Natural Bridge")
};

UENUM(BlueprintType)
enum class EArch_ConstructionMaterial : uint8
{
    Limestone       UMETA(DisplayName = "Limestone"),
    Sandstone       UMETA(DisplayName = "Sandstone"),
    Basalt          UMETA(DisplayName = "Basalt"),
    Granite         UMETA(DisplayName = "Granite"),
    Shale           UMETA(DisplayName = "Shale"),
    Clay            UMETA(DisplayName = "Clay")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StoneArch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_ConstructionMaterial PrimaryMaterial = EArch_ConstructionMaterial::Limestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(400.0f, 200.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float MossGrowth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasPrimitiveToolMarks = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 EstimatedAge = 65000000;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StoneArch;
        PrimaryMaterial = EArch_ConstructionMaterial::Limestone;
        Dimensions = FVector(400.0f, 200.0f, 300.0f);
        WeatheringLevel = 0.7f;
        MossGrowth = 0.5f;
        bHasPrimitiveToolMarks = true;
        EstimatedAge = 65000000;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> FirePitLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> SleepingAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> ToolStorageAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> FoodStorageAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float FloorArea = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float CeilingHeight = 250.0f;

    FArch_InteriorLayout()
    {
        FloorArea = 2500.0f;
        CeilingHeight = 250.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CretaceousStructure : public AActor
{
    GENERATED_BODY()

public:
    AArch_CretaceousStructure();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteriorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FArch_InteriorLayout InteriorLayout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType AssociatedBiome = EBiomeType::Temperate;

public:
    UFUNCTION(BlueprintCallable, Category = "Structure")
    void InitializeStructure(const FArch_StructureData& InStructureData);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetupInteriorLayout(const FArch_InteriorLayout& Layout);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    FArch_StructureData GetStructureData() const { return StructureData; }

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool IsHabitable() const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    float GetStructuralIntegrity() const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_CretaceousStructureSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_CretaceousStructureSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxStructuresPerBiome = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinDistanceBetweenStructures = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<EArch_StructureType> AllowedStructureTypes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AArch_CretaceousStructure*> SpawnedStructures;

public:
    UFUNCTION(BlueprintCallable, Category = "Structure Generation")
    void GenerateStructuresForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Structure Generation")
    AArch_CretaceousStructure* SpawnStructureAtLocation(const FVector& Location, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ValidateStructurePlacement(const FVector& Location, bool& bIsValidLocation, FString& ValidationMessage);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void CleanupExcessStructures();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    int32 GetStructureCount() const { return SpawnedStructures.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    TArray<AArch_CretaceousStructure*> GetStructuresInRadius(const FVector& Center, float Radius) const;

private:
    bool IsLocationSuitableForStructure(const FVector& Location, EArch_StructureType StructureType) const;
    FArch_StructureData GenerateStructureDataForBiome(EBiomeType BiomeType, EArch_StructureType StructureType) const;
    void ApplyBiomeSpecificMaterials(AArch_CretaceousStructure* Structure, EBiomeType BiomeType) const;
};