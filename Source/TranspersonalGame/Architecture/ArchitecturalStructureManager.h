#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Shelter         UMETA(DisplayName = "Shelter"),
    Watchtower      UMETA(DisplayName = "Watchtower"),
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    Ruins           UMETA(DisplayName = "Ruins"),
    Bridge          UMETA(DisplayName = "Bridge"),
    Wall            UMETA(DisplayName = "Wall")
};

UENUM(BlueprintType)
enum class EArch_ConstructionMaterial : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Bone            UMETA(DisplayName = "Bone"),
    Clay            UMETA(DisplayName = "Clay"),
    Mixed           UMETA(DisplayName = "Mixed Materials")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_ConstructionMaterial Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 EstimatedAge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasInterior;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Shelter;
        Material = EArch_ConstructionMaterial::Stone;
        StructuralIntegrity = 100.0f;
        WeatheringLevel = 0.0f;
        EstimatedAge = 0;
        bIsHabitable = false;
        bHasInterior = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    EBiomeType AssociatedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<UStaticMesh*> StructureMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> WeatheringMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxStructuresPerBiome;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(EArch_StructureType Type, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureMesh(UStaticMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructuralIntegrity(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanPlayerEnter() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetInteriorSpawnPoint() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void SpawnStructuresInBiome();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void ClearAllStructures();

    UFUNCTION(BlueprintPure, Category = "Architecture")
    FString GetStructureDescription() const;

    UFUNCTION(BlueprintPure, Category = "Architecture")
    float GetStructureAge() const;

private:
    void UpdateMaterialBasedOnWeathering();
    void ValidateStructureData();
    UStaticMesh* SelectMeshForBiome(EBiomeType Biome) const;
    UMaterialInterface* SelectMaterialForWeathering(float WeatheringLevel) const;
};