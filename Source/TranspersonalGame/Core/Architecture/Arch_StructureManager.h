#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Arch_StructureManager.generated.h"

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
    bool bIsRuin;

    FArch_StructureData()
    {
        StructureName = TEXT("DefaultStructure");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        BiomeType = EBiomeType::Savana;
        StructureHealth = 100.0f;
        bIsRuin = false;
    }
};

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Pillar      UMETA(DisplayName = "Stone Pillar"),
    Shelter     UMETA(DisplayName = "Primitive Shelter"),
    Platform    UMETA(DisplayName = "Stone Platform"),
    Ruin        UMETA(DisplayName = "Ancient Ruin"),
    Bridge      UMETA(DisplayName = "Stone Bridge")
};

UCLASS()
class TRANSPERSONALGAME_API UArch_StructureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtBiome(EBiomeType BiomeType, EArch_StructureType StructureType, int32 Count = 5);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnPillarCluster(FVector CenterLocation, int32 PillarCount = 3);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnShelterInterior(FVector Location, bool bIncludeFurniture = true);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetStructuresInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateAllBiomesWithStructures();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FArch_StructureData> SpawnedStructures;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<AActor*> StructureActors;

private:
    void CreatePrimitivePillar(FVector Location, FRotator Rotation);
    void CreatePrimitiveShelter(FVector Location, FRotator Rotation);
    void CreateStoneRuin(FVector Location, FRotator Rotation);
    
    FVector GetBiomeCenter(EBiomeType BiomeType);
    FVector GetRandomLocationInBiome(EBiomeType BiomeType, float Radius = 5000.0f);
};