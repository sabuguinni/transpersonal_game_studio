#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Dwelling        UMETA(DisplayName = "Dwelling"),
    Shelter         UMETA(DisplayName = "Shelter"), 
    StorageHut      UMETA(DisplayName = "Storage Hut"),
    Watchtower      UMETA(DisplayName = "Watchtower"),
    CraftingArea    UMETA(DisplayName = "Crafting Area"),
    FirePit         UMETA(DisplayName = "Fire Pit"),
    DefensiveWall   UMETA(DisplayName = "Defensive Wall"),
    Bridge          UMETA(DisplayName = "Bridge")
};

UENUM(BlueprintType)
enum class EArch_ConstructionMaterial : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Thatch          UMETA(DisplayName = "Thatch"),
    Bone            UMETA(DisplayName = "Bone"),
    Hide            UMETA(DisplayName = "Hide"),
    Clay            UMETA(DisplayName = "Clay"),
    Mixed           UMETA(DisplayName = "Mixed Materials")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Dwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_ConstructionMaterial PrimaryMaterial = EArch_ConstructionMaterial::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatherResistance = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasInterior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName = TEXT("Unnamed Structure");

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Dwelling;
        PrimaryMaterial = EArch_ConstructionMaterial::Stone;
        StructuralIntegrity = 100.0f;
        WeatherResistance = 75.0f;
        MaxOccupants = 4;
        bHasInterior = true;
        bIsHabitable = true;
        StructureName = TEXT("Unnamed Structure");
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
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    TArray<UStaticMesh*> StructureMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    TArray<UMaterialInterface*> MaterialVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<AActor*> InteriorObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType AssignedBiome = EBiomeType::Savana;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void InitializeStructure(EArch_StructureType Type, EArch_ConstructionMaterial Material);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetStructureMesh(UStaticMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void PopulateInterior();

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void AddInteriorObject(AActor* NewObject);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void ClearInterior();

    UFUNCTION(BlueprintCallable, Category = "Biome Adaptation")
    void AdaptToBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    bool CanEnterStructure() const;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    FVector GetInteriorSpawnPoint() const;

    UFUNCTION(BlueprintPure, Category = "Structure Management")
    float GetStructuralIntegrity() const { return StructureInfo.StructuralIntegrity; }

    UFUNCTION(BlueprintPure, Category = "Structure Management")
    EArch_StructureType GetStructureType() const { return StructureInfo.StructureType; }

    UFUNCTION(BlueprintPure, Category = "Structure Management")
    bool IsHabitable() const { return StructureInfo.bIsHabitable && StructureInfo.StructuralIntegrity > 25.0f; }

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void UpdateStructureAppearance();
    void GenerateInteriorLayout();
    UStaticMesh* SelectBiomeAppropriateVariant() const;
    UMaterialInterface* SelectWeatherAppropriateVariant() const;

    UPROPERTY()
    bool bPlayerNearby = false;

    UPROPERTY()
    float LastWeatherDamageTime = 0.0f;
};