#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float AgeYears;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel;

    FArch_StructureData()
        : StructureName(TEXT("Unknown"))
        , BiomeType(EBiomeType::Savanna)
        , Location(FVector::ZeroVector)
        , Rotation(FRotator::ZeroRotator)
        , AgeYears(0.0f)
        , WeatheringLevel(0.0f)
    {}
};

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Pillar      UMETA(DisplayName = "Stone Pillar"),
    Archway     UMETA(DisplayName = "Stone Archway"),
    Platform    UMETA(DisplayName = "Stone Platform"),
    Ruins       UMETA(DisplayName = "Ancient Ruins"),
    Shelter     UMETA(DisplayName = "Cave Shelter"),
    Monument    UMETA(DisplayName = "Stone Monument")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructure : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructure();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MossyMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetBiomeAdaptation(EBiomeType BiomeType);

    UFUNCTION(BlueprintPure, Category = "Structure")
    FArch_StructureData GetStructureData() const { return StructureData; }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitecturalStructureManager : public UObject
{
    GENERATED_BODY()

public:
    UArchitecturalStructureManager();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructureManager();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AArchitecturalStructure* SpawnStructure(EArch_StructureType StructureType, FVector Location, FRotator Rotation, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AArchitecturalStructure*> GetStructuresInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyGlobalWeathering(float WeatheringFactor);

protected:
    UPROPERTY()
    TArray<AArchitecturalStructure*> ManagedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TSubclassOf<AArchitecturalStructure> StructureClass;

private:
    void SetupBiomeSpecificMaterials(AArchitecturalStructure* Structure, EBiomeType BiomeType);
    FVector GetRandomLocationInBiome(EBiomeType BiomeType);
};