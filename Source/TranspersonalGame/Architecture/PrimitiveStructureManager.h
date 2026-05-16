#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "../Core/SharedTypes.h"
#include "PrimitiveStructureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructureHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable;

    FArch_StructureData()
    {
        StructureName = TEXT("Unknown Structure");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        BiomeType = EBiomeType::Savanna;
        StructureHealth = 100.0f;
        bIsHabitable = false;
    }
};

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
    Bridge          UMETA(DisplayName = "Bridge"),
    Ruins           UMETA(DisplayName = "Ruins")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPrimitiveStructureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPrimitiveStructureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Structure Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool SpawnStructure(EArch_StructureType StructureType, FVector Location, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RemoveStructure(const FString& StructureName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetStructuresInBiome(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_StructureData GetNearestStructure(FVector PlayerLocation, float SearchRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationSuitableForStructure(FVector Location, EArch_StructureType StructureType);

    // Interior Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetupStructureInterior(const FString& StructureName, bool bAddFirePit = true, bool bAddStorage = true);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanPlayerEnterStructure(const FString& StructureName);

    // Damage and Repair
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void DamageStructure(const FString& StructureName, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RepairStructure(const FString& StructureName, float RepairAmount);

    // Weather Resistance
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetStructureWeatherResistance(const FString& StructureName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatherEffects(float DeltaTime);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FArch_StructureData> SpawnedStructures;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, AActor*> StructureActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatherDamageRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxStructureHealth;

private:
    void InitializeDefaultStructures();
    AActor* CreateStructureActor(EArch_StructureType StructureType, FVector Location, FRotator Rotation);
    FString GenerateStructureName(EArch_StructureType StructureType, EBiomeType Biome);
    void SetupStructureMaterials(AActor* StructureActor, EBiomeType Biome);
    bool ValidateStructurePlacement(FVector Location, EArch_StructureType StructureType);
};